#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "Interface/CorrelatedData.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Stream/Stream.h"

#include "CorrelatorPipeline.h"
#include "WorkQueues/CorrelatorWorkQueue.h"
#include "Input/BeamletBufferToComputeNode.h"
#include <SubbandMetaData.h>

namespace LOFAR
{
  namespace RTCP 
  {

    CorrelatorPipeline::CorrelatorPipeline(const Parset &ps)
      :
    Pipeline(ps),
      filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER)
    {

      filterBank.negateWeights();

      double startTime = omp_get_wtime();

      //#pragma omp parallel sections
      {
        programs.firFilterProgram = createProgram("FIR.cl");
        programs.delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
#if defined USE_NEW_CORRELATOR
        programs.correlatorProgram = createProgram("NewCorrelator.cl");
#else
        programs.correlatorProgram = createProgram("Correlator.cl");
#endif
      }

      std::cout << "compile time = " << omp_get_wtime() - startTime << std::endl;
    }

    void CorrelatorPipeline::doWork()
    {
      //sections = program segments defined by the following omp section directive
      //           are distributed for parallel execution among available threads
      //parallel = directive explicitly instructs the compiler to parallelize the chosen block of code.
      //  The two sections in this function are done in parallel with a seperate set of threads.
#     pragma omp parallel sections 
      {
        // Allow the super class to do its work
#       pragma omp section
        Pipeline::doWork();

        // Forward the input
#       pragma omp section
        {
          double startTime = ps.startTime(), stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

          size_t nrStations = ps.nrStations();

          // The data from the input buffer to the input stream is run in a seperate thread
#         pragma omp parallel for num_threads(nrStations)
          for (size_t stat = 0; stat < nrStations; stat++) 
          {
            double currentTime;

            for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) 
            {
              // TODO: Connect the input buffer to the input stream, is this correct description??
              sendNextBlock(stat);
            }
          }
        }

        // Perform the calculations
#       pragma omp section
        {
          // Keep track of performance totals
          map<string, PerformanceCounter::figures> total_performance;

          // Start a set of workqueue, the number depending on the number of GPU's: 2 for each.
          // Each WorkQueue gets its own thread.
#         pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
          {
            CorrelatorWorkQueue queue(ps,               // Configuration
               context,                                 // Opencl context
               devices[omp_get_thread_num() % nrGPUs],  // The GPU this workQueue is connected to
               omp_get_thread_num() % nrGPUs,           // The GPU index
               programs,                                // The compiled kernels, const
            filterBank);                             // The filter set to use. Const

            // run the queue
            doWorkQueue(queue);                            

            // gather performance figures
            for (map<string, SmartPtr<PerformanceCounter> >::iterator i = queue.counters.begin(); i != queue.counters.end(); ++i) {
              const string &name = i->first;
              PerformanceCounter *counter = i->second.get();

              counter->waitForAllOperations();
              total_performance[name] += counter->getTotal();


          }
        }

          // Log all performance totals
          for (map<string, PerformanceCounter::figures>::const_iterator i = total_performance.begin(); i != total_performance.end(); ++i) {
            i->second.log();
      }
    }
      }
    }


    void CorrelatorPipeline::receiveSubbandSamples(
         CorrelatorWorkQueue &workQueue, unsigned block,
         unsigned subband)
    {
      // Read the samples from the input stream in parallel
#     pragma omp parallel for
      for (unsigned station = 0; station < ps.nrStations(); station ++) 
      {
        // each input stream contains the data from a single station
        Stream *inputStream = bufferToGPUstreams[station];      

        // 
        workQueue.inputData.read(inputStream, station, subband, ps.subbandToSAPmapping()[subband]);
      }
    }


    void CorrelatorPipeline::sendSubbandVisibilities(CorrelatorWorkQueue &workQueue, unsigned block, unsigned subband)
    {
      // Create an data object to Storage around our visibilities
      CorrelatedData data(ps.nrStations(), ps.nrChannelsPerSubband(), ps.integrationSteps(), workQueue.visibilities.origin(), workQueue.visibilities.num_elements(), heapAllocator, 1);

      // Add weights
      // TODO: base weights on flags
      for (size_t bl = 0; bl < data.itsNrBaselines; ++bl)
        for (size_t ch = 0; ch < ps.nrChannelsPerSubband(); ++ch)
          data.setNrValidSamples(bl, ch, ps.integrationSteps());

      // Write the block to Storage
      writeOutput(block, subband, data);
    }


    //This whole block should be parallel: this allows the thread to pick up a subband from the next block
    void CorrelatorPipeline::doWorkQueue(CorrelatorWorkQueue &workQueue) //todo: name is not correct
    {
      // get details regarding the observation from the parset. 
      double currentTime;                         // set in the block processing for loop
      double startTime  = ps.startTime();         // start of the observation 
      double stopTime   = ps.stopTime();          // end of the observation
      double blockTime  = ps.CNintegrationTime(); // Total integration time: How big a timeslot should be integrated

      // wait until all other threads in this section reach the same point:
      // This is the start of the work. We need a barier because not all WorkQueue objects might be created at once, 
      // each is  created in a seperate (OMP) Tread.
#     pragma omp barrier
      double executionStartTime = omp_get_wtime();
      double lastTime = omp_get_wtime(); 

      // loop all available blocks. use the blocks to determine if we are within the valid observation times
      // This loop is not parallel
      for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) 
      {
#       pragma omp single nowait    // Only a single thread should perform the cout 
#       pragma omp critical (cout)  // Only one cout statement application wide can be active at a single time.
        std::cout << "block = "  << block 
                  << ", time = " << to_simple_string(from_ustime_t(currentTime))  //current time
                  << ", exec = " << omp_get_wtime() - lastTime << std::endl;      //

        // Save the current time: This will be used to display the execution time for this block
        lastTime = omp_get_wtime();

        // process each subband in a seperate omp tread
        // This is the main loop.
        // Get data from an input, send to the gpu, process, assign to the output.
        // schedule(dynamic) = the iterations requiring varying, or even unpredictable, amounts of work.
        // nowait = Use this clause to avoid the implied barrier at the end of the for directive. 
        //          Threads do not synchronize at the end of the parallel loop. 
        // ordered =  Specifies that the iterations of the loop must be executed as they would be in a serial program
#       pragma omp for schedule(dynamic), nowait, ordered  // no parallel: this no new threads
        for (unsigned subband = 0; subband < ps.nrSubbands(); subband ++) 
        {
          // Each input block is sent in order. Therefore wait for the correct block
          inputSynchronization.waitFor(block * ps.nrSubbands() + subband);
          receiveSubbandSamples( workQueue,  block,  subband);
          // Advance the block index
          inputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);

          // Perform calculations
          workQueue.doSubband(block, subband);

          // Send output to Storage
          sendSubbandVisibilities(workQueue, block, subband);
        }  // end pragma omp for 

      }

      //The omp for loop was nowait: We need a barier to assure that 
#     pragma omp barrier 

      //master = a section of code that must be run only by the master thread.
#     pragma omp master

      if (!profiling)
#       pragma omp critical (cout)
        std::cout << "run time = " << omp_get_wtime() - executionStartTime << std::endl;
    }
  }
}



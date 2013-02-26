#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "CorrelatorPipeline.h"
#include "WorkQueues/CorrelatorWorkQueue.h"

namespace LOFAR
{
  namespace RTCP 
  {

    CorrelatorPipeline::CorrelatorPipeline(const Parset &ps)
      :
    Pipeline(ps),
      filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER),
      counters(
#if defined USE_NEW_CORRELATOR
      "cor.triangle",
      "cor.rectangle",
#else
      "correlator",
#endif
      "FIR filter", "delay/bp", "FFT", "samples", "visibilities")
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

#       pragma omp section
        {
          // Start a set of workqueue, the number depending on the number of GPU's: 2 for each.
          // Each WorkQueue gets its own thread.
#         pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
          doWorkQueue(CorrelatorWorkQueue(ps,        // Configuration
            context,                                 // Opencl context
            devices[omp_get_thread_num() % nrGPUs],  // The GPU this workQueue is connected to
            omp_get_thread_num() % nrGPUs,           // The GPU index
            programs,                                // The compiled kernels, const
            counters,                                // Performance counters, shared between queues!!!
            filterBank));                            // The filter set to use. Const
        }
      }
    }

    void CorrelatorPipeline::receiveSubbandSamples(unsigned block, unsigned subband)
    {
#ifdef USE_INPUT_SECTION

      // No specific parallelizations options needed: All synchronisation is performed outside
      // this function.
#     pragma omp parallel for
      for (unsigned stat = 0; stat < ps.nrStations(); stat ++) 
      {
        Stream *stream = pipeline.bufferToGPUstreams[stat];

        // read header
        struct BeamletBufferToComputeNode<i16complex>::header header;
        size_t subbandSize = inputSamples[stat].num_elements() * sizeof *inputSamples.origin();

        stream->read(&header, sizeof header);

        ASSERTSTR(subband == header.subband, "Expected subband " << subband << ", got subband " << header.subband);
        ASSERTSTR(subbandSize == header.nrSamples * header.sampleSize, "Expected " << subbandSize << " bytes, got " << header.nrSamples * header.sampleSize << " bytes (= " << header.nrSamples << " samples * " << header.sampleSize << " bytes/sample)");

        // read subband
        stream->read(inputSamples[stat].origin(), subbandSize);

        unsigned beam = ps.subbandToSAPmapping()[subband];

        // read meta data
        SubbandMetaData metaData(1, header.nrDelays);
        metaData.read(stream);

        // the first set of delays represents the central beam, which is the one we correlate
        struct SubbandMetaData::beamInfo &beamInfo = metaData.beams(0)[0];

        for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
          delaysAtBegin[beam][stat][pol]  = beamInfo.delayAtBegin;
          delaysAfterEnd[beam][stat][pol] = beamInfo.delayAfterEnd;

          phaseOffsets[beam][pol] = 0.0;
        }
      }

#endif
    }


    //This whole block should be parallel: this allows the thread to pick up a subband from the next block
    void CorrelatorPipeline::doWorkQueue(CorrelatorWorkQueue workQueue) //todo: name is not correct
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
          // Each input block needs to be processed in order. Therefore wait for the correct block
          inputSynchronization.waitFor(block * ps.nrSubbands() + subband);
          //receiveSubbandSamples
          // Advance the block index
          inputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);

          // 
          workQueue.doSubband(block, subband);

          // Output needs to be ordered: wait for the correct output block
          outputSynchronization.waitFor(block * ps.nrSubbands() + subband);
          //Write gpu to storage
          GPUtoStorageStreams[subband]->write(
            workQueue.visibilities.origin(),
            workQueue.visibilities.num_elements() * sizeof(std::complex<float>)
            );
          // Advance the output index
          outputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);
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



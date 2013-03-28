//# CorrelatorPipeline.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "CorrelatorPipeline.h"

#include <iomanip>

#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <Stream/Stream.h>
#include <CoInterface/CorrelatedData.h>

#include <GPUProc/OpenMP_Support.h>
#include <GPUProc/createProgram.h>
#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>
#include <GPUProc/WorkQueues/WorkQueue.h>
using namespace std;

namespace LOFAR
{
  namespace Cobalt
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
      LOG_DEBUG_STR("compile time = " << omp_get_wtime() - startTime);
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
            for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block++)
            {
              // TODO: Connect the input buffer to the input stream, is this correct description??
              sendNextBlock(stat);
            }
          }
        }

        // Perform the calculations
#       pragma omp section
        {
          size_t nrWorkQueues = (profiling ? 1 : 2) * nrGPUs;

          // Start a set of workqueue, the number depending on the number of GPU's: 2 for each.
          // Each WorkQueue gets its own thread.
#         pragma omp parallel num_threads(nrWorkQueues)
          {
            CorrelatorWorkQueue queue(ps,               // Configuration
                                      context,          // Opencl context
                                      devices[omp_get_thread_num() % nrGPUs], // The GPU this workQueue is connected to
                                      omp_get_thread_num() % nrGPUs, // The GPU index
                                      programs,         // The compiled kernels, const
                                      filterBank);   // The filter set to use. Const

            // run the queue
            doWorkQueue(queue);

            // gather performance figures
            performance.addQueue(queue);
          }
          // Signal end of data
          noMoreOutput();

          // log performance figures
          performance.log(nrWorkQueues);
        }
      }
    }


    void CorrelatorPipeline::receiveSubbandSamples(
      CorrelatorWorkQueue &workQueue, unsigned subband)
    {
      // Read the samples from the input stream in parallel
#     pragma omp parallel for
      for (unsigned station = 0; station < ps.nrStations(); station++)
      {
        // each input stream contains the data from a single station
        Stream *inputStream = bufferToGPUstreams[station];

        //
        workQueue.inputData.read(inputStream, station, subband, ps.subbandToSAPmapping()[subband]);
      }
    }


    //This whole block should be parallel: this allows the thread to pick up a subband from the next block
    void CorrelatorPipeline::doWorkQueue(CorrelatorWorkQueue &workQueue) //todo: name is not correct
    {
      // get details regarding the observation from the parset.
      double currentTime;                         // set in the block processing for loop
      double startTime = ps.startTime();          // start of the observation
      double stopTime = ps.stopTime();            // end of the observation
      double blockTime = ps.CNintegrationTime();  // Total integration time: How big a timeslot should be integrated

      // wait until all other threads in this section reach the same point:
      // This is the start of the work. We need a barier because not all WorkQueue objects might be created at once,
      // each is  created in a seperate (OMP) Tread.
#     pragma omp barrier
      double executionStartTime = omp_get_wtime();
      double lastTime = omp_get_wtime();

      workQueue.timers["CPU - total"]->start();

      // loop all available blocks. use the blocks to determine if we are within the valid observation times
      // This loop is not parallel
      for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block++)
      {
#       pragma omp single nowait    // Only a single thread should perform the cout
        LOG_INFO_STR("block = " << block
                  << ", time = " << to_simple_string(from_ustime_t(currentTime))  //current time
                  << ", exec = " << omp_get_wtime() - lastTime);

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
        for (unsigned subband = 0; subband < ps.nrSubbands(); subband++)
        {
          // Create an data object to Storage around our visibilities
          SmartPtr<CorrelatedData> output = new CorrelatedData(ps.nrStations(), ps.nrChannelsPerSubband(), ps.integrationSteps(), heapAllocator, 1);

          workQueue.timers["CPU - input"]->start();
          // Each input block is sent in order. Therefore wait for the correct block
          inputSynchronization.waitFor(block * ps.nrSubbands() + subband);
          receiveSubbandSamples(workQueue, subband);
          // Advance the block index
          inputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);
          workQueue.timers["CPU - input"]->stop();

          // Perform calculations
          workQueue.timers["CPU - compute"]->start();
          workQueue.doSubband(subband, *output);
          workQueue.timers["CPU - compute"]->stop();

          // Hand off the block to Storage
          workQueue.timers["CPU - output"]->start();
          writeOutput(block, subband, output.release());
          workQueue.timers["CPU - output"]->stop();
        }  // end pragma omp for
      }

      workQueue.timers["CPU - total"]->stop();

      //The omp for loop was nowait: We need a barier to assure that
#     pragma omp barrier

      //master = a section of code that must be run only by the master thread.
#     pragma omp master

      if (!profiling)
        LOG_INFO_STR("run time = " << omp_get_wtime() - executionStartTime);
    }
  }
}


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
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/Stream.h>

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
      subbandPool(ps.nrSubbands()),
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
      size_t nrWorkQueues = (profiling ? 1 : 2) * nrGPUs;
      vector< SmartPtr<CorrelatorWorkQueue> > workQueues(nrWorkQueues);

      for (size_t i = 0; i < workQueues.size(); ++i) {
        workQueues[i] = new CorrelatorWorkQueue(ps,               // Configuration
                                      context,          // Opencl context
                                      devices[i % nrGPUs], // The GPU this workQueue is connected to
                                      i % nrGPUs, // The GPU index
                                      programs,         // The compiled kernels, const
                                      filterBank);   // The filter set to use. Const
      }

      for (unsigned sb = 0; sb < ps.nrSubbands(); sb++) {
        // Allow 10 blocks to be in the best-effort queue.
        // TODO: make this dynamic based on memory or time
        subbandPool[sb].bequeue = new BestEffortQueue< SmartPtr<CorrelatedDataHostBuffer> >(10, ps.realTime());
      }

      double startTime = ps.startTime();
      double stopTime = ps.stopTime();
      double blockTime = ps.CNintegrationTime();

      size_t nrBlocks = floor((stopTime - startTime) / blockTime);

      //sections = program segments defined by the following omp section directive
      //           are distributed for parallel execution among available threads
      //parallel = directive explicitly instructs the compiler to parallelize the chosen block of code.
      //  The two sections in this function are done in parallel with a seperate set of threads.
#     pragma omp parallel sections
      {
        /*
         * CIRCULAR BUFFER -> STATION STREAMS
         *
         * Handles one block per station.
         */
#       pragma omp section
        {
          size_t nrStations = ps.nrStations();
          // The data from the input buffer to the input stream is run in a seperate thread
#         pragma omp parallel for num_threads(nrStations)
          for (size_t stat = 0; stat < nrStations; stat++) {
            for (size_t block = 0; block < nrBlocks; block++) {
              sendNextBlock(stat);
            }
          }
        }

        /*
         * STATION STREAMS -> WORKQUEUE INPUTPOOL
         *
         * Collects one subband of all stations.
         */
#       pragma omp section
        {
          // Let the queues pick up work as soon as they're ready.
#         pragma omp parallel num_threads(workQueues.size())
          {
            CorrelatorWorkQueue &queue = *workQueues[omp_get_thread_num() % workQueues.size()];

            for (size_t block = 0; block < nrBlocks; block++) {
              // process each subband in a seperate omp tread
              // This is the main loop.
              // Get data from an input, send to the gpu, process, assign to the output.
              // schedule(dynamic) = the iterations requiring varying, or even unpredictable, amounts of work.
              // nowait = Use this clause to avoid the implied barrier at the end of the for directive.
              //          Threads do not synchronize at the end of the parallel loop.
              // ordered =  Specifies that the iterations of the loop must be executed as they would be in a serial program
#             pragma omp for schedule(dynamic), nowait, ordered  // no parallel: this no new threads
              for (unsigned subband = 0; subband < ps.nrSubbands(); subband++) {
                receiveSubbandSamples(queue, block, subband);
              }
            }

            // Signal end of input
            queue.inputPool.filled.append(NULL);
          }
        }

        /*
         * WORKQUEUE INPUTPOOL -> SUBBANDPOOL
         *
         * Perform calculations, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
            CorrelatorWorkQueue &queue = *workQueues[i];

            // run the queue
            processSubbands(queue);
          }

          // Signal end of output
          for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
            subbandPool[subband].bequeue->noMore();
          }
        }

        /*
         * SUBBANDPOOL -> STORAGE STREAMS (best effort)
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(ps.nrSubbands())
          for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
            // write subband to Storage
            writeSubband(subband);
          }
        }
      }

      // gather performance figures
      for (size_t i = 0; i < workQueues.size(); ++i ) {
        performance.addQueue(*workQueues[i]);
      }

      // log performance figures
      performance.log(workQueues.size());
    }


    void CorrelatorPipeline::receiveSubbandSamples(
      CorrelatorWorkQueue &workQueue, size_t block, unsigned subband)
    {
      // Fetch the next input object to fill
      SmartPtr<WorkQueueInputData> inputData(workQueue.inputPool.free.remove());

      inputData->block   = block;
      inputData->subband = subband;

      // Wait for previous data to be read
      inputSynchronization.waitFor(block * ps.nrSubbands() + subband);

      workQueue.timers["CPU - input"]->start();

      // Read the samples from the input stream in parallel
#     pragma omp parallel for
      for (unsigned station = 0; station < ps.nrStations(); station++)
      {

        // each input stream contains the data from a single station
        Stream *inputStream = bufferToGPUstreams[station];

        inputData->read(inputStream, station, subband, ps.subbandToSAPmapping()[subband]);
      }

      workQueue.timers["CPU - input"]->stop();

      // Advance the block index
      inputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);

      // Register this block as workQueue input
      workQueue.inputPool.filled.append(inputData);
      ASSERT(!inputData);
    }


    void CorrelatorPipeline::processSubbands(CorrelatorWorkQueue &workQueue)
    {
#if 0
      double lastTime = omp_get_wtime();
#endif

      SmartPtr<WorkQueueInputData> input;

      // Keep fetching input objects until end-of-input
      while ((input = workQueue.inputPool.filled.remove()) != NULL) {
        workQueue.timers["CPU - total"]->start();

        size_t block     = input->block;
        unsigned subband = input->subband;

        // Also fetch an output object to store results
        SmartPtr<CorrelatedDataHostBuffer> output = workQueue.outputPool.free.remove();

        ASSERT(output != NULL); // Only we signal end-of-data, so we should never receive it

        // Perform calculations
        workQueue.timers["CPU - compute"]->start();
        workQueue.doSubband(*input, *output);
        workQueue.timers["CPU - compute"]->stop();

        // Hand off output, force in-order as Storage expects it that way
        subbandPool[subband].sync.waitFor(block);

        // We do the ordering, so we set the sequence numbers
        output->setSequenceNumber(block);

        if (!subbandPool[subband].bequeue->append(output)) {
          LOG_WARN_STR("Dropping subband " << subband << " block " << block);

          // Give back to queue
          workQueue.outputPool.free.append(output);
        }

        ASSERT(!output);

        subbandPool[subband].sync.advanceTo(block + 1);

        // Give back input data for a refill
        workQueue.inputPool.free.append(input);
        ASSERT(!input);

        workQueue.timers["CPU - total"]->stop();
      }

#if 0
#       pragma omp single nowait    // Only a single thread should perform the cout
        LOG_INFO_STR("block = " << block
                  << ", time = " << to_simple_string(from_ustime_t(currentTime))  //current time
                  << ", exec = " << omp_get_wtime() - lastTime);

        // Save the current time: This will be used to display the execution time for this block
        lastTime = omp_get_wtime();
#endif
    }


    void CorrelatorPipeline::writeSubband( unsigned subband )
    {
      SmartPtr<Stream> outputStream;

      // Connect to output stream
      try {
        if (ps.getHostName(CORRELATED_DATA, subband) == "") {
          // an empty host name means 'write to disk directly', to
          // make debugging easier for now
          outputStream = new FileStream(ps.getFileName(CORRELATED_DATA, subband), 0666);
        } else {
          // connect to the Storage_main process for this output
          const std::string desc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, subband);

          outputStream = createStream(desc, false, 0);
        }
      } catch(Exception &ex) {
        LOG_ERROR_STR("Dropping rest of subband " << subband << ": " << ex);

        outputStream = new NullStream;
      }

      SmartPtr<CorrelatedDataHostBuffer> output;

      // Process pool elements until end-of-output
      while ((output = subbandPool[subband].bequeue->remove()) != NULL) {
        try {
          output->write(outputStream.get(), true);
        } catch(Exception &ex) {
          LOG_ERROR_STR("Dropping rest of subband " << subband << ": " << ex);

          outputStream = new NullStream;
        }

        // Hand the object back to the workQueue it originally came from
        CorrelatorWorkQueue &queue = output->queue; // cache queue object, because `output' will be destroyed
        queue.outputPool.free.append(output);

        ASSERT(!output);
      }
    }

  }
}


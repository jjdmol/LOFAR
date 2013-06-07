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
#include <map>
#include <vector>
#include <string>

#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <Stream/Stream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/Stream.h>

#include <InputProc/SampleType.h>
#include <InputProc/Transpose/MPIReceiveStations.h>

#include <GPUProc/OpenMP_Lock.h>
#include <GPUProc/WorkQueues/WorkQueue.h>
#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>
#include <GPUProc/gpu_utils.h>

#ifdef USE_B7015
# include <GPUProc/global_defines.h>
#endif

#define NR_WORKQUEUES_PER_DEVICE  2

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {

    CorrelatorPipeline::CorrelatorPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices)
      :
      Pipeline(ps),
      subbandIndices(subbandIndices),
      subbandPool(subbandIndices.size()),
      filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER)
    {
      filterBank.negateWeights();


      // If profiling, use one workqueue: with >1 workqueues decreased
      // computation / I/O overlap can affect optimization gains.
      unsigned nrWorkQueues = (profiling ? 1 : NR_WORKQUEUES_PER_DEVICE) * devices.size();
      workQueues.resize(nrWorkQueues);

      // Compile all required kernels to ptx
      LOG_INFO("Compiling device kernels");
      double startTime = omp_get_wtime();
      vector<string> kernels;
      map<string, string> ptx;
      kernels.push_back("FIR_Filter.cu");
      kernels.push_back("DelayAndBandPass.cu");
#if defined USE_NEW_CORRELATOR
      kernels.push_back("NewCorrelator.cu");
#else
      kernels.push_back("Correlator.cu");
#endif

      for (vector<string>::const_iterator i = kernels.begin(); i != kernels.end(); ++i) {
        ptx[*i] = createPTX(*i);
      }

      double stopTime = omp_get_wtime();
      LOG_INFO("Compiling device kernels done");
      LOG_DEBUG_STR("Compile time = " << stopTime - startTime);

      // Create the WorkQueues
      CorrelatorPipelinePrograms programs;
      for (size_t i = 0; i < nrWorkQueues; ++i) {
        gpu::Context context(devices[i % devices.size()]);

        programs.firFilterProgram = createModule(context, "FIR_Filter.cu", ptx["FIR_Filter.cu"]);
        programs.delayAndBandPassProgram = createModule(context, "DelayAndBandPass.cu", ptx["DelayAndBandPass.cu"]);
#if defined USE_NEW_CORRELATOR
        programs.correlatorProgram = createModule(context, "NewCorrelator.cu", ptx["NewCorrelator.cu"]);
#else
        programs.correlatorProgram = createModule(context, "Correlator.cu", ptx["Correlator.cu"]);
#endif

        workQueues[i] = new CorrelatorWorkQueue(ps, context, programs, filterBank);
      }

    }

    void CorrelatorPipeline::doWork()
    {
      for (size_t i = 0; i < subbandPool.size(); i++) {
        // Allow 10 blocks to be in the best-effort queue.
        // TODO: make this dynamic based on memory or time
        subbandPool[i].bequeue = new BestEffortQueue< SmartPtr<CorrelatedDataHostBuffer> >(3, ps.realTime());
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
         * BLOCK OF SUBBANDS -> WORKQUEUE INPUTPOOL
         */
#       pragma omp section
        {
          switch (ps.nrBitsPerSample()) {
          default:
          case 16:
            receiveInput< SampleType<i16complex> >(nrBlocks);
            break;
          case 8:
            receiveInput< SampleType<i8complex> >(nrBlocks);
            break;
          case 4:
            receiveInput< SampleType<i4complex> >(nrBlocks);
            break;
          }
        }


        /*
         * WORKQUEUE INPUTPOOL -> WORKQUEUE OUTPUTPOOL
         *
         * Perform GPU processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
#ifdef USE_B7015
            unsigned gpuNr = i % devices.size();
            set_affinity(gpuNr);
#endif
            CorrelatorWorkQueue &queue = *workQueues[i];

            // run the queue
            queue.timers["CPU - total"]->start();
            processSubbands(queue);
            queue.timers["CPU - total"]->stop();

            // Signal end of output
            queue.outputPool.filled.append(NULL);
          }
        }

        /*
         * WORKQUEUE OUTPUTPOOL -> SUBBANDPOOL
         *
         * Perform post-processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
            CorrelatorWorkQueue &queue = *workQueues[i];

            // run the queue
            postprocessSubbands(queue);
          }

          // Signal end of output
          for (size_t i = 0; i < subbandPool.size(); ++i) {
            subbandPool[i].bequeue->noMore();
          }
        }

        /*
         * SUBBANDPOOL -> STORAGE STREAMS (best effort)
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(subbandPool.size())
          for (size_t i = 0; i < subbandPool.size(); ++i) {
            // write subband to Storage
            writeSubband(subbandIndices[i], subbandPool[i]);
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


    // Record type needed by receiveInput. Before c++0x, a local type
    // can't be a template argument, so we'll have to define this type
    // globally.
    struct inputData_t {
      // An InputData object suited for storing one subband from all
      // stations.
      SmartPtr<WorkQueueInputData> data;

      // The WorkQueue associated with the data
      CorrelatorWorkQueue *queue;
    };

    template<typename SampleT> void CorrelatorPipeline::receiveInput( size_t nrBlocks )
    {
      // The length of a block in samples
      size_t blockSize = ps.nrHistorySamples() + ps.nrSamplesPerSubband();

      // SEND: For now, the n stations are sent by the first n ranks.
      vector<int> stationRanks(ps.nrStations());
      for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
        stationRanks[stat] = stat;
      }

      // RECEIVE: Set up to receive our subbands as indicated by subbandIndices

      // Set up the MPI environment.
      MPIReceiveStations receiver(stationRanks, subbandIndices, blockSize);

      // Create a block object to hold all information for receiving one
      // block.
      vector<struct MPIReceiveStations::Block<SampleT> > blocks(ps.nrStations());

      for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
        blocks[stat].beamlets.resize(subbandIndices.size());
      }

      size_t workQueueIterator = 0;

      for (size_t block = 0; block < nrBlocks; block++) {
        // Receive the samples of all subbands from the stations for this
        // block.

        // The set of InputData objects we're using for this block.
        vector<struct inputData_t> inputDatas(subbandIndices.size());

        for (size_t inputIdx = 0; inputIdx < inputDatas.size(); ++inputIdx) {
          // Fetch an input object to store this inputIdx. For now, blindly
          // round-robin over the work queues.
          CorrelatorWorkQueue &queue = *workQueues[workQueueIterator++ % workQueues.size()];

          // Fetch an input object to fill from the selected queue.
          // NOTE: We'll put it in a SmartPtr right away!
          SmartPtr<WorkQueueInputData> data = queue.inputPool.free.remove();

          // Annotate the block
          struct BlockID id;
          id.block            = block;
          id.globalSubbandIdx = subbandIndices[inputIdx];
          id.localSubbandIdx  = inputIdx;
          data->blockID = id;

          // Incorporate it in the receiver's input set.
          for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
            blocks[stat].beamlets[inputIdx].samples = reinterpret_cast<SampleT*>(&data->inputSamples[stat][0][0][0]);
          }

          // Record the block (transfers ownership)
          inputDatas[inputIdx].data = data;
          inputDatas[inputIdx].queue = &queue;
        }

        // Receive all subbands from all stations
        LOG_INFO_STR("[block " << block << "] Reading input samples");
        receiver.receiveBlock<SampleT>(blocks);

        // Process and forward the received input to the processing threads
        for (size_t inputIdx = 0; inputIdx < inputDatas.size(); ++inputIdx) {
          CorrelatorWorkQueue &queue = *inputDatas[inputIdx].queue;
          SmartPtr<WorkQueueInputData> data = inputDatas[inputIdx].data;

          // Translate the metadata as provided by receiver
          for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
            SubbandMetaData &metaData = blocks[stat].beamlets[inputIdx].metaData;

            // extract and apply the flags
            // TODO: Not in this thread! Add a preprocess thread maybe?
            data->inputFlags[stat] = metaData.flags;

            data->flagInputSamples(stat, metaData);

            // extract and assign the delays for the station beams
            for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++)
            {
              unsigned sap = ps.settings.subbands[data->blockID.globalSubbandIdx].SAP;

              data->delaysAtBegin[sap][stat][pol] = metaData.stationBeam.delayAtBegin;
              data->delaysAfterEnd[sap][stat][pol] = metaData.stationBeam.delayAfterEnd;
              data->phaseOffsets[stat][pol] = 0.0;
            }
          }

          queue.inputPool.filled.append(data);
        }

        LOG_DEBUG_STR("[block " << block << "] Forwarded input to processing");
      }

      // Signal end of input
      for (size_t i = 0; i < workQueues.size(); ++i) {
        workQueues[i]->inputPool.filled.append(NULL);
      }
    }

    template void CorrelatorPipeline::receiveInput< SampleType<i16complex> >( size_t nrBlocks );
    template void CorrelatorPipeline::receiveInput< SampleType<i8complex> >( size_t nrBlocks );
    template void CorrelatorPipeline::receiveInput< SampleType<i4complex> >( size_t nrBlocks );


    void CorrelatorPipeline::processSubbands(CorrelatorWorkQueue &workQueue)
    {
      SmartPtr<WorkQueueInputData> input;

      // Keep fetching input objects until end-of-input
      while ((input = workQueue.inputPool.filled.remove()) != NULL) {
        const struct BlockID id = input->blockID;

        LOG_INFO_STR("[" << id << "] Processing start");

        // Also fetch an output object to store results
        SmartPtr<CorrelatedDataHostBuffer> output = workQueue.outputPool.free.remove();
        ASSERT(output != NULL); // Only we signal end-of-data, so we should never receive it

        output->blockID = id;

        // Perform calculations
        workQueue.timers["CPU - process"]->start();
        workQueue.processSubband(*input, *output);
        workQueue.timers["CPU - process"]->stop();

        // Hand off output to post processing
        workQueue.outputPool.filled.append(output);
        ASSERT(!output);

        // Give back input data for a refill
        workQueue.inputPool.free.append(input);
        ASSERT(!input);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to post processing");
      }
    }


    void CorrelatorPipeline::postprocessSubbands(CorrelatorWorkQueue &workQueue)
    {
      SmartPtr<CorrelatedDataHostBuffer> output;

      size_t nrBlocksForwarded = 0;
      size_t nrBlocksDropped = 0;
      time_t lastLogTime = 0;

      // Keep fetching output objects until end-of-output
      while ((output = workQueue.outputPool.filled.remove()) != NULL) {
        const struct BlockID id = output->blockID;

        LOG_INFO_STR("[" << id << "] Post processing start");

        workQueue.timers["CPU - postprocess"]->start();
        workQueue.postprocessSubband(*output);
        workQueue.timers["CPU - postprocess"]->stop();

        // Hand off output, force in-order as Storage expects it that way
        struct Output &pool = subbandPool[id.localSubbandIdx];

        pool.sync.waitFor(id.block);

        // We do the ordering, so we set the sequence numbers
        output->setSequenceNumber(id.block);

        if (!pool.bequeue->append(output)) {
          nrBlocksDropped++;
          //LOG_WARN_STR("[block " << block << "] Dropped for subband " << globalSubbandIdx);

          // Give back to queue
          workQueue.outputPool.free.append(output);
        } else {
          nrBlocksForwarded++;
        }

        // Allow next block to be written
        pool.sync.advanceTo(id.block + 1);

        ASSERT(!output);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to writer");

        if (time(0) != lastLogTime) {
          lastLogTime = time(0);

          LOG_INFO_STR("Forwarded " << nrBlocksForwarded << " blocks, dropped " << nrBlocksDropped << " blocks");
        }
      }
    }


    void CorrelatorPipeline::writeSubband( unsigned globalSubbandIdx, struct Output &output )
    {
      SmartPtr<Stream> outputStream;

      // Connect to output stream
      try {
        if (ps.getHostName(CORRELATED_DATA, globalSubbandIdx) == "") {
          // an empty host name means 'write to disk directly', to
          // make debugging easier for now
          outputStream = new FileStream(ps.getFileName(CORRELATED_DATA, globalSubbandIdx), 0666);
        } else {
          // connect to the Storage_main process for this output
          const std::string desc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, globalSubbandIdx);

          outputStream = createStream(desc, false, 0);
        }
      } catch(Exception &ex) {
        LOG_ERROR_STR("Dropping rest of subband " << globalSubbandIdx << ": " << ex);

        outputStream = new NullStream;
      }

      SmartPtr<CorrelatedDataHostBuffer> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.bequeue->remove()) != NULL) {
        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );

        // Cache workQueue reference, because `output' will be destroyed.
        CorrelatorWorkQueue &workQueue = outputData->workQueue;

        LOG_INFO_STR("[" << id << "] Writing start");

        // Write block to disk 
        try {
          outputData->write(outputStream.get(), true);
        } catch(Exception &ex) {
          LOG_ERROR_STR("Dropping rest of subband " << id.globalSubbandIdx << ": " << ex);

          outputStream = new NullStream;
        }

        // Hand the object back to the workQueue it originally came from
        workQueue.outputPool.free.append(outputData);

        ASSERT(!outputData);

        LOG_INFO_STR("[" << id << "] Done");
      }
    }

  }
}


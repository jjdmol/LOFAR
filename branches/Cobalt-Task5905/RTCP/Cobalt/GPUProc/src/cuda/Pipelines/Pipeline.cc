//# Pipeline.cc
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

#include "Pipeline.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Common/lofar_iomanip.h>
#include <ApplCommon/PosixTime.h>
#include <ApplCommon/PVSSDatapointDefs.h>
#include <Stream/Stream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>

#include <CoInterface/Align.h>
#include <CoInterface/Stream.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/Kernel.h>
#include <InputProc/SampleType.h>
#include <InputProc/RSPTimeStamp.h>

#ifdef HAVE_MPI
#include <InputProc/Transpose/MPIReceiveStations.h>
#include <InputProc/Transpose/MPIProtocol.h>
#else
#include <GPUProc/Station/StationInput.h>
#endif

#include <cmath>

#define NR_WORKQUEUES_PER_DEVICE  1

// Actually do any processing.
//
// If not set, station input is received but discarded immediately.
#define DO_PROCESSING

// The number of seconds to wait for output to flush to outputProc.
//
// This timer is required to kill slow connections by aborting the
// write().
const double outputFlushTimeout = 10.0;

namespace LOFAR
{
  namespace Cobalt
  {
    using namespace std;
    using boost::format;
    using boost::lexical_cast;

    static TABTranspose::MultiSender::HostMap hostMap(const Parset &ps, const vector<size_t> &subbandIndices, int hostID)
    {
      TABTranspose::MultiSender::HostMap hostMap;

      if (!ps.settings.beamFormer.enabled)
        return hostMap;

      // The requested service is an unique identifier for this observation,
      // and for our process.
      const string service = str(format("2nd-transpose-obs-%u-fromrank-%u")
                             % ps.settings.observationID
                             % hostID);

      // The PortBroker listen port is the same for all outputProc nodes.
      const uint16_t brokerPort = storageBrokerPort(ps.settings.observationID);

      // Create the list of all outputProcs we send data to.
      for (size_t fileIdx = 0; fileIdx < ps.settings.beamFormer.files.size(); ++fileIdx) {
        const struct ObservationSettings::BeamFormer::File &file = ps.settings.beamFormer.files[fileIdx];
        struct TABTranspose::MultiSender::Host host;

        // Check whether we really will write to this file
        bool willUse = false;
        for (size_t i = 0; i < subbandIndices.size(); ++i) {
          // All files to our SAPs and subbands (parts) will be relevant.
          const unsigned globalSubbandIdx = subbandIndices[i];
          const unsigned SAP = ps.settings.subbands[globalSubbandIdx].SAP;

          if (file.sapNr != SAP)
            continue;

          if (globalSubbandIdx < file.firstSubbandIdx || globalSubbandIdx >= file.lastSubbandIdx)
            continue;

          willUse = true;
          break;
        }

        if (!willUse)
          continue;

        // Add file to our list of outputs
        host.hostName = file.location.host;
        host.brokerPort = brokerPort;
        host.service = service;

        hostMap[fileIdx] = host;
      }

      return hostMap;
    }


    Pipeline::Pipeline(const Parset &ps, 
         const std::vector<size_t> &subbandIndices, 
         const std::vector<gpu::Device> &devices, 
         Pool<struct MPIRecvData> &pool,
         RTmetadata &mdLogger, const std::string &mdKeyPrefix,
         int hostID)
      :
      subbandProcs(std::max(1UL, (profiling ? 1 : NR_WORKQUEUES_PER_DEVICE) * devices.size())),
      ps(ps),
      devices(devices),
      subbandIndices(subbandIndices),
      processingSubband0(std::find(subbandIndices.begin(), subbandIndices.end(), 0U) != subbandIndices.end()),
      nrSubbandsPerSubbandProc(ceilDiv(subbandIndices.size(), subbandProcs.size())),
      itsMdLogger(mdLogger),
      itsMdKeyPrefix(mdKeyPrefix),
      mpiPool(pool),
      writePool(subbandIndices.size()),
      factories(ps, nrSubbandsPerSubbandProc),

      // Each work queue needs an output element for each subband it processes, because the GPU output can
      // be in bulk: if processing is cheap, all subbands will be output right after they have been received.
      //
      // Allow queue to drop items older than 3 seconds.
      multiSender(hostMap(ps, subbandIndices, hostID), ps, mdLogger, mdKeyPrefix, 3.0)
    {
      ASSERTSTR(!devices.empty(), "Not bound to any GPU!");

      // Write data point(s) for monitoring (PVSS).
      itsMdLogger.log(itsMdKeyPrefix + PN_CGP_OBSERVATION_NAME, boost::lexical_cast<string>(ps.observationID()));
      for (unsigned i = 0; i < subbandIndices.size(); ++i) {
        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_SUBBAND + '[' + boost::lexical_cast<string>(subbandIndices[i]) + ']',
                        (int)subbandIndices[i]);
      }

      string dataProductType;

      switch (1 * (int)ps.settings.beamFormer.enabled
            + 2 * (int)ps.settings.correlator.enabled) {
        case 3:
          dataProductType = "Correlated + Beamformed";
          break;
        case 2:
          dataProductType = "Correlated";
          break;
        case 1:
          dataProductType = "Beamformed";
          break;
        case 0:
        default:
          dataProductType = "None";
          break;
      }

      itsMdLogger.log(itsMdKeyPrefix + PN_CGP_DATA_PRODUCT_TYPE, dataProductType);
    }

    Pipeline::~Pipeline()
    {
      if (ps.settings.realTime) {
        // Ensure all output is stopped, even if we didn't start processing.
        outputThreads.killAll();
      }
    }

    void Pipeline::allocateResources()
    {
      for (size_t i = 0; i < writePool.size(); i++) {
        writePool[i].queue = new Queue< SmartPtr<SubbandProcOutputData> >(str(boost::format("Pipeline::writePool [local subband %u]") % i));
      }

      // Create the SubbandProcs, which in turn allocate the GPU buffers and
      // functions.
      for (size_t i = 0; i < subbandProcs.size(); ++i) {
        gpu::Context context(devices[i % devices.size()]);

        subbandProcs[i] = new SubbandProc(ps, context, factories, nrSubbandsPerSubbandProc);
      }
    }


    void Pipeline::processObservation()
    {

      LOG_INFO("----- Allocating resources");
      allocateResources();


      //sections = program segments defined by the following omp section directive
      //           are distributed for parallel execution among available threads
      //parallel = directive explicitly instructs the compiler to parallelize the chosen block of code.
      //  The two sections in this function are done in parallel with a seperate set of threads.
#     pragma omp parallel sections num_threads(6)
      {

        /*
         * MPIQUEUE -> WORKQUEUE INPUTPOOL
         */
#       pragma omp section
        {
          transposeInput();
        }

        /*
         * WORKQUEUE INPUTPOOL -> PROCESSPOOL
         *
         * Perform pre-processing, one thread per subbandProc.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(subbandProcs.size())
          for (size_t i = 0; i < subbandProcs.size(); ++i) {
            SubbandProc &queue = *subbandProcs[i];

            // run the queue
            preprocessSubbands(queue);

            queue.processPool.filled.append(NULL);
          }
        }


        /*
         * WORKQUEUE INPUTPOOL -> WORKQUEUE OUTPUTPOOL
         *
         * Perform GPU processing, one thread per subbandProc.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(subbandProcs.size())
          for (size_t i = 0; i < subbandProcs.size(); ++i) 
          {
            SubbandProc &queue = *subbandProcs[i];

            // run the queue
            processSubbands(queue);

            // Signal end of output
            queue.outputPool.filled.append(NULL);
          }
        }

        /*
         * WORKQUEUE OUTPUTPOOL -> WRITEPOOL
         *
         * Perform post-processing, one thread per subbandProc.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(subbandProcs.size())
          for (size_t i = 0; i < subbandProcs.size(); ++i) {
            SubbandProc &queue = *subbandProcs[i];

            // run the queue
            postprocessSubbands(queue);
          }

          // Signal end of output
          for (size_t i = 0; i < writePool.size(); ++i) {
            writePool[i].queue->append(NULL);
          }

          // Wait for data to propagate towards outputProc,
          // and kill lingering outputThreads.
          if (ps.realTime()) {
            struct timespec deadline = TimeSpec::now();
            TimeSpec::inc(deadline, outputFlushTimeout);

            LOG_INFO_STR("Pipeline: Flushing data for at most " << outputFlushTimeout << " seconds.");
            size_t numKilled = outputThreads.killAll(deadline);

            if (numKilled == 0) {
              LOG_INFO("Pipeline: Data flushed succesfully.");
            } else {
              LOG_WARN_STR("Pipeline: Data flushed, but had to kill " << numKilled << " writer threads.");
            }
          }
        }

        /*
         * WRITEPOOL -> STORAGE STREAMS (best effort)
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(writePool.size())
          for (size_t i = 0; i < writePool.size(); ++i) {
            writeOutput(subbandIndices[i], *writePool[i].queue, subbandProcs[i % subbandProcs.size()]->outputPool.free);
          }

          // Signal end-of-output to unlock
          // outputThreads that are waiting for their NULL marker. Because of
          // the 2nd transpose, writeBeamformedOutput cannot
          // insert the NULL itself; it needs to be a collective action.
          multiSender.finish();
        }

        // Output processing
#       pragma omp section
        {
          multiSender.process(&outputThreads);
        }
      }
    }


    template<typename SampleT>
    void Pipeline::transposeInput()
    {
      SmartPtr<struct MPIRecvData> input;

      NSTimer copyTimer("transpose input data", true, true);

      // Keep fetching input objects until end-of-output
      while ((input = mpiPool.filled.remove()) != NULL) {
        const ssize_t block = input->block;

        vector<size_t> nrFlaggedSamples(ps.nrStations(), 0);

#ifdef DO_PROCESSING
        MultiDimArray<SampleT,3> data(
          boost::extents[ps.nrStations()][subbandIndices.size()][ps.settings.blockSize],
          (SampleT*)input->data.get(), false);

        MultiDimArray<struct MPIProtocol::MetaData,2> metaData(
          boost::extents[ps.nrStations()][subbandIndices.size()],
          (struct MPIProtocol::MetaData*)input->metaData.get(), false);

        // The set of InputData objects we're using for this block.
        vector< SmartPtr<SubbandProcInputData> > inputDatas(subbandIndices.size());

        for (size_t subbandIdx = 0; subbandIdx < subbandIndices.size(); ++subbandIdx) {
          // Fetch an input object to store this subband.
          SubbandProc &queue = *subbandProcs[subbandIdx % subbandProcs.size()];

          // Fetch an input object to fill from the selected queue.
          SmartPtr<SubbandProcInputData> subbandData = queue.inputPool.free.remove();

          // Annotate the block
          struct BlockID id;
          id.block                 = block;
          id.globalSubbandIdx      = subbandIndices[subbandIdx];
          id.localSubbandIdx       = subbandIdx;
          id.subbandProcSubbandIdx = subbandIdx / subbandProcs.size();
          subbandData->blockID = id;

          copyTimer.start();
          for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
            // Copy the data
#if 1
            memcpy(&subbandData->inputSamples[stat][0][0][0],
                   &data[stat][subbandIdx][0],
                   ps.settings.blockSize * sizeof(SampleT));
#endif
            // Copy the metadata
            subbandData->metaData[stat] = metaData[stat][subbandIdx];

            nrFlaggedSamples[stat] += subbandData->metaData[stat].flags.count();
          }
          copyTimer.stop();

          queue.inputPool.filled.append(subbandData);
        }
#endif

        mpiPool.free.append(input);
        ASSERT(!input);

        // Report flags per antenna field
        stringstream flagStr;  // antenna fields with >0% flags
        stringstream cleanStr; // antenna fields with  0% flags

        for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
          const double flagPerc = 100.0 * nrFlaggedSamples[stat] / subbandIndices.size() / ps.settings.blockSize;

          if (flagPerc == 0.0)
            cleanStr << str(boost::format("%s, ") % ps.settings.antennaFields[stat].name);
          else
            flagStr << str(boost::format("%s: %.1f%%, ") % ps.settings.antennaFields[stat].name % flagPerc);
        }

        LOG_DEBUG_STR("[block " << block << "] No flagging: " << cleanStr.str());

        if (!flagStr.str().empty()) {
          LOG_WARN_STR("[block " << block << "] Flagging:    " << flagStr.str());
        }

        LOG_DEBUG_STR("[block " << block << "] Forwarded input to pre processing");
      }

      // Signal end of input
      for (size_t i = 0; i < subbandProcs.size(); ++i) {
        subbandProcs[i]->inputPool.filled.append(NULL);
      }
    }

    void Pipeline::transposeInput()
    {
      switch (ps.nrBitsPerSample()) {
      default:
      case 16:
        transposeInput< SampleType<i16complex> >();
        break;
      case 8:
        transposeInput< SampleType<i8complex> >();
        break;
      case 4:
        transposeInput< SampleType<i4complex> >();
        break;
      }
    }


    void Pipeline::preprocessSubbands(SubbandProc &subbandProc)
    {
      SmartPtr<SubbandProcInputData> input;

      NSTimer preprocessTimer("preprocess", true, true);

      // Keep fetching input objects until end-of-output
      while ((input = subbandProc.inputPool.filled.remove()) != NULL) {
        const struct BlockID &id = input->blockID;

        LOG_DEBUG_STR("[" << id << "] Pre processing start");

        /* PREPROCESS START */
        preprocessTimer.start();

        const unsigned SAP = ps.settings.subbands[id.globalSubbandIdx].SAP;

        // Translate the metadata as provided by receiver
        for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
          input->applyMetaData(ps, stat, SAP, input->metaData[stat]);
        }

        preprocessTimer.stop();
        /* PREPROCESS END */

        // Hand off output to processing
        subbandProc.processPool.filled.append(input);
        ASSERT(!input);

        LOG_DEBUG_STR("[" << id << "] Forwarded input to processing");
      }
    }


    void Pipeline::processSubbands(SubbandProc &subbandProc)
    {
      SmartPtr<SubbandProcInputData> input;

      NSTimer processTimer("process", true, true);

      // Keep fetching input objects until end-of-input
      while ((input = subbandProc.processPool.filled.remove()) != NULL) {
        const struct BlockID id = input->blockID;

        LOG_DEBUG_STR("[" << id << "] Processing start");

        // Also fetch an output object to store results
        SmartPtr<SubbandProcOutputData> output = subbandProc.outputPool.free.remove();

        // Only _we_ signal end-of-data, so we should _never_ receive it
        ASSERT(output != NULL); 

        output->blockID = id;

        // Perform calculations
        processTimer.start();
        subbandProc.processSubband(*input, *output);
        processTimer.stop();

        if (id.block < 0) {
          // Ignore block; only used to initialize FIR history samples
          subbandProc.outputPool.free.append(output);
        } else {
          // Hand off output to post processing
          subbandProc.outputPool.filled.append(output);
        }
        ASSERT(!output);

        // Give back input data for a refill
        subbandProc.inputPool.free.append(input);
        ASSERT(!input);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to post processing");
      }
    }


    void Pipeline::postprocessSubbands(SubbandProc &subbandProc)
    {
      SmartPtr<SubbandProcOutputData> output;

      NSTimer postprocessTimer("postprocess", true, true);

      // Keep fetching output objects until end-of-output
      while ((output = subbandProc.outputPool.filled.remove()) != NULL) {
        const struct BlockID id = output->blockID;

        LOG_DEBUG_STR("[" << id << "] Post processing start");

        postprocessTimer.start();
        subbandProc.postprocessSubband(*output);
        postprocessTimer.stop();

        struct Output &pool = writePool[id.localSubbandIdx];

        pool.queue->append(output);
        ASSERT(!output);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to writer");
      }
    }

    void Pipeline::writeOutput(
      unsigned globalSubbandIdx,
      Queue< SmartPtr<SubbandProcOutputData> > &inputQueue,
      Queue< SmartPtr<SubbandProcOutputData> > &outputQueue )
    {
      Queue< SmartPtr<SubbandProcOutputData> > queue(str(boost::format("Pipeline::writeOutput [subband %u]") % globalSubbandIdx));

#     pragma omp parallel sections num_threads(2)
      {
        // Let parent do work
#       pragma omp section
        {
          writeBeamformedOutput(globalSubbandIdx, inputQueue, queue, outputQueue);
          queue.append(NULL);
        }

        // Output processing
#       pragma omp section
        {
          writeCorrelatedOutput(globalSubbandIdx, queue, outputQueue);
        }
      }
    }

    // Write the blocks of rtcp bf output via the MultiSender towards outputProc.
    // Removes the blocks the queue in 'output'.
    // All output corresponds to the subband indexed by globalSubbandIdx in the list of sb.
    void Pipeline::writeBeamformedOutput(
      unsigned globalSubbandIdx,
      Queue< SmartPtr<SubbandProcOutputData> > &inputQueue,
      Queue< SmartPtr<SubbandProcOutputData> > &outputQueue,
      Queue< SmartPtr<SubbandProcOutputData> > &spillQueue )
    {
      NSTimer transposeTimer(str(format("Pipeline::writeOutput(subband %u) transpose/file") % globalSubbandIdx), true, true);
      NSTimer forwardTimer(str(format("Pipeline::writeOutput(subband %u) forward/file") % globalSubbandIdx), true, true);

      const unsigned SAP = ps.settings.subbands[globalSubbandIdx].SAP;

      // Statistics for forwarding blocks to writeCorrelatedOutput
      bool dropping = false;
      size_t blocksWritten = 0;
      size_t blocksDropped = 0;

      SmartPtr<SubbandProcOutputData> data;

      // Process pool elements until end-of-output
      while ((data = inputQueue.remove()) != NULL) 
      {
        const struct BlockID id = data->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );
        ASSERT( id.block >= 0 ); // Negative blocks should not reach storage

        LOG_DEBUG_STR("[" << id << "] Writing start");

        if (ps.settings.beamFormer.enabled) {
          // Try all files until we found the file(s) this block belongs to.
          // TODO: This could be optimized, either by recognizing that only the Stokes can
          // change the file idx, or by preparing a better data structure for 1 lookup here.
          // Idem for the data copying (assign()) below.
          for (size_t fileIdx = 0;
               fileIdx < ps.settings.beamFormer.files.size();
               ++fileIdx) 
          {
            const struct ObservationSettings::BeamFormer::File &file = 
                  ps.settings.beamFormer.files[fileIdx];

            // Skip SAPs and subbands (parts) that we are not responsible for.
            if (file.sapNr != SAP)
              continue;

            if (globalSubbandIdx < file.firstSubbandIdx || globalSubbandIdx >= file.lastSubbandIdx)
              continue;

            // Note that the 'file' encodes 1 Stokes of 1 TAB, so each TAB we've
            // produced can be visited 1 or 4 times.

            // Compute shape of block
            const ObservationSettings::BeamFormer::StokesSettings &stokes =
              file.coherent
              ? ps.settings.beamFormer.coherentSettings
              : ps.settings.beamFormer.incoherentSettings;

            const size_t nrChannels = stokes.nrChannels;
            const size_t nrSamples =  stokes.nrSamples;

            // Our data has the shape
            //   data->(in)coherentData[tab][stokes][sample][channel]
            //
            // To transpose our data, we copy a slice representing
            //   slice[sample][channel]
            // and send it to outputProc to combine with the other subbands.
            //
            // We create a copy to be able to release outputData, since our
            // slices can be blocked by writes to any number of outputProcs.
            SmartPtr<struct TABTranspose::Subband> subband = 
                  new TABTranspose::Subband(nrSamples, nrChannels);

            // These 3 values are guarded with ASSERTSTR() on the other side at
            // outputProc (Block::addSubband()).
            subband->id.fileIdx  = file.streamNr;
            // global to local sb idx: here local means to the TAB Transpose,
            // which only knows about #subbands and #blocks in a file (part).
            unsigned sbIdxInFile = globalSubbandIdx - file.firstSubbandIdx;
            subband->id.subband  = sbIdxInFile;
            subband->id.block    = id.block;

            // Create view of subarray 
            MultiDimArray<float, 2> srcData(
                boost::extents[nrSamples][nrChannels],
                file.coherent
                     ? data->coherentData[file.coherentIdxInSAP][file.stokesNr].origin()
                     : data->incoherentData[file.incoherentIdxInSAP][file.stokesNr].origin(),
                false);

            // Copy data to block
            transposeTimer.start();
            subband->data.assign(srcData.origin(), srcData.origin() + srcData.num_elements());
            transposeTimer.stop();

            // Forward block to MultiSender, who takes ownership.
            forwardTimer.start();
            multiSender.append(subband);
            forwardTimer.stop();

            // If `subband' is still alive, it has been dropped instead of sent.
            ASSERT(ps.realTime() || !subband); 
          }
        }

        // Return outputData back to the subbandProc.
        const double maxRetentionTime = 3.0;
        using namespace TimeSpec;
        if (ps.settings.realTime && TimeSpec::now() - outputQueue.oldest() > maxRetentionTime) {
          // Drop
          spillQueue.append(data);
          dropping = true;
          blocksDropped++;
        } else {
          // Forward to correlator
          outputQueue.append(data);
          dropping = false;
          blocksWritten++;
        }
        ASSERT(!data);

        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_DROPPING + '[' + lexical_cast<string>(globalSubbandIdx) + ']',
                        dropping);
        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_WRITTEN  + '[' + lexical_cast<string>(globalSubbandIdx) + ']',
                        blocksWritten * static_cast<float>(ps.settings.blockDuration()));
        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_DROPPED  + '[' + lexical_cast<string>(globalSubbandIdx) + ']',
                        blocksDropped * static_cast<float>(ps.settings.blockDuration()));

        if (id.localSubbandIdx == 0 || id.localSubbandIdx == subbandIndices.size() - 1)
          LOG_INFO_STR("[" << id << "] Done"); 
        else
          LOG_DEBUG_STR("[" << id << "] Done"); 
      }
    }


    void Pipeline::writeCorrelatedOutput(
      unsigned globalSubbandIdx,
      Queue< SmartPtr<SubbandProcOutputData> > &inputQueue,
      Queue< SmartPtr<SubbandProcOutputData> > &outputQueue )
    {
      // Register our thread to be killable at exit
      OMPThreadSet::ScopedRun sr(outputThreads);

      SmartPtr<Stream> outputStream;

      if (ps.settings.correlator.enabled) {
        const string desc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, globalSubbandIdx);

        try {
          outputStream = createStream(desc, false, ps.realTime() ? ps.stopTime() : 0);
        } catch (Exception &ex) {
          LOG_ERROR_STR("Error writing subband " << globalSubbandIdx << ", dropping all subsequent blocks: " << ex.what());
          return;
        }
      }

      SmartPtr<SubbandProcOutputData> data;

      // Process pool elements until end-of-output
      while ((data = inputQueue.remove()) != NULL) {
        CorrelatedData &correlatedData = data->correlatedData;

        const struct BlockID id = data->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );

        if (data->emit_correlatedData) {
          ASSERT(ps.settings.correlator.enabled);
          ASSERT(outputStream.get());

          LOG_DEBUG_STR("[" << id << "] Writing start");

          // Write block to outputProc 
          try {
            correlatedData.write(outputStream.get(), true);
          } catch (Exception &ex) {
            // No reconnect, as outputProc doesn't yet re-listen when the conn drops.
            LOG_ERROR_STR("Error writing subband " << id.globalSubbandIdx << ", dropping all subsequent blocks: " << ex.what());
            return;
          }

          if (id.localSubbandIdx == 0 || id.localSubbandIdx == subbandIndices.size() - 1)
            LOG_INFO_STR("[" << id << "] Done"); 
          else
            LOG_DEBUG_STR("[" << id << "] Done"); 
        }

        outputQueue.append(data);
        ASSERT(!data);
      }
    }
  }
}

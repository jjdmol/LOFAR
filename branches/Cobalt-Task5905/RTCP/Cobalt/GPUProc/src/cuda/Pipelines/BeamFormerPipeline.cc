//# BeamFormerPipeline.cc
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

#include "BeamFormerPipeline.h"

#include <vector>
#include <string>
#include <iomanip>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <ApplCommon/PVSSDatapointDefs.h>

#include <CoInterface/SmartPtr.h>
#include <CoInterface/Stream.h>
#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    using namespace std;
    using boost::format;

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

    BeamFormerPipeline::BeamFormerPipeline(const Parset &ps, 
         const std::vector<size_t> &subbandIndices, 
         const std::vector<gpu::Device> &devices, 
         Pool<struct MPIRecvData> &pool,
         RTmetadata &mdLogger, const std::string &mdKeyPrefix,
         int hostID)
      :
      Pipeline(ps, subbandIndices, devices, pool, mdLogger, mdKeyPrefix),
      // Each work queue needs an output element for each subband it processes, because the GPU output can
      // be in bulk: if processing is cheap, all subbands will be output right after they have been received.
      //
      // Allow queue to drop items older than 3 seconds.
      multiSender(hostMap(ps, subbandIndices, hostID), ps,
                  mdLogger, mdKeyPrefix, 3.0),
      factories(ps, nrSubbandsPerSubbandProc)
    {

      // Write data point(s) for monitoring (PVSS).
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

    void BeamFormerPipeline::allocateResources()
    {
      Pipeline::allocateResources();

      // Create the SubbandProcs, which in turn allocate the GPU buffers and
      // functions.
      for (size_t i = 0; i < workQueues.size(); ++i) {
        gpu::Context context(devices[i % devices.size()]);

        workQueues[i] = new BeamFormerSubbandProc(ps, context, factories, nrSubbandsPerSubbandProc);
      }
    }

    BeamFormerPipeline::~BeamFormerPipeline()
    {
    }


    void BeamFormerPipeline::processObservation()
    {
#     pragma omp parallel sections num_threads(2)
      {
        // Let parent do work
#       pragma omp section
        {
          Pipeline::processObservation();
        }

        // Output processing
#       pragma omp section
        {
          multiSender.process(&outputThreads);
        }
      }
    }

    // Write the blocks of rtcp bf output via the MultiSender towards outputProc.
    // Removes the blocks the queue in 'output'.
    // All output corresponds to the subband indexed by globalSubbandIdx in the list of sb.
    void BeamFormerPipeline::writeOutput( unsigned globalSubbandIdx,
           struct Output &output )
    {
      NSTimer transposeTimer(str(format("BeamFormerPipeline::writeOutput(subband %u) transpose/file") % globalSubbandIdx), true, true);
      NSTimer forwardTimer(str(format("BeamFormerPipeline::writeOutput(subband %u) forward/file") % globalSubbandIdx), true, true);

      const unsigned SAP = ps.settings.subbands[globalSubbandIdx].SAP;

      SmartPtr<SubbandProcOutputData> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.queue->remove()) != NULL) 
      {
        BeamFormedData &beamFormedData = dynamic_cast<BeamFormedData&>(*outputData);

        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );
        ASSERT( id.block >= 0 ); // Negative blocks should not reach storage

        LOG_DEBUG_STR("[" << id << "] Writing start");

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
          //   beamFormedData.(in)coherentData[tab][stokes][sample][channel]
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
                   ? beamFormedData.coherentData[file.coherentIdxInSAP][file.stokesNr].origin()
                   : beamFormedData.incoherentData[file.incoherentIdxInSAP][file.stokesNr].origin(),
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

        // Return outputData back to the workQueue.
        SubbandProc &workQueue = *workQueues[id.localSubbandIdx % workQueues.size()];
        workQueue.outputPool.free.append(outputData);

        ASSERT(!outputData);

        if (id.localSubbandIdx == 0 || id.localSubbandIdx == subbandIndices.size() - 1)
          LOG_INFO_STR("[" << id << "] Done"); 
        else
          LOG_DEBUG_STR("[" << id << "] Done"); 
      }
    }


    void BeamFormerPipeline::doneWritingOutput()
    {
      // Done producing output
      multiSender.finish();
    }
  }
}

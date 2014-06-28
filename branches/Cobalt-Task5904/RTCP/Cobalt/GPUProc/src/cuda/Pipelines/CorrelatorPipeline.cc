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

#include <map>
#include <vector>
#include <string>
#include <iomanip>
#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <ApplCommon/PVSSDatapointDefs.h>
#include <Stream/Stream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>

#include <CoInterface/Stream.h>
#include <CoInterface/RunningStatistics.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/PerformanceCounter.h>

namespace LOFAR
{
  namespace Cobalt
  {
    using boost::lexical_cast;

    CorrelatorPipeline::CorrelatorPipeline(const Parset &ps,
     const std::vector<size_t> &subbandIndices, 
     const std::vector<gpu::Device> &devices,
     Pool<struct MPIRecvData> &pool,
     RTmetadata &mdLogger, const std::string &mdKeyPrefix)
      :
      Pipeline(ps, subbandIndices, devices, pool, mdLogger, mdKeyPrefix),
      factories(ps, nrSubbandsPerSubbandProc),
      itsBlocksWritten(0),
      itsBlocksDropped(0),
      itsNextSequenceNumber(0)
    {
      // Write data point(s) for monitoring (PVSS).
      itsMdLogger.log(itsMdKeyPrefix + PN_CGP_DATA_PRODUCT_TYPE, "Correlated");
    }

    void CorrelatorPipeline::allocateResources()
    {
      Pipeline::allocateResources();

      // Create the SubbandProcs
      for (size_t i = 0; i < workQueues.size(); ++i) 
      {
        gpu::Context context(devices[i % devices.size()]);

        workQueues[i] = new CorrelatorSubbandProc(ps, context, factories, nrSubbandsPerSubbandProc);
      }
    }

    CorrelatorPipeline::~CorrelatorPipeline()
    {
    }


    void CorrelatorPipeline::writeOutput( unsigned globalSubbandIdx, struct Output &output )
    {
      // Register our thread to be killable at exit
      OMPThreadSet::ScopedRun sr(outputThreads);

      SmartPtr<Stream> outputStream = connectToOutput(globalSubbandIdx);

      SmartPtr<SubbandProcOutputData> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.bequeue->remove()) != NULL) {
        CorrelatedData &correlatedData = dynamic_cast<CorrelatedData&>(*outputData);

        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );

        LOG_DEBUG_STR("[" << id << "] Writing start");

        size_t droppedBlocks = correlatedData.sequenceNumber() - itsNextSequenceNumber;
        itsNextSequenceNumber = correlatedData.sequenceNumber() + 1;

        // Write block to outputProc 
        try {
          correlatedData.write(outputStream.get(), true);

          itsBlocksWritten += 1;

        } catch (Exception &ex) {
          // No reconnect, as outputProc doesn't yet re-listen when the conn drops.
          LOG_ERROR_STR("Error writing subband " << id.globalSubbandIdx << ", dropping all subsequent blocks: " << ex.what());

          outputStream = new NullStream;

          droppedBlocks += 1;
        }

        SubbandProc &workQueue = *workQueues[id.localSubbandIdx % workQueues.size()];
        workQueue.outputPool.free.append(outputData);

        ASSERT(!outputData);

        if (id.localSubbandIdx == 0 || id.localSubbandIdx == subbandIndices.size() - 1)
          LOG_INFO_STR("[" << id << "] Done"); 
        else
          LOG_DEBUG_STR("[" << id << "] Done"); 

        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_DROPPING + '[' + lexical_cast<string>(globalSubbandIdx) + ']',
                        droppedBlocks > 0);
        itsBlocksDropped += droppedBlocks;
        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_WRITTEN  + '[' + lexical_cast<string>(globalSubbandIdx) + ']',
                        itsBlocksWritten * static_cast<float>(ps.settings.blockDuration()));
        itsMdLogger.log(itsMdKeyPrefix + PN_CGP_DROPPED  + '[' + lexical_cast<string>(globalSubbandIdx) + ']',
                        itsBlocksDropped * static_cast<float>(ps.settings.blockDuration()));
      }
    }


    SmartPtr<Stream> CorrelatorPipeline::connectToOutput(unsigned globalSubbandIdx) const
    {
      SmartPtr<Stream> outputStream;

      try {
        if (ps.getHostName(CORRELATED_DATA, globalSubbandIdx) == "") {
          // an empty host name means 'write to disk directly', to
          // make debugging easier for now
          outputStream = new FileStream(ps.getFileName(CORRELATED_DATA, globalSubbandIdx), 0666);
        } else {
          // connect to the output process for this output
          const std::string desc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, globalSubbandIdx);
          outputStream = createStream(desc, false, ps.realTime() ? ps.stopTime() : 0);
        }
      } catch (Exception &ex) {
        LOG_ERROR_STR("Failed to connect to output proc; dropping rest of subband " << globalSubbandIdx << ": " << ex.what());

        outputStream = new NullStream;
      }

      return outputStream;
    }
  }
}

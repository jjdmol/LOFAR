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

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>

#include <CoInterface/Stream.h>
#include <GPUProc/SubbandProcs/CorrelatorSubbandProc.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/RunningStatistics.h>

namespace LOFAR
{
  namespace Cobalt
  {

    CorrelatorPipeline::CorrelatorPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices, const std::vector<gpu::Device> &devices)
      :
      Pipeline(ps, subbandIndices, devices)
    {
      CorrelatorFactories factories(ps, nrSubbandsPerSubbandProc);

      // Create the SubbandProcs
      for (size_t i = 0; i < workQueues.size(); ++i) 
      {
        gpu::Context context(devices[i % devices.size()]);

        workQueues[i] = new CorrelatorSubbandProc(ps, context, factories, nrSubbandsPerSubbandProc);
      }
    }

    CorrelatorPipeline::~CorrelatorPipeline()
    {
      try
      { 
        // TODO: I'm not really happy with this construction: Pipeline needs to know
        // to much about the subbandProc, codesmell.
        if(gpuProfiling)
        {
          // gpu kernel counters
          RunningStatistics fir;
          RunningStatistics fft;
          RunningStatistics delayBp;
          RunningStatistics correlator;

          // gpu transfer counters
          RunningStatistics samples;
          RunningStatistics visibilities;
          for (size_t idx_queue = 0; idx_queue < workQueues.size(); ++idx_queue)
          {
            //We know we are in the correlator pipeline, this queue can only contain correlatorSubbandprocs
            CorrelatorSubbandProc *proc = dynamic_cast<CorrelatorSubbandProc *>(workQueues[idx_queue].get());

            // Print the individual counters
            proc->counters.printStats();

            // Calculate aggregate statistics for the whole pipeline
            fir += proc->counters.fir.stats;
            fft += proc->counters.fft.stats;
            delayBp += proc->counters.delayBp.stats;
            correlator += proc->counters.correlator.stats;
            samples += proc->counters.samples.stats;
            visibilities += proc->counters.visibilities.stats;
          }

          // Now print the aggregate statistics.
          LOG_INFO_STR("**** GPU runtimes for the complete Correlator pipeline n=" << workQueues.size()
                        << " ****" << endl <<
                        std::setw(20) << "(fir)" << fir << endl <<
                        std::setw(20) << "(fft)" << fft << endl <<
                        std::setw(20) << "(delayBp)" << delayBp << endl <<
                        std::setw(20) << "(correlator)" << correlator << endl <<
                        std::setw(20) << "(samples)" << samples << endl <<
                        std::setw(20) << "(visibilities)" << visibilities << endl);
        }
      }
      catch(...) // Log all errors at this stage. DO NOT THROW IN DESTRUCTOR
      {
        LOG_ERROR_STR("Received an Exception desctructing CorrelatorPipline, while print performance");
      }
    }


    void CorrelatorPipeline::writeOutput( unsigned globalSubbandIdx, struct Output &output )
    {
      SmartPtr<Stream> outputStream = connectToOutput(globalSubbandIdx);

      SmartPtr<StreamableData> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.bequeue->remove()) != NULL) {
        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );

        LOG_DEBUG_STR("[" << id << "] Writing start");

        // Write block to disk 
        try {
          outputData->write(outputStream.get(), true);
        } catch (Exception &ex) {
          LOG_ERROR_STR("Error writing subband " << id.globalSubbandIdx << ", dropping all subsequent blocks: " << ex.what());

          outputStream = new NullStream;
        }

        SubbandProc &workQueue = *workQueues[id.localSubbandIdx % workQueues.size()];
        workQueue.outputPool.free.append(outputData);

        ASSERT(!outputData);

        if (id.localSubbandIdx == 0 || id.localSubbandIdx == subbandIndices.size() - 1)
          LOG_INFO_STR("[" << id << "] Done"); 
        else
          LOG_DEBUG_STR("[" << id << "] Done"); 
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
          outputStream = createStream(desc, false, 0);
        }
      } catch (Exception &ex) {
        LOG_ERROR_STR("Failed to connect to output proc; dropping rest of subband " << globalSubbandIdx << ": " << ex);

        outputStream = new NullStream;
      }

      return outputStream;
    }
  }
}

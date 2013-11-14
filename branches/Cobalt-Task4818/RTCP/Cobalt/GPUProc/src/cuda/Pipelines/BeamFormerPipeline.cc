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

#include <map>
#include <vector>
#include <string>
#include <iomanip>

#include "BeamFormerPipeline.h"

#include <Common/LofarLogger.h>

#include <GPUProc/SubbandProcs/BeamFormerFactories.h>
#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerPipeline::BeamFormerPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices, const std::vector<gpu::Device> &devices)
      :
      Pipeline(ps, subbandIndices, devices)
    {
      BeamFormerFactories factories(ps, nrSubbandsPerSubbandProc);

      for (size_t i = 0; i < workQueues.size(); ++i) {
        gpu::Context context(devices[i % devices.size()]);

        workQueues[i] = new BeamFormerSubbandProc(ps, context, factories, nrSubbandsPerSubbandProc);
      }

      // Set up send engine for 2nd transpose of beam-formed data
      MultiSender::HostMap hostMap;

      if (ps.settings.beamFormer.enabled) {
        // The requested service is an unique identifier for this observation,
        // and for our process.
        const string service = str(format("2nd-transpose-obs-%u-fromrank-%u")
                               % ps.settings.observationID
                               % MPI_Rank());

        // The PortBroker listen port is the same for all outputProc nodes.
        const uint16_t brokerPort = storageBrokerPort(ps.settings.observationID);

        // Create the list of all outputProcs we send data to.
        for (size_t fileIdx = 0; fileIdx < ps.settings.beamFormer.files.size(); ++fileIdx) {
          const struct ObservationSettings::BeamFormer::File &file = ps.settings.beamFormer.files[fileIdx];
          struct Host host;

          host.hostName = file.location.host;
          host.brokerPort = brokerPort;
          host.service = service;

          hostMap[fileIdx] = host;
        }
      }

      MultiSender sender(hostMap, 3, ps.realTime());
    }

    BeamFormerPipeline::~BeamFormerPipeline()
    {
      try
      { 
        // TODO: I'm not really happy with this construction: Pipeline needs to know
        // to much about the subbandProc, codesmell.
        if(gpuProfiling)
        {
        //shared bu coherent and incoherent
        RunningStatistics intToFloat;
        RunningStatistics firstFFT;
        RunningStatistics delayBp;
        RunningStatistics secondFFT;
        RunningStatistics correctBandpass;

        // Coherent stokes
        RunningStatistics beamformer;
        RunningStatistics transpose;
        RunningStatistics inverseFFT;
        RunningStatistics firFilterKernel;
        RunningStatistics finalFFT;
        RunningStatistics coherentStokes;

        //incoherent
        RunningStatistics incoherentInverseFFT;
        RunningStatistics incoherentFirFilterKernel;
        RunningStatistics incoherentFinalFFT;
        RunningStatistics incoherentStokes;

        // gpu transfer counters
        RunningStatistics samples;
        RunningStatistics visibilities;
        RunningStatistics copyBuffers;
        RunningStatistics incoherentOutput;
          for (size_t idx_queue = 0; idx_queue < workQueues.size(); ++idx_queue)
          {
            //We know we are in the correlator pipeline, this queue can only contain correlatorSubbandprocs
            BeamFormerSubbandProc *proc = dynamic_cast<BeamFormerSubbandProc *>(workQueues[idx_queue].get());

            // Print the individual counters
            proc->counters.printStats();
            
            // Calculate aggregate statistics for the whole pipeline
            intToFloat += proc->counters.intToFloat.stats;
            firstFFT += proc->counters.firstFFT.stats;
            delayBp += proc->counters.delayBp.stats;
            secondFFT += proc->counters.secondFFT.stats;
            correctBandpass += proc->counters.correctBandpass.stats;
            beamformer += proc->counters.beamformer.stats;
            transpose += proc->counters.transpose.stats;
            inverseFFT += proc->counters.inverseFFT.stats;
            firFilterKernel += proc->counters.firFilterKernel.stats;
            finalFFT += proc->counters.finalFFT.stats;
            coherentStokes += proc->counters.coherentStokes.stats;
            incoherentInverseFFT += proc->counters.incoherentInverseFFT.stats;
            incoherentFirFilterKernel += proc->counters.incoherentFirFilterKernel.stats;
            incoherentFinalFFT += proc->counters.incoherentFinalFFT.stats;
            incoherentStokes += proc->counters.incoherentStokes.stats;
          
            samples += proc->counters.samples.stats;
            visibilities += proc->counters.visibilities.stats;
            copyBuffers += proc->counters.copyBuffers.stats;
            incoherentOutput += proc->counters.incoherentOutput.stats;
          }

          // Now print the aggregate statistics.
          LOG_INFO_STR( "**** GPU runtimes for the complete BeamFormer pipeline n=" << workQueues.size() 
                       << " ****" << endl <<
                       std::setw(20) << "(intToFloat)" << intToFloat << endl <<
                       std::setw(20) << "(firstFFT)" << firstFFT << endl <<
                       std::setw(20) << "(delayBp)" << delayBp << endl <<
                       std::setw(20) << "(secondFFT)" << secondFFT << endl <<
                       std::setw(20) << "(correctBandpass)" << correctBandpass << endl <<
                       std::setw(20) << "(beamformer)" << beamformer << endl <<
                       std::setw(20) << "(transpose)" << transpose << endl <<
                       std::setw(20) << "(inverseFFT)" << inverseFFT << endl <<
                       std::setw(20) << "(firFilterKernel)" << firFilterKernel << endl <<
                       std::setw(20) << "(finalFFT)" << finalFFT << endl <<
                       std::setw(20) << "(coherentStokes)" << coherentStokes << endl <<
                       std::setw(20) << "(incoherentInverseFFT)" << incoherentInverseFFT << endl <<
                       std::setw(20) << "(incoherentFirFilterKernel)" << incoherentFirFilterKernel << endl <<
                       std::setw(20) << "(incoherentFinalFFT)" << incoherentFinalFFT << endl <<
                       std::setw(20) << "(incoherentStokes)" << incoherentStokes << endl <<
                       std::setw(20) << "(samples)" << samples << endl <<
                       std::setw(20) << "(copyBuffers)" << copyBuffers << endl <<
                       std::setw(20) << "(incoherentOutput)" << incoherentOutput << endl <<
                       std::setw(20) << "(visibilities)" << visibilities << endl);
        }
      }
      catch(...) // Log all errors at this stage. DO NOT THROW IN DESTRUCTOR
      {
        LOG_ERROR_STR("Received an Exception desctructing BeamFormerPipline, while print performance");
      }
    }


    void BeamFormerPipeline::writeOutput( unsigned globalSubbandIdx, struct Output &output )
    {
      SmartPtr<Stream> outputStream = connectToOutput(globalSubbandIdx);

      SmartPtr<StreamableData> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.bequeue->remove()) != NULL) {
        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );

        // Cast it to our output, which we know will succeed because BeamFormerSubbandProc
        // only ingests BeamFormedData into the pipeline.
        BeamFormedData *beamFormedData = dynamic_cast<BeamFormedData*>(outputData.get());

        // beamFormedData dimensions are [nrStokes][nrSamples][nrChannels]
        for (size_t stokesIdx = 0; stokesIdx < beamFormedData.shape()[0]; ++stokesIdx) {
        }

        LOG_DEBUG_STR("[" << id << "] Writing start");

        // Forward block to MultiSender
        try {
          outputData->write(outputStream.get(), true);
        } catch (Exception &ex) {
          LOG_ERROR_STR("Dropping rest of subband " << id.globalSubbandIdx << ": " << ex);

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
  }
}

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
#include <boost/format.hpp>

#include "BeamFormerPipeline.h"

#include <Common/LofarLogger.h>

#include <CoInterface/SmartPtr.h>
#include <CoInterface/Stream.h>
#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>

using boost::format;
using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {
    static TABTranspose::MultiSender::HostMap hostMap(const Parset &ps, const vector<size_t> &subbandIndices, int hostID)
    {
      TABTranspose::MultiSender::HostMap hostMap;

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
          // All files to our SAPs will be relevant
          //
          // TODO: nrSubbandsPerFile hook here
          if (file.sapNr != ps.settings.subbands[subbandIndices[i]].SAP)
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

    BeamFormerPipeline::BeamFormerPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices, const std::vector<gpu::Device> &devices, int hostID)
      :
      Pipeline(ps, subbandIndices, devices),
      factories(ps, nrSubbandsPerSubbandProc),
      multiSender(hostMap(ps, subbandIndices, hostID), 3, ps.realTime())
    {
      ASSERT(ps.settings.beamFormer.enabled);
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


    void BeamFormerPipeline::processObservation()
    {
#     pragma omp parallel sections num_threads(2)
      {
        // Let parent do work
#       pragma omp section
        {
          Pipeline::processObservation();

          // Done producing output
          multiSender.finish();
        }

        // Output processing
#       pragma omp section
        {
          multiSender.process();
        }

      }
    }

    // TODO: Needs more documentation
    void BeamFormerPipeline::writeOutput( unsigned globalSubbandIdx,
           struct Output &output )
    {
      const unsigned SAP = ps.settings.subbands[globalSubbandIdx].SAP;

      SmartPtr<SubbandProcOutputData> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.bequeue->remove()) != NULL) 
      {
        BeamFormedData &beamFormedData = dynamic_cast<BeamFormedData&>(*outputData);

        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );
        ASSERT( id.block >= 0 ); // Negative blocks should not reach storage

        LOG_DEBUG_STR("[" << id << "] Writing start");

        for (size_t fileIdx = 0;
             fileIdx < ps.settings.beamFormer.files.size();
             ++fileIdx) 
        {
          const struct ObservationSettings::BeamFormer::File &file = 
                ps.settings.beamFormer.files[fileIdx];

          // Skip what we aren't part of
          //
          // TODO: nrSubbandsPerFile hook here
          if (file.sapNr != SAP)
            continue;

          // Note that the 'file' encodes 1 Stokes of 1 TAB, so each TAB we've
          // produced can be visited 1 or 4 times.

          // Compute shape of block
          const ObservationSettings::BeamFormer::StokesSettings &stokes =
            file.coherent
            ? ps.settings.beamFormer.coherentSettings
            : ps.settings.beamFormer.incoherentSettings;

          const size_t nrChannels = stokes.nrChannels;
          const size_t nrSamples =  stokes.nrSamples(ps.settings.blockSize);

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

          subband->id.fileIdx = file.streamNr;
          subband->id.subband = ps.settings.subbands[globalSubbandIdx].idxInSAP;
          subband->id.block   = id.block;

          // Create view of subarray 
          MultiDimArray<float, 2> srcData(
              boost::extents[nrSamples][nrChannels],
              file.coherent
                   ? beamFormedData.coherentData[file.coherentIdxInSAP][file.stokesNr].origin()
                   : beamFormedData.incoherentData[file.incoherentIdxInSAP][file.stokesNr].origin(),
              false);

          // Copy data to block
          subband->data.assign(srcData.origin(), srcData.origin() + srcData.num_elements());

          // Forward block to MultiSender, who takes ownership.
          multiSender.append(subband);
          ASSERT(!subband);
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
  }
}

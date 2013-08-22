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

#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/global_defines.h>

#define NR_WORKQUEUES_PER_DEVICE  2

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerPipeline::BeamFormerPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices, const std::vector<gpu::Device> &devices)
      :
      Pipeline(ps, subbandIndices, devices)
    {

      // If profiling, use one workqueue: with >1 workqueues decreased
      // computation / I/O overlap can affect optimization gains.
      unsigned nrSubbandProcs = (profiling ? 1 : NR_WORKQUEUES_PER_DEVICE) * devices.size();
      workQueues.resize(nrSubbandProcs);

      BeamFormerFactories factories(ps);

      for (size_t i = 0; i < nrSubbandProcs; ++i) {
        gpu::Context context(devices[i % devices.size()]);

        workQueues[i] = new BeamFormerSubbandProc(ps, context, factories);
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
        // gpu kernel counters
        RunningStatistics intToFloat;
        RunningStatistics firstFFT;
        RunningStatistics delayBp;
        RunningStatistics secondFFT;
        RunningStatistics correctBandpass;
        RunningStatistics beamformer;
        RunningStatistics transpose;
        RunningStatistics inverseFFT;
        RunningStatistics firFilterKernel;
        RunningStatistics finalFFT;

        // gpu transfer counters
        RunningStatistics samples;
        RunningStatistics visibilities;
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
            
            samples += proc->counters.samples.stats;
            visibilities += proc->counters.visibilities.stats;
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
                       std::setw(20) << "(samples)" << samples << endl <<
                       std::setw(20) << "(visibilities)" << visibilities << endl);
        }
      }
      catch(...) // Log all errors at this stage. DO NOT THROW IN DESTRUCTOR
      {
        LOG_ERROR_STR("Received an Exception desctructing BeamFormerPipline, while print performance");
      }
    }
  }
}

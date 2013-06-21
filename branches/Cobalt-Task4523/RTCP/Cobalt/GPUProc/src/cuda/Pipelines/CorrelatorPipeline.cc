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

#include <Common/LofarLogger.h>

#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include "CorrelatorPipelinePrograms.h"

#define NR_WORKQUEUES_PER_DEVICE  2

namespace LOFAR
{
  namespace Cobalt
  {

    CorrelatorPipeline::CorrelatorPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices)
      :
      Pipeline(ps, subbandIndices),
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
  }
}


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

#include "BeamFormerPipeline.h"

#include <Common/LofarLogger.h>

#include <GPUProc/WorkQueues/BeamFormerWorkQueue.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>

#define NR_WORKQUEUES_PER_DEVICE  2

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerPipeline::BeamFormerPipeline(const Parset &ps, const std::vector<size_t> &subbandIndices)
      :
      Pipeline(ps, subbandIndices),
      filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER)
    {
      filterBank.negateWeights();

      // If profiling, use one workqueue: with >1 workqueues decreased
      // computation / I/O overlap can affect optimization gains.
      unsigned nrWorkQueues = (profiling ? 1 : NR_WORKQUEUES_PER_DEVICE) * devices.size();
      workQueues.resize(nrWorkQueues);

      for (size_t i = 0; i < nrWorkQueues; ++i) {
        gpu::Context context(devices[i % devices.size()]);

        workQueues[i] = new BeamFormerWorkQueue(ps, context, filterBank);
      }
    }
  }
}


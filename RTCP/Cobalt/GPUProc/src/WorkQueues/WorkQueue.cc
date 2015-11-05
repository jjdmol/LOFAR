/* Align.h
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#include <lofar_config.h>

#include "WorkQueue.h"

#include <Common/LofarLogger.h>

#include <global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    WorkQueue::WorkQueue(cl::Context &context, cl::Device &device, unsigned gpuNumber, const Parset &ps)
      :
      gpu(gpuNumber),
      device(device),
      ps(ps)
    {
#if defined __linux__ && defined USE_B7015
      set_affinity(gpu);
#endif

      queue = cl::CommandQueue(context, device, profiling ? CL_QUEUE_PROFILING_ENABLE : 0);
    }


    void WorkQueue::addCounter(const std::string &name)
    {
      counters[name] = new PerformanceCounter(name, profiling);
    }


    void WorkQueue::addTimer(const std::string &name)
    {
      timers[name] = new NSTimer(name, false, false);
    }

  }
}


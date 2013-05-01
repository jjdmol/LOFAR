//# Kernel.cc
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

#include "Kernel.h"

namespace LOFAR
{
  namespace Cobalt
  {
    Kernel::Kernel(const Parset &ps, gpu::Module& module, const string &name)
      :
      gpu::Function(module, name),
      ps(ps)
    {
    }

    void Kernel::enqueue(gpu::Stream &queue/*, PerformanceCounter &counter*/)
    {
      // AMD APP (OpenCL) complains if we submit 0-sized work
      if (globalWorkSize.x == 0)
        return;

      // TODO: to globalWorkSize in terms of localWorkSize (CUDA)
      // TODO: check assumption that this divides with no remainder
      globalWorkSize.x /= localWorkSize.x;
      globalWorkSize.y /= localWorkSize.y;
      globalWorkSize.z /= localWorkSize.z;
      const unsigned dynSharedMemBytes = 0; // our kernels do not use dyn shmem
      //queue.enqueueNDRangeKernel(*this, gpu::nullDim, globalWorkSize, localWorkSize, 0, &event);
      queue.launchKernel(*this, globalWorkSize, localWorkSize, dynSharedMemBytes);
//      counter.doOperation(event, nrOperations, nrBytesRead, nrBytesWritten);
    }

  }
}


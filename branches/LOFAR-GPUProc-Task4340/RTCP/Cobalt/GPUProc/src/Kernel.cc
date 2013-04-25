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
    Kernel(const Parset &ps, gpu::Module& module, const string &name)
      :
      gpu::Kernel(module, name),
      ps(ps)
    {
    }

    void Kernel::enqueue(gpu::CommandQueue &queue, PerformanceCounter &counter)
    {
      // AMD APP (OpenCL) complains if we submit 0-sized work
      // TODO: no need to check all dims: dim 0 cannot be 0 while other dims are non-zero (maybe enforce when creating Kernel obj).
      for (unsigned dim = 0; dim < globalWorkSize.dimensions(); dim++)
        if (globalWorkSize[dim] == 0)
          return;

      queue.enqueueNDRangeKernel(*this, gpu::dim3, globalWorkSize, localWorkSize, 0, &event);
      counter.doOperation(event, nrOperations, nrBytesRead, nrBytesWritten);
    }

  }
}


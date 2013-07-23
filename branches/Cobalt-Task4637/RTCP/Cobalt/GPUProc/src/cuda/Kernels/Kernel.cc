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

#include <ostream>
#include <boost/format.hpp>

#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/gpu_utils.h>    // for createModule()

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {
    Kernel::Parameters::Parameters(const Parset& ps) :
      nrStations(ps.nrStations()),
      nrChannelsPerSubband(ps.nrChannelsPerSubband()),
      nrSamplesPerChannel(ps.nrSamplesPerChannel()),
      nrSamplesPerSubband(ps.nrSamplesPerSubband()),
      nrPolarizations(NR_POLARIZATIONS)
    {
    }

    Kernel::Kernel(const gpu::Stream& stream, 
                   const gpu::Function& function)
      : 
      gpu::Function(function),
      event(stream.getContext()),
      itsStream(stream)
    {
    }

    void Kernel::enqueue(const gpu::Stream &queue
                         /*, PerformanceCounter &counter*/) const
    {
      // Unlike OpenCL, no need to check for 0-sized work. CUDA can handle it.
      //if (globalWorkSize.x == 0)
      //  return;

      // TODO: to globalWorkSize in terms of localWorkSize (CUDA) (+ remove assertion): add protected setThreadDim()
      gpu::Block block(localWorkSize);
      assert(globalWorkSize.x % block.x == 0 &&
             globalWorkSize.y % block.y == 0 &&
             globalWorkSize.z % block.z == 0);
      gpu::Grid grid(globalWorkSize.x / block.x,
                     globalWorkSize.y / block.y,
                     globalWorkSize.z / block.z);
      //queue.enqueueNDRangeKernel(*this, gpu::nullDim, globalWorkSize, localWorkSize, 0, &event);
      queue.launchKernel(*this, grid, block);
//      counter.doOperation(event, nrOperations, nrBytesRead, nrBytesWritten);
    }

    void Kernel::enqueue() const
    {
      enqueue(itsStream);
    }
  }
}


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
#include <cuda_runtime.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/Kernel.h>

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
      itsStream(stream),
      counter(stream.getContext())
    {}

    void Kernel::enqueue(const gpu::Stream &queue
                         /*, PerformanceCounter &counter*/) const
    {
      // TODO: to globalWorkSize in terms of localWorkSize (CUDA) (+ remove assertion): add protected setThreadDim()
      gpu::Block block(localWorkSize);
      assert(globalWorkSize.x % block.x == 0 &&
             globalWorkSize.y % block.y == 0 &&
             globalWorkSize.z % block.z == 0);

      gpu::Grid grid(globalWorkSize.x / block.x,
                     globalWorkSize.y / block.y,
                     globalWorkSize.z / block.z);

      // Perform a timed lauch of the Kernel
      queue.recordEvent(counter.start);     
      queue.launchKernel(*this, grid, block);
      queue.recordEvent(counter.stop);
      
    }

    void Kernel::logTime()
    {
      counter.logTime();
    }

    Kernel::Counter::Counter(const LOFAR::Cobalt::gpu::Context &context)
      :
    start(context),
    stop(context)
    {}


    void Kernel::Counter::logTime()
    {
      // get the difference between start and stop. push it on the stats object
      stats.push(stop.elapsedTime(start));
    }

    void Kernel::enqueue() const
    {
      enqueue(itsStream);
    }
  }
}


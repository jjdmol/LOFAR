//# Kernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_KERNEL_H
#define LOFAR_GPUPROC_CUDA_KERNEL_H

#include <string>
#include <iosfwd>
#include <cuda.h>

#include <CoInterface/Parset.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/PerformanceCounter.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class Kernel : public gpu::Function
    {
    public:
      // Parameters that must be passed to the constructor of this Kernel class.
      struct Parameters
      {
        Parameters(const Parset& ps);
        size_t nrStations;
        size_t nrChannelsPerSubband;
        size_t nrSamplesPerChannel;
        size_t nrSamplesPerSubband;
        size_t nrPolarizations;
      };

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA
      };

      // Buffers that must be passed to the constructor of this Kernel class.
      struct Buffers
      {
        Buffers(const gpu::DeviceMemory& in, 
                const gpu::DeviceMemory& out) :
          input(in), output(out)
        {}
        gpu::DeviceMemory input;
        gpu::DeviceMemory output;
      };

      void enqueue(const gpu::Stream &queue) const;

      void enqueue(const gpu::Stream &queue, PerformanceCounter &counter) const;

      void enqueue(PerformanceCounter &counter) const;

      void enqueue() const;

    protected:
      // Construct a kernel.
      Kernel(const gpu::Stream& stream, const gpu::Function& function);

      gpu::Event event;
      gpu::Stream itsStream;
      gpu::Grid globalWorkSize;
      gpu::Block localWorkSize;
      size_t nrOperations, nrBytesRead, nrBytesWritten;
    };
  }
}

#endif


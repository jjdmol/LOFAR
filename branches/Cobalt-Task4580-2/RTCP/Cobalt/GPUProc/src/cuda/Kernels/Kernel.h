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
//#include <GPUProc/PerformanceCounter.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class Kernel : public gpu::Function
    {
    public:
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

      // Parameters that must be passed to the constructor of this Kernel class.
      struct Parameters
      {
        size_t nrStations;
        size_t nrChannelsPerSubband;
        size_t nrSamplesPerChannel;
        size_t nrSamplesPerSubband;
        size_t nrPolarizations;
      };

      // Construct a kernel. The parset \a ps contains numerous parameters that
      // will be used to compile the kernel source to PTX code. Compiling to
      // PTX-code is relatively expensive and should therefore be done only
      // once. The easiest way to achieve this is to make a so-called
      // PTX-store. The actual compiler will put the PTX-code in the store,
      // where it will be cached for subsequent use. The \a context is needed to
      // create a Module form the PTX-code. The \a srcFilename contains the name
      // of the (CUDA/OpenCL) source file; \a functioName is the name of the
      // function in the module that will be loaded when the Module object is
      // constructed.
      Kernel(const Parset &ps, const gpu::Context &context,
             const std::string &srcFilename, const std::string &functionName);

      void enqueue(gpu::Stream &queue/*, PerformanceCounter &counter*/);

      // TODO: Make this a (virtual?) kernel-specific function.
      // Return required compile definitions given the Parset \a ps.
      static const CompileDefinitions& compileDefinitions(const Parset& ps);

    protected:
      // Construct a kernel.
      Kernel(const gpu::Stream& stream, const gpu::Function& function);

      // TODO: Remove once we decide we will only create a Kernel from source.
      Kernel(const Parset &ps, const gpu::Module& module, const std::string &name);

      gpu::Event event;
      // gpu::Stream itsStream;
      Parset ps;
      gpu::Grid globalWorkSize;
      gpu::Block localWorkSize;
      size_t nrOperations, nrBytesRead, nrBytesWritten;
    };
  }
}

#endif


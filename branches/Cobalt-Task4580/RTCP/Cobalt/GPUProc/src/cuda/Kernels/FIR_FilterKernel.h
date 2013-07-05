//# FIR_FilterKernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_FIR_FILTER_KERNEL_H
#define LOFAR_GPUPROC_CUDA_FIR_FILTER_KERNEL_H

#include <CoInterface/Parset.h>

#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/KernelFactory.h>
#include <GPUProc/FilterBank.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class FIR_FilterKernel : public Kernel
    {
    public:
      // Generic parameter structure that contains all the information to
      // construct a FIR_FilterKernel object.
      struct Parameters : Kernel::Parameters
      {
        Parameters(const Kernel::Parameters& kernelParams, 
                   const gpu::DeviceMemory & firWeights) :
          Kernel::Parameters(kernelParams), firWeights(firWeights)
        {}
        gpu::DeviceMemory firWeights;
      };

      FIR_FilterKernel(const Parameters& ps);

      // FIR_FilterKernel(const Parset &ps,
      //                  gpu::Context &context,
      //                  gpu::DeviceMemory &devFilteredData,
      //                  gpu::DeviceMemory &devInputSamples,
      //                  gpu::Stream &stream);

      FIR_FilterKernel(const Parset &ps,
                       const gpu::Module &module,
                       gpu::DeviceMemory &devFilteredData,
                       gpu::DeviceMemory &devInputSamples,
                       gpu::Stream &stream);

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA,
        FILTER_WEIGHTS
      };

      // Return required buffer size for \a bufferType
      static size_t bufferSize(const Parset& ps, BufferType bufferType);

    private:
      void init(gpu::DeviceMemory &devFilteredData,
                gpu::DeviceMemory &devInputSamples,
                gpu::Stream &stream);

      gpu::DeviceMemory devFIRweights;

    };

#if 0
    // Template specialization of the KernelFactory::create() method.
    template<> inline
    FIR_FilterKernel* KernelFactory<FIR_FilterKernel>::
    create(const gpu::Context &context,
           const gpu::DeviceMemory &input,
           const gpu::DeviceMemory &output) const
    {
      gpu::DeviceMemory coeff(context, size_t(1));
      gpu::Function func;
      FIR_FilterKernel::Parameters
        params(Kernel::Parameters(context, func, input, output), coeff);
      return new FIR_FilterKernel(params);
    }
#endif

  }
}

#endif


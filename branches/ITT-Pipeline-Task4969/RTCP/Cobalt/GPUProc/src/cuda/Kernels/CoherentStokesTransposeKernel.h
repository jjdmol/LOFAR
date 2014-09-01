//# CoherentStokesTransposeKernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_COHERENT_STOKES_TRANSPOSE_KERNEL_H
#define LOFAR_GPUPROC_CUDA_COHERENT_STOKES_TRANSPOSE_KERNEL_H

#include <CoInterface/Parset.h>

#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/KernelFactory.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class CoherentStokesTransposeKernel : public CompiledKernel
    {
    public:
      static std::string theirSourceFile;
      static std::string theirFunction;

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA
      };

      // Parameters that must be passed to the constructor of the
      // BeamFormerKernel class.
      struct Parameters : Kernel::Parameters
      {
        Parameters(const Parset& ps);
        unsigned nrChannels;
        unsigned nrSamplesPerChannel;

        unsigned nrTABs;

        size_t bufferSize(BufferType bufferType) const;
      };

      CoherentStokesTransposeKernel(const gpu::Stream &stream,
                             const gpu::Module &module,
                             const Buffers &buffers,
                             const Parameters &param);

    };

    //# --------  Template specializations for KernelFactory  -------- #//

    template<> CompileDefinitions
    KernelFactory<CoherentStokesTransposeKernel>::compileDefinitions() const;

  }
}

#endif


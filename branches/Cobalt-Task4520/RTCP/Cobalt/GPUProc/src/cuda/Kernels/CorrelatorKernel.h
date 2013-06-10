//# CorrelatorKernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_CORRELATOR_KERNEL_H
#define LOFAR_GPUPROC_CUDA_CORRELATOR_KERNEL_H

#include <CoInterface/Parset.h>

#include "Kernel.h"
#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
#if !defined USE_NEW_CORRELATOR

    class CorrelatorKernel : public Kernel
    {
    public:
      CorrelatorKernel(const Parset &ps, 
                       gpu::Module &program, 
                       gpu::DeviceMemory &devVisibilities, 
                       gpu::DeviceMemory &devCorrectedData);

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA
      };

      // Return required buffer size for \a bufferType
      static size_t bufferSize(const Parset& ps, BufferType bufferType);

    };

#else

    class CorrelatorKernel : public Kernel
    {
    public:
      CorrelatorKernel(const Parset &ps, 
                       gpu::Module &program,
                       gpu::DeviceMemory &devVisibilities,
                       gpu::DeviceMemory &devCorrectedData);

    };

    class CorrelateRectangleKernel : public Kernel
    {
    public:
      CorrelateRectangleKernel(const Parset &ps,
                               gpu::Module &program,
                               gpu::DeviceMemory &devVisibilities,
                               gpu::DeviceMemory &devCorrectedData);
    };

    class CorrelateTriangleKernel : public Kernel
    {
    public:
      CorrelateTriangleKernel(const Parset &ps, 
                              gpu::Module &program,
                              gpu::DeviceMemory &devVisibilities,
                              gpu::DeviceMemory &devCorrectedData);
    };

#endif

  }
}

#endif


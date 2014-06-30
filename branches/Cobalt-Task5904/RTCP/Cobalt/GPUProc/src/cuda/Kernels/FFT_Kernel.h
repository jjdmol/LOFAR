//# FFT_Kernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_FFT_KERNEL_H
#define LOFAR_GPUPROC_CUDA_FFT_KERNEL_H

#include <GPUProc/gpu_wrapper.h>
#include "FFT_Plan.h"
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/KernelFactory.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class FFT_Kernel: public Kernel
    {
    public:
      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA
      };

      // Parameters that must be passed to the constructor of the
      // BandPassCorrectionKernel class.
      struct Parameters : Kernel::Parameters
      {
        Parameters(unsigned fftSize, unsigned nrSamples, bool forward);

        unsigned fftSize;
        unsigned nrSamples;
        bool forward;

        size_t bufferSize(FFT_Kernel::BufferType bufferType) const;
      };

      FFT_Kernel(const gpu::Stream &stream,
                 const Buffers& buffers,
                 const Parameters& params);

    protected:
      void launch() const;

    private:
      const unsigned nrFFTs, nrMajorFFTs, nrMinorFFTs;
      const int direction;
      FFT_Plan planMajor, planMinor;

      void executePlan(const cufftHandle &plan, cufftComplex *in_data, cufftComplex *out_data) const;
    };

    //# --------  Template specializations for KernelFactory  -------- #//

    // The default KernelFactory tries to compile a source,
    // but FFT_Kernel has nothing to compile, so we implement short cuts.
    template<> std::string KernelFactory<FFT_Kernel>::_createPTX() const;
    template<> FFT_Kernel* KernelFactory<FFT_Kernel>::create(
              const gpu::Stream& stream,
              gpu::DeviceMemory &inputBuffer,
              gpu::DeviceMemory &outputBuffer) const;
  }
}
#endif

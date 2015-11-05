//# FFT_Plan.cc
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

#include "FFT_Plan.h"
#include <GPUProc/gpu_wrapper.h>

#if 0
// CUDA 5.5RC crashes if cuFFT is linked in but not used,
// so we provide a workaround by always using cuFFT.
//
// CUDA 5.5 final release fixes this issue, so this code will
// soon be dead.
#if CUDA_VERSION == 5050
namespace {
  int use_cuFFT() {
      cufftHandle plan;
      cufftResult error;

      error = cufftPlan1d(&plan, 256, CUFFT_C2C, 1);

      if (error == CUFFT_SUCCESS)
        cufftDestroy(plan);

      return 0;
  }
};

static int __using_cuFFT = use_cuFFT();
#endif
#endif

namespace LOFAR
{
  namespace Cobalt
  {
    FFT_Plan::FFT_Plan(gpu::Context &context, unsigned fftSize, unsigned nrFFTs)
      :
      context(context)
    {
      gpu::ScopedCurrentContext scc(context);

      cufftResult error;

      error = cufftPlan1d(&plan, fftSize, CUFFT_C2C, nrFFTs);

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftPlan1d: " << gpu::cufftErrorMessage(error));
    }

    FFT_Plan::~FFT_Plan()
    {
      gpu::ScopedCurrentContext scc(context);

      cufftDestroy(plan);
    }

    void FFT_Plan::setStream(gpu::Stream &stream)
    {
      cufftResult error;

      gpu::ScopedCurrentContext scc(context);

      error = cufftSetStream(plan, stream.get());

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftSetStream: " << gpu::cufftErrorMessage(error));
    }
  }
}


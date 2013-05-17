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

#include "lofar_config.h"

#include "FFT_Plan.h"

namespace LOFAR
{
  namespace Cobalt
  {
    FFT_Plan::FFT_Plan(unsigned fftSize, unsigned nrFFTs)
    {
      cufftResult error;

      error = cufftPlan1d(&plan, fftSize, CUFFT_C2C, nrFFTs);

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftPlan1d: " << gpu::cufftErrorMessage(error));
    }

    FFT_Plan::~FFT_Plan()
    {
      cufftDestroy(plan);
    }

    void FFT_Plan::setStream(gpu::Stream &stream)
    {
      cufftResult error;

      error = cufftSetStream(plan, stream.stream());

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftSetStream: " << gpu::cufftErrorMessage(error));
    }
  }
}


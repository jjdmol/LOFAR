//# FFT_Kernel.cc
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

#include <vector>
#include <cufft.h>

#include <Common/LofarLogger.h>
#include <GPUProc/global_defines.h>
#include "FFT_Kernel.h"

namespace LOFAR
{
  namespace Cobalt
  {

    FFT_Kernel::FFT_Kernel(gpu::Context &context, unsigned fftSize, unsigned nrFFTs, bool forward, gpu::DeviceMemory &buffer)
      :
      context(context),
      nrFFTs(nrFFTs),
      fftSize(fftSize),
      direction(forward ? CUFFT_FORWARD : CUFFT_INVERSE),
      plan(context, fftSize, nrFFTs),
      buffer(buffer)
    {
    }

    void FFT_Kernel::enqueue(gpu::Stream &stream/*, PerformanceCounter &counter*/)
    {
      gpu::ScopedCurrentContext scc(context);

      cufftResult error;

      // Tie our plan to the specified stream
      plan.setStream(stream);

      LOG_DEBUG("Launching cuFFT");

      // Enqueue the FFT execution
      error = cufftExecC2C(plan.plan,
                           static_cast<cufftComplex*>(buffer.get()),
                           static_cast<cufftComplex*>(buffer.get()),
                           direction);

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftExecC2C: " << gpu::cufftErrorMessage(error));

      if (stream.isSynchronous()) {
        stream.synchronize();
      }

/*
      counter.doOperation(event,
                          (size_t) nrFFTs * 5 * fftSize * log2(fftSize),
                          (size_t) nrFFTs * fftSize * sizeof(std::complex<float>),
                          (size_t) nrFFTs * fftSize * sizeof(std::complex<float>));*/
    }

    size_t FFT_Kernel::bufferSize(const Parset& ps, BufferType bufferType)
    {
      switch (bufferType) {
      case INPUT_DATA: 
      case OUTPUT_DATA:
        return
          ps.nrStations() * NR_POLARIZATIONS * 
          ps.nrSamplesPerSubband() * sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

  }
}


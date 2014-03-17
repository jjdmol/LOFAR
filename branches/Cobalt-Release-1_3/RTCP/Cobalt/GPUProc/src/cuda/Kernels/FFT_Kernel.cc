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
#include <CoInterface/BlockID.h>

#include "FFT_Kernel.h"

namespace LOFAR
{
  namespace Cobalt
  {

    /*
     * cuFFT uses a plan for each FFT, tied to a (context, fftSize, nrFFTs)
     * tuple. The plan is allocated on the GPU, with a size equal to the
     * size of the input (!).
     *
     * Since we need to span a lot of points with small FFTs, we use the
     * following strategy:
     *
     *  1) Repeatedly FFT 1M points (maxNrFFTpoints).
     *  2) FFT the rest.
     *
     * This value maxNrFFTpoints balances between:
     *   - a small number of points creates a smaller plan (8 bytes/point)
     *   - a big number of points creates more parallellism
     *
     * Note that fftSize == 1 will result in a no-op in cuFFT.
     */
    const size_t maxNrFFTpoints = 1024 * 1024;

    FFT_Kernel::FFT_Kernel(const gpu::Stream &stream, unsigned fftSize,
                           unsigned nrFFTs, bool forward, 
                           const gpu::DeviceMemory &buffer)
      :
      itsCounter(stream.getContext()),
      context(stream.getContext()),
      nrMajorFFTs(nrFFTs / (maxNrFFTpoints / fftSize)),
      nrMinorFFTs(nrFFTs % (maxNrFFTpoints / fftSize)),
      fftSize(fftSize),
      direction(forward ? CUFFT_FORWARD : CUFFT_INVERSE),
      planMajor(context, fftSize, maxNrFFTpoints / fftSize),
      planMinor(context, fftSize, nrMinorFFTs),
      buffer(buffer),
      itsStream(stream)
    {
      // fftSize must fit into maxNrFFTpoints an exact number of times
      ASSERT(maxNrFFTpoints % fftSize == 0);

      LOG_DEBUG_STR("FFT_Kernel: " <<
                    "fftSize=" << fftSize << 
                    ", direction=" << (forward ? "forward" : "inverse") <<
                    ", nrFFTs=" << nrFFTs);
    }

    void FFT_Kernel::executePlan(const cufftHandle &plan, cufftComplex *data) const
    {
      cufftResult error = cufftExecC2C(plan, data, data, direction);

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftExecC2C: " << gpu::cufftErrorMessage(error));
    }


    void FFT_Kernel::enqueue(const BlockID &/*blockId*/) const
    {
      gpu::ScopedCurrentContext scc(context);

      // Tie our plan to the specified stream
      planMajor.setStream(itsStream);
      planMinor.setStream(itsStream);

      // Enqueue the FFT execution
      itsStream.recordEvent(itsCounter.start);
      LOG_DEBUG("Launching cuFFT");

      // Running pointer of the GPU data we're about to FFT
      cufftComplex *data = static_cast<cufftComplex*>(buffer.get());

      // Execute the major FFTs
      for (size_t major = 0; major < nrMajorFFTs; ++major) {
        executePlan(planMajor.plan, data);
        data += maxNrFFTpoints;
      }

      // Execute the minor FFT
      executePlan(planMinor.plan, data);

      // Wrap up
      itsStream.recordEvent(itsCounter.stop);

      if (itsStream.isSynchronous()) {
        itsStream.synchronize();
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
          (size_t) ps.nrStations() * NR_POLARIZATIONS * 
            ps.nrSamplesPerSubband() * sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }
  }
}


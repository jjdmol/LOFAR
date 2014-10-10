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
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>

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

    FFT_Kernel::Parameters::Parameters(unsigned fftSize, unsigned nrSamples, bool forward, const std::string &name)
    :
      Kernel::Parameters(name),
      fftSize(fftSize),
      nrSamples(nrSamples),
      forward(forward)
    {
    }

    size_t FFT_Kernel::Parameters::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case FFT_Kernel::INPUT_DATA: 
      case FFT_Kernel::OUTPUT_DATA:
        return (size_t) nrSamples * sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    FFT_Kernel::FFT_Kernel(const gpu::Stream &stream,
                           const Buffers& buffers,
                           const Parameters& params)
      :
      Kernel(stream, buffers, params),
      nrFFTs(params.nrSamples / params.fftSize),
      nrMajorFFTs(nrFFTs / (maxNrFFTpoints / params.fftSize)),
      nrMinorFFTs(nrFFTs % (maxNrFFTpoints / params.fftSize)),
      direction(params.forward ? CUFFT_FORWARD : CUFFT_INVERSE),
      planMajor(stream.getContext(), params.fftSize, maxNrFFTpoints / params.fftSize),
      planMinor(stream.getContext(), params.fftSize, nrMinorFFTs)
    {
      // fftSize must fit into nrSamples an exact number of times
      ASSERT(params.nrSamples % params.fftSize == 0);

      // fftSize must fit into maxNrFFTpoints an exact number of times
      ASSERT(maxNrFFTpoints % params.fftSize == 0);

      // Tie our plan to the specified stream
      planMajor.setStream(itsStream);
      planMinor.setStream(itsStream);

      // buffer must be big enough for the job
      // Untill we have optional kernel compilation this test will fail on unused and thus incorrect kernels
      //ASSERT(buffer.size() >= fftSize * nrFFTs * sizeof(fcomplex));

      LOG_DEBUG_STR("FFT_Kernel: " <<
                    "fftSize=" << params.fftSize << 
                    ", direction=" << (params.forward ? "forward" : "inverse") <<
                    ", nrFFTs=" << nrFFTs);
    }

    void FFT_Kernel::executePlan(const cufftHandle &plan, cufftComplex *in_data, cufftComplex *out_data) const
    {
      cufftResult error = cufftExecC2C(plan, in_data, out_data, direction);

      if (error != CUFFT_SUCCESS)
        THROW(gpu::CUDAException, "cufftExecC2C: " << gpu::cufftErrorMessage(error));
    }


    void FFT_Kernel::launch() const
    {
      gpu::ScopedCurrentContext scc(itsStream.getContext());

      // Enqueue the FFT execution
      LOG_DEBUG("Launching cuFFT");

      // Running pointer of the GPU data we're about to FFT
      cufftComplex *in_data  = static_cast<cufftComplex*>(itsBuffers.input.get());
      cufftComplex *out_data = static_cast<cufftComplex*>(itsBuffers.output.get());

      // Execute the major FFTs
      for (size_t major = 0; major < nrMajorFFTs; ++major) {
        executePlan(planMajor.plan, in_data, out_data);
        in_data  += maxNrFFTpoints;
        out_data += maxNrFFTpoints;
      }

      // Execute the minor FFT
      if (nrMinorFFTs > 0) {
        executePlan(planMinor.plan, in_data, out_data);
      }
    }

    //# --------  Template specializations for KernelFactory  -------- #//

    template<> std::string KernelFactory<FFT_Kernel>::_createPTX() const {
      return "";
    }

    template <> FFT_Kernel* KernelFactory<FFT_Kernel>::create(
              const gpu::Stream& stream,
              gpu::DeviceMemory &inputBuffer,
              gpu::DeviceMemory &outputBuffer) const
    {
      const FFT_Kernel::Buffers buffers(inputBuffer, outputBuffer);

      return new FFT_Kernel(stream, buffers, itsParameters);
    }
  }
}


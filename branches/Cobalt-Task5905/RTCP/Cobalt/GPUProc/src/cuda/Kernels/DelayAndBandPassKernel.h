//# DelayAndBandPassKernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_DELAY_AND_BAND_PASS_KERNEL_H
#define LOFAR_GPUPROC_CUDA_DELAY_AND_BAND_PASS_KERNEL_H

#include <CoInterface/Parset.h>

#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/KernelFactory.h>
#include <GPUProc/gpu_wrapper.h>
//#include <GPUProc/PerformanceCounter.h>

namespace LOFAR
{
  namespace Cobalt
  {

    class DelayAndBandPassKernel : public CompiledKernel
    {
    public:
      static std::string theirSourceFile;
      static std::string theirFunction;

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA,
        DELAYS,
        PHASE_ZEROS,
        BAND_PASS_CORRECTION_WEIGHTS
      };

      // Parameters that must be passed to the constructor of the
      // DelayAndBandPassKernel class.
      struct Parameters : Kernel::Parameters
      {
        Parameters(const Parset& ps, bool correlator);
        unsigned nrStations;
        unsigned nrBitsPerSample;
        bool inputIsStationData;

        unsigned nrChannels;
        unsigned nrSamplesPerChannel;
        double subbandBandwidth;

        unsigned nrSAPs;

        bool delayCompensation;
        bool correctBandPass;
        bool transpose;

        unsigned nrSamplesPerSubband() const;
        unsigned nrBytesPerComplexSample() const;

        size_t bufferSize(BufferType bufferType) const;
      };

      DelayAndBandPassKernel(const gpu::Stream &stream,
                             const gpu::Module &module,
                             const Buffers &buffers,
                             const Parameters &param);


      void enqueue(const BlockID &blockId, 
                   double subbandFrequency, unsigned SAP);

      // Input parameters for the delay compensation
      gpu::DeviceMemory delaysAtBegin;
      gpu::DeviceMemory delaysAfterEnd;
      gpu::DeviceMemory phase0s;

    private:
      // The weights to correct the bandpass with, per channel
      gpu::DeviceMemory bandPassCorrectionWeights;
    };

    //# --------  Template specializations for KernelFactory  -------- #//

    template<> CompileDefinitions
    KernelFactory<DelayAndBandPassKernel>::compileDefinitions() const;
  }
}

#endif


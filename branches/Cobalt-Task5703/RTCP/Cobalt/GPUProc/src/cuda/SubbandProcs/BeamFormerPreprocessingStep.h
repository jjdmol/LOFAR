//# BeamFormerPreprocessingStep.h
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

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_PREPROCESSING_STEP_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_PREPROCESSING_STEP_H

#include <complex>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <boost/shared_ptr.hpp>
#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>

#include "SubbandProc.h"
#include "BeamFormerSubbandProcStep.h"

#include <GPUProc/Kernels/BandPassCorrectionKernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/FFTShiftKernel.h>
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/IntToFloatKernel.h>


namespace LOFAR
{
  namespace Cobalt
  {

    //# Forward declarations
    struct BeamFormerFactories;


    class BeamFormerPreprocessingStep: public BeamFormerSubbandProcStep
    {
    public:

      BeamFormerPreprocessingStep(const Parset &parset, 
        gpu::Stream &i_queue,
        boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
        boost::shared_ptr<gpu::DeviceMemory> i_devA,
        boost::shared_ptr<gpu::DeviceMemory> i_devB,
        boost::shared_ptr<gpu::DeviceMemory> i_devNull);

      BeamFormerPreprocessingStep(const Parset &parset,
        gpu::Stream &i_queue, 
        gpu::Context &context,
        BeamFormerFactories &factories,
        boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
        boost::shared_ptr<gpu::DeviceMemory> i_devA,
        boost::shared_ptr<gpu::DeviceMemory> i_devB,
        boost::shared_ptr<gpu::DeviceMemory> i_devNull);

      void initMembers(gpu::Context &context,
        BeamFormerFactories &factories);

      void process(BlockID blockID,
        unsigned subband);

      void printStats();

      void logTime();

      ~BeamFormerPreprocessingStep();

    private:

      //Data members
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> devInput;
      boost::shared_ptr<gpu::DeviceMemory> devA;
      boost::shared_ptr<gpu::DeviceMemory> devB;
      boost::shared_ptr<gpu::DeviceMemory> devNull;

      // Int -> Float conversion
      std::auto_ptr<IntToFloatKernel::Buffers> intToFloatBuffers;
      std::auto_ptr<IntToFloatKernel> intToFloatKernel;

      // First FFT-shift
      std::auto_ptr<FFTShiftKernel::Buffers> firstFFTShiftBuffers;
      std::auto_ptr<FFTShiftKernel> firstFFTShiftKernel;

      // First (64 points) FFT
      std::auto_ptr<FFT_Kernel> firstFFT;

      // Delay compensation
      std::auto_ptr<DelayAndBandPassKernel::Buffers> delayCompensationBuffers;
      std::auto_ptr<DelayAndBandPassKernel> delayCompensationKernel;

      // Second FFT-shift
      std::auto_ptr<FFTShiftKernel::Buffers> secondFFTShiftBuffers;
      std::auto_ptr<FFTShiftKernel> secondFFTShiftKernel;

      // Second (64 points) FFT
      std::auto_ptr<FFT_Kernel> secondFFT;

      // Bandpass correction and tranpose
      std::auto_ptr<gpu::DeviceMemory> devBandPassCorrectionWeights;
      std::auto_ptr<BandPassCorrectionKernel::Buffers> bandPassCorrectionBuffers;
      std::auto_ptr<BandPassCorrectionKernel> bandPassCorrectionKernel;

      // Flag that indicates if we need to perform a second FFT
      bool doSecondFFT;
    };
  }
}

#endif

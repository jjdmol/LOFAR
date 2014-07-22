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

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_COHERENT_STEP_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_COHERENT_STEP_H


#include <complex>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <boost/shared_ptr.hpp>
#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>

#include "SubbandProc.h"
#include "ProcessStep.h"

#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <GPUProc/Kernels/CoherentStokesTransposeKernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/FFTShiftKernel.h>
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/CoherentStokesKernel.h>

namespace LOFAR
{
  namespace Cobalt
  {
    //# Forward declarations
    struct BeamFormerFactories;
    class BeamFormedData;

    class BeamFormerCoherentStep: public ProcessStep
    {
    public:
      struct Factories
      {
        Factories(const Parset &ps, size_t nrSubbandsPerSubbandProc = 1);

        KernelFactory<BeamFormerKernel> beamFormer;
        KernelFactory<CoherentStokesTransposeKernel> coherentTranspose;
        KernelFactory<FFT_Kernel> coherentInverseFFT;
        KernelFactory<FFTShiftKernel> coherentInverseFFTShift;
        SmartPtr< KernelFactory<FIR_FilterKernel> > coherentFirFilter;
        SmartPtr< KernelFactory<FFT_Kernel> > coherentFinalFFT;
        KernelFactory<CoherentStokesKernel> coherentStokes;
      };

      BeamFormerCoherentStep(const Parset &parset,
        gpu::Stream &i_queue,
        gpu::Context &context,
        Factories &factories,
        boost::shared_ptr<gpu::DeviceMemory> i_devB);

      void initMembers(gpu::Context &context,
        Factories &factories);

      gpu::DeviceMemory outputBuffer();

      void writeInput(const SubbandProcInputData &input);

      void process(const SubbandProcInputData &input);

      void readOutput(BeamFormedData &output);

    private:

      const bool coherentStokesPPF;

      // Data members
      boost::shared_ptr<gpu::DeviceMemory> devB;
      gpu::DeviceMemory devC;
      gpu::DeviceMemory devD;

      // Kernel members
      std::auto_ptr<BeamFormerKernel> beamFormerKernel;

      // Transpose 
      std::auto_ptr<CoherentStokesTransposeKernel> coherentTransposeKernel;

      // inverse (4k points) FFT
      std::auto_ptr<FFT_Kernel> inverseFFT;

      // inverse FFT-shift
      std::auto_ptr<FFTShiftKernel> inverseFFTShiftKernel;

      // Poly-phase filter (FIR + FFT)
      std::auto_ptr<FIR_FilterKernel> firFilterKernel;
      std::auto_ptr<FFT_Kernel> coherentFinalFFT;

      // Coherent Stokes
      std::auto_ptr<CoherentStokesKernel> coherentStokesKernel;

      PerformanceCounter outputCounter;

      size_t nrCoherent(const BlockID &blockID) const;
    };


  }
}

#endif



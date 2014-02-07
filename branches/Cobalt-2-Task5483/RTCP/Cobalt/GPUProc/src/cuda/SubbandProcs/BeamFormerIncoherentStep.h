//# BeamFormerIncoherentStep.h
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

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_INCOHERENT_STEP_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_INCOHERENT_STEP_H


#include <complex>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <boost/shared_ptr.hpp>
#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>

#include "SubbandProc.h"
#include "BeamFormerSubbandProcStep.h"

#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/FFTShiftKernel.h>
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>


#include <GPUProc/Kernels/FFTShiftKernel.h>
#include <GPUProc/Kernels/IncoherentStokesKernel.h>
#include <GPUProc/Kernels/IncoherentStokesTransposeKernel.h>




namespace LOFAR
{
  namespace Cobalt
  {
    //# Forward declarations
    struct BeamFormerFactories;

    class BeamFormerIncoherentStep : public BeamFormerSubbandProcStep
    {
    public:
      BeamFormerIncoherentStep(const Parset &parset,
        gpu::Stream &i_queue,
        boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
        boost::shared_ptr<gpu::DeviceMemory> i_devA,
        boost::shared_ptr<gpu::DeviceMemory> i_devB,
        boost::shared_ptr<gpu::DeviceMemory> i_devC,
        boost::shared_ptr<gpu::DeviceMemory> i_devD,
        boost::shared_ptr<gpu::DeviceMemory> i_devE,
        boost::shared_ptr<gpu::DeviceMemory> i_devNull);

      void initMembers(gpu::Context &context,
        BeamFormerFactories &factories);

      void process(BlockID blockID,
        unsigned subband);

      void printStats();

      void logTime();

      ~BeamFormerIncoherentStep();

    private:

      bool outputComplexVoltages;
      bool coherentStokesPPF;

      // Data members
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> devInput;
      boost::shared_ptr<gpu::DeviceMemory> devA;
      boost::shared_ptr<gpu::DeviceMemory> devB;
      boost::shared_ptr<gpu::DeviceMemory> devC;
      boost::shared_ptr<gpu::DeviceMemory> devD;
      boost::shared_ptr<gpu::DeviceMemory> devE;
      boost::shared_ptr<gpu::DeviceMemory> devNull;



      // *****************************************************************
      //  Objects needed to produce incoherent stokes output
      bool incoherentStokesPPF;

      // Transpose 
      std::auto_ptr<IncoherentStokesTransposeKernel::Buffers> incoherentTransposeBuffers;
      std::auto_ptr<IncoherentStokesTransposeKernel> incoherentTranspose;

      // Inverse (4k points) FFT
      std::auto_ptr<FFT_Kernel> incoherentInverseFFT;

      // Inverse FFT-shift
      std::auto_ptr<FFTShiftKernel::Buffers> incoherentInverseFFTShiftBuffers;
      std::auto_ptr<FFTShiftKernel> incoherentInverseFFTShiftKernel;

      // Poly-phase filter (FIR + FFT)
      std::auto_ptr<gpu::DeviceMemory> devIncoherentFilterWeights;
      std::auto_ptr<gpu::DeviceMemory> devIncoherentFilterHistoryData;

      std::auto_ptr<FIR_FilterKernel::Buffers> incoherentFirFilterBuffers;
      std::auto_ptr<FIR_FilterKernel> incoherentFirFilterKernel;
      std::auto_ptr<FFT_Kernel> incoherentFinalFFT;

      // Incoherent Stokes
      std::auto_ptr<IncoherentStokesKernel::Buffers> incoherentStokesBuffers;
      std::auto_ptr<IncoherentStokesKernel> incoherentStokesKernel;

      bool coherentBeamformer; // TODO temporary hack to allow typing of subband proc
    };
  }
}

#endif




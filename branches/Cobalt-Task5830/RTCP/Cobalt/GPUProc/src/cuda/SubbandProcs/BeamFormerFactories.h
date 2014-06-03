//# BeamFormerFactories.h
//#
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

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_FACTORIES_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_FACTORIES_H

#include <GPUProc/KernelFactory.h>
#include <GPUProc/Kernels/BandPassCorrectionKernel.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <GPUProc/Kernels/CoherentStokesTransposeKernel.h>
#include <GPUProc/Kernels/CoherentStokesKernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/FFTShiftKernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/IntToFloatKernel.h>
#include <GPUProc/Kernels/IncoherentStokesKernel.h>
#include <GPUProc/Kernels/IncoherentStokesTransposeKernel.h>

namespace LOFAR
{
  namespace Cobalt
  {
    //# Forward declarations
    class Parset;

    struct BeamFormerFactories
    {
      BeamFormerFactories(const Parset &ps, 
                            size_t nrSubbandsPerSubbandProc = 1);

      KernelFactory<IntToFloatKernel> intToFloat;
      KernelFactory<FFTShiftKernel> fftShift;
      KernelFactory<DelayAndBandPassKernel> delayCompensation;
      KernelFactory<BandPassCorrectionKernel> bandPassCorrection;

      KernelFactory<BeamFormerKernel> beamFormer;
      KernelFactory<CoherentStokesTransposeKernel> coherentTranspose;
      KernelFactory<FFTShiftKernel> coherentInverseFFTShift;
      KernelFactory<FIR_FilterKernel> coherentFirFilter;
      KernelFactory<CoherentStokesKernel> coherentStokes;

      KernelFactory<IncoherentStokesTransposeKernel> incoherentStokesTranspose;
      KernelFactory<FFTShiftKernel> incoherentInverseFFTShift;
      KernelFactory<FIR_FilterKernel> incoherentFirFilter;
      KernelFactory<IncoherentStokesKernel> incoherentStokes;

      static BandPassCorrectionKernel::Parameters
      bandPassCorrectionParams(const Parset &ps);

      static BeamFormerKernel::Parameters
      beamFormerParams(const Parset &ps);

      static CoherentStokesTransposeKernel::Parameters
      coherentTransposeParams(const Parset &ps);

      static CoherentStokesKernel::Parameters
      coherentStokesParams(const Parset &ps);

      static DelayAndBandPassKernel::Parameters
      delayCompensationParams(const Parset &ps);

      static FFTShiftKernel::Parameters
      fftShiftParams(const Parset &ps);

      static FFTShiftKernel::Parameters
      coherentInverseFFTShiftParams(const Parset &ps);

      static FIR_FilterKernel::Parameters
      coherentFirFilterParams(const Parset &ps, 
                              size_t nrSubbandsPerSubbandProc);

      static FIR_FilterKernel::Parameters
      incoherentFirFilterParams(const Parset &ps,
                                size_t nrSubbandsPerSubbandProc);

      static FFTShiftKernel::Parameters
      incoherentInverseFFTShiftParams(const Parset &ps);

      static IncoherentStokesKernel::Parameters 
      incoherentStokesParams(const Parset &ps);

      static IncoherentStokesTransposeKernel::Parameters
      incoherentStokesTransposeParams(const Parset &ps);


    };

  }
}

#endif

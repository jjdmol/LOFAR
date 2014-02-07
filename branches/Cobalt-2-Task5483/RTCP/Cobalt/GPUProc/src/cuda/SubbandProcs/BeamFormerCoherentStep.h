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

#include <complex>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <boost/shared_ptr.hpp>
#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>

#include "SubbandProc.h"
#include "BeamFormerSubbandProcStep.h"

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

    class BeamFormerCoherentStep: public BeamFormerSubbandProcStep
    {
    public:
      BeamFormerCoherentStep(const Parset &parset,
        gpu::Stream &i_queue,
        boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
        boost::shared_ptr<gpu::DeviceMemory> i_devA,
        boost::shared_ptr<gpu::DeviceMemory> i_devB,
        boost::shared_ptr<gpu::DeviceMemory> i_devC,
        boost::shared_ptr<gpu::DeviceMemory> i_devD,
        boost::shared_ptr<gpu::DeviceMemory> i_devNull);


      void initMembers(gpu::Context &context,
        BeamFormerFactories &factories);

      void process(BlockID blockID,
        unsigned subband);

      void printStats();

      void logTime();

      ~BeamFormerCoherentStep();

    private:

      std::auto_ptr<gpu::DeviceMemory> devBeamFormerDelays;
      std::auto_ptr<BeamFormerKernel::Buffers> beamFormerBuffers;
      std::auto_ptr<BeamFormerKernel> beamFormerKernel;

      // Transpose 
      std::auto_ptr<CoherentStokesTransposeKernel::Buffers> coherentTransposeBuffers;
      std::auto_ptr<CoherentStokesTransposeKernel> coherentTransposeKernel;

      // inverse (4k points) FFT
      std::auto_ptr<FFT_Kernel> inverseFFT;

      // inverse FFT-shift
      std::auto_ptr<FFTShiftKernel::Buffers> inverseFFTShiftBuffers;
      std::auto_ptr<FFTShiftKernel> inverseFFTShiftKernel;

      // Poly-phase filter (FIR + FFT)
      std::auto_ptr<gpu::DeviceMemory> devFilterWeights;
      std::auto_ptr<gpu::DeviceMemory> devFilterHistoryData;
      std::auto_ptr<FIR_FilterKernel::Buffers> firFilterBuffers;
      std::auto_ptr<FIR_FilterKernel> firFilterKernel;
      std::auto_ptr<FFT_Kernel>finalFFT;

      // Coherent Stokes
      std::auto_ptr<CoherentStokesKernel::Buffers> coherentStokesBuffers;
      std::auto_ptr<CoherentStokesKernel> coherentStokesKernel;




    };


  }
}

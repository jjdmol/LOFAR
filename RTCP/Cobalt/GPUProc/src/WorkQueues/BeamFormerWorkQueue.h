//# BeamFormerWorkQueue.h
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
//# $Id: $

#ifndef LOFAR_GPUPROC_BEAM_FORMER_WORKQUEUE_H
#define LOFAR_GPUPROC_BEAM_FORMER_WORKQUEUE_H

#include <complex>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <OpenCL_Support.h>
#include <BandPass.h>
#include <Pipelines/BeamFormerPipeline.h>

#include <Kernels/IntToFloatKernel.h>
#include <Kernels/Filter_FFT_Kernel.h>
#include <Kernels/DelayAndBandPassKernel.h>
#include <Kernels/BeamFormerKernel.h>
#include <Kernels/BeamFormerTransposeKernel.h>
#include <Kernels/DedispersionForwardFFTkernel.h>
#include <Kernels/DedispersionBackwardFFTkernel.h>
#include <Kernels/DedispersionChirpKernel.h>

#include "WorkQueue.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class BeamFormerWorkQueue : public WorkQueue
    {
    public:
      BeamFormerWorkQueue(BeamFormerPipeline &, unsigned queueNumber);

      void doWork();

      BeamFormerPipeline  &pipeline;

      MultiArraySharedBuffer<char, 4>                inputSamples;
      cl::Buffer devFilteredData;
      MultiArraySharedBuffer<float, 1>               bandPassCorrectionWeights;
      MultiArraySharedBuffer<float, 3>               delaysAtBegin, delaysAfterEnd;
      MultiArraySharedBuffer<float, 2>               phaseOffsets;
      cl::Buffer devCorrectedData;
      MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights;
      cl::Buffer devComplexVoltages;
      MultiArraySharedBuffer<std::complex<float>, 4> transposedComplexVoltages;
      MultiArraySharedBuffer<float, 1>               DMs;

    private:
      IntToFloatKernel intToFloatKernel;
      Filter_FFT_Kernel fftKernel;
      DelayAndBandPassKernel delayAndBandPassKernel;
      BeamFormerKernel beamFormerKernel;
      BeamFormerTransposeKernel transposeKernel;
      DedispersionForwardFFTkernel dedispersionForwardFFTkernel;
      DedispersionBackwardFFTkernel dedispersionBackwardFFTkernel;
      DedispersionChirpKernel dedispersionChirpKernel;
    };

  }
}

#endif


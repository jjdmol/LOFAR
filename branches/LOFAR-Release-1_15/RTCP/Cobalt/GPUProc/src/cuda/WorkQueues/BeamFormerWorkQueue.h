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
//# $Id$

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_WORKQUEUE_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_WORKQUEUE_H

#include <complex>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <CoInterface/StreamableData.h>

#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/FilterBank.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Pipelines/BeamFormerPipeline.h>

#include <GPUProc/Kernels/IntToFloatKernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/Filter_FFT_Kernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <GPUProc/Kernels/BeamFormerTransposeKernel.h>
#include <GPUProc/Kernels/DedispersionForwardFFTkernel.h>
#include <GPUProc/Kernels/DedispersionBackwardFFTkernel.h>
#include <GPUProc/Kernels/DedispersionChirpKernel.h>

#include "WorkQueue.h"

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * Our output data type
     */
    class BeamFormedData: public MultiDimArrayHostBuffer<fcomplex, 3>, public StreamableData
    {
    public:
      BeamFormedData(unsigned nrStokes, unsigned nrChannels, size_t nrSamples, gpu::Context &context)
      :
        MultiDimArrayHostBuffer<fcomplex, 3>(boost::extents[nrStokes][nrChannels][nrSamples], context, 0)
      {
      }

    protected:
      virtual void readData(Stream *str, unsigned) {
        str->read(origin(), size());
      }

      virtual void writeData(Stream *str, unsigned) {
        str->write(origin(), size());
      }
    };

    class BeamFormerWorkQueue : public WorkQueue
    {
    public:
      BeamFormerWorkQueue(const Parset &parset, gpu::Context &context);

      // Beam form the data found in the input data buffer
      virtual void processSubband(WorkQueueInputData &input, StreamableData &output);

      // Do post processing on the CPU
      virtual void postprocessSubband(StreamableData &output);

    private:
      // The previously processed SAP/block, or -1 if nothing has been
      // processed yet. Used in order to determine if new delays have
      // to be uploaded.
      ssize_t prevBlock;
      signed int prevSAP;

      // Raw buffers, these are mapped with boost multiarrays 
      // in the InputData class
      WorkQueueInputData::DeviceBuffers devInput;

      gpu::DeviceMemory devFilteredData;

    private:
#if 0
      // Compiled kernels
      FIR_FilterKernel firFilterKernel;
      Filter_FFT_Kernel fftKernel;
      DelayAndBandPassKernel delayAndBandPassKernel;
#endif
#if 0
      MultiArraySharedBuffer<char, 4>                inputSamples;
      DeviceBuffer devFilteredData;
      MultiArraySharedBuffer<float, 1>               bandPassCorrectionWeights;
      MultiArraySharedBuffer<float, 3>               delaysAtBegin, delaysAfterEnd;
      MultiArraySharedBuffer<float, 2>               phaseOffsets;
      DeviceBuffer devCorrectedData;
      MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights;
      DeviceBuffer devComplexVoltages;
      MultiArraySharedBuffer<std::complex<float>, 4> transposedComplexVoltages;
      MultiArraySharedBuffer<float, 1>               DMs;

      IntToFloatKernel intToFloatKernel;
      Filter_FFT_Kernel fftKernel;
      DelayAndBandPassKernel delayAndBandPassKernel;
      BeamFormerKernel beamFormerKernel;
      BeamFormerTransposeKernel transposeKernel;
      DedispersionForwardFFTkernel dedispersionForwardFFTkernel;
      DedispersionBackwardFFTkernel dedispersionBackwardFFTkernel;
      DedispersionChirpKernel dedispersionChirpKernel;
#endif
    };

  }
}

#endif


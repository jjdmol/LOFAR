//# BeamFormerSubbandProc.h
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

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_SUBBAND_PROC_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_SUBBAND_PROC_H

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
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <GPUProc/Kernels/BeamFormerTransposeKernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>

#include "SubbandProc.h"

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

    struct BeamFormerFactories
    {
      BeamFormerFactories(const Parset &ps) :
        intToFloat(ps),
        delayCompensation(delayCompensationParams(ps)),
        correctBandPass(correctBandPassParams(ps)),
        beamFormer(ps),
        transpose(ps),
        firFilter(firFilterParams(ps))
      {
      }

      KernelFactory<IntToFloatKernel> intToFloat;
      KernelFactory<DelayAndBandPassKernel> delayCompensation;
      KernelFactory<DelayAndBandPassKernel> correctBandPass;
      KernelFactory<BeamFormerKernel> beamFormer;
      KernelFactory<BeamFormerTransposeKernel> transpose;
      KernelFactory<FIR_FilterKernel> firFilter;

      DelayAndBandPassKernel::Parameters delayCompensationParams(const Parset &ps) const {
        DelayAndBandPassKernel::Parameters params(ps);
        params.nrChannelsPerSubband = 64;
        params.correctBandPass = false;

        // TODO: Don't transpose data

        return params;
      }

      DelayAndBandPassKernel::Parameters correctBandPassParams(const Parset &ps) const {
        DelayAndBandPassKernel::Parameters params(ps);
        params.nrChannelsPerSubband = 2048;
        params.delayCompensation = false;

        // TODO: Don't transpose data

        return params;
      }

      FIR_FilterKernel::Parameters firFilterParams(const Parset &ps) const {
        FIR_FilterKernel::Parameters params(ps);

        // TODO: Set BF params, not correlator ones

        return params;
      }
    };

    class BeamFormerSubbandProc : public SubbandProc
    {
    public:
      BeamFormerSubbandProc(const Parset &parset, gpu::Context &context, BeamFormerFactories &factories);

      // Beam form the data found in the input data buffer
      virtual void processSubband(SubbandProcInputData &input, StreamableData &output);

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
      SubbandProcInputData::DeviceBuffers devInput;

      gpu::DeviceMemory devA;
      gpu::DeviceMemory devB;

    private:
      // NULL placeholder for unused DeviceMemory parameters
      gpu::DeviceMemory devNull;

      /*
       * Kernels
       */

      // int -> float
      IntToFloatKernel::Buffers intToFloatBuffers;
      std::auto_ptr<IntToFloatKernel> intToFloatKernel;

      // first FFT
      FFT_Kernel firstFFT;

      // delay compensation
      DelayAndBandPassKernel::Buffers delayCompensationBuffers;
      std::auto_ptr<DelayAndBandPassKernel> delayCompensationKernel;

      // second FFT
      FFT_Kernel secondFFT;

      // bandpass correction
      gpu::DeviceMemory devBandPassCorrectionWeights;
      DelayAndBandPassKernel::Buffers correctBandPassBuffers;
      std::auto_ptr<DelayAndBandPassKernel> correctBandPassKernel;

      // beam former
      gpu::DeviceMemory devBeamFormerWeights;
      BeamFormerKernel::Buffers beamFormerBuffers;
      std::auto_ptr<BeamFormerKernel> beamFormerKernel;

      BeamFormerTransposeKernel::Buffers transposeBuffers;
      std::auto_ptr<BeamFormerTransposeKernel> transposeKernel;

      // inverse FFT
      FFT_Kernel inverseFFT;

      // PPF
      gpu::DeviceMemory devFilterWeights;
      FIR_FilterKernel::Buffers firFilterBuffers;
      std::auto_ptr<FIR_FilterKernel> firFilterKernel;
      FFT_Kernel finalFFT;
    };

  }
}

#endif


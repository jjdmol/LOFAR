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
#include <memory>

#include <boost/shared_ptr.hpp>
#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/Pipelines/BeamFormerPipeline.h>

#include <GPUProc/Kernels/BandPassCorrectionKernel.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <GPUProc/Kernels/CoherentStokesTransposeKernel.h>
#include <GPUProc/Kernels/CoherentStokesKernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/FFTShiftKernel.h>
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/IncoherentStokesKernel.h>
#include <GPUProc/Kernels/IncoherentStokesTransposeKernel.h>
#include <GPUProc/Kernels/IntToFloatKernel.h>

#include "BeamFormerPreprocessingStep.h"
#include "BeamFormerCoherentStep.h"

#include "SubbandProc.h"

namespace LOFAR
{
  namespace Cobalt
  {
    //# Forward declarations
    struct BeamFormerFactories;

    // Our output data type
    class BeamFormedData: public SubbandProcOutputData
    {
    public:
      MultiDimArrayHostBuffer<float, 4> coherentData;
      MultiDimArrayHostBuffer<float, 4> incoherentData;
     
      BeamFormedData(unsigned nrCoherentTABs,
                     unsigned nrCoherentStokes,
                     size_t nrCoherentSamples,
                     unsigned nrCoherentChannels,
                     unsigned nrIncoherentTABs,
                     unsigned nrIncoherentStokes,
                     size_t nrIncoherentSamples,
                     unsigned nrIncoherentChannels,
                     gpu::Context &context);

      /* Short-hand constructor to fetch all dimensions
       * from a Parset */
      BeamFormedData(const Parset &ps,
                     gpu::Context &context);
    };

    class BeamFormerSubbandProc : public SubbandProc
    {
    public:
      BeamFormerSubbandProc(const Parset &parset, gpu::Context &context,
                            BeamFormerFactories &factories,
                            size_t nrSubbandsPerSubbandProc = 1);

      // Beam form the data found in the input data buffer
      virtual void processSubband(SubbandProcInputData &input,
                                  SubbandProcOutputData &output);

      // Do post processing on the CPU
      virtual bool postprocessSubband(SubbandProcOutputData &output);

      void printStats();

      void logTime(unsigned nrCoherent,
        unsigned nrIncoherent, 
        bool incoherentStokesPPF);

      // Beamformer specific collection of PerformanceCounters
      class Counters
      {
      public:
        Counters(gpu::Context &context);

        // gpu transfer counters
        PerformanceCounter samples;
        PerformanceCounter visibilities;
        PerformanceCounter incoherentOutput;

        // Print the mean and std of each performance counter on the logger
        void printStats();
      };

      Counters counters;

    private:


      void initIncoherentMembers(gpu::Context &context,
        BeamFormerFactories &factories);



      void processIncoherentStage(BlockID blockID) ;


      void logTimeIncoherentStage(bool incoherentStokesPPF);


      void printStatsIncoherentStage();

      // Whether we form any coherent beams
      bool formCoherentBeams;

      // Whether we form any incoherent beams
      bool formIncoherentBeams;

      // The previously processed SAP/block, or -1 if nothing has been
      // processed yet. Used in order to determine if new delays have
      // to be uploaded.
      ssize_t prevBlock;
      signed int prevSAP;

      // Raw buffers, these are mapped with boost multiarrays 
      // in the InputData class
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> devInput;

      // @{
      // Device memory buffers. These buffers are used interleaved.
      boost::shared_ptr<gpu::DeviceMemory> devA;
      boost::shared_ptr<gpu::DeviceMemory> devB;
      boost::shared_ptr<gpu::DeviceMemory> devC;
      boost::shared_ptr<gpu::DeviceMemory> devD;
      boost::shared_ptr<gpu::DeviceMemory> devE;
      // @}

      // NULL placeholder for unused DeviceMemory parameters
      boost::shared_ptr<gpu::DeviceMemory> devNull;

      boost::shared_ptr<gpu::DeviceMemory> devBeamFormerDelays;

      std::auto_ptr<BeamFormerPreprocessingStep> preprocessingPart;
      std::auto_ptr<BeamFormerCoherentStep> coherentStep;

      /*
      * Kernels
      */


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


//# CorrelatorStep.h
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

#ifndef LOFAR_GPUPROC_CUDA_CORRELATOR_STEP_H
#define LOFAR_GPUPROC_CUDA_CORRELATOR_STEP_H

#include <complex>
#include <vector>
#include <utility> // for std::pair

#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>

#include <boost/shared_ptr.hpp>
#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/CorrelatedData.h>

#include "SubbandProcInputData.h"
#include "SubbandProcOutputData.h"
#include "ProcessStep.h"

#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/KernelFactory.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>


namespace LOFAR
{
  namespace Cobalt
  {
    class CorrelatorStep: public ProcessStep
    {
    public:
      struct Factories {
        Factories(const Parset &ps, size_t nrSubbandsPerSubbandProc);

        SmartPtr< KernelFactory<FFT_Kernel> > fft;
        SmartPtr< KernelFactory<FIR_FilterKernel> > firFilter;

        KernelFactory<DelayAndBandPassKernel> delayAndBandPass;

        KernelFactory<CorrelatorKernel> correlator;
      };

      CorrelatorStep(const Parset &parset,
        gpu::Stream &i_queue, 
        gpu::Context &context,
        Factories &factories,
        boost::shared_ptr<gpu::DeviceMemory> i_devA,
        boost::shared_ptr<gpu::DeviceMemory> i_devB,
        size_t nrSubbandsPerSubbandProc);

      void writeInput(const SubbandProcInputData &input);

      void process(const SubbandProcInputData &input);
      void processCPU(const SubbandProcInputData &input, SubbandProcOutputData &output);

      void readOutput(SubbandProcOutputData &output);

      bool postprocessSubband(SubbandProcOutputData &output);

      // Collection of functions to tranfer the input flags to the output.
      // \c propagateFlags can be called parallel to the kernels.
      // After the data is copied from the the shared buffer
      // \c applyNrValidSamples can be used to weight the visibilities
      class Flagger
      {
      public:
        // 1. Convert input flags to channel flags, calculate the amount flagged
        // samples and save this in output
        static void propagateFlags(Parset const & parset,
          MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
          SubbandProcOutputData::CorrelatedData &output);

        // 1.1 Convert the flags per station to channel flags, change time scale
        // if nchannel > 1
        static void convertFlagsToChannelFlags(Parset const &ps,
          MultiDimArray<SparseSet<unsigned>, 1> const &inputFlags,
          MultiDimArray<SparseSet<unsigned>, 1> &flagsPerChannel);

        // 2. Calculate the weight based on the number of flags and apply this
        // weighting to all output values
        static void applyNrValidSamples(Parset const &parset, LOFAR::Cobalt::CorrelatedData &output);

        // 1.2 Calculate the number of flagged samples and set this on the
        // output dataproduct This function is aware of the used filter width a
        // corrects for this.
        static void
        calcNrValidSamples(Parset const &parset,
                    MultiDimArray<SparseSet<unsigned>, 1> const &flagsPerChannel,
                    SubbandProcOutputData::CorrelatedData &output);

        // 2.1 Apply the supplied weight to the complex values in the channels
        // for the given baseline. If nrChannels > 1, visibilities for channel 0 will be set to 0.0.
        static void applyWeight(unsigned baseline, unsigned nrChannels,
                                float weight, LOFAR::Cobalt::CorrelatedData &output);
      private:
        template<typename T>
        static void applyNrValidSamples(Parset const &parset, LOFAR::Cobalt::CorrelatedData &output);

        template<typename T>
        static void
        calcNrValidSamples(Parset const &parset,
                    MultiDimArray<SparseSet<unsigned>, 1> const &flagsPerChannel,
                    SubbandProcOutputData::CorrelatedData &output);

      };

    private:
      const bool correlatorPPF;

      //Data members
      boost::shared_ptr<gpu::DeviceMemory> devA;
      boost::shared_ptr<gpu::DeviceMemory> devB;
      gpu::DeviceMemory devE;

      /*
       * Kernels
       */

      // FIR filter
      SmartPtr<FIR_FilterKernel> firFilterKernel;

      // FFT
      SmartPtr<FFT_Kernel> fftKernel;

      // Delay and Bandpass
      std::auto_ptr<DelayAndBandPassKernel> delayAndBandPassKernel;

      // Correlator
      std::auto_ptr<CorrelatorKernel> correlatorKernel;

      PerformanceCounter outputCounter;

      // Buffers for long-time integration; one buffer for each subband that
      // will be processed by this class instance. Each element of the vector
      // contains a counter that tracks the number of additions made to the data
      // buffer and the data buffer itself.
      std::vector< std::pair< size_t, SmartPtr<LOFAR::Cobalt::CorrelatedData> > >
      integratedData;

      bool integrate(SubbandProcOutputData &output);
    };
  }
}

#endif

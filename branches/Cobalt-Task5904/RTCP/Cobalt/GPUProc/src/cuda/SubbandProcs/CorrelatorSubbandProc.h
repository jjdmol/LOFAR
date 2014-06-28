//# CorrelatorSubbandProc.h
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

#ifndef LOFAR_GPUPROC_CUDA_CORRELATOR_SUBBAND_PROC_H
#define LOFAR_GPUPROC_CUDA_CORRELATOR_SUBBAND_PROC_H

// @file
#include <cmath>
#include <complex>
#include <utility>
#include <memory>

#include <Stream/Stream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/SparseSet.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>
#include <GPUProc/PerformanceCounter.h>

#include "SubbandProc.h"

namespace LOFAR
{
  namespace Cobalt
  {

    // A CorrelatedData object tied to a HostBuffer. Represents an output
    // data item that can be efficiently filled from the GPU.
    class CorrelatedDataHostBuffer: public MultiDimArrayHostBuffer<fcomplex, 4>,
                                    public CorrelatedData,
                                    public SubbandProcOutputData
    {
    public:
      CorrelatedDataHostBuffer(unsigned nrStations, 
                               unsigned nrChannels,
                               unsigned maxNrValidSamples,
                               gpu::Context &context);
      // Reset the MultiDimArrayHostBuffer and the CorrelatedData
      void reset();
    };

    struct CorrelatorFactories
    {
      CorrelatorFactories(const Parset &ps, 
                          size_t nrSubbandsPerSubbandProc = 1):
        firFilter(ps.settings.correlator.nrChannels > 1
          ? new KernelFactory<FIR_FilterKernel>(FIR_FilterKernel::Parameters(ps,
            ps.settings.antennaFields.size(),
            true,
            nrSubbandsPerSubbandProc,
            ps.settings.correlator.nrChannels,

            // Scale to always output visibilities or stokes with the same flux scale.
            // With the same bandwidth, twice the (narrower) channels _average_ (not
            // sum) to the same fluxes (and same noise). Twice the channels (twice the
            // total bandwidth) _average_ to the _same_ flux, but noise * 1/sqrt(2).
            // Note: FFTW/CUFFT do not normalize, correlation or stokes calculation
            // effectively squares, integr on fewer channels averages over more values.
            std::sqrt((double)ps.settings.correlator.nrChannels)))
          : NULL),
        delayAndBandPass(DelayAndBandPassKernel::Parameters(ps, true)),
        correlator(ps)
      {
      }

      SmartPtr< KernelFactory<FIR_FilterKernel> > firFilter;
      KernelFactory<DelayAndBandPassKernel> delayAndBandPass;
      KernelFactory<CorrelatorKernel> correlator;
    };

    class CorrelatorSubbandProc : public SubbandProc
    {
    public:
      CorrelatorSubbandProc(const Parset &parset, 
                            gpu::Context &context,
                            CorrelatorFactories &factories,
                            size_t nrSubbandsPerSubbandProc = 1);

      virtual ~CorrelatorSubbandProc();

      // Print statistics of all kernels and transfers
      void printStats();

      // Correlate the data found in the input data buffer
      virtual void processSubband(SubbandProcInputData &input,
                                  SubbandProcOutputData &output);

      // Do post processing on the CPU
      virtual bool postprocessSubband(SubbandProcOutputData &output);

      // Collection of functions to tranfer the input flags to the output.
      // \c propagateFlags can be called parallel to the kernels.
      // After the data is copied from the the shared buffer 
      // \c applyWeights can be used to weight the visibilities 
      class Flagger: public SubbandProc::Flagger
      {
      public:
        // 1. Convert input flags to channel flags, calculate the amount flagged
        // samples and save this in output
        static void propagateFlags(Parset const & parset,
          MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
          CorrelatedData &output);

        // 2. Calculate the weight based on the number of flags and apply this
        // weighting to all output values
        template<typename T>
        static void applyWeights(Parset const &parset, CorrelatedData &output);

        // 1.2 Calculate the number of flagged samples and set this on the
        // output dataproduct This function is aware of the used filter width a
        // corrects for this.
        template<typename T> 
        static void
        calcWeights(Parset const &parset,
                    MultiDimArray<SparseSet<unsigned>, 2>const &flagsPerChannel,
                    CorrelatedData &output);

        // 2.1 Apply the supplied weight to the complex values in the channel
        // and baseline
        static void applyWeight(unsigned baseline, unsigned channel,
                                float weight, CorrelatedData &output);
      };

      // Correlator specific collection of PerformanceCounters
      class Counters
      {
      public:
        Counters(gpu::Context &context);

        // gpu transfer counters
        PerformanceCounter samples;
        PerformanceCounter visibilities;
      };

      Counters counters;
    private:
      const bool correlatorPPF;

      // The previously processed SAP/block, or -1 if nothing has been
      // processed yet. Used in order to determine if new delays have
      // to be uploaded.
      ssize_t prevBlock;
      signed int prevSAP;

      gpu::DeviceMemory devA;      
      gpu::DeviceMemory devB;

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

      // Buffers for long-time integration; one buffer for each subband that
      // will be processed by this class instance. Each element of the vector
      // contains a counter that tracks the number of additions made to the data
      // buffer and the data buffer itself.
      vector< std::pair< size_t, SmartPtr<CorrelatedDataHostBuffer> > >
      integratedData;

      // Perform long-time integration. Returns `true' if integration has
      // been completed. In that case, `output' will contain the integration
      // result.
      bool integrate(CorrelatedDataHostBuffer &output);
    };

  }
}

#endif


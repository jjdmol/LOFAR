//# CorrelatorStep.cc
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

#include <lofar_config.h>

#include "CorrelatorStep.h"
#include "SubbandProc.h"

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

#include <CoInterface/Parset.h>
#include <ApplCommon/PosixTime.h>
#include <Common/LofarLogger.h>

#include <iomanip>

namespace LOFAR
{
  namespace Cobalt
  {


    CorrelatorStep::Factories::Factories(const Parset &ps, size_t nrSubbandsPerSubbandProc) :
      fft(ps.settings.correlator.nrChannels > 1
        ? new KernelFactory<FFT_Kernel>(FFT_Kernel::Parameters(
          ps.settings.correlator.nrChannels,
          ps.settings.antennaFields.size() * NR_POLARIZATIONS * ps.settings.blockSize,
          true,
          "FFT (correlator)"))
        : NULL),
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
            std::sqrt((double)ps.settings.correlator.nrChannels),
            "FIR (correlator)"))
          : NULL),

      delayAndBandPass(DelayAndBandPassKernel::Parameters(ps, true)),

      correlator(ps)
    {
    }


    void CorrelatorStep::Flagger::propagateFlags(
      Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      LOFAR::Cobalt::CorrelatedData &output)
    {   
      // Object for storing transformed flags
      MultiDimArray<SparseSet<unsigned>, 2> flagsPerChannel(
        boost::extents[parset.settings.correlator.nrChannels][parset.settings.antennaFields.size()]);

      // First transform the flags to channel flags: taking in account 
      // reduced resolution in time and the size of the filter
      SubbandProc::Flagger::convertFlagsToChannelFlags(parset, inputFlags, flagsPerChannel);

      // Calculate the number of flags per baseline and assign to
      // output object.
      calcWeights(parset, flagsPerChannel, output);
    }


    namespace {
      // Return the baseline number for a pair of stations
      unsigned baseline(unsigned major, unsigned minor)
      {
        ASSERT(major >= minor);

        return major * (major + 1) / 2 + minor;
      }
    }

    template<typename T> void CorrelatorStep::Flagger::calcWeights(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      LOFAR::Cobalt::CorrelatedData &output)
    {
      unsigned nrSamplesPerIntegration = parset.settings.correlator.nrSamplesPerChannel;

      // loop the stations
      for (unsigned stat1 = 0; stat1 < parset.nrStations(); stat1 ++) {
        for (unsigned stat2 = 0; stat2 <= stat1; stat2 ++) {
          unsigned bl = baseline(stat1, stat2);

          // If there is a single channel then the index 0 contains real data
          if (parset.settings.correlator.nrChannels == 1) 
          {                                            
            // The number of invalid (flagged) samples is the union of the
            // flagged samples in the two stations
            unsigned nrValidSamples = nrSamplesPerIntegration -
              (flagsPerChannel[0][stat1] | flagsPerChannel[0][stat2]).count();

            // Moet worden toegekend op de correlated dataobject
            output.nrValidSamples<T>(bl, 0) = nrValidSamples;
          } 
          else 
          {
            // channel 0 does not contain valid data
            output.nrValidSamples<T>(bl, 0) = 0;

            for(unsigned ch = 1; ch < parset.settings.correlator.nrChannels; ch ++) 
            {
              // valid samples is total number of samples minus the union of the
              // Two stations.
              unsigned nrValidSamples = nrSamplesPerIntegration -
                (flagsPerChannel[ch][stat1] | 
                 flagsPerChannel[ch][stat2]).count();

              output.nrValidSamples<T>(bl, ch) = nrValidSamples;
            }
          }
        }
      }
    }


    void CorrelatorStep::Flagger::calcWeights(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      LOFAR::Cobalt::CorrelatedData &output)
    {
      switch (output.itsNrBytesPerNrValidSamples) {
        case 4:
          calcWeights<uint32_t>(parset, flagsPerChannel, output);
          break;

        case 2:
          calcWeights<uint16_t>(parset, flagsPerChannel, output);
          break;

        case 1:
          calcWeights<uint8_t>(parset, flagsPerChannel, output);
          break;
      }
    }


    void CorrelatorStep::Flagger::applyWeight(unsigned baseline, 
      unsigned channel, float weight, LOFAR::Cobalt::CorrelatedData &output)
    {
      for(unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; ++pol1)
        for(unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; ++pol2)
          output.visibilities[baseline][channel][pol1][pol2] *= weight;
    }


    template<typename T> void 
    CorrelatorStep::Flagger::applyWeights(Parset const &parset,
                                                 LOFAR::Cobalt::CorrelatedData &output)
    {
      for (unsigned bl = 0; bl < output.itsNrBaselines; ++bl)
      {
        // Calculate the weights for the channels
        //
        // Channel 0 is already flagged according to specs, so we can simply
        // include it both for 1 and >1 channels/subband.
        for (unsigned ch = 0; ch < parset.settings.correlator.nrChannels; ch++) 
        {
          T nrValidSamples = output.nrValidSamples<T>(bl, ch);

          // If all samples flagged, weights is zero.
          // TODO: make a lookup table for the expensive division; measure first
          float weight = nrValidSamples ? 1.0f / nrValidSamples : 0;  

          applyWeight(bl, ch, weight, output);
        }
      }
    }


    void CorrelatorStep::Flagger::applyWeights(Parset const &parset,
                                                 LOFAR::Cobalt::CorrelatedData &output)
    {
      switch (output.itsNrBytesPerNrValidSamples) {
        case 4:
          applyWeights<uint32_t>(parset, output);  
          break;

        case 2:
          applyWeights<uint16_t>(parset, output);  
          break;

        case 1:
          applyWeights<uint8_t>(parset, output);  
          break;
      }
    }

    CorrelatorStep::CorrelatorStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      Factories &factories,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB,
      size_t nrSubbandsPerSubbandProc)
      :
      ProcessStep(parset, i_queue),
      correlatorPPF(ps.settings.correlator.nrChannels > 1),
      devE(context, correlatorPPF
                    ? std::max(factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA),
                               factories.correlator.bufferSize(CorrelatorKernel::OUTPUT_DATA))
                    : factories.correlator.bufferSize(CorrelatorKernel::OUTPUT_DATA)),
      outputCounter(context, "output (correlator)"),
      integratedData(nrSubbandsPerSubbandProc)
    {
      devA=i_devA;
      devB=i_devB;
      initMembers(context, factories);
    }

    void CorrelatorStep::initMembers(gpu::Context &context,
      Factories &factories){
      if (correlatorPPF) {
        // FIR filter: A -> B
        firFilterKernel = factories.firFilter->create(queue, *devA, *devB);

        // FFT: B -> E
        fftKernel = factories.fft->create(queue, *devB, devE);
      }

      // Delay and Bandpass: A/E -> B
      delayAndBandPassKernel = std::auto_ptr<DelayAndBandPassKernel>(factories.delayAndBandPass.create(queue, 
        correlatorPPF ? devE : *devA, *devB));

      // Correlator: B -> E
      correlatorKernel = std::auto_ptr<CorrelatorKernel>(factories.correlator.create(queue,
        *devB, devE));

      // Initialize the output buffers for the long-time integration
      for (size_t i = 0; i < integratedData.size(); i++) {
        integratedData[i] = 
          make_pair(0, new LOFAR::Cobalt::CorrelatedData(ps.settings.antennaFields.size(), 
                                          ps.settings.correlator.nrChannels,
                                          ps.settings.correlator.nrSamplesPerChannel));
      }
    }

    void CorrelatorStep::writeInput(const SubbandProcInputData &input)
    {
      if (ps.settings.delayCompensation.enabled) {
        queue.writeBuffer(delayAndBandPassKernel->delaysAtBegin,
          input.delaysAtBegin, false);
        queue.writeBuffer(delayAndBandPassKernel->delaysAfterEnd,
          input.delaysAfterEnd, false);
        queue.writeBuffer(delayAndBandPassKernel->phase0s,
          input.phase0s, false);
      }
    }

    void CorrelatorStep::process(const SubbandProcInputData &input)
    {
      if (correlatorPPF) {
        // The subbandIdx immediate kernel arg must outlive kernel runs.
        firFilterKernel->enqueue(input.blockID, 
                                 input.blockID.subbandProcSubbandIdx);
        fftKernel->enqueue(input.blockID);
      }

      // Even if we skip delay compensation and bandpass correction (rare), run
      // that kernel, as it also reorders the data for the correlator kernel.
      //
      // The centralFrequency and SAP immediate kernel args must outlive kernel runs.
      delayAndBandPassKernel->enqueue(
        input.blockID, 
        ps.settings.subbands[input.blockID.globalSubbandIdx].centralFrequency,
        ps.settings.subbands[input.blockID.globalSubbandIdx].SAP);

      correlatorKernel->enqueue(input.blockID);
    }


    void CorrelatorStep::readOutput(SubbandProcOutputData &output)
    {
      // Read data back from the kernel
      queue.readBuffer(output.correlatedData, devE, outputCounter, false);
    }


    void CorrelatorStep::processCPU(const SubbandProcInputData &input, SubbandProcOutputData &output)
    {
      // Propagate the flags.
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1> flags = input.inputFlags;

      if (correlatorPPF) {
        // Put the history flags in front of the sample flags,
        // because Flagger::propagateFlags expects it that way.
        firFilterKernel->prefixHistoryFlags(
          flags, input.blockID.subbandProcSubbandIdx);
      }

      Flagger::propagateFlags(ps, flags, output.correlatedData);
    }


    bool CorrelatorStep::integrate(SubbandProcOutputData &output)
    {
      const size_t idx = output.blockID.subbandProcSubbandIdx;
      const size_t nblock = ps.settings.correlator.nrBlocksPerIntegration;
      
      // We don't want to copy the data if we don't need to integrate.
      if (nblock == 1) {
        output.correlatedData.setSequenceNumber(output.blockID.block);
        return true;
      }

      integratedData[idx].first++;

      if (integratedData[idx].first < nblock) {
        *integratedData[idx].second += output.correlatedData;
        return false;
      }
      else {
        output.correlatedData += *integratedData[idx].second;
        output.correlatedData.setSequenceNumber(output.blockID.block / nblock);
        integratedData[idx].first = 0;
        integratedData[idx].second->reset();
        return true;
      }
    }


    bool CorrelatorStep::postprocessSubband(SubbandProcOutputData &output)
    {
      if (!integrate(output)) {
        // Not yet done constructing output block 
        return false;
      }

      // The flags are already copied to the correct location
      // now the flagged amount should be applied to the visibilities
      Flagger::applyWeights(ps, output.correlatedData);  

      return true;
    }
  }
}

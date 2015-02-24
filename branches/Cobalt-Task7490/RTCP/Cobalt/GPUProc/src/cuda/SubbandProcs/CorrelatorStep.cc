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

    void CorrelatorStep::Flagger::convertFlagsToChannelFlags(Parset const &ps,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      MultiDimArray<SparseSet<unsigned>, 1>& flagsPerChannel)
    {
      unsigned numberOfChannels = ps.settings.correlator.nrChannels;
      unsigned log2NrChannels = log2(numberOfChannels);
      //Convert the flags per sample to flags per channel
      for (unsigned station = 0; station < ps.settings.correlator.stations.size(); station ++) 
      {
        // get the flag ranges
        const SparseSet<unsigned>::Ranges &ranges = inputFlags[station].getRanges();
        for (SparseSet<unsigned>::const_iterator it = ranges.begin();
          it != ranges.end(); it ++) 
        {
          unsigned begin_idx;
          unsigned end_idx;
          if (numberOfChannels == 1)
          {
            // do nothing, just take the ranges as supplied
            begin_idx = it->begin; 
            end_idx = std::min(static_cast<unsigned>(ps.settings.correlator.nrSamplesPerBlock), it->end);
          }
          else
          {
            // Never flag before the start of the time range               
            // use bitshift to divide to the number of channels. 
            //
            // NR_TAPS is the width of the filter: they are
            // absorbed by the FIR and thus should be excluded
            // from the original flag set.
            //
            // The original flag set can span up to
            //    [0, nrSamplesPerBlock + nrChannels * (NR_TAPS - 1))
            // of which the FIRST (NR_TAPS - 1) samples belong to
            // the previous block, and are used to initialise the
            // FIR filter. Every sample i of the current block is thus
            // actually at index (i + nrChannels * (NR_TAPS - 1)),
            // or, after converting to channels, at index (i' + NR_TAPS - 1).
            //
            // At the same time, every sample is affected by
            // the NR_TAPS-1 samples before it. So, any flagged
            // sample in the input flags NR_TAPS samples in
            // the channel.
            begin_idx = std::max(0, 
              (signed) (it->begin >> log2NrChannels) - NR_TAPS + 1);

            // The min is needed, because flagging the last input
            // samples would cause NR_TAPS subsequent samples to
            // be flagged, which aren't necessarily part of this block.
            end_idx = std::min(static_cast<unsigned>(ps.settings.correlator.nrSamplesPerBlock), 
              ((it->end - 1) >> log2NrChannels) + 1);
          }

          // Now copy the transformed ranges to the channelflags
          flagsPerChannel[station].include(begin_idx, end_idx);
        }
      }
    }


    void CorrelatorStep::Flagger::propagateFlags(
      Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      SubbandProcOutputData::CorrelatedData &output)
    {   
      // Object for storing transformed flags
      MultiDimArray<SparseSet<unsigned>, 1> flagsPerChannel(
        boost::extents[parset.settings.antennaFields.size()]);

      // First transform the flags to channel flags: taking in account 
      // reduced resolution in time and the size of the filter
      convertFlagsToChannelFlags(parset, inputFlags, flagsPerChannel);

      // Calculate the number of flags per baseline and assign to
      // output object.
      calcNrValidSamples(parset, flagsPerChannel, output);
    }


    namespace {
      // Return the baseline number for a pair of stations
      unsigned baseline(unsigned major, unsigned minor)
      {
        ASSERT(major >= minor);

        return major * (major + 1) / 2 + minor;
      }
    }

    template<typename T> void CorrelatorStep::Flagger::calcNrValidSamples(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 1>const & flagsPerChannel,
      SubbandProcOutputData::CorrelatedData &output)
    {
      /*
       * NOTE: This routine is performance critical. This is called as part
       *       of processCPU(). The tCorrelatorStep test validates its performance.
       */

      // The number of samples per integration within this block.
      const unsigned nrSamples =
          parset.settings.correlator.nrSamplesPerBlock /
          parset.settings.correlator.nrIntegrationsPerBlock;

      // loop the stations
      for (unsigned stat1 = 0; stat1 < parset.settings.correlator.stations.size(); stat1 ++) {
        for (unsigned stat2 = 0; stat2 <= stat1; stat2 ++) {
          const unsigned bl = baseline(stat1, stat2);

          // The number of invalid (flagged) samples is the union of the
          // flagged samples in the two stations
          const SparseSet<unsigned> flags =
            flagsPerChannel[stat1] | flagsPerChannel[stat2];

          for (size_t i = 0; i < parset.settings.correlator.nrIntegrationsPerBlock; ++i) {
            LOFAR::Cobalt::CorrelatedData &correlatedData = *output.subblocks[i];

            // Count the flags for this subblock
            const T nrValidSamples =
              nrSamples - flags.count(i * nrSamples, (i+1) * nrSamples);

            // Channel zero is invalid, unless we have only one channel
            if (parset.settings.correlator.nrChannels > 1) {
              correlatedData.nrValidSamples<T>(bl, 0) = 0;
            } else {
              correlatedData.nrValidSamples<T>(bl, 0) = nrValidSamples;
            }

            // Set the channels from 1 onward
            for(unsigned ch = 1; ch < parset.settings.correlator.nrChannels; ch ++) {
              correlatedData.nrValidSamples<T>(bl, ch) = nrValidSamples;
            }
          }
        }
      }
    }


    void CorrelatorStep::Flagger::calcNrValidSamples(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 1>const & flagsPerChannel,
      SubbandProcOutputData::CorrelatedData &output)
    {
      switch (output.subblocks[0]->itsNrBytesPerNrValidSamples) {
        case 4:
          calcNrValidSamples<uint32_t>(parset, flagsPerChannel, output);
          break;

        case 2:
          calcNrValidSamples<uint16_t>(parset, flagsPerChannel, output);
          break;

        case 1:
          calcNrValidSamples<uint8_t>(parset, flagsPerChannel, output);
          break;
      }
    }


    void CorrelatorStep::Flagger::applyWeight(unsigned baseline, 
      unsigned nrChannels, float weight, LOFAR::Cobalt::CorrelatedData &output)
    {
      /*
       * All channels and polarisations are stored consecutively, so
       * we can just grab a pointer to the first sample and walk over
       * all samples in the baseline.
       */
      fcomplex *s = &output.visibilities[baseline][0][0][0];

      unsigned i = 0;

      if (nrChannels > 1) {
        // Channel 0 has weight 0.0 (unless it's the only channel)
        for(; i < NR_POLARIZATIONS * NR_POLARIZATIONS; ++i)
          *(s++) = 0.0;
      }

      // Remaining channels are adjusted by the provided weight
      for(; i < nrChannels * NR_POLARIZATIONS * NR_POLARIZATIONS; ++i)
        *(s++) *= weight;
    }


    template<typename T> void 
    CorrelatorStep::Flagger::applyNrValidSamples(Parset const &parset,
                                                 LOFAR::Cobalt::CorrelatedData &output)
    {
      const bool singleChannel = parset.settings.correlator.nrChannels == 1;

      for (unsigned bl = 0; bl < output.itsNrBaselines; ++bl)
      {
        // Calculate the weights for the channels
        //
        // NOTE: We assume all channels to have the same nrValidSamples (except possibly channel 0).
        const T nrValidSamples = output.nrValidSamples<T>(bl, singleChannel ? 0 : 1);

        // If all samples flagged, weights is zero.
        const float weight = nrValidSamples ? 1.0f / nrValidSamples : 0;  

        // Apply the weight to this sample, turning the visibilities into the
        // average visibility over the non-flagged samples.
        //
        // This step thus normalises the visibilities for any integration time.
        applyWeight(bl, parset.settings.correlator.nrChannels, weight, output);
      }
    }


    void CorrelatorStep::Flagger::applyNrValidSamples(Parset const &parset,
                                                 LOFAR::Cobalt::CorrelatedData &output)
    {
      switch (output.itsNrBytesPerNrValidSamples) {
        case 4:
          applyNrValidSamples<uint32_t>(parset, output);  
          break;

        case 2:
          applyNrValidSamples<uint16_t>(parset, output);  
          break;

        case 1:
          applyNrValidSamples<uint8_t>(parset, output);  
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
          // Note that we always integrate complete blocks
          make_pair(0, new LOFAR::Cobalt::CorrelatedData(ps.settings.antennaFields.size(), 
                                          ps.settings.correlator.nrChannels,
                                          ps.settings.correlator.nrSamplesPerBlock));
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
      queue.readBuffer(output.correlatedData.data, devE, outputCounter, false);
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
      const size_t nblock    = ps.settings.correlator.nrBlocksPerIntegration;
      const size_t nsubblock = ps.settings.correlator.nrIntegrationsPerBlock;
      
      // We don't want to copy the data if we don't need to integrate.
      if (nblock == 1) {
        for (size_t i = 0; i < nsubblock; ++i) {
          output.correlatedData.subblocks[i]->setSequenceNumber(output.blockID.block * nsubblock + i);
        }
        return true;
      }

      // We don't have subblocks if we integrate multiple blocks.
      ASSERT( nsubblock == 1 );

      integratedData[idx].first++;

      if (integratedData[idx].first < nblock) {
        *integratedData[idx].second += *output.correlatedData.subblocks[0];
        return false;
      }
      else {
        *output.correlatedData.subblocks[0] += *integratedData[idx].second;
        output.correlatedData.subblocks[0]->setSequenceNumber(output.blockID.block / nblock);
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
      for (size_t i = 0; i < ps.settings.correlator.nrIntegrationsPerBlock; ++i) {
        Flagger::applyNrValidSamples(ps, *output.correlatedData.subblocks[i]);  
      }

      return true;
    }
  }
}

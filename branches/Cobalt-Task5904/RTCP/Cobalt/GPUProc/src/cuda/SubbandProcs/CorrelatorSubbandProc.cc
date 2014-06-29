//# CorrelatorSubbandProc.cc
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

#include "CorrelatorSubbandProc.h"

#include <cstring>
#include <algorithm>
#include <iomanip>

#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace Cobalt
  {
    CorrelatedDataHostBuffer::CorrelatedDataHostBuffer(
      unsigned nrStations, unsigned nrChannels,
      unsigned maxNrValidSamples, gpu::Context &context)
      :
      MultiDimArrayHostBuffer<fcomplex, 4>(
        boost::extents
        [nrStations * (nrStations + 1) / 2]
        [nrChannels][NR_POLARIZATIONS]
        [NR_POLARIZATIONS], 
        context, 0),
      CorrelatedData(nrStations, nrChannels, 
                     maxNrValidSamples, this->origin(),
                     this->num_elements(), heapAllocator, 1)
    {
    }


    void CorrelatedDataHostBuffer::reset()
    {
      CorrelatedData::reset();
    }

    CorrelatorSubbandProc::CorrelatorSubbandProc(
      const Parset &parset, gpu::Context &context, 
      CorrelatorFactories &factories, size_t nrSubbandsPerSubbandProc)
      :
      SubbandProc(parset, context, nrSubbandsPerSubbandProc),       
      counters(context),
      correlatorPPF(ps.settings.correlator.nrChannels > 1),
      prevBlock(-1),
      prevSAP(-1),
      devA(context, 
        correlatorPPF ? factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA)
                      : std::max(factories.delayAndBandPass.bufferSize(DelayAndBandPassKernel::INPUT_DATA),
                                 factories.correlator.bufferSize(CorrelatorKernel::OUTPUT_DATA))),
      devB(context,
        correlatorPPF ? std::max(factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA),
                                 factories.correlator.bufferSize(CorrelatorKernel::OUTPUT_DATA))
                      : factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA)),

      // Buffers for long-time integration
      integratedData(nrSubbandsPerSubbandProc)

      //## --------  Start of constructor body  -------- ##//
    {
      if (correlatorPPF) {
        // FIR filter
        firFilterKernel = factories.firFilter->create(queue, devA, devB);

        // FFT
        fftKernel = new FFT_Kernel(queue, ps.settings.correlator.nrChannels, ps.settings.antennaFields.size() * NR_POLARIZATIONS * ps.settings.blockSize, true, devB);
      }

      // Delay and Bandpass
      delayAndBandPassKernel = std::auto_ptr<DelayAndBandPassKernel>(factories.delayAndBandPass.create(queue, 
        correlatorPPF ? devB : devA,
        correlatorPPF ? devA : devB));

      // Correlator
      correlatorKernel = std::auto_ptr<CorrelatorKernel>(factories.correlator.create(queue,
        correlatorPPF ? devA : devB,
        correlatorPPF ? devB : devA));


      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i) {
        outputPool.free.append(new CorrelatedDataHostBuffer(
                                 ps.settings.antennaFields.size(),
                                 ps.settings.correlator.nrChannels,
                                 ps.settings.correlator.nrSamplesPerChannel,
                                 context));
      }

      // Initialize the output buffers for the long-time integration
      for (size_t i = 0; i < integratedData.size(); i++) {
        integratedData[i] = 
          make_pair(0, new CorrelatedDataHostBuffer(ps.settings.antennaFields.size(), 
                                                    ps.settings.correlator.nrChannels,
                                                    ps.settings.correlator.nrSamplesPerChannel,
                                                    context));
      }
    }

    CorrelatorSubbandProc::~CorrelatorSubbandProc()
    {
      printStats();
    }

    void CorrelatorSubbandProc::printStats()
    {
      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** CorrelatorSubbandProc GPU mean and stDev ****" << endl <<
        std::setw(20) << "(fir)" << (correlatorPPF ? firFilterKernel->itsCounter.stats : RunningStatistics()) << endl <<
        std::setw(20) << "(fft)" << (correlatorPPF ? fftKernel->itsCounter.stats : RunningStatistics()) << endl <<
        std::setw(20) << "(delayBp)" << delayAndBandPassKernel->itsCounter.stats << endl <<
        std::setw(20) << "(correlator)" << correlatorKernel->itsCounter.stats << endl <<
        std::setw(20) << "(samples)" << counters.samples.stats << endl <<
        std::setw(20) << "(visibilities)" << counters.visibilities.stats << endl);
    }

    CorrelatorSubbandProc::Counters::Counters(gpu::Context &context)
      :
    samples(context),
    visibilities(context)
    {}

    void CorrelatorSubbandProc::Flagger::propagateFlags(
      Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      CorrelatedData &output)
    {   
      // Object for storing transformed flags
      MultiDimArray<SparseSet<unsigned>, 2> flagsPerChannel(
        boost::extents[parset.settings.correlator.nrChannels][parset.settings.antennaFields.size()]);

      // First transform the flags to channel flags: taking in account 
      // reduced resolution in time and the size of the filter
      convertFlagsToChannelFlags(parset, inputFlags, flagsPerChannel);

      // Calculate the number of flafs per baseline and assign to
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

    template<typename T> void CorrelatorSubbandProc::Flagger::calcWeights(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      CorrelatedData &output)
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


    void CorrelatorSubbandProc::Flagger::calcWeights(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      CorrelatedData &output)
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


    void CorrelatorSubbandProc::Flagger::applyWeight(unsigned baseline, 
      unsigned channel, float weight, CorrelatedData &output)
    {
      for(unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; ++pol1)
        for(unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; ++pol2)
          output.visibilities[baseline][channel][pol1][pol2] *= weight;
    }


    template<typename T> void 
    CorrelatorSubbandProc::Flagger::applyWeights(Parset const &parset,
                                                 CorrelatedData &output)
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


    void CorrelatorSubbandProc::Flagger::applyWeights(Parset const &parset,
                                                 CorrelatedData &output)
    {
      switch (output.itsNrBytesPerNrValidSamples) {
        case 4:
          Flagger::applyWeights<uint32_t>(parset, output);  
          break;

        case 2:
          Flagger::applyWeights<uint16_t>(parset, output);  
          break;

        case 1:
          Flagger::applyWeights<uint8_t>(parset, output);  
          break;
      }
    }


    void CorrelatorSubbandProc::processSubband(SubbandProcInputData &input,
                                               SubbandProcOutputData &_output)
    {
      CorrelatedDataHostBuffer &output = 
        dynamic_cast<CorrelatedDataHostBuffer&>(_output);

      // Get the id of the block we are processing
      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;


      // ***************************************************
      // Copy data to the GPU 
      queue.writeBuffer(devA, input.inputSamples, counters.samples, true);
   
      if (ps.settings.delayCompensation.enabled) {
        const unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) 
        {
          queue.writeBuffer(
            delayAndBandPassKernel->delaysAtBegin, input.delaysAtBegin, false);
          queue.writeBuffer(
            delayAndBandPassKernel->delaysAfterEnd, input.delaysAfterEnd, false);
          queue.writeBuffer(
            delayAndBandPassKernel->phase0s, input.phase0s, false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      // *********************************************
      // Run the kernels

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
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      correlatorKernel->enqueue(input.blockID);

      // The GPU will be occupied for a while, do some calculations in the
      // background.

      // Propagate the flags.
      if (correlatorPPF) {
        // Put the history flags in front of the sample flags,
        // because Flagger::propagateFlags expects it that way.
        firFilterKernel->prefixHistoryFlags(
          input.inputFlags, input.blockID.subbandProcSubbandIdx);
      }

      Flagger::propagateFlags(ps, input.inputFlags, output);

      // Wait for the GPU to finish.
      queue.synchronize();

      // Read data back from the kernel
      queue.readBuffer(output, correlatorPPF ? devB : devA, counters.visibilities, true);

      // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling)
      {
        // assure that the queue is done so all events are fished
        queue.synchronize();

        // Update the counters
        if (correlatorPPF) {
          firFilterKernel->itsCounter.logTime();
          fftKernel->itsCounter.logTime();
        }

        delayAndBandPassKernel->itsCounter.logTime();
        correlatorKernel->itsCounter.logTime();

        counters.samples.logTime();
        counters.visibilities.logTime();
      }
    }


    bool CorrelatorSubbandProc::integrate(CorrelatedDataHostBuffer &output)
    {
      const size_t idx = output.blockID.subbandProcSubbandIdx;
      const size_t nblock = ps.settings.correlator.nrBlocksPerIntegration;
      
      // We don't want to copy the data if we don't need to integrate.
      if (nblock == 1) {
        output.setSequenceNumber(output.blockID.block);
        return true;
      }

      integratedData[idx].first++;

      if (integratedData[idx].first < nblock) {
        *integratedData[idx].second += output;
        return false;
      }
      else {
        output += *integratedData[idx].second;
        output.setSequenceNumber(output.blockID.block / nblock);
        integratedData[idx].first = 0;
        integratedData[idx].second->reset();
        return true;
      }
    }


    bool CorrelatorSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      CorrelatedDataHostBuffer &output = 
        dynamic_cast<CorrelatedDataHostBuffer&>(_output);

      if (!integrate(output)) {
        // Not yet done constructing output block 
        return false;
      }

      // The flags are already copied to the correct location
      // now the flagged amount should be applied to the visibilities
      Flagger::applyWeights(ps, output);  

      return true;
    }

  }
}


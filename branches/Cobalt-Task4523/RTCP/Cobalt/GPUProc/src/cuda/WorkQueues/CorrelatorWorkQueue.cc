//# CorrelatorWorkQueue.cc
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

#include "CorrelatorWorkQueue.h"

#include <cstring>
#include <algorithm>

#include <Common/LofarLogger.h>

#include <GPUProc/OpenMP_Lock.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Pipelines/CorrelatorPipelinePrograms.h>

namespace LOFAR
{
  namespace Cobalt
  {
    /* The data travels as follows:
     *
     *              > 1 channel/subband                  1 channel/subband
     *            ----------------------               ---------------------
     * [input]  -> devInput.inputSamples            -> devInput.inputSamples
     *             -> firFilterKernel
     *          -> devFilteredData
     *             -> fftKernel
     *          -> devFilteredData
     *             -> delayAndBandPassKernel           -> delayAndBandPassKernel
     *          -> devInput.inputSamples            -> devFilteredData
     *             -> correlatorKernel                 -> correlatorKernel
     *          -> devFilteredData                  -> devInput.inputSamples
     * [output] <- = visibilities                   <- = visibilities
     *
     * For #channels/subband == 1, skip the FIR and FFT kernels,
     * and provide the input in devFilteredData.
     */
    CorrelatorWorkQueue::CorrelatorWorkQueue(const Parset &parset,
      gpu::Context &context, CorrelatorPipelinePrograms &programs,
      FilterBank &filterBank)
    :
      WorkQueue( parset, context ),
      prevBlock(-1),
      prevSAP(-1),
      devInput(ps.nrChannelsPerSubband() == 1
               ? std::max(DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::INPUT_DATA),
                          CorrelatorKernel::bufferSize(ps, CorrelatorKernel::OUTPUT_DATA))
               : std::max(FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::INPUT_DATA),
                          CorrelatorKernel::bufferSize(ps, CorrelatorKernel::INPUT_DATA)),
               DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::DELAYS),
               DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::PHASE_OFFSETS),
               context),
      // devInput(ps.nrBeams(), ps.nrStations(), NR_POLARIZATIONS,
      //          ps.nrHistorySamples() + ps.nrSamplesPerSubband(),
      //          ps.nrBytesPerComplexSample(), context,

      //          // reserve enough space in inputSamples for the output of the delayAndBandPassKernel.
      //          // TODO: if ps.nrChannelsPerSubband() == 1, we only need this space, not the one above.
      //          ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>)
      //         ),

      devFilteredData(context, 
                      ps.nrChannelsPerSubband() == 1
                      ? CorrelatorKernel::bufferSize(ps, CorrelatorKernel::INPUT_DATA)
                      : std::max(DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::INPUT_DATA),
                                 CorrelatorKernel::bufferSize(ps, CorrelatorKernel::OUTPUT_DATA))),
      // devFilteredData(context, std::max(
      //                   ps.nrChannelsPerSubband() == 1 ?
      //                     // reserve enough space for the input of the delayAndBandPass kernel,
      //                     ps.nrSamplesPerSubband() * ps.nrStations() * NR_POLARIZATIONS * ps.nrBytesPerComplexSample() // from WorkQueueInputData::DeviceBuffers(...)
      //                   : // or, reserve enough space for the output of the FIR filter kernel,
      //                     ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>),
      //                   // and the correlator kernel.
      //                   ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>)
      //                 )),

      devFIRweights(context, FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::FILTER_WEIGHTS)),
      // devFIRweights(context, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float)),

      firFilterKernel(ps, programs.firFilterProgram,
                      devFilteredData, devInput.inputSamples, devFIRweights),
      fftKernel(ps, context, devFilteredData),
      delayAndBandPassKernel(ps, programs.delayAndBandPassProgram,
                             devInput.inputSamples,
                             devFilteredData,
                             devInput.delaysAtBegin,
                             devInput.delaysAfterEnd,
                             devInput.phaseOffsets,
                             queue),
#if defined USE_NEW_CORRELATOR
      correlateTriangleKernel(ps, programs.correlatorProgram,
                              devFilteredData, devInput.inputSamples),
      correlateRectangleKernel(ps, programs.correlatorProgram, 
                               devFilteredData, devInput.inputSamples)
#else
      correlatorKernel(ps, programs.correlatorProgram, 
                       devFilteredData, devInput.inputSamples)
#endif
    {
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < 3; ++i) {
        outputPool.free.append(new CorrelatedDataHostBuffer(
                ps.nrStations(),
                ps.nrChannelsPerSubband(),
                ps.integrationSteps(),
                context));
      }

      // create all the counters
#if defined USE_NEW_CORRELATOR
      addCounter("compute - cor.triangle");
      addCounter("compute - cor.rectangle");
#else
      addCounter("compute - correlator");
#endif

      addCounter("compute - FIR");
      addCounter("compute - delay/bp");
      addCounter("compute - FFT");
      addCounter("input - samples");
      addCounter("output - visibilities");

      // CPU timers are set by CorrelatorPipeline
      addTimer("CPU - read input");
      addTimer("CPU - process");
      addTimer("CPU - postprocess");
      addTimer("CPU - total");

      // GPU timers are set by us
      addTimer("GPU - total");
      addTimer("GPU - input");
      addTimer("GPU - output");
      addTimer("GPU - compute");
      addTimer("GPU - wait");

      // Copy the FIR filter and bandpass weights to the device.
      // Note that these constant weights are now (unnecessarily) stored on the
      // device for every workqueue. A single copy per device could be used, but
      // first verify that the device platform still allows workqueue overlap.
      size_t firWeightsSize = filterBank.getWeights().num_elements() * sizeof(float);
      gpu::HostMemory firWeights(context, firWeightsSize);
      std::memcpy(firWeights.get<void>(), filterBank.getWeights().origin(), firWeightsSize);
      queue.writeBuffer(devFIRweights, firWeights, true);
    }

    void CorrelatorWorkQueue::Flagger::propagateFlags(
      Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      CorrelatedData &output)
    {   
      // Object for storing transformed flags
      MultiDimArray<SparseSet<unsigned>, 2> flagsPerChannel(
        boost::extents[parset.nrChannelsPerSubband()][parset.nrStations()]);

      // First transform the flags to channel flags: taking in account 
      // reduced resolution in time and the size of the filter
      convertFlagsToChannelFlags(parset, inputFlags, flagsPerChannel);

      // Calculate the number of flafs per baseline and assign to
      // output object.
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

    namespace {
      unsigned baseline(unsigned stat1, unsigned stat2)
      {
        //baseline(stat1, stat2); This function should be moved to a helper class
        return stat2 * (stat2 + 1) / 2 + stat1;
      }
    }

    template<typename T> void CorrelatorWorkQueue::Flagger::calcWeights(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      CorrelatedData &output)
    {
      unsigned nrSamplesPerIntegration = parset.nrSamplesPerChannel();

      // loop the stations
      for (unsigned stat2 = 0; stat2 < parset.nrStations(); stat2 ++) {
        for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) {
          unsigned bl = baseline(stat1, stat2);

          // If there is a single channel then the index 0 contains real data
          if (parset.nrChannelsPerSubband() == 1) 
          {                                            
            // The number of invalid (flagged) samples is the union of the flagged samples in the two stations
            unsigned nrValidSamples = nrSamplesPerIntegration -
              (flagsPerChannel[0][stat1] | flagsPerChannel[0][stat2]).count();

            // Moet worden toegekend op de correlated dataobject
            output.nrValidSamples<T>(bl, 0) = nrValidSamples;
          } 
          else 
          {
            // channel 0 does not contain valid data
            output.nrValidSamples<T>(bl, 0) = 0; //channel zero, has zero valid samples

            for(unsigned ch = 1; ch < parset.nrChannelsPerSubband(); ch ++) 
            {
              // valid samples is total number of samples minus the union of the
              // Two stations.
              unsigned nrValidSamples = nrSamplesPerIntegration -
                (flagsPerChannel[ch][stat1] | flagsPerChannel[ch][stat2]).count();

              output.nrValidSamples<T>(bl, ch) = nrValidSamples;
            }
          }
        }
      }
    }

    // Instantiate required templates
    template void CorrelatorWorkQueue::Flagger::calcWeights<uint32_t>(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      CorrelatedData &output);
    template void CorrelatorWorkQueue::Flagger::calcWeights<uint16_t>(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      CorrelatedData &output);
    template void CorrelatorWorkQueue::Flagger::calcWeights<uint8_t>(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
      CorrelatedData &output);

    void CorrelatorWorkQueue::Flagger::applyWeight(unsigned baseline, 
      unsigned channel, float weight, CorrelatedData &output)
    {
      for(unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; ++pol1)
        for(unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; ++pol2)
          output.visibilities[baseline][channel][pol1][pol2] *= weight;
    }

    template<typename T> void CorrelatorWorkQueue::Flagger::applyWeights(Parset const &parset,
      CorrelatedData &output)
    {
      for (unsigned bl = 0; bl < output.itsNrBaselines; ++bl)
      {
        // Calculate the weights for the channels
        //
        // Channel 0 is already flagged according to specs, so we can simply
        // include it both for 1 and >1 channels/subband.
        for (unsigned ch = 0; ch < parset.nrChannelsPerSubband(); ch++) 
        {
          T nrValidSamples = output.nrValidSamples<T>(bl, ch);

          // If all samples flagged, weights is zero.
          // TODO: make a lookup table for the expensive division; measure first
          float weight = nrValidSamples ? 1e-6f / nrValidSamples : 0;  

          applyWeight(bl, ch, weight, output);
        }
      }
    }

    // Instantiate required templates
    template void CorrelatorWorkQueue::Flagger::applyWeights<uint32_t>(Parset const &parset,
      CorrelatedData &output);
    template void CorrelatorWorkQueue::Flagger::applyWeights<uint16_t>(Parset const &parset,
      CorrelatedData &output);
    template void CorrelatorWorkQueue::Flagger::applyWeights<uint8_t>(Parset const &parset,
      CorrelatedData &output);


    void CorrelatorWorkQueue::processSubband(WorkQueueInputData &input, StreamableData &_output)
    {
      CorrelatedDataHostBuffer &output = static_cast<CorrelatedDataHostBuffer&>(_output);

      timers["GPU - total"]->start();

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;

      {
        timers["GPU - input"]->start();

#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
        // If #ch/sb==1, copy the input to the device buffer where the DelayAndBandPass kernel reads from.
        if (ps.nrChannelsPerSubband() == 1) {
          queue.writeBuffer(devFilteredData, input.inputSamples, true);
        } else { // #ch/sb > 1
          queue.writeBuffer(devInput.inputSamples, input.inputSamples, true);
        }
//        counters["input - samples"]->doOperation(input.inputSamples.deviceBuffer.event, 0, 0, input.inputSamples.bytesize());

        timers["GPU - input"]->stop();
      }

      timers["GPU - compute"]->start();

      if (ps.delayCompensation())
      {
        unsigned SAP = ps.subbandToSAPmapping()[subband];

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
          queue.writeBuffer(devInput.delaysAtBegin,  input.delaysAtBegin,  false);
          queue.writeBuffer(devInput.delaysAfterEnd, input.delaysAfterEnd, false);
          queue.writeBuffer(devInput.phaseOffsets,   input.phaseOffsets,   false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      if (ps.nrChannelsPerSubband() > 1) {
        firFilterKernel.enqueue(queue/*, *counters["compute - FIR"]*/);
        fftKernel.enqueue(queue/*, *counters["compute - FFT"]*/);
      }

      // Even if we skip delay compensation and bandpass correction (rare),
      // run that kernel, as it also reorders the data for the correlator kernel.
      delayAndBandPassKernel.enqueue(queue/*, *counters["compute - delay/bp"]*/, subband);

#if defined USE_NEW_CORRELATOR
      correlateTriangleKernel.enqueue(queue/*, *counters["compute - cor.triangle"]*/);
      correlateRectangleKernel.enqueue(queue/*, *counters["compute - cor.rectangle"]*/);
#else
      correlatorKernel.enqueue(queue/*, *counters["compute - correlator"]*/);
#endif

      //queue.flush(); // CUDA doesn't have/need flush() (OpenCL)

      // ***** The GPU will be occupied for a while, do some calculations in the
      // background.

      // Propagate the flags.
      Flagger::propagateFlags(ps, input.inputFlags, output);

      // Wait for the GPU to finish.
      timers["GPU - wait"]->start();
      queue.synchronize();
      timers["GPU - wait"]->stop();

      timers["GPU - compute"]->stop();

      {
        timers["GPU - output"]->start();

#ifdef USE_B7015
        OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
        queue.readBuffer(output, devFilteredData, true);
        // now perform weighting of the data based on the number of valid samples; TODO???

//        counters["output - visibilities"]->doOperation(output.deviceBuffer.event, 0, output.bytesize(), 0);

        timers["GPU - output"]->stop();
      }

      timers["GPU - total"]->stop();
    }


    void CorrelatorWorkQueue::postprocessSubband(StreamableData &_output)
    {
      CorrelatedDataHostBuffer &output = static_cast<CorrelatedDataHostBuffer&>(_output);

      // The flags are already copied to the correct location
      // now the flagged amount should be applied to the visibilities
      switch (output.itsNrBytesPerNrValidSamples) {
        case 4:
          Flagger::applyWeights<uint32_t>(ps, output);  
          break;

        case 2:
          Flagger::applyWeights<uint16_t>(ps, output);  
          break;

        case 1:
          Flagger::applyWeights<uint8_t>(ps, output);  
          break;
      }
    }

  }
}


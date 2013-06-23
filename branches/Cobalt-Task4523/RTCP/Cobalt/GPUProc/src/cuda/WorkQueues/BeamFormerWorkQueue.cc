//# BeamFormerWorkQueue.cc
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

#include "BeamFormerWorkQueue.h"

#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/Parset.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormerWorkQueue::BeamFormerWorkQueue(const Parset &parset,
      gpu::Context &context, FilterBank &filterBank)
    :
      WorkQueue( parset, context ),
      prevBlock(-1),
      prevSAP(-1),

      // NOTE: Make sure the history samples are dealt with properly until the
      // FIR, which the beam former does in a later stage!
      //
      // NOTE: Sizes are probably completely wrong.
      devInput(ps.nrChannelsPerSubband() == 1
               ? DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::INPUT_DATA)
               : FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::INPUT_DATA),
               DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::DELAYS),
               DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::PHASE_OFFSETS),
               context),

      devFilteredData(context, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::INPUT_DATA)),
      devFIRweights(context, FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::FILTER_WEIGHTS)),
      devBandPassCorrectionWeights(context, 
                                   DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::BAND_PASS_CORRECTION_WEIGHTS))
#if 0
      firFilterKernel(ps, programs.firFilterProgram,
                      devFilteredData, devInput.inputSamples, devFIRweights),
      fftKernel(ps, context, devFilteredData),
      delayAndBandPassKernel(ps, programs.delayAndBandPassProgram,
                             devInput.inputSamples,
                             devFilteredData,
                             devInput.delaysAtBegin,
                             devInput.delaysAfterEnd,
                             devInput.phaseOffsets,
                             devBandPassCorrectionWeights)
#endif
#if 0
      devFilteredData(queue, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
      bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devCorrectedData(queue, CL_MEM_READ_WRITE, ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>)),
      beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devComplexVoltages(queue, CL_MEM_READ_WRITE, ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>)),
      //transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE)
      transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE),
      DMs(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY),

      intToFloatKernel(ps, queue, pipeline.intToFloatProgram, devFilteredData, inputSamples),
      beamFormerKernel(ps, pipeline.beamFormerProgram, devComplexVoltages, devCorrectedData, beamFormerWeights),
      transposeKernel(ps, pipeline.transposeProgram, transposedComplexVoltages, devComplexVoltages),
      dedispersionForwardFFTkernel(ps, pipeline.context, transposedComplexVoltages),
      dedispersionBackwardFFTkernel(ps, pipeline.context, transposedComplexVoltages),
      dedispersionChirpKernel(ps, pipeline.dedispersionChirpProgram, queue, transposedComplexVoltages, DMs)
#endif
    {
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < 3; ++i) {
        outputPool.free.append(new BeamFormedData(
                4,
                ps.nrChannelsPerSubband(),
                ps.integrationSteps(),
                context));
      }

      // Copy the FIR filter and bandpass weights to the device.
      // Note that these constant weights are now (unnecessarily) stored on the
      // device for every workqueue. A single copy per device could be used, but
      // first verify that the device platform still allows workqueue overlap.
      size_t firWeightsSize = filterBank.getWeights().num_elements() * sizeof(float);
      gpu::HostMemory firWeights(context, firWeightsSize);
      std::memcpy(firWeights.get<void>(), filterBank.getWeights().origin(), firWeightsSize);
      queue.writeBuffer(devFIRweights, firWeights, true);

      if (ps.correctBandPass())
      {
        gpu::HostMemory bpWeights(context, ps.nrChannelsPerSubband() * sizeof(float));
        BandPass::computeCorrectionFactors(bpWeights.get<float>(),
                                           ps.nrChannelsPerSubband());
        queue.writeBuffer(devBandPassCorrectionWeights, bpWeights, true);
      }
    }


    void BeamFormerWorkQueue::processSubband(WorkQueueInputData &input, StreamableData &_output)
    {
      (void)input;
      (void)_output;

#if 0
      //queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, firWeightsSize, firFilterWeights);
      bandPassCorrectionWeights.hostToDevice(true);
      DMs.hostToDevice(true);

        delaysAtBegin.hostToDevice(false);
        delaysAfterEnd.hostToDevice(false);
        phaseOffsets.hostToDevice(false);
        beamFormerWeights.hostToDevice(false);

          {
#if defined USE_B7015
            OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
            inputSamples.hostToDevice(true);
//            pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
          }

          {
            if (ps.nrChannelsPerSubband() > 1) {
              intToFloatKernel.enqueue(queue/*, pipeline.intToFloatCounter*/);
              fftKernel.enqueue(queue/*, pipeline.fftCounter*/);
            }

            delayAndBandPassKernel.enqueue(queue/*, pipeline.delayAndBandPassCounter*/, subband);
            beamFormerKernel.enqueue(queue/*, pipeline.beamFormerCounter*/);
            transposeKernel.enqueue(queue/*, pipeline.transposeCounter*/);
            dedispersionForwardFFTkernel.enqueue(queue/*, pipeline.dedispersionForwardFFTcounter*/);
            dedispersionChirpKernel.enqueue(queue/*, pipeline.dedispersionChirpCounter*/, ps.subbandToFrequencyMapping()[subband]);
            dedispersionBackwardFFTkernel.enqueue(queue/*, pipeline.dedispersionBackwardFFTcounter*/);

            queue.synchronize();
          }

          //queue.enqueueReadBuffer(devComplexVoltages, CL_TRUE, 0, hostComplexVoltages.bytesize(), hostComplexVoltages.origin());
          //dedispersedData.deviceToHost(true);
        }
      }
#endif
    }

    void BeamFormerWorkQueue::postprocessSubband(StreamableData &_output)
    {
      (void)_output;
    }
  }
}


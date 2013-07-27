//# BeamFormerSubbandProc.cc
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

#include "BeamFormerSubbandProc.h"

#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/Parset.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormerSubbandProc::BeamFormerSubbandProc(const Parset &parset,
      gpu::Context &context, BeamFormerFactories &factories)
    :
      SubbandProc( parset, context ),
      prevBlock(-1),
      prevSAP(-1),

      // NOTE: Make sure the history samples are dealt with properly until the
      // FIR, which the beam former does in a later stage!
      devInput(std::max(
                 factories.intToFloat.bufferSize(IntToFloatKernel::INPUT_DATA),
                 factories.beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA)
               ),
               factories.delayCompensation.bufferSize(DelayAndBandPassKernel::DELAYS),
               factories.delayCompensation.bufferSize(DelayAndBandPassKernel::PHASE_OFFSETS),
               context),
      devA(devInput.inputSamples),
      devB(context, devA.size()),
      devNull(context, 1),

      // intToFloat: input -> B
      intToFloatBuffers(devInput.inputSamples, devB),
      intToFloatKernel(factories.intToFloat.create(queue, intToFloatBuffers)),

      // FFT: B -> B
      firstFFT(context, DELAY_COMPENSATION_NR_CHANNELS, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / DELAY_COMPENSATION_NR_CHANNELS, true, devB),

      // delayComp: B -> A
      delayCompensationBuffers(devB, devA, devInput.delaysAtBegin, devInput.delaysAfterEnd, devInput.phaseOffsets, devNull),
      delayCompensationKernel(factories.delayCompensation.create(queue, delayCompensationBuffers)),

      // FFT: A -> A
      secondFFT(context, BEAM_FORMER_NR_CHANNELS / DELAY_COMPENSATION_NR_CHANNELS, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / BEAM_FORMER_NR_CHANNELS / DELAY_COMPENSATION_NR_CHANNELS, true, devA),

      // bandPass: A -> B
      devBandPassCorrectionWeights(context, factories.correctBandPass.bufferSize(DelayAndBandPassKernel::BAND_PASS_CORRECTION_WEIGHTS)),
      correctBandPassBuffers(devA, devB, devNull, devNull, devNull, devBandPassCorrectionWeights),
      correctBandPassKernel(factories.correctBandPass.create(queue, correctBandPassBuffers)),

      // beamForm: B -> A
      devBeamFormerWeights(context, factories.beamFormer.bufferSize(BeamFormerKernel::BEAM_FORMER_WEIGHTS)),
      beamFormerBuffers(devB, devA, devBeamFormerWeights),
      beamFormerKernel(factories.beamFormer.create(queue, beamFormerBuffers)),

      // transpose after beamforming: A -> B
      transposeBuffers(devA, devB),
      transposeKernel(factories.transpose.create(queue, transposeBuffers)),

      // inverse FFT: B -> B
      inverseFFT(context, BEAM_FORMER_NR_CHANNELS, ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / BEAM_FORMER_NR_CHANNELS, false, devB),

      // FIR filter: B -> A
      // TODO: provide history samples separately
      // TODO: do a FIR for each individual TAB!!
      devFilterWeights(context, factories.firFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)),
      firFilterBuffers(devB, devA, devFilterWeights),
      firFilterKernel(factories.firFilter.create(queue, firFilterBuffers)),

      // final FFT: A -> A
      finalFFT(context, 16, ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / 16, true, devA)
    {
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < 3; ++i) {
        outputPool.free.append(new BeamFormedData(
                4,
                ps.nrChannelsPerSubband(),
                ps.integrationSteps(),
                context));
      }

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
    }


    void BeamFormerSubbandProc::processSubband(SubbandProcInputData &input, StreamableData &_output)
    {
      (void)_output;

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;

      {
#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
        queue.writeBuffer(devInput.inputSamples, input.inputSamples, true);
//      counters["input - samples"]->doOperation(input.inputSamples.deviceBuffer.event, 0, 0, input.inputSamples.bytesize());
      }

      if (ps.delayCompensation())
      {
        unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
          queue.writeBuffer(devInput.delaysAtBegin,  input.delaysAtBegin,  false);
          queue.writeBuffer(devInput.delaysAfterEnd, input.delaysAfterEnd, false);
          queue.writeBuffer(devInput.phaseOffsets,   input.phaseOffsets,   false);
          // beamFormerWeights.hostToDevice(false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      intToFloatKernel->enqueue();

      firstFFT.enqueue(queue);
      delayCompensationKernel->enqueue(queue,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      secondFFT.enqueue(queue);
      correctBandPassKernel->enqueue(queue,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      beamFormerKernel->enqueue();
      transposeKernel->enqueue();

      inverseFFT.enqueue(queue);
      firFilterKernel->enqueue();
      finalFFT.enqueue(queue);

      queue.synchronize();

      // queue.readBuffer()
    }

    void BeamFormerSubbandProc::postprocessSubband(StreamableData &_output)
    {
      (void)_output;
    }
  }
}


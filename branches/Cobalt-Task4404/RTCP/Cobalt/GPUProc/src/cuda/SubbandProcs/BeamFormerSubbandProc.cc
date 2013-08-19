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

#include <iomanip>
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
      counters(context),
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
      // TODO: support >1 SAP
      devBeamFormerWeights(context, factories.beamFormer.bufferSize(BeamFormerKernel::BEAM_FORMER_WEIGHTS)),
      beamFormerBuffers(devB, devA, devBeamFormerWeights),
      beamFormerKernel(factories.beamFormer.create(queue, beamFormerBuffers)),

      // transpose after beamforming: A -> B
      transposeBuffers(devA, devB),
      transposeKernel(factories.transpose.create(queue, transposeBuffers)),

      // inverse FFT: B -> B
      inverseFFT(context, BEAM_FORMER_NR_CHANNELS, ps.settings.beamFormer.maxNrTABsPerSAP() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / BEAM_FORMER_NR_CHANNELS, false, devB),

      // FIR filter: B -> A
      // TODO: provide history samples separately
      // TODO: do a FIR for each individual TAB!!
      devFilterWeights(context, factories.firFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)),
      firFilterBuffers(devB, devA, devFilterWeights),
      firFilterKernel(factories.firFilter.create(queue, firFilterBuffers)),

      // final FFT: A -> A
      finalFFT(context, ps.settings.beamFormer.coherentSettings.nrChannels, ps.settings.beamFormer.maxNrTABsPerSAP() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / ps.settings.beamFormer.coherentSettings.nrChannels, true, devA),

      // result buffer
      devResult(ps.settings.beamFormer.coherentSettings.nrChannels > 1 ? devA : devB)
    {
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < 3; ++i) {
        outputPool.free.append(new BeamFormedData(
                ps.settings.beamFormer.maxNrTABsPerSAP() * ps.settings.beamFormer.coherentSettings.nrStokes,
                ps.settings.beamFormer.coherentSettings.nrChannels,
                ps.settings.beamFormer.coherentSettings.nrSamples(ps.nrSamplesPerSubband()),
                context));
      }

      //// CPU timers are set by CorrelatorPipeline
      //addTimer("CPU - read input");
      //addTimer("CPU - process");
      //addTimer("CPU - postprocess");
      //addTimer("CPU - total");

      //// GPU timers are set by us
      //addTimer("GPU - total");
      //addTimer("GPU - input");
      //addTimer("GPU - output");
      //addTimer("GPU - compute");
      //addTimer("GPU - wait");
    }

    BeamFormerSubbandProc::Counters::Counters(gpu::Context &context)
      :
    intToFloat(context),
    firstFFT(context),
    delayBp(context),
    secondFFT(context),
    correctBandpass(context),
    beamformer(context),
    transpose(context),
    inverseFFT(context),
    firFilterKernel(context),
    finalFFT(context),
    samples(context),
    visibilities(context)
    {
    }

    void BeamFormerSubbandProc::Counters::printStats()
    {     

      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR("**** CorrelatorSubbandProc GPU mean and stDev ****" <<
        std::setw(20) << "(intToFloat)" << intToFloat.stats << endl <<
        std::setw(20) << "(firstFFT)" << firstFFT.stats << endl <<
        std::setw(20) << "(delayBp)" << delayBp.stats << endl <<
        std::setw(20) << "(secondFFT)" << secondFFT.stats << endl <<
        std::setw(20) << "(correctBandpass)" << correctBandpass.stats << endl <<
        std::setw(20) << "(beamformer)" << beamformer.stats << endl <<
        std::setw(20) << "(transpose)" << transpose.stats << endl <<
        std::setw(20) << "(inverseFFT)" << inverseFFT.stats << endl <<
        std::setw(20) << "(firFilterKernel)" << firFilterKernel.stats << endl <<
        std::setw(20) << "(finalFFT)" << finalFFT.stats << endl <<
        std::setw(20) << "(samples)" << samples.stats << endl <<
        std::setw(20) << "(visibilities)" << visibilities.stats << endl);

    }

    void BeamFormerSubbandProc::processSubband(SubbandProcInputData &input, StreamableData &_output)
    {
      // We are in the BeamFormerSubbandProc, we know that we are beamforming.
      // therefore gogo static cast ( see outputPool)
      BeamFormedData &output = static_cast<BeamFormedData&>(_output);

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;

      queue.writeBuffer(devInput.inputSamples, input.inputSamples, true);

      if (ps.delayCompensation())
      {
        unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
          queue.writeBuffer(devInput.delaysAtBegin,  input.delaysAtBegin,  false);
          queue.writeBuffer(devInput.delaysAfterEnd, input.delaysAfterEnd, false);
          queue.writeBuffer(devInput.phaseOffsets,   input.phaseOffsets,   false);

          // TODO: propagate beam-former weights to here 
          //queue.writeBuffer(devBeamFormerWeights,    ,   false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      //****************************************
      // Enqueue the kernels
      intToFloatKernel->enqueue(counters.intToFloat);

      firstFFT.enqueue(queue, counters.firstFFT);
      delayCompensationKernel->enqueue(queue, counters.delayBp,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      secondFFT.enqueue(queue, counters.secondFFT);
      correctBandPassKernel->enqueue(queue, counters.correctBandpass,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      beamFormerKernel->enqueue(counters.beamformer);
      transposeKernel->enqueue(counters.transpose);

      inverseFFT.enqueue(queue, counters.inverseFFT);

      if (ps.settings.beamFormer.coherentSettings.nrChannels > 1) {
        firFilterKernel->enqueue( counters.firFilterKernel);
        finalFFT.enqueue(queue, counters.finalFFT);
      }

      // TODO: Propagate flags

      queue.synchronize();

      queue.readBuffer(output, devResult, true);

            // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling)
      {
        // assure that the queue is done so all events are fished
        queue.synchronize();
        // Update the counters
        if (ps.settings.beamFormer.coherentSettings.nrChannels > 1) 
        {
          counters.firFilterKernel.logTime();
          counters.finalFFT.logTime();
        }
        counters.intToFloat.logTime();
        counters.firstFFT.logTime();
        counters.delayBp.logTime();
        counters.secondFFT.logTime();
        counters.correctBandpass.logTime();
        counters.beamformer.logTime();
        counters.transpose.logTime();
        counters.inverseFFT.logTime();
      }
    }

    void BeamFormerSubbandProc::postprocessSubband(StreamableData &_output)
    {
      (void)_output;
    }
  }
}



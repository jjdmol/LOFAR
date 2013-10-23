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
      gpu::Context &context, BeamFormerFactories &factories, size_t nrSubbandsPerSubbandProc)
    :
      SubbandProc(parset, context, nrSubbandsPerSubbandProc),
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
      // coherent stokes buffers
      devA(devInput.inputSamples),
      devB(context, devA.size()),
      // Buffers for incoherent stokes
      devC(context, devA.size()),
      devD(context, devA.size()),
      devNull(context, 1),

      // intToFloat: input -> B
      intToFloatBuffers(devInput.inputSamples, devB),
      intToFloatKernel(factories.intToFloat.create(queue, intToFloatBuffers)),

      // FFT: B -> B
      firstFFT(queue, DELAY_COMPENSATION_NR_CHANNELS, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / DELAY_COMPENSATION_NR_CHANNELS, true, devB),

      // delayComp: B -> A
      delayCompensationBuffers(devB, devA, devInput.delaysAtBegin, devInput.delaysAfterEnd, devInput.phaseOffsets, devNull),
      delayCompensationKernel(factories.delayCompensation.create(queue, delayCompensationBuffers)),

      // FFT: A -> A
      secondFFT(queue, BEAM_FORMER_NR_CHANNELS / DELAY_COMPENSATION_NR_CHANNELS, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() / (BEAM_FORMER_NR_CHANNELS / DELAY_COMPENSATION_NR_CHANNELS), true, devA),

      // bandPass: A -> B
      devBandPassCorrectionWeights(context, factories.correctBandPass.bufferSize(DelayAndBandPassKernel::BAND_PASS_CORRECTION_WEIGHTS)),
      correctBandPassBuffers(devA, devB, devNull, devNull, devNull, devBandPassCorrectionWeights),
      correctBandPassKernel(factories.correctBandPass.create(queue, correctBandPassBuffers)),

      //**************************************************************
      //coherent stokes
      // beamForm: B -> A
      // TODO: support >1 SAP
      devBeamFormerDelays(context, factories.beamFormer.bufferSize(BeamFormerKernel::BEAM_FORMER_DELAYS)),
      beamFormerBuffers(devB, devA, devBeamFormerDelays),
      beamFormerKernel(factories.beamFormer.create(queue, beamFormerBuffers)),

      // transpose after beamforming: A -> B
      transposeBuffers(devA, devB),
      transposeKernel(factories.transpose.create(queue, transposeBuffers)),

      // inverse FFT: B -> B
      inverseFFT(queue, BEAM_FORMER_NR_CHANNELS, 
          ps.settings.beamFormer.maxNrTABsPerSAP() * NR_POLARIZATIONS * 
          ps.nrSamplesPerSubband() / BEAM_FORMER_NR_CHANNELS, false, devB),

      // FIR filter: B -> A
      // TODO: provide history samples separately
      // TODO: do a FIR for each individual TAB!!
      devFilterWeights(context, factories.firFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)),
      devFilterHistoryData(context, factories.firFilter.bufferSize(FIR_FilterKernel::HISTORY_DATA)),
      firFilterBuffers(devB, devA, devFilterWeights, devFilterHistoryData),
      firFilterKernel(factories.firFilter.create(queue, firFilterBuffers)),

      // final FFT: A -> A
      finalFFT(queue, ps.settings.beamFormer.coherentSettings.nrChannels,
          ps.settings.beamFormer.maxNrTABsPerSAP() * NR_POLARIZATIONS * 
          ps.nrSamplesPerSubband() / ps.settings.beamFormer.coherentSettings.nrChannels,
          true, devA),

      // coherentStokes: 1ch: A -> B, Nch: B -> A
      coherentStokesBuffers(
          ps.settings.beamFormer.coherentSettings.nrChannels > 1 ? devA : devB,
          ps.settings.beamFormer.coherentSettings.nrChannels > 1 ? devB : devA),
      coherentStokesKernel(factories.coherentStokes.create(queue, coherentStokesBuffers)),

      // result buffer
      devResult(ps.settings.beamFormer.coherentSettings.nrChannels > 1 ? devB : devA),
      //**************************************************************
      //incoherent stokes
      // TODO: Add a transpose
      // inverse FFT: C -> C
      incoherentInverseFFT(queue, BEAM_FORMER_NR_CHANNELS,
                 NR_POLARIZATIONS * 
                 ps.nrSamplesPerSubband() / BEAM_FORMER_NR_CHANNELS, false, devC),

      // FIR filter: C -> D
      // TODO: provide history samples separately
      // TODO: do a FIR for each individual TAB!!
      devIncoherentFilterWeights(context,
           factories.incoherentFirFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)),
      devIncoherentFilterHistoryData(context,
           factories.incoherentFirFilter.bufferSize(FIR_FilterKernel::HISTORY_DATA)),
      incoherentFirFilterBuffers(devC, devD,
              devIncoherentFilterWeights, devIncoherentFilterHistoryData),
      incoherentFirFilterKernel(
          factories.incoherentFirFilter.create(
                queue, incoherentFirFilterBuffers)),

      // final FFT: D -> D
      incoherentFinalFFT(queue, ps.settings.beamFormer.incoherentSettings.nrChannels,
                                NR_POLARIZATIONS * ps.nrSamplesPerSubband() / 
                                ps.settings.beamFormer.incoherentSettings.nrChannels, true, devD),

      // incoherentstokes kernel
      incoherentStokesBuffers(
          ps.settings.beamFormer.incoherentSettings.nrChannels > 1 ? devD : devC,
          ps.settings.beamFormer.incoherentSettings.nrChannels > 1 ? devC : devD),
      incoherentStokesKernel(
          factories.incoherentStokes.create(queue, incoherentStokesBuffers)),

      devIncoherentStokes(ps.settings.beamFormer.incoherentSettings.nrChannels > 1 ? devC : devD)
    {
      // initialize history data
      devFilterHistoryData.set(0);

      // TODO For now we only allow pure coherent and incoherent runs
      // count the number of coherent and incoherent saps
      size_t nrCoherent = 0;
      size_t nrIncoherent = 0;
      for (size_t idx_sap = 0; idx_sap < ps.settings.beamFormer.SAPs.size(); ++idx_sap)
      {
        if (ps.settings.beamFormer.SAPs[idx_sap].nrIncoherent != 0)
          nrIncoherent++;
        if (ps.settings.beamFormer.SAPs[idx_sap].nrCoherent != 0)
          nrCoherent++;
      }

      // raise exception if the parset contained an incorrect configuration
      if (nrCoherent != 0 && nrIncoherent != 0)
        THROW(GPUProcException, 
           "Parset contained both incoherent and coherent stokes SAPS. This is not supported");

      if (nrCoherent)
        coherentBeamformer = true;
      else
        coherentBeamformer = false;
      
      // put enough objects in the outputPool to operate
      
      for (size_t i = 0; i < std::max(3UL, 2 * nrSubbandsPerSubbandProc); ++i) 
      {
        //**********************************************************************
        // Coherent/incoheren switch
        if (coherentBeamformer) 
          outputPool.free.append(new BeamFormedData(
                ps.settings.beamFormer.maxNrTABsPerSAP() * ps.settings.beamFormer.coherentSettings.nrStokes,
                ps.settings.beamFormer.coherentSettings.nrChannels,
                ps.settings.beamFormer.coherentSettings.nrSamples(ps.nrSamplesPerSubband()),
                context));
        else
          outputPool.free.append(new BeamFormedData(
                    ps.settings.beamFormer.incoherentSettings.nrStokes,
                ps.settings.beamFormer.incoherentSettings.nrChannels,
                ps.settings.beamFormer.incoherentSettings.nrSamples(ps.nrSamplesPerSubband()),
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
    coherentStokes(context),
    incoherentInverseFFT(context),
    incoherentFirFilterKernel(context),
    incoherentFinalFFT(context),
    incoherentStokes(context),
    samples(context),
    visibilities(context),
    copyBuffers(context),
    incoherentOutput(context)
    {
    }

    void BeamFormerSubbandProc::Counters::printStats()
    {     

      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR("**** BeamFormerSubbandProc GPU mean and stDev ****" << endl <<
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
        std::setw(20) << "(coherentStokes)" << coherentStokes.stats << endl <<
        std::setw(20) << "(samples)" << samples.stats << endl <<       
        std::setw(20) << "(visibilities)" << visibilities.stats << endl <<

        std::setw(20) << "(copyBuffers)" << copyBuffers.stats << endl <<
        std::setw(20) << "(incoherentOutput )" << incoherentOutput.stats << endl <<
        std::setw(20) << "(incoherentInverseFFT)" << incoherentInverseFFT.stats << endl <<
        std::setw(20) << "(incoherentFirFilterKernel)" << incoherentFirFilterKernel.stats << endl <<
        std::setw(20) << "(incoherentFinalFFT)" << incoherentFinalFFT.stats << endl <<
        std::setw(20) << "(incoherentStokes)" <<  incoherentStokes.stats << endl );

    }

    void BeamFormerSubbandProc::processSubband(SubbandProcInputData &input,
      StreamableData &_output)
    {
      // We are in the BeamFormerSubbandProc, we know that we are beamforming.
      // therefore gogo static cast ( see outputPool)
      BeamFormedData &output = static_cast<BeamFormedData&>(_output);
      BeamFormedData &incoherentOutput = static_cast<BeamFormedData&>(_output);

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;
      queue.writeBuffer(devInput.inputSamples, input.inputSamples, counters.samples, true);

      if (ps.delayCompensation())
      {
        unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
          queue.writeBuffer(devInput.delaysAtBegin,  input.delaysAtBegin,  false);
          queue.writeBuffer(devInput.delaysAfterEnd, input.delaysAfterEnd, false);
          queue.writeBuffer(devInput.phaseOffsets,   input.phaseOffsets,   false);

          queue.writeBuffer(devBeamFormerDelays,     input.tabDelays,      false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      //****************************************
      // Enqueue the kernels
      // Note: make sure to call the right enqueue() for each kernel.
      // Otherwise, a kernel arg may not be set...
      intToFloatKernel->enqueue(input.blockID, counters.intToFloat);

      firstFFT.enqueue(input.blockID, counters.firstFFT);
      delayCompensationKernel->enqueue(input.blockID, counters.delayBp,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      secondFFT.enqueue(input.blockID, counters.secondFFT);
      correctBandPassKernel->enqueue(input.blockID, counters.correctBandpass,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      // TODO: To allow the copy of data to new buffer we need a sync here?  
      queue.copyBuffer(devC, devB, counters.copyBuffers, true);

      // ********************************************************************
      // coherent stokes kernels
      if (coherentBeamformer)
      {
        beamFormerKernel->enqueue(input.blockID, counters.beamformer,
          ps.settings.subbands[subband].centralFrequency,
          ps.settings.subbands[subband].SAP);

        transposeKernel->enqueue(input.blockID, counters.transpose);

        inverseFFT.enqueue(input.blockID, counters.inverseFFT);

        if (ps.settings.beamFormer.coherentSettings.nrChannels > 1) 
        {
          firFilterKernel->enqueue(input.blockID, 
            counters.firFilterKernel,
            input.blockID.subbandProcSubbandIdx);
          finalFFT.enqueue(input.blockID, counters.finalFFT);
        }
        
        coherentStokesKernel->enqueue(input.blockID, counters.coherentStokes);
      }
      else
      {
        // ********************************************************************
        // incoherent stokes kernels
        incoherentInverseFFT.enqueue(input.blockID, counters.incoherentInverseFFT);

        if (ps.settings.beamFormer.incoherentSettings.nrChannels > 1) 
        {
          incoherentFirFilterKernel->enqueue(input.blockID, counters.incoherentFirFilterKernel,
            input.blockID.subbandProcSubbandIdx);
          incoherentFinalFFT.enqueue(input.blockID, counters.incoherentFinalFFT);
        }

        incoherentStokesKernel->enqueue(input.blockID, counters.incoherentStokes);
        // TODO: Propagate flags
      }
      queue.synchronize();

      if (coherentBeamformer)
        queue.readBuffer(output, devResult, counters.visibilities, true);
      else
        queue.readBuffer(incoherentOutput, devIncoherentStokes, counters.incoherentOutput, true);

            // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling)
      {
        // assure that the queue is done so all events are fished
        queue.synchronize();
        // Update the counters
        counters.intToFloat.logTime();
        counters.firstFFT.logTime();
        counters.delayBp.logTime();
        counters.secondFFT.logTime();
        counters.correctBandpass.logTime();

        counters.samples.logTime();
        counters.copyBuffers.logTime();
        if (coherentBeamformer)
        {
          if (ps.settings.beamFormer.coherentSettings.nrChannels > 1) 
          {
            counters.firFilterKernel.logTime();
            counters.finalFFT.logTime();
          }

          counters.beamformer.logTime();
          counters.transpose.logTime();
          counters.inverseFFT.logTime();
          counters.coherentStokes.logTime();
          counters.visibilities.logTime();
        }
        else
        {
          counters.incoherentInverseFFT.logTime();
          if (ps.settings.beamFormer.incoherentSettings.nrChannels > 1) 
          {
            counters.incoherentFirFilterKernel.logTime();
            counters.incoherentFinalFFT.logTime();
          }
          counters.incoherentStokes.logTime();
          counters.incoherentOutput.logTime();
        }
      }
    }

    void BeamFormerSubbandProc::postprocessSubband(StreamableData &_output)
    {
      (void)_output;
    }

  }
}

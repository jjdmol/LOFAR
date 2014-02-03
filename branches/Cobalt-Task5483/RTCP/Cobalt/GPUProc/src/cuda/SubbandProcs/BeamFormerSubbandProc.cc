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
#include "BeamFormerFactories.h"

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

    BeamFormedData::BeamFormedData(unsigned nrStokes,
                                   unsigned nrChannels,
                                   size_t nrSamples,
                                   gpu::Context &context) :
    MultiDimArrayHostBuffer<float, 4>(
      boost::extents[1][nrStokes][nrSamples][nrChannels], context, 0)
    {
    }

    BeamFormedData::BeamFormedData(unsigned nrStokes,
                                   unsigned nrChannels,
                                   size_t nrSamples, 
                                   unsigned nrTabs, 
                                   gpu::Context &context) :
      MultiDimArrayHostBuffer<float, 4>(
          boost::extents[nrTabs][nrStokes][nrSamples][nrChannels], context, 0)
    {
    }

    void BeamFormedData::readData(Stream *str, unsigned)
    {
      str->read(origin(), size());
    }

    void BeamFormedData::writeData(Stream *str, unsigned)
    {
      str->write(origin(), size());
    }


    BeamFormerSubbandProc::BeamFormerSubbandProc(
      const Parset &parset,
      gpu::Context &context,
      BeamFormerFactories &factories,
      size_t nrSubbandsPerSubbandProc)
    :
      SubbandProc(parset, context, nrSubbandsPerSubbandProc),
      counters(context),
      prevBlock(-1),
      prevSAP(-1),

      // NOTE: Make sure the history samples are dealt with properly until the
      // FIR, which the beam former does in a later stage!
      devInput(
        std::max(
          factories.intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA),
          factories.beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA)),
        factories.delayCompensation.bufferSize(
          DelayAndBandPassKernel::DELAYS),
        factories.delayCompensation.bufferSize(
          DelayAndBandPassKernel::PHASE_ZEROS),
        context),
      // coherent stokes buffers
      devA(devInput.inputSamples),
      devB(context, devA.size()),
      // Buffers for incoherent stokes
      devC(context, devA.size()),
      devD(context, devA.size()),
      devE(context, factories.incoherentStokes.bufferSize(
             IncoherentStokesKernel::OUTPUT_DATA)),
      devNull(context, 1),

      // intToFloat: input -> B
      intToFloatBuffers(devInput.inputSamples, devB),
      intToFloatKernel(factories.intToFloat.create(queue, intToFloatBuffers)),

      // FFTShift: B -> B
      firstFFTShiftBuffers(devB, devB),
      firstFFTShiftKernel(
        factories.fftShift.create(queue, firstFFTShiftBuffers)),

      // FFT: B -> B
      firstFFT(queue,
        ps.settings.beamFormer.nrDelayCompensationChannels,
        (ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
         ps.settings.beamFormer.nrDelayCompensationChannels),
        true, devB),

      // delayComp: B -> A
      delayCompensationBuffers(devB, devA, devInput.delaysAtBegin,
                               devInput.delaysAfterEnd,
                               devInput.phase0s, devNull),
      delayCompensationKernel(
        factories.delayCompensation.create(queue, delayCompensationBuffers)),

      // FFTShift: A -> A
      secondFFTShiftBuffers(devA, devA),
      secondFFTShiftKernel(
        factories.fftShift.create(queue, secondFFTShiftBuffers)),

      // FFT: A -> A
      secondFFT(queue,
        ps.settings.beamFormer.nrHighResolutionChannels /
        ps.settings.beamFormer.nrDelayCompensationChannels,
        (ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
         (ps.settings.beamFormer.nrHighResolutionChannels /
          ps.settings.beamFormer.nrDelayCompensationChannels)),
        true, devA),

      // bandPass: A -> B
      devBandPassCorrectionWeights(
        context,
        factories.bandPassCorrection.bufferSize(
          BandPassCorrectionKernel::BAND_PASS_CORRECTION_WEIGHTS)),
      bandPassCorrectionBuffers(
        devA, devB, devBandPassCorrectionWeights),
      bandPassCorrectionKernel(
        factories.bandPassCorrection.create(queue, bandPassCorrectionBuffers)),

      //**************************************************************
      //coherent stokes
      //bool:
      outputComplexVoltages(  
        ps.settings.beamFormer.coherentSettings.type == STOKES_XXYY),
      //bool: 
      coherentStokesPPF(ps.settings.beamFormer.coherentSettings.nrChannels > 1), 

      // beamForm: B -> A
      // TODO: support >1 SAP
      devBeamFormerDelays(
        context,
        factories.beamFormer.bufferSize(BeamFormerKernel::BEAM_FORMER_DELAYS)),
      beamFormerBuffers(devB, devA, devBeamFormerDelays),
      beamFormerKernel(factories.beamFormer.create(queue, beamFormerBuffers)),

      // transpose after beamforming: A -> C/D
      //
      // Output buffer: 
      // 1ch: CS: C, CV: D
      // PPF: CS: D, CV: C
      coherentTransposeBuffers(
        devA, outputComplexVoltages ^ coherentStokesPPF ? devD : devC),
      coherentTransposeKernel(
          factories.coherentTranspose.create(
             queue, coherentTransposeBuffers)),

      // inverse FFT: C/D -> C/D (in-place) = transposeBuffers.output
      inverseFFT(
        queue,
        ps.settings.beamFormer.nrHighResolutionChannels,
        (ps.settings.beamFormer.maxNrTABsPerSAP() * NR_POLARIZATIONS *
         ps.nrSamplesPerSubband() /
         ps.settings.beamFormer.nrHighResolutionChannels),
         false, coherentTransposeBuffers.output),

      // fftshift: C/D -> C/D (in-place) = transposeBuffers.output
      inverseFFTShiftBuffers(
        coherentTransposeBuffers.output, coherentTransposeBuffers.output),
      inverseFFTShiftKernel(
        factories.fftShift.create(queue, inverseFFTShiftBuffers)),

      // FIR filter: D/C -> C/D
      //
      // Input buffer:
      // 1ch: CS: -, CV: - (no FIR will be done)
      // PPF: CS: D, CV: C = transposeBuffers.output
      //
      // Output buffer:
      // 1ch: CS: -, CV: - (no FIR will be done)
      // PPF: CS: C, CV: D = transposeBuffers.input
      devFilterWeights(
        context,
        factories.firFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)),
      devFilterHistoryData(
        context,
        factories.firFilter.bufferSize(FIR_FilterKernel::HISTORY_DATA)),
      firFilterBuffers(
        coherentTransposeBuffers.output, coherentTransposeBuffers.input,
        devFilterWeights, devFilterHistoryData),
      firFilterKernel(factories.firFilter.create(queue, firFilterBuffers)),

      // final FFT: C/D -> C/D (in-place) = firFilterBuffers.output
      finalFFT(
        queue,
        ps.settings.beamFormer.coherentSettings.nrChannels,
        (ps.settings.beamFormer.maxNrTABsPerSAP() *
         NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
         ps.settings.beamFormer.coherentSettings.nrChannels),
        true, firFilterBuffers.output),

      // coherentStokes: C -> D
      //
      // 1ch: input comes from inverseFFT in C
      // Nch: input comes from finalFFT in C
      coherentStokesBuffers(
        devC,
        devD),
      coherentStokesKernel(
        factories.coherentStokes.create(queue, coherentStokesBuffers)),

      //**************************************************************
      //incoherent stokes
      incoherentStokesPPF(
        ps.settings.beamFormer.incoherentSettings.nrChannels > 1),

      // Transpose: B -> A
      incoherentTransposeBuffers(devB, devA),

      incoherentTranspose(
        factories.incoherentStokesTranspose.create(
          queue, incoherentTransposeBuffers)),

      // inverse FFT: A -> A
      incoherentInverseFFT(
        queue, ps.settings.beamFormer.nrHighResolutionChannels,
        (ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
         ps.settings.beamFormer.nrHighResolutionChannels),
        false, devA),

      // inverse FFTShift: A -> A
      incoherentInverseFFTShiftBuffers(devA, devA),
      incoherentInverseFFTShiftKernel(
        factories.fftShift.create(queue, incoherentInverseFFTShiftBuffers)),

      // FIR filter: A -> B
      devIncoherentFilterWeights(
        context,
        factories.incoherentFirFilter.bufferSize(
          FIR_FilterKernel::FILTER_WEIGHTS)),

      devIncoherentFilterHistoryData(
        context,
        factories.incoherentFirFilter.bufferSize(
          FIR_FilterKernel::HISTORY_DATA)),

      incoherentFirFilterBuffers(
        devA, devB, devIncoherentFilterWeights, devIncoherentFilterHistoryData),

      incoherentFirFilterKernel(
        factories.incoherentFirFilter.create(
          queue, incoherentFirFilterBuffers)),

      // final FFT: B -> B
      incoherentFinalFFT(
        queue, ps.settings.beamFormer.incoherentSettings.nrChannels,
        (ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
         ps.settings.beamFormer.incoherentSettings.nrChannels),
        true, devB),

      // incoherentstokes kernel: A/B -> E
      //
      // 1ch: input comes from incoherentInverseFFT in A
      // Nch: input comes from incoherentFinalFFT in B
      incoherentStokesBuffers(
        incoherentStokesPPF ? devB : devA,
        devE),

      incoherentStokesKernel(
        factories.incoherentStokes.create(queue, incoherentStokesBuffers))
    {
      // initialize history data for both coherent and incoherent stokes.
      devFilterHistoryData.set(0);
      devIncoherentFilterHistoryData.set(0);

      // TODO For now we only allow pure coherent and incoherent runs
      // count the number of coherent and incoherent saps
      size_t nrCoherent = 0;
      size_t nrIncoherent = 0;
      for (size_t idx_sap = 0; 
           idx_sap < ps.settings.beamFormer.SAPs.size();
           ++idx_sap)
      {
        if (ps.settings.beamFormer.SAPs[idx_sap].nrIncoherent != 0)
          nrIncoherent++;
        if (ps.settings.beamFormer.SAPs[idx_sap].nrCoherent != 0)
          nrCoherent++;
      }

      // raise exception if the parset contained an incorrect configuration
      if (nrCoherent != 0 && nrIncoherent != 0)
        THROW(GPUProcException, 
              "Parset contained both incoherent and coherent stokes SAPS. "
              "This is not supported");

      if (nrCoherent)
        coherentBeamformer = true;
      else
        coherentBeamformer = false;

      LOG_INFO_STR("Running "
                   << (coherentBeamformer ? "a coherent" : "an incoherent")
                   << " Stokes beamformer pipeline");
      
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i)
      {
        //**********************************************************************
        // Coherent/incoheren switch
        if (coherentBeamformer) 
        outputPool.free.append(
          new BeamFormedData(
            ps.settings.beamFormer.incoherentSettings.nrStokes,
            ps.settings.beamFormer.incoherentSettings.nrChannels,
            ps.settings.beamFormer.incoherentSettings.nrSamples(
              ps.nrSamplesPerSubband()),
            ps.settings.beamFormer.maxNrTABsPerSAP(),
            context));
        else
        {
          outputPool.free.append(
          new BeamFormedData(
            ps.settings.beamFormer.incoherentSettings.nrStokes,
            ps.settings.beamFormer.incoherentSettings.nrChannels,
            ps.settings.beamFormer.incoherentSettings.nrSamples(
              ps.nrSamplesPerSubband()),
            ps.settings.beamFormer.maxNrTABsPerSAP(),
            context));
         }
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
    firstFFTShift(context),
    firstFFT(context),
    delayBp(context),
    secondFFTShift(context),
    secondFFT(context),
    correctBandpass(context),
    beamformer(context),
    transpose(context),
    inverseFFT(context),
    inverseFFTShift(context),
    firFilterKernel(context),
    finalFFT(context),
    coherentStokes(context),
    incoherentInverseFFT(context),
    incoherentInverseFFTShift(context),
    incoherentFirFilterKernel(context),
    incoherentFinalFFT(context),
    incoherentStokes(context),
    incoherentStokesTranspose(context),
    samples(context),
    visibilities(context),
    incoherentOutput(context)
    {
    }

    void BeamFormerSubbandProc::Counters::printStats()
    {     
      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc GPU mean and stDev ****" << endl <<
        std::setw(20) << "(intToFloat)" << intToFloat.stats << endl <<
        std::setw(20) << "(firstFFTShift)" << firstFFTShift.stats << endl <<
        std::setw(20) << "(firstFFT)" << firstFFT.stats << endl <<
        std::setw(20) << "(delayBp)" << delayBp.stats << endl <<
        std::setw(20) << "(secondFFTShift)" << secondFFTShift.stats << endl <<
        std::setw(20) << "(secondFFT)" << secondFFT.stats << endl <<
        std::setw(20) << "(correctBandpass)" << correctBandpass.stats << endl <<
        std::setw(20) << "(beamformer)" << beamformer.stats << endl <<
        std::setw(20) << "(transpose)" << transpose.stats << endl <<
        std::setw(20) << "(inverseFFT)" << inverseFFT.stats << endl <<
        std::setw(20) << "(inverseFFTShift)" << inverseFFTShift.stats << endl <<
        std::setw(20) << "(firFilterKernel)" << firFilterKernel.stats << endl <<
        std::setw(20) << "(finalFFT)" << finalFFT.stats << endl <<
        std::setw(20) << "(coherentStokes)" << coherentStokes.stats << endl <<
        std::setw(20) << "(samples)" << samples.stats << endl <<       
        std::setw(20) << "(visibilities)" << visibilities.stats << endl <<
        std::setw(20) << "(incoherentOutput )" << incoherentOutput.stats << endl <<
        std::setw(20) << "(incoherentInverseFFT)" << incoherentInverseFFT.stats << endl <<
        std::setw(20) << "(incoherentInverseFFTShift)" << incoherentInverseFFTShift.stats << endl <<
        std::setw(20) << "(incoherentFirFilterKernel)" << incoherentFirFilterKernel.stats << endl <<
        std::setw(20) << "(incoherentFinalFFT)" << incoherentFinalFFT.stats << endl <<
        std::setw(20) << "(incoherentStokes)" << incoherentStokes.stats << endl <<
        std::setw(20) << "(incoherentStokesTranspose)" << incoherentStokesTranspose.stats << endl);
    }

    void BeamFormerSubbandProc::processSubband(
      SubbandProcInputData &input,
      StreamableData &_output)
    {
      BeamFormedData &output = dynamic_cast<BeamFormedData&>(_output);
      BeamFormedData &incoherentOutput = dynamic_cast<BeamFormedData&>(_output);

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;
      queue.writeBuffer(devInput.inputSamples, input.inputSamples,
                        counters.samples, true);

      if (ps.delayCompensation())
      {
        unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
          queue.writeBuffer(devInput.delaysAtBegin,
                            input.delaysAtBegin, false);
          queue.writeBuffer(devInput.delaysAfterEnd,
                            input.delaysAfterEnd, false);
          queue.writeBuffer(devInput.phase0s,
                            input.phase0s, false);
          queue.writeBuffer(devBeamFormerDelays,
                            input.tabDelays, false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      //****************************************
      // Enqueue the kernels
      // Note: make sure to call the right enqueue() for each kernel.
      // Otherwise, a kernel arg may not be set...
      intToFloatKernel->enqueue(input.blockID, counters.intToFloat);

      firstFFTShiftKernel->enqueue(input.blockID, counters.firstFFTShift);
      firstFFT.enqueue(input.blockID, counters.firstFFT);

      delayCompensationKernel->enqueue(
        input.blockID, counters.delayBp,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      secondFFTShiftKernel->enqueue(input.blockID, counters.secondFFTShift);
      secondFFT.enqueue(input.blockID, counters.secondFFT);

      bandPassCorrectionKernel->enqueue(
        input.blockID, counters.correctBandpass);

      // ********************************************************************
      // coherent stokes kernels
      if (coherentBeamformer)
      {
        beamFormerKernel->enqueue(input.blockID, counters.beamformer,
          ps.settings.subbands[subband].centralFrequency,
          ps.settings.subbands[subband].SAP);

        coherentTransposeKernel->enqueue(input.blockID, counters.transpose);

        inverseFFT.enqueue(input.blockID, counters.inverseFFT);
        inverseFFTShiftKernel->enqueue(input.blockID, counters.inverseFFTShift);

        if (coherentStokesPPF) 
        {
          firFilterKernel->enqueue(input.blockID, 
            counters.firFilterKernel,
            input.blockID.subbandProcSubbandIdx);
          finalFFT.enqueue(input.blockID, counters.finalFFT);
        }

        if (!outputComplexVoltages)
        {
          coherentStokesKernel->enqueue(input.blockID, counters.coherentStokes);
        }
      }
      else
      {
        // ********************************************************************
        // incoherent stokes kernels
        incoherentTranspose->enqueue(
          input.blockID, counters.incoherentStokesTranspose);

        incoherentInverseFFT.enqueue(
          input.blockID, counters.incoherentInverseFFT);
        incoherentInverseFFTShiftKernel->enqueue(
          input.blockID, counters.incoherentInverseFFTShift);

        if (incoherentStokesPPF) 
        {
          incoherentFirFilterKernel->enqueue(
            input.blockID, counters.incoherentFirFilterKernel,
            input.blockID.subbandProcSubbandIdx);

          incoherentFinalFFT.enqueue(
            input.blockID, counters.incoherentFinalFFT);
        }

        incoherentStokesKernel->enqueue(
          input.blockID, counters.incoherentStokes);

        // TODO: Propagate flags
      }

      queue.synchronize();

      // Output in devD and devE, by design.
      if (coherentBeamformer)
        queue.readBuffer(output,
                         devD,
                         counters.visibilities, true);
      else
        queue.readBuffer(incoherentOutput,
                         devE,
                         counters.incoherentOutput, true);

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
        if (coherentBeamformer)
        {
          if (coherentStokesPPF) 
          {
            counters.firFilterKernel.logTime();
            counters.finalFFT.logTime();
          }

          counters.beamformer.logTime();
          counters.transpose.logTime();
          counters.inverseFFT.logTime();

          if (!outputComplexVoltages)
          {
            counters.coherentStokes.logTime();
          }

          counters.visibilities.logTime();
        }
        else
        {
          counters.incoherentStokesTranspose.logTime();
          counters.incoherentInverseFFT.logTime();
          if (incoherentStokesPPF) 
          {
            counters.incoherentFirFilterKernel.logTime();
            counters.incoherentFinalFFT.logTime();
          }
          counters.incoherentStokes.logTime();
          counters.incoherentOutput.logTime();
        }
      }
    }

    bool BeamFormerSubbandProc::postprocessSubband(StreamableData &_output)
    {
      (void)_output;
      return true;
    }

  }
}

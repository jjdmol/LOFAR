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
#include "BeamFormerPreprocessingPart.h"

// Set to true to get detailed buffer informatio
#if 0
  #define DUMPBUFFER(a,b) dumpBuffer((a),  (b))
#else
  #define DUMPBUFFER(a,b)
#endif


namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormedData::BeamFormedData(
        unsigned nrCoherentTABs,
        unsigned nrCoherentStokes,
        size_t nrCoherentSamples,
        unsigned nrCoherentChannels,
        unsigned nrIncoherentTABs,
        unsigned nrIncoherentStokes,
        size_t nrIncoherentSamples,
        unsigned nrIncoherentChannels,
        gpu::Context &context) :
      coherentData(boost::extents[nrCoherentTABs]
                                 [nrCoherentStokes]
                                 [nrCoherentSamples]
                                 [nrCoherentChannels], context, 0),
      incoherentData(boost::extents[nrIncoherentTABs]
                                   [nrIncoherentStokes]
                                   [nrIncoherentSamples]
                                   [nrIncoherentChannels], context, 0)
    {
    }

    BeamFormedData::BeamFormedData(
        const Parset &ps,
        gpu::Context &context) :
      coherentData(boost::extents[ps.settings.beamFormer.maxNrCoherentTABsPerSAP()]
                                 [ps.settings.beamFormer.coherentSettings.nrStokes]
                                 [ps.settings.beamFormer.coherentSettings.nrSamples(ps.nrSamplesPerSubband())]
                                 [ps.settings.beamFormer.coherentSettings.nrChannels],
                                 context, 0),
      incoherentData(boost::extents[ps.settings.beamFormer.maxNrIncoherentTABsPerSAP()]
                                   [ps.settings.beamFormer.incoherentSettings.nrStokes]
                                   [ps.settings.beamFormer.incoherentSettings.nrSamples(ps.nrSamplesPerSubband())]
                                   [ps.settings.beamFormer.incoherentSettings.nrChannels],
                                   context, 0)
    {
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
      prevSAP(-1)

    {
      // NOTE: Make sure the history samples are dealt with properly until the
      // FIR, which the beam former does in a later stage!
      devInput.reset(new SubbandProcInputData::DeviceBuffers(
        std::max(
        factories.intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA),
        factories.beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA)),
        factories.delayCompensation.bufferSize(
        DelayAndBandPassKernel::DELAYS),
        factories.delayCompensation.bufferSize(
        DelayAndBandPassKernel::PHASE_ZEROS),
        context));
      // coherent stokes buffers
      unsigned sizeKernelBuffers = devInput->inputSamples.size();

      devA.reset(
        new gpu::DeviceMemory(context, sizeKernelBuffers));
      devB.reset(
        new gpu::DeviceMemory(context, sizeKernelBuffers));
      // Buffers for incoherent stokes
      devC.reset(
        new gpu::DeviceMemory(context, sizeKernelBuffers));
      devD.reset(
        new gpu::DeviceMemory(context, sizeKernelBuffers));
      devE.reset(
        new gpu::DeviceMemory(context,
        factories.incoherentStokes.bufferSize(
        IncoherentStokesKernel::OUTPUT_DATA)));
      devNull.reset(
        new gpu::DeviceMemory(context, 1));

      LOG_INFO_STR("************************************");
      LOG_INFO_STR("debug 1");
      preprocessingPart = std::auto_ptr<BeamFormerPreprocessingPart>(
        new BeamFormerPreprocessingPart(parset,
        queue,
        devInput, devA, devB,  devNull));

      LOG_INFO_STR("debug 2");
      //################################################

      preprocessingPart->initFFTAndFlagMembers(context, factories);

      LOG_INFO_STR("debug 3");

      initCoherentMembers(context, factories);
      LOG_INFO_STR("debug 4");
      initIncoherentMembers(context, factories);
      LOG_INFO_STR("debug 5");
      // initialize history data for both coherent and incoherent stokes.
      devFilterHistoryData->set(0);
      devIncoherentFilterHistoryData->set(0);

      formCoherentBeams = ps.settings.beamFormer.maxNrCoherentTABsPerSAP() > 0;
      formIncoherentBeams = ps.settings.beamFormer.maxNrIncoherentTABsPerSAP() > 0;

      LOG_INFO_STR("Running coherent pipeline: " << (formCoherentBeams ? "yes" : "no")
                << ", incoherent pipeline: " <<   (formIncoherentBeams ? "yes" : "no"));
      
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i)
      {
        outputPool.free.append(new BeamFormedData(ps, context));
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
      



      void BeamFormerSubbandProc::initCoherentMembers(gpu::Context &context,
        BeamFormerFactories &factories)
      {
        //**************************************************************
        //coherent stokes
        //bool:
        outputComplexVoltages =
          ps.settings.beamFormer.coherentSettings.type == STOKES_XXYY;
        //bool: 
        coherentStokesPPF = ps.settings.beamFormer.coherentSettings.nrChannels > 1;

        // beamForm: B -> A
        // TODO: support >1 SAP
        devBeamFormerDelays = std::auto_ptr<gpu::DeviceMemory>(
          new gpu::DeviceMemory(context,
          factories.beamFormer.bufferSize(BeamFormerKernel::BEAM_FORMER_DELAYS)
          )
          );
        beamFormerBuffers = std::auto_ptr<BeamFormerKernel::Buffers>(
          new BeamFormerKernel::Buffers(*devB, *devA, *devBeamFormerDelays));

        beamFormerKernel = std::auto_ptr<BeamFormerKernel>(
          factories.beamFormer.create(queue, *beamFormerBuffers));
        // transpose after beamforming: A -> C/D
        //
        // Output buffer: 
        // 1ch: CS: C, CV: D
        // PPF: CS: D, CV: C

        coherentTransposeBuffers = std::auto_ptr<CoherentStokesTransposeKernel::Buffers>(
          new CoherentStokesTransposeKernel::Buffers(*devA,
          outputComplexVoltages ^ coherentStokesPPF ? *devD : *devC));

        coherentTransposeKernel = std::auto_ptr<CoherentStokesTransposeKernel>(
          factories.coherentTranspose.create(
          queue, *coherentTransposeBuffers));

        // inverse FFT: C/D -> C/D (in-place) = transposeBuffers.output
        unsigned nrInverFFTs = ps.settings.beamFormer.maxNrTABsPerSAP() *
          NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
          ps.settings.beamFormer.nrHighResolutionChannels;
        inverseFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
          queue, ps.settings.beamFormer.nrHighResolutionChannels,
          nrInverFFTs, false, coherentTransposeBuffers->output));

        // fftshift: C/D -> C/D (in-place) = transposeBuffers.output
        inverseFFTShiftBuffers = std::auto_ptr<FFTShiftKernel::Buffers>(
          new FFTShiftKernel::Buffers(coherentTransposeBuffers->output,
          coherentTransposeBuffers->output));

        inverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
          factories.fftShift.create(queue, *inverseFFTShiftBuffers));

        // FIR filter: D/C -> C/D
        //
        // Input buffer:
        // 1ch: CS: -, CV: - (no FIR will be done)
        // PPF: CS: D, CV: C = transposeBuffers.output
        //
        // Output buffer:
        // 1ch: CS: -, CV: - (no FIR will be done)
        // PPF: CS: C, CV: D = transposeBuffers.input
        devFilterWeights = std::auto_ptr<gpu::DeviceMemory>(
          new gpu::DeviceMemory(context,
          factories.firFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)));

        devFilterHistoryData = std::auto_ptr<gpu::DeviceMemory>(
          new gpu::DeviceMemory(context,
          factories.firFilter.bufferSize(FIR_FilterKernel::HISTORY_DATA)));

        firFilterBuffers = std::auto_ptr<FIR_FilterKernel::Buffers>(
          new FIR_FilterKernel::Buffers(
          coherentTransposeBuffers->output, coherentTransposeBuffers->input,
          *devFilterWeights, *devFilterHistoryData));

        firFilterKernel = std::auto_ptr<FIR_FilterKernel>(
          factories.firFilter.create(queue, *firFilterBuffers));
        // final FFT: C/D -> C/D (in-place) = firFilterBuffers.output

        unsigned nrFinalFFTs = ps.settings.beamFormer.maxNrTABsPerSAP() *
          NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
          ps.settings.beamFormer.coherentSettings.nrChannels;
        finalFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
          queue, ps.settings.beamFormer.coherentSettings.nrChannels,
          nrFinalFFTs, true, firFilterBuffers->output));

        // coherentStokes: C -> D
        //
        // 1ch: input comes from inverseFFT in C
        // Nch: input comes from finalFFT in C

        coherentStokesBuffers = std::auto_ptr<CoherentStokesKernel::Buffers>(
          new CoherentStokesKernel::Buffers(*devC, *devD));

        coherentStokesKernel = std::auto_ptr<CoherentStokesKernel>(
          factories.coherentStokes.create(queue, *coherentStokesBuffers));




      }

      void BeamFormerSubbandProc::initIncoherentMembers(gpu::Context &context,
        BeamFormerFactories &factories)
      {
        //**************************************************************
        //incoherent stokes
        incoherentStokesPPF =
          ps.settings.beamFormer.incoherentSettings.nrChannels > 1;
        // Transpose: B -> A
        incoherentTransposeBuffers =
          std::auto_ptr<IncoherentStokesTransposeKernel::Buffers>(
          new IncoherentStokesTransposeKernel::Buffers(*devB, *devA));

        incoherentTranspose = std::auto_ptr<IncoherentStokesTransposeKernel>(
          factories.incoherentStokesTranspose.create(queue,
          *incoherentTransposeBuffers));

        unsigned incInversNrFFTs = ps.nrStations() * NR_POLARIZATIONS *
          ps.nrSamplesPerSubband() /
          ps.settings.beamFormer.nrHighResolutionChannels;

        incoherentInverseFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
          queue, ps.settings.beamFormer.nrHighResolutionChannels,
          incInversNrFFTs, false, *devA));

        // inverse FFTShift: A -> A
        incoherentInverseFFTShiftBuffers =
          std::auto_ptr<FFTShiftKernel::Buffers>(
          new FFTShiftKernel::Buffers(*devA, *devA));

        incoherentInverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
          factories.fftShift.create(queue, *incoherentInverseFFTShiftBuffers));

        devIncoherentFilterWeights = std::auto_ptr<gpu::DeviceMemory>(
          new gpu::DeviceMemory(context,
          factories.incoherentFirFilter.bufferSize(
          FIR_FilterKernel::FILTER_WEIGHTS)));

        devIncoherentFilterHistoryData = std::auto_ptr<gpu::DeviceMemory>(
          new gpu::DeviceMemory(context,
          factories.incoherentFirFilter.bufferSize(
          FIR_FilterKernel::HISTORY_DATA)
          ));
        incoherentFirFilterBuffers =
          std::auto_ptr<FIR_FilterKernel::Buffers>(
          new FIR_FilterKernel::Buffers(*devA, *devB,
          *devIncoherentFilterWeights,
          *devIncoherentFilterHistoryData));

        incoherentFirFilterKernel = std::auto_ptr<FIR_FilterKernel>(
          factories.incoherentFirFilter.create(
          queue, *incoherentFirFilterBuffers));

        // final FFT: B -> B
        unsigned nrFFTs = ps.nrStations() * NR_POLARIZATIONS *
          ps.nrSamplesPerSubband() /
          ps.settings.beamFormer.incoherentSettings.nrChannels;
        incoherentFinalFFT = std::auto_ptr<FFT_Kernel>(
          new FFT_Kernel(
          queue, ps.settings.beamFormer.incoherentSettings.nrChannels,
          nrFFTs, true, *devB));

        // incoherentstokes kernel: A/B -> E
        //
        // 1ch: input comes from incoherentInverseFFT in A
        // Nch: input comes from incoherentFinalFFT in B
        incoherentStokesBuffers =
          std::auto_ptr<IncoherentStokesKernel::Buffers>(
          new IncoherentStokesKernel::Buffers(
          incoherentStokesPPF ? *devB : *devA, *devE));
        incoherentStokesKernel = std::auto_ptr<IncoherentStokesKernel>(
          factories.incoherentStokes.create(queue, *incoherentStokesBuffers));

      }


    BeamFormerSubbandProc::Counters::Counters(gpu::Context &context)
      :

    samples(context),
    visibilities(context),
    incoherentOutput(context)
    {
    }

    void BeamFormerSubbandProc::logTime(unsigned nrCoherent,
      unsigned nrIncoherent, bool coherentStokesPPF, bool outputComplexVoltages,
      bool incoherentStokesPPF)
    {
      preprocessingPart->logTimeFirstStage();
      // samples.logTime();  // performance count the transfer      
      if (nrCoherent > 0)
        logTimeCoherentStage(coherentStokesPPF, outputComplexVoltages);

      if (nrIncoherent > 0) 
        logTimeIncoherentStage( incoherentStokesPPF);
    }


    void BeamFormerSubbandProc::logTimeCoherentStage(bool coherentStokesPPF,
      bool outputComplexVoltages)
    {
      if (coherentStokesPPF)
      {
        firFilterKernel->itsCounter.logTime();
        finalFFT->itsCounter.logTime();
      }

      beamFormerKernel->itsCounter.logTime();
      coherentTransposeKernel->itsCounter.logTime();
      inverseFFT->itsCounter.logTime();
      if (!outputComplexVoltages)
      {
        coherentStokesKernel->itsCounter.logTime();
      }
      //visibilities.logTime(); //transfer

    }

    void BeamFormerSubbandProc::logTimeIncoherentStage(bool incoherentStokesPPF)
    {
      incoherentTranspose->itsCounter.logTime();
      incoherentInverseFFT->itsCounter.logTime();
      if (incoherentStokesPPF)
      {
        incoherentFirFilterKernel->itsCounter.logTime();
        incoherentFinalFFT->itsCounter.logTime();
      }
      incoherentStokesKernel->itsCounter.logTime();
      //incoherentOutput.logTime();  //transfer
    }

    void BeamFormerSubbandProc::printStats()
    {
      preprocessingPart->printStatsFirstStage();

      printStatsCoherentStage();

      printStatsIncoherentStage();

      counters.printStats();

    }



    void BeamFormerSubbandProc::printStatsCoherentStage()
    {
          // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc coherent stage GPU mean and stDev ****" << endl <<
        std::setw(20) << "(firFilterKernel)" << firFilterKernel->itsCounter.stats << endl <<
        std::setw(20) << "(finalFFT)" << finalFFT->itsCounter.stats << endl <<
        std::setw(20) << "(beamformer)" << beamFormerKernel->itsCounter.stats << endl <<
        std::setw(20) << "(coherentTranspose)" << coherentTransposeKernel->itsCounter.stats << endl <<
        std::setw(20) << "(inverseFFT)" << inverseFFT->itsCounter.stats << endl <<
        //std::setw(20) << "(inverseFFTShift)" << inverseFFTShift.stats << endl <<
        std::setw(20) << "(coherentStokes)" << coherentStokesKernel->itsCounter.stats << endl );
     
    }

    void BeamFormerSubbandProc::printStatsIncoherentStage()
    {
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc incoherent stage GPU mean and stDev ****" << endl <<
        std::setw(20) << "(incoherentStokesTranspose)" << incoherentTranspose->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentInverseFFT)" << incoherentInverseFFT->itsCounter.stats << endl <<
        // std::setw(20) << "(incoherentInverseFFTShift)" << incoherentInverseFFTShift.stats << endl <<
        std::setw(20) << "(incoherentFirFilterKernel)" << incoherentFirFilterKernel->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentFinalFFT)" << incoherentFinalFFT->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentStokes)" << incoherentStokesKernel->itsCounter.stats << endl);
    }


    void BeamFormerSubbandProc::Counters::printStats()
    {     
      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc cpu to GPU transfers GPU mean and stDev ****" << endl <<

        std::setw(20) << "(samples)" << samples.stats << endl <<       
        std::setw(20) << "(visibilities)" << visibilities.stats << endl <<
        std::setw(20) << "(incoherentOutput )" << incoherentOutput.stats << endl );

    }

    void BeamFormerSubbandProc::processSubband(
      SubbandProcInputData &input,
      SubbandProcOutputData &_output)
    {
      BeamFormedData &output = dynamic_cast<BeamFormedData&>(_output);

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;
      unsigned SAP = ps.settings.subbands[subband].SAP;
      unsigned nrCoherent   = ps.settings.beamFormer.SAPs[SAP].nrCoherent;
      unsigned nrIncoherent = ps.settings.beamFormer.SAPs[SAP].nrIncoherent;

      //****************************************
      // Send input to GPU
      queue.writeBuffer(devInput->inputSamples, input.inputSamples,
                        counters.samples, true);

      // Only upload delays if they changed w.r.t. the previous subband.
      if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
        if (ps.delayCompensation())
        {
          queue.writeBuffer(devInput->delaysAtBegin,
            input.delaysAtBegin, false);
          queue.writeBuffer(devInput->delaysAfterEnd,
            input.delaysAfterEnd, false);
          queue.writeBuffer(devInput->phase0s,
            input.phase0s, false);
        }

        // Upload the new beamformerDelays (pointings) to the GPU 
        queue.writeBuffer(*devBeamFormerDelays,
          input.tabDelays, false);

        prevSAP = SAP;
        prevBlock = block;
      }


      preprocessingPart->processFirstStage(input.blockID, subband);
      

      // ********************************************************************
      // coherent stokes kernels
      if (nrCoherent > 0)
      {
        processCoherentStage(input.blockID, subband);

        // Reshape output to only read nrCoherent TABs
        output.coherentData.resizeOneDimensionInplace(0, nrCoherent);

        // Output in devD, by design
        queue.readBuffer(
          output.coherentData, *devD, counters.visibilities, false);
      }

      if (nrIncoherent > 0)
      {
         processIncoherentStage(input.blockID);

        // Reshape output to only read nrIncoherent TABs
        output.incoherentData.resizeOneDimensionInplace(0, nrIncoherent);

        // Output in devE, by design
        queue.readBuffer( output.incoherentData, *devE, 
          counters.incoherentOutput, false);

        // TODO: Propagate flags
      }

      queue.synchronize();

      // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling)
      {      
        // assure that the queue is done so all events are fished
        queue.synchronize();
        logTime(nrCoherent, nrIncoherent, coherentStokesPPF,
          outputComplexVoltages, incoherentStokesPPF);

        /*counters.logTime(nrCoherent, nrIncoherent, coherentStokesPPF, 
         outputComplexVoltages, incoherentStokesPPF);*/
      }
    }



    void BeamFormerSubbandProc::processCoherentStage(BlockID blockID,
      unsigned subband)
    {
      beamFormerKernel->enqueue(blockID, 
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);

      coherentTransposeKernel->enqueue(blockID);
      DUMPBUFFER(coherentTransposeBuffers.output, "coherentTransposeBuffers.output.dat");

      inverseFFT->enqueue(blockID);
      DUMPBUFFER(inverseFFTShiftBuffers.input, "inverseFFTBuffers.output.dat");

      inverseFFTShiftKernel->enqueue(blockID);
      DUMPBUFFER(inverseFFTShiftBuffers.output, "inverseFFTShift.output.dat");

      if (coherentStokesPPF)
      {
        firFilterKernel->enqueue(blockID,
          blockID.subbandProcSubbandIdx);
        finalFFT->enqueue(blockID);
      }

      if (!outputComplexVoltages)
      {
        DUMPBUFFER(coherentStokesBuffers.input, "coherentStokesBuffers.input.dat");
        coherentStokesKernel->enqueue(blockID);
      }

    }

    void BeamFormerSubbandProc::processIncoherentStage(BlockID blockID)
    {
      // ********************************************************************
      // incoherent stokes kernels
      incoherentTranspose->enqueue(
        blockID);

      incoherentInverseFFT->enqueue(
        blockID);

      DUMPBUFFER(incoherentInverseFFTShiftBuffers.input,
        "incoherentInverseFFTShiftBuffers.input.dat");

      incoherentInverseFFTShiftKernel->enqueue(
        blockID);

      DUMPBUFFER(incoherentInverseFFTShiftBuffers.output,
        "incoherentInverseFFTShiftBuffers.output.dat");

      if (incoherentStokesPPF)
      {
        incoherentFirFilterKernel->enqueue(
          blockID,
          blockID.subbandProcSubbandIdx);

        incoherentFinalFFT->enqueue(
          blockID);
      }

      incoherentStokesKernel->enqueue(
        blockID);
    }

    bool BeamFormerSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      (void)_output;
      return true;
    }



  }
}

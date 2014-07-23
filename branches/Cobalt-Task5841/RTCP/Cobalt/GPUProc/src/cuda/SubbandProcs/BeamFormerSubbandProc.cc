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
                                 [ps.settings.beamFormer.coherentSettings.nrSamples]
                                 [ps.settings.beamFormer.coherentSettings.nrChannels],
                                 context, 0),
      incoherentData(boost::extents[ps.settings.beamFormer.maxNrIncoherentTABsPerSAP()]
                                   [ps.settings.beamFormer.incoherentSettings.nrStokes]
                                   [ps.settings.beamFormer.incoherentSettings.nrSamples]
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
      // See doc/bf-pipeline.txt
      size_t devA_size = factories.preprocessing.intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA);
      size_t devB_size = factories.preprocessing.intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA);
      size_t devC_size = 1;
      size_t devD_size = 1;

      if (factories.coherentStokes) {
        devA_size = std::max(devA_size,
          factories.coherentStokes->beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA));
        devC_size = std::max(devC_size,
          factories.coherentStokes->beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA));
        devD_size = std::max(devD_size,
          factories.coherentStokes->beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA));
      }

      if (factories.incoherentStokes) {
        /* buffers of the preprocessing step are big enough */
      }

      devInput.reset(new SubbandProcInputData::DeviceBuffers(
        devA_size,
        factories.preprocessing.delayCompensation.bufferSize(
        DelayAndBandPassKernel::DELAYS),
        factories.preprocessing.delayCompensation.bufferSize(
        DelayAndBandPassKernel::PHASE_ZEROS),
        context));

      // NOTE: For an explanation of the different buffers being used, please refer
      // to the document bf-pipeline.txt in the GPUProc/doc directory.
      devA = devInput->inputSamples;
      devB.reset(new gpu::DeviceMemory(context, devB_size));
      devC.reset(new gpu::DeviceMemory(context, devC_size));
      devD.reset(new gpu::DeviceMemory(context, devD_size));
      devE = devInput->inputSamples;

      // Null buffer for unused parts of the pipeline
      devNull.reset(new gpu::DeviceMemory(context, 1));

      //################################################
      // Create objects containing the kernel and device buffers
      preprocessingPart = std::auto_ptr<BeamFormerPreprocessingStep>(
        new BeamFormerPreprocessingStep(parset, queue, context, factories.preprocessing, 
        devInput, devA, devB, devNull));

      if (factories.coherentStokes) {
        devBeamFormerDelays.reset(new gpu::DeviceMemory(context,
          factories.coherentStokes->beamFormer.bufferSize(BeamFormerKernel::BEAM_FORMER_DELAYS)));

        coherentStep = std::auto_ptr<BeamFormerCoherentStep>(
          new BeamFormerCoherentStep(parset, queue, context, *factories.coherentStokes,
          devInput, devA, devB, devC, devD, devBeamFormerDelays));
      }

      if (factories.incoherentStokes) {
        incoherentStep = std::auto_ptr<BeamFormerIncoherentStep>(
          new BeamFormerIncoherentStep(parset, queue, context, *factories.incoherentStokes, 
              devInput, devA, devB));
      }


      LOG_INFO_STR("Running coherent pipeline: " 
        << (coherentStep.get() ? "yes" : "no")  
        << ", incoherent pipeline: " 
        << (incoherentStep.get() ? "yes" : "no"));
      
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i)
      {
        outputPool.free.append(new BeamFormedData(ps, context));
      }
    }
     
    BeamFormerSubbandProc::Counters::Counters(gpu::Context &context)
      :
      inputsamples(context),
      coherentOutput(context),
      incoherentOutput(context)
    {
    }

    void BeamFormerSubbandProc::logTime(unsigned nrCoherent,
      unsigned nrIncoherent)
    {
      preprocessingPart->logTime();
      // samples.logTime();  // performance count the transfer      
      if (nrCoherent > 0)
        coherentStep->logTime();

      if (nrIncoherent > 0) 
        incoherentStep->logTime();

      counters.logTime( nrCoherent,
         nrIncoherent);
    }

    void BeamFormerSubbandProc::printStats()
    {
      preprocessingPart->printStats();
      if (coherentStep.get())
        coherentStep->printStats();
      if (incoherentStep.get())
        incoherentStep->printStats();
      counters.printStats();
    }

    void BeamFormerSubbandProc::Counters::logTime(
      unsigned nrCoherent, unsigned nrIncoherent)
    {
      inputsamples.logTime();
      if (nrCoherent)
        coherentOutput.logTime();
      if (nrIncoherent)
        incoherentOutput.logTime();      
    }

    void BeamFormerSubbandProc::Counters::printStats()
    {     
      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc cpu to GPU transfers GPU mean and stDev ****" << endl <<
        std::setw(20) << "(inputsamples)" << inputsamples.stats << endl <<
        std::setw(20) << "(coherentOutput)" << coherentOutput.stats << endl <<
        std::setw(20) << "(incoherentOutput )" << incoherentOutput.stats << endl );
    }

    void BeamFormerSubbandProc::processSubband( SubbandProcInputData &input,
      SubbandProcOutputData &_output)
    {
      BeamFormedData &output = dynamic_cast<BeamFormedData&>(_output);

      //*******************************************************************
      // calculate some variables depending on the input subband
      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;
      unsigned SAP = ps.settings.subbands[subband].SAP;
      unsigned nrCoherent   = ps.settings.beamFormer.SAPs[SAP].nrCoherent;
      unsigned nrIncoherent = ps.settings.beamFormer.SAPs[SAP].nrIncoherent;

      //****************************************
      // Send inputs to GPU
      queue.writeBuffer(*devInput->inputSamples, input.inputSamples,
        counters.inputsamples, true);

      // Some additional buffers
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

        if (nrCoherent > 0) {
          ASSERT(devBeamFormerDelays.get());

          // Upload the new beamformerDelays (pointings) to the GPU 
          queue.writeBuffer(*devBeamFormerDelays, input.tabDelays, false);
        }

        prevSAP = SAP;
        prevBlock = block;
      }

      // ************************************************
      // Start the processing
      // Preprocessing, the same for all
      preprocessingPart->process(input.blockID, subband);

      if (nrCoherent > 0)
      {
        ASSERT(coherentStep.get());

        coherentStep->process(input.blockID, subband);

        // Reshape output to only read nrCoherent TABs
        output.coherentData.resizeOneDimensionInplace(0, nrCoherent);

        // Output in devD, by design
        queue.readBuffer( output.coherentData, *devD,
         counters.coherentOutput, false);
      }

      if (nrIncoherent > 0)
      {
        ASSERT(incoherentStep.get());

        incoherentStep->process(input.blockID, subband);

        // Reshape output to only read nrIncoherent TABs
        output.incoherentData.resizeOneDimensionInplace(0, nrIncoherent);

        // Output in devE, by design
        queue.readBuffer(output.incoherentData, incoherentStep->outputBuffer(),
          counters.incoherentOutput, false);

        // TODO: Propagate flags
      }
      // Synchronise to assure that all the work in the data is done
      queue.synchronize();

      // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling)
      {      
        logTime(nrCoherent, nrIncoherent);
      }
    }

    bool BeamFormerSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      (void)_output;
      return true;
    }

  }
}


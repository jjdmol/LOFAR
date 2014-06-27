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
      prevBlock(-1),
      prevSAP(-1),
      inputCounter(context)
    {
      // See doc/bf-pipeline.txt
      size_t devA_size = factories.preprocessing.intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA);
      size_t devB_size = factories.preprocessing.intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA);

      if (factories.coherentStokes) {
        devA_size = std::max(devA_size,
          factories.coherentStokes->beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA));
      }

      if (factories.incoherentStokes) {
        /* buffers of the preprocessing step are big enough */
      }

      devInput.reset(new SubbandProcInputData::DeviceBuffers(
        devA_size, context));

      // NOTE: For an explanation of the different buffers being used, please refer
      // to the document bf-pipeline.txt in the GPUProc/doc directory.
      devA = devInput->inputSamples;
      devB.reset(new gpu::DeviceMemory(context, devB_size));

      //################################################
      // Create objects containing the kernel and device buffers
      preprocessingPart = std::auto_ptr<BeamFormerPreprocessingStep>(
        new BeamFormerPreprocessingStep(parset, queue, context, factories.preprocessing, 
        devInput, devA, devB));

      if (factories.coherentStokes) {
        coherentStep = std::auto_ptr<BeamFormerCoherentStep>(
          new BeamFormerCoherentStep(parset, queue, context, *factories.coherentStokes,
          devA, devB));
      }

      if (factories.incoherentStokes) {
        incoherentStep = std::auto_ptr<BeamFormerIncoherentStep>(
          new BeamFormerIncoherentStep(parset, queue, context, *factories.incoherentStokes, 
              devA, devB));
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
     
    void BeamFormerSubbandProc::logTime()
    {
      inputCounter.logTime();

      preprocessingPart->logTime();

      if (coherentStep.get())
        coherentStep->logTime();

      if (incoherentStep.get())
        incoherentStep->logTime();
    }

    void BeamFormerSubbandProc::printStats()
    {
      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc cpu to GPU transfers GPU mean and stDev ****" << endl <<
        std::setw(20) << "(input)" << inputCounter.stats << endl);

      preprocessingPart->printStats();

      if (coherentStep.get())
        coherentStep->printStats();

      if (incoherentStep.get())
        incoherentStep->printStats();
    }

    void BeamFormerSubbandProc::processSubband( SubbandProcInputData &input,
      SubbandProcOutputData &_output)
    {
      BeamFormedData &output = dynamic_cast<BeamFormedData&>(_output);

      //*******************************************************************
      // calculate some variables depending on the input subband
      size_t block = input.blockID.block;
      unsigned SAP = ps.settings.subbands[input.blockID.globalSubbandIdx].SAP;

      //****************************************
      // Send inputs to GPU
      queue.writeBuffer(*devInput->inputSamples, input.inputSamples,
        inputCounter, true);

      // Some additional buffers
      // Only upload delays if they changed w.r.t. the previous subband.
      if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
        preprocessingPart->writeInput(input);

        if (coherentStep.get()) {
          coherentStep->writeInput(input);
        }

        prevSAP = SAP;
        prevBlock = block;
      }

      // ************************************************
      // Start the processing
      // Preprocessing, the same for all
      preprocessingPart->process(input);

      if (coherentStep.get())
      {
        coherentStep->process(input);
        coherentStep->readOutput(output);
      }

      if (incoherentStep.get())
      {
        incoherentStep->process(input);
        incoherentStep->readOutput(output);
      }

      // Synchronise to assure that all the work in the data is done
      queue.synchronize();

      // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling) {      
        logTime();
      }
    }

    bool BeamFormerSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      (void)_output;
      return true;
    }

  }
}


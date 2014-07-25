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
        const Parset &ps,
        gpu::Context &context) :
      coherentData(ps.settings.beamFormer.anyCoherentTABs()
        ? boost::extents[ps.settings.beamFormer.maxNrCoherentTABsPerSAP()]
                        [ps.settings.beamFormer.coherentSettings.nrStokes]
                        [ps.settings.beamFormer.coherentSettings.nrSamples]
                        [ps.settings.beamFormer.coherentSettings.nrChannels]
        : boost::extents[0][0][0][0],
        context, 0),

      incoherentData(ps.settings.beamFormer.anyIncoherentTABs()
        ? boost::extents[ps.settings.beamFormer.maxNrIncoherentTABsPerSAP()]
                        [ps.settings.beamFormer.incoherentSettings.nrStokes]
                        [ps.settings.beamFormer.incoherentSettings.nrSamples]
                        [ps.settings.beamFormer.incoherentSettings.nrChannels]
        : boost::extents[0][0][0][0],
        context, 0),

      correlatedData(ps.settings.correlator.enabled ? ps.settings.antennaFields.size()           : 0,
                     ps.settings.correlator.enabled ? ps.settings.correlator.nrChannels          : 0,
                     ps.settings.correlator.enabled ? ps.settings.correlator.nrSamplesPerChannel : 0,
                     context),
      emit_correlatedData(false)
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
      inputCounter(context, "input")
    {
      // See doc/bf-pipeline.txt
      size_t devA_size = 0;
      size_t devB_size = 0;

      if (factories.correlator) {
        CorrelatorStep::Factories &cf = *factories.correlator;

        devA_size = std::max(devA_size,
          cf.firFilter ? cf.firFilter->bufferSize(FIR_FilterKernel::INPUT_DATA)
                       : cf.delayAndBandPass.bufferSize(DelayAndBandPassKernel::INPUT_DATA));
        devB_size = std::max(devB_size,
                      cf.correlator.bufferSize(CorrelatorKernel::INPUT_DATA));
      }

      if (factories.preprocessing) {
        devA_size = std::max(devA_size,
          factories.preprocessing->intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA));
        devB_size = std::max(devB_size,
          factories.preprocessing->intToFloat.bufferSize(IntToFloatKernel::OUTPUT_DATA));
      }

      if (factories.incoherentStokes) {
        ASSERT(factories.preprocessing);

        /* incoherentStokes uses devA and devB, but the sizes provided b the preprocessing
           pipeline are already sufficient. */
      }

      // NOTE: For an explanation of the different buffers being used, please refer
      // to the document bf-pipeline.txt in the GPUProc/doc directory.
      devA.reset(new gpu::DeviceMemory(context, devA_size));
      devB.reset(new gpu::DeviceMemory(context, devB_size));

      //################################################
      // Create objects containing the kernel and device buffers

      if (factories.correlator) {
        correlatorStep = std::auto_ptr<CorrelatorStep>(
          new CorrelatorStep(parset, queue, context, *factories.correlator,
          devA, devB, nrSubbandsPerSubbandProc));
      }

      if (factories.preprocessing) {
        preprocessingStep = std::auto_ptr<BeamFormerPreprocessingStep>(
          new BeamFormerPreprocessingStep(parset, queue, context, *factories.preprocessing, 
          devA, devB));
      }

      if (factories.coherentStokes) {
        coherentStep = std::auto_ptr<BeamFormerCoherentStep>(
          new BeamFormerCoherentStep(parset, queue, context, *factories.coherentStokes,
          devB));
      }

      if (factories.incoherentStokes) {
        incoherentStep = std::auto_ptr<BeamFormerIncoherentStep>(
          new BeamFormerIncoherentStep(parset, queue, context, *factories.incoherentStokes, 
              devA, devB));
      }


      LOG_INFO_STR("Pipeline configuration: "
        << (correlatorStep.get() ?    "[correlator] " : "")
        << (preprocessingStep.get() ? "[bf preproc] " : "")
        << (coherentStep.get() ?      "[coh stokes] " : "")
        << (incoherentStep.get() ?    "[incoh stokes] " : "")
      );
      
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i)
      {
        outputPool.free.append(new BeamFormedData(ps, context));
      }
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
      queue.writeBuffer(*devA, input.inputSamples, inputCounter, true);

      // Some additional buffers
      // Only upload delays if they changed w.r.t. the previous subband.
      if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
        if (correlatorStep.get()) {
          correlatorStep->writeInput(input);
        }

        if (preprocessingStep.get()) {
          preprocessingStep->writeInput(input);
        }

        if (coherentStep.get()) {
          coherentStep->writeInput(input);
        }

        prevSAP = SAP;
        prevBlock = block;
      }

      // ************************************************
      // Start the GPU processing

      if (correlatorStep.get()) {
        output.correlatedData.blockID = input.blockID;

        correlatorStep->process(input);
        correlatorStep->readOutput(output.correlatedData);
      }

      if (preprocessingStep.get()) {
        preprocessingStep->process(input);
      }

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

      // ************************************************
      // Do CPU computations while the GPU is working

      if (correlatorStep.get()) {
        correlatorStep->processCPU(input, output.correlatedData);
      }

      // Synchronise to assure that all the work in the data is done
      queue.synchronize();
    }

    bool BeamFormerSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      BeamFormedData &output = dynamic_cast<BeamFormedData&>(_output);

      if (correlatorStep.get()) {
        output.emit_correlatedData = correlatorStep->postprocessSubband(output.correlatedData);
      } else {
        output.emit_correlatedData = false;
      }

      return true;
    }

  }
}


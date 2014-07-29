//# SubbandProc.cc
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

#include "SubbandProc.h"
#include "KernelFactories.h"

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

#include <CoInterface/Parset.h>
#include <CoInterface/Align.h>

#include <ApplCommon/PosixTime.h>
#include <Common/LofarLogger.h>

#include <iomanip>

namespace LOFAR
{
  namespace Cobalt
  {
    SubbandProc::SubbandProc(
      const Parset &ps,
      gpu::Context &context,
      KernelFactories &factories,
      size_t nrSubbandsPerSubbandProc)
    :
      inputPool("SubbandProc::inputPool"),
      processPool("SubbandProc::processPool"),
      outputPool("SubbandProc::outputPool"),

      ps(ps),
      nrSubbandsPerSubbandProc(nrSubbandsPerSubbandProc),
      queue(gpu::Stream(context)),
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
          new CorrelatorStep(ps, queue, context, *factories.correlator,
          devA, devB, nrSubbandsPerSubbandProc));
      }

      if (factories.preprocessing) {
        preprocessingStep = std::auto_ptr<BeamFormerPreprocessingStep>(
          new BeamFormerPreprocessingStep(ps, queue, context, *factories.preprocessing, 
          devA, devB));
      }

      if (factories.coherentStokes) {
        coherentStep = std::auto_ptr<BeamFormerCoherentStep>(
          new BeamFormerCoherentStep(ps, queue, context, *factories.coherentStokes,
          devB));
      }

      if (factories.incoherentStokes) {
        incoherentStep = std::auto_ptr<BeamFormerIncoherentStep>(
          new BeamFormerIncoherentStep(ps, queue, context, *factories.incoherentStokes, 
              devA, devB));
      }


      LOG_INFO_STR("Pipeline configuration: "
        << (correlatorStep.get() ?    "[correlator] " : "")
        << (preprocessingStep.get() ? "[bf preproc] " : "")
        << (coherentStep.get() ?      "[coh stokes] " : "")
        << (incoherentStep.get() ?    "[incoh stokes] " : "")
      );

      // put enough objects in the inputPool to operate
      //
      // At least 3 items are needed for a smooth Pool operation.
      size_t nrInputDatas = std::max(3UL, 2 * nrSubbandsPerSubbandProc);
      for (size_t i = 0; i < nrInputDatas; ++i) {
        inputPool.free.append(new SubbandProcInputData(ps, context), false);
      }
      
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i)
      {
        outputPool.free.append(new SubbandProcOutputData(ps, context));
      }
    }


    size_t SubbandProc::nrOutputElements() const
    {
      /*
       * Output elements can get stuck in:
       *   process()                1 element
       *   Best-effort queue:       3 elements
       *   In flight to BE queue:   1 element
       *   In flight to outputProc: 1 element
       *
       * which means we'll need at least 7 elements
       * in the pool to get a smooth operation.
       */
      return 7 * nrSubbandsPerSubbandProc;
    }

    void SubbandProc::Flagger::convertFlagsToChannelFlags(Parset const &ps,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      MultiDimArray<SparseSet<unsigned>, 2>& flagsPerChannel)
    {
      unsigned numberOfChannels = ps.nrChannelsPerSubband();
      unsigned log2NrChannels = log2(numberOfChannels);
      //Convert the flags per sample to flags per channel
      for (unsigned station = 0; station < ps.nrStations(); station ++) 
      {
        // get the flag ranges
        const SparseSet<unsigned>::Ranges &ranges = inputFlags[station].getRanges();
        for (SparseSet<unsigned>::const_iterator it = ranges.begin();
          it != ranges.end(); it ++) 
        {
          unsigned begin_idx;
          unsigned end_idx;
          if (numberOfChannels == 1)
          {
            // do nothing, just take the ranges as supplied
            begin_idx = it->begin; 
            end_idx = std::min(ps.nrSamplesPerChannel(), it->end );
          }
          else
          {
            // Never flag before the start of the time range               
            // use bitshift to divide to the number of channels. 
            //
            // NR_TAPS is the width of the filter: they are
            // absorbed by the FIR and thus should be excluded
            // from the original flag set.
            //
            // At the same time, every sample is affected by
            // the NR_TAPS-1 samples before it. So, any flagged
            // sample in the input flags NR_TAPS samples in
            // the channel.
            begin_idx = std::max(0, 
              (signed) (it->begin >> log2NrChannels) - NR_TAPS + 1);

            // The min is needed, because flagging the last input
            // samples would cause NR_TAPS subsequent samples to
            // be flagged, which aren't necessarily part of this block.
            end_idx = std::min(ps.nrSamplesPerChannel() + 1, 
              ((it->end - 1) >> log2NrChannels) + 1);
          }

          // Now copy the transformed ranges to the channelflags
          for (unsigned ch = 0; ch < numberOfChannels; ch++) {
            flagsPerChannel[ch][station].include(begin_idx, end_idx);
          }
        }
      }
    }


    void SubbandProc::processSubband( SubbandProcInputData &input,
      SubbandProcOutputData &output)
    {
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
        correlatorStep->process(input);
        correlatorStep->readOutput(output);
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
        correlatorStep->processCPU(input, output);
      }

      // Synchronise to assure that all the work in the data is done
      queue.synchronize();
    }


    void SubbandProc::postprocessSubband(SubbandProcOutputData &output)
    {
      if (correlatorStep.get()) {
        output.emit_correlatedData = correlatorStep->postprocessSubband(output);
      } else {
        output.emit_correlatedData = false;
      }
    }
  }
}



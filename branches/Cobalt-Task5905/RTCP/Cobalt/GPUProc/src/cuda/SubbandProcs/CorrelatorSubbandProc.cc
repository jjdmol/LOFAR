//# CorrelatorSubbandProc.cc
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

#include "CorrelatorSubbandProc.h"

#include <cstring>
#include <algorithm>
#include <iomanip>

#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace Cobalt
  {
    CorrelatorSubbandProc::CorrelatorSubbandProc(
      const Parset &parset, gpu::Context &context, 
      CorrelatorStep::Factories &factories, size_t nrSubbandsPerSubbandProc)
      :
      SubbandProc(parset, context, nrSubbandsPerSubbandProc),       
      inputCounter(context, "input"),
      prevBlock(-1),
      prevSAP(-1)
    {
      devA.reset(new gpu::DeviceMemory(context, 
        factories.firFilter ? factories.firFilter->bufferSize(FIR_FilterKernel::INPUT_DATA)
                            : factories.delayAndBandPass.bufferSize(DelayAndBandPassKernel::INPUT_DATA)));

      devB.reset(new gpu::DeviceMemory(context, 
                      factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA)));
      correlatorStep = new CorrelatorStep(parset, queue, context, factories, devA, devB, nrSubbandsPerSubbandProc);

      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i) {
        outputPool.free.append(new CorrelatorStep::CorrelatedData(
                                 ps.settings.antennaFields.size(),
                                 ps.settings.correlator.nrChannels,
                                 ps.settings.correlator.nrSamplesPerChannel,
                                 context));
      }
    }


    void CorrelatorSubbandProc::processSubband(SubbandProcInputData &input,
                                               SubbandProcOutputData &_output)
    {
      CorrelatorStep::CorrelatedData &output = 
        dynamic_cast<CorrelatorStep::CorrelatedData&>(_output);

      // ***************************************************
      // Copy data to the GPU 
      queue.writeBuffer(*devA, input.inputSamples, inputCounter, true);
   
      if (ps.settings.delayCompensation.enabled) {
        const size_t block = input.blockID.block;
        const unsigned subband = input.blockID.globalSubbandIdx;
        const unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) 
        {
          correlatorStep->writeInput(input);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      correlatorStep->process(input);
      correlatorStep->readOutput(output);

      // The GPU will be occupied for a while, do some calculations in the
      // background.
      correlatorStep->processCPU(input, output);

      // Wait for the GPU to finish.
      queue.synchronize();
    }


    bool CorrelatorSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      CorrelatorStep::CorrelatedData &output = 
        dynamic_cast<CorrelatorStep::CorrelatedData&>(_output);

      return correlatorStep->postprocessSubband(output);
    }

  }
}


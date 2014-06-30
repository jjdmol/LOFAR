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
    CorrelatedDataHostBuffer::CorrelatedDataHostBuffer(
      unsigned nrStations, unsigned nrChannels,
      unsigned maxNrValidSamples, gpu::Context &context)
      :
      MultiDimArrayHostBuffer<fcomplex, 4>(
        boost::extents
        [nrStations * (nrStations + 1) / 2]
        [nrChannels][NR_POLARIZATIONS]
        [NR_POLARIZATIONS], 
        context, 0),
      CorrelatedData(nrStations, nrChannels, 
                     maxNrValidSamples, this->origin(),
                     this->num_elements(), heapAllocator, 1)
    {
    }


    void CorrelatedDataHostBuffer::reset()
    {
      CorrelatedData::reset();
    }

    CorrelatorSubbandProc::CorrelatorSubbandProc(
      const Parset &parset, gpu::Context &context, 
      CorrelatorStep::Factories &factories, size_t nrSubbandsPerSubbandProc)
      :
      SubbandProc(parset, context, nrSubbandsPerSubbandProc),       
      counters(context),
      prevBlock(-1),
      prevSAP(-1)
    {
      const bool correlatorPPF = ps.settings.correlator.nrChannels > 1;

      devA.reset(new gpu::DeviceMemory(context, 
        correlatorPPF ? factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA)
                      : std::max(factories.delayAndBandPass.bufferSize(DelayAndBandPassKernel::INPUT_DATA),
                                 factories.correlator.bufferSize(CorrelatorKernel::OUTPUT_DATA))));

      devB.reset(new gpu::DeviceMemory(context, 
        correlatorPPF ? std::max(factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA),
                                 factories.correlator.bufferSize(CorrelatorKernel::OUTPUT_DATA))
                      : factories.correlator.bufferSize(CorrelatorKernel::INPUT_DATA)));

      correlatorStep = new CorrelatorStep(parset, queue, context, factories, devA, devB, nrSubbandsPerSubbandProc);

      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < nrOutputElements(); ++i) {
        outputPool.free.append(new CorrelatedDataHostBuffer(
                                 ps.settings.antennaFields.size(),
                                 ps.settings.correlator.nrChannels,
                                 ps.settings.correlator.nrSamplesPerChannel,
                                 context));
      }
    }

    CorrelatorSubbandProc::~CorrelatorSubbandProc()
    {
    }

    void CorrelatorSubbandProc::printStats()
    {
      correlatorStep->printStats();
    }

    CorrelatorSubbandProc::Counters::Counters(gpu::Context &context)
      :
    samples(context)
    {}


    void CorrelatorSubbandProc::processSubband(SubbandProcInputData &input,
                                               SubbandProcOutputData &_output)
    {
      CorrelatedDataHostBuffer &output = 
        dynamic_cast<CorrelatedDataHostBuffer&>(_output);

      // ***************************************************
      // Copy data to the GPU 
      queue.writeBuffer(*devA, input.inputSamples, counters.samples, true);
   
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

      // ************************************************
      // Perform performance statistics if needed
      if (gpuProfiling)
      {
        correlatorStep->logTime();

        counters.samples.logTime();
      }
    }


    bool CorrelatorSubbandProc::postprocessSubband(SubbandProcOutputData &_output)
    {
      CorrelatedDataHostBuffer &output = 
        dynamic_cast<CorrelatedDataHostBuffer&>(_output);

      return correlatorStep->postprocessSubband(output);
    }

  }
}


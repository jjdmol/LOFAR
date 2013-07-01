//# BeamFormerKernel.cc
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

#include "BeamFormerKernel.h"
#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerKernel::BeamFormerKernel(const Parset &ps, 
                                       gpu::Context &context,
                                       gpu::DeviceMemory &devComplexVoltages,
                                       gpu::DeviceMemory &devCorrectedData,
                                       gpu::DeviceMemory &devBeamFormerWeights)
      :
      Kernel(ps, context, "BeamFormer.cu", "beamFormer")
    {
      itsFunction.setArg(0, devComplexVoltages);
      itsFunction.setArg(1, devCorrectedData);
      itsFunction.setArg(2, devBeamFormerWeights);
      // TODO: Hoe moet ik deze ook maar weer instellen?? Want 
      globalWorkSize = gpu::Grid(NR_POLARIZATIONS, 
                                 ps.nrTABs(0), 
                                 ps.nrChannelsPerSubband());
      localWorkSize = gpu::Block(NR_POLARIZATIONS, 
                                 ps.nrTABs(0), 
                                 ps.nrChannelsPerSubband());

      size_t nrWeightsBytes = bufferSize(ps, BEAM_FORMER_WEIGHTS);
      size_t nrSampleBytesPerPass = bufferSize(ps, INPUT_DATA);
      size_t nrComplexVoltagesBytesPerPass = bufferSize(ps, OUTPUT_DATA);

      size_t count = 
        ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS;
      unsigned nrPasses = std::max((ps.nrStations() + 6) / 16, 1U);

      nrOperations = count * ps.nrStations() * ps.nrTABs(0) * 8;
      nrBytesRead = 
        nrWeightsBytes + nrSampleBytesPerPass + (nrPasses - 1) * 
        nrComplexVoltagesBytesPerPass;
      nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
    }

    size_t 
    BeamFormerKernel::bufferSize(const Parset& ps, BufferType bufferType)
    {
      switch (bufferType) {
      case INPUT_DATA: 
        return
          ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * 
          NR_POLARIZATIONS * ps.nrStations() * sizeof(std::complex<float>);
      case OUTPUT_DATA:
        return
          ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * 
          NR_POLARIZATIONS * ps.maxNrTABs() * sizeof(std::complex<float>);
      case BEAM_FORMER_WEIGHTS:
        return 
          ps.nrStations() * ps.maxNrTABs() * ps.nrChannelsPerSubband() * 
          sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

  }
}


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
      setArg(0, devComplexVoltages);
      setArg(1, devCorrectedData);
      setArg(2, devBeamFormerWeights);
      // TODO: Hoe moet ik deze ook maar weer instellen?? Want 
      globalWorkSize = gpu::Grid(NR_POLARIZATIONS, ps.nrTABs(0), ps.nrChannelsPerSubband());
      localWorkSize = gpu::Block(NR_POLARIZATIONS, ps.nrTABs(0), ps.nrChannelsPerSubband());

      // FIXME: nrTABs
      //queue.enqueueNDRangeKernel(*this, cl::NullRange, gpu::dim3(16, ps.nrTABs(0), ps.nrChannelsPerSubband()), gpu::dim3(16, ps.nrTABs(0), 1), 0, &event);
      //queue.launchKernel(*this, gpu::dim3(16, ps.nrTABs(0), ps.nrChannelsPerSubband()), gpu::dim3(16, ps.nrTABs(0), 0); // TODO: extend/use Kernel::enqueue(). This will also correct the CUDA vs OpenCL interpret of gridSize (when fixed/enabled).

      size_t count = ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS;
      size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(0) * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      size_t nrSampleBytesPerPass = count * ps.nrStations() * sizeof(std::complex<float>);
      size_t nrComplexVoltagesBytesPerPass = count * ps.nrTABs(0) * sizeof(std::complex<float>);
      unsigned nrPasses = std::max((ps.nrStations() + 6) / 16, 1U);
      nrOperations = count * ps.nrStations() * ps.nrTABs(0) * 8;
      nrBytesRead = nrWeightsBytes + nrSampleBytesPerPass + (nrPasses - 1) * nrComplexVoltagesBytesPerPass;
      nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
    }

    size_t 
    BeamFormerKernel::bufferSize(const Parset& ps, BufferType bufferType)
    {
      switch (bufferType) {
      case INPUT_DATA: 
      case OUTPUT_DATA:
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

  }
}


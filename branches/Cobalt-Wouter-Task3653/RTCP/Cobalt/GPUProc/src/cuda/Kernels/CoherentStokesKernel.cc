//# CoherentStokesKernel.cc
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

#include "CoherentStokesKernel.h"

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>

#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    string CoherentStokesKernel::theirSourceFile = "BeamFormer/CoherentStokes.cu";
    string CoherentStokesKernel::theirFunction = "coherentStokes";

    CoherentStokesKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrTABs(ps.nrTABs(0)),
      nrStokes(ps.settings.beamFormer.coherentSettings.nrStokes),
      timeIntegrationFactor(ps.settings.beamFormer.coherentSettings.timeIntegrationFactor)
    {
    }


    CoherentStokesKernel::CoherentStokesKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction))
    {
      ASSERT(params.nrChannelsPerSubband >= 16 && params.nrChannelsPerSubband % 16 == 0);
      ASSERT(params.nrStokes == 1 || params.nrStokes == 4);
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      // TODO: params.nrTABs only works for one SAP
      globalWorkSize = gpu::Grid(256, (params.nrTABs + 15) / 16, (params.nrChannelsPerSubband + 15) / 16);
      localWorkSize = gpu::Block(256, 1, 1);

      nrOperations = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * (params.nrStokes == 1 ? 8 : 20 + 2.0 / params.timeIntegrationFactor);
      nrBytesRead = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) params.nrTABs * params.nrStokes * params.nrSamplesPerChannel / params.timeIntegrationFactor * params.nrChannelsPerSubband * sizeof(float);
    }

    template<> CompileDefinitions
    KernelFactory<CoherentStokesKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      return defs;
    }

  }
}


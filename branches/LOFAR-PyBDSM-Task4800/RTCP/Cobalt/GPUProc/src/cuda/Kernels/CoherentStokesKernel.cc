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

#include <boost/lexical_cast.hpp>

#include "CoherentStokesKernel.h"

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <GPUProc/global_defines.h>

using boost::lexical_cast;

namespace LOFAR
{
  namespace Cobalt
  {
    string CoherentStokesKernel::theirSourceFile = "CoherentStokes.cu";
    string CoherentStokesKernel::theirFunction = "coherentStokes";

    CoherentStokesKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrTABs(ps.settings.beamFormer.maxNrTABsPerSAP()),
      nrStokes(ps.settings.beamFormer.coherentSettings.nrStokes),
      timeIntegrationFactor(ps.settings.beamFormer.coherentSettings.timeIntegrationFactor)
    {
      nrChannelsPerSubband = ps.settings.beamFormer.coherentSettings.nrChannels;
      nrSamplesPerChannel  = ps.settings.beamFormer.coherentSettings.nrSamples(ps.nrSamplesPerSubband());

      // TODO: Add sensing of hardware to allow usage of the 1024 limit of k10
      // The TIME_PARALLEL_FACTOR depends on the size of the workload
      // We cannot have more then 512 threads on a average hw solution
      // We already asserted the itsParameters.nrTABs * itsParameters.nrChannelsPerSubband < 512
      timeParallelFactor = 512 / (nrTABs * nrChannelsPerSubband);
    }


    CoherentStokesKernel::CoherentStokesKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction))
    {
      ASSERT(params.nrChannelsPerSubband >= 16 && params.nrChannelsPerSubband % 16 == 0);
      ASSERT(params.timeIntegrationFactor > 0 && params.nrSamplesPerChannel % params.timeIntegrationFactor == 0);
      ASSERT(params.nrStokes == 1 || params.nrStokes == 4);
      ASSERT(params.nrChannelsPerSubband * params.nrTABs <= 512); // TODO This is a test for pre k10 hardware. We should increase this to 1024
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      // TODO: params.nrTABs only works for one SAP
      // TODO: Worksize larger the 512 are not supported yet.
      //       From sm 2.5 we could increase this to 1024
      globalWorkSize = gpu::Grid(params.nrTABs, params.timeParallelFactor, params.nrChannelsPerSubband);
      localWorkSize = gpu::Block(params.nrTABs, params.timeParallelFactor, params.nrChannelsPerSubband);

      nrOperations = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * (params.nrStokes == 1 ? 8 : 20 + 2.0 / params.timeIntegrationFactor);
      nrBytesRead = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) params.nrTABs * params.nrStokes * params.nrSamplesPerChannel / params.timeIntegrationFactor * params.nrChannelsPerSubband * sizeof(float);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t
    KernelFactory<CoherentStokesKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case CoherentStokesKernel::INPUT_DATA:
        return
          itsParameters.nrChannelsPerSubband * itsParameters.nrSamplesPerChannel *
          NR_POLARIZATIONS * itsParameters.nrTABs * sizeof(std::complex<float>);
      case CoherentStokesKernel::OUTPUT_DATA:
        return 
          itsParameters.nrTABs * itsParameters.nrStokes * itsParameters.nrSamplesPerChannel /
          itsParameters.timeIntegrationFactor * itsParameters.nrChannelsPerSubband * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
    KernelFactory<CoherentStokesKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["NR_TABS"] =
        lexical_cast<string>(itsParameters.nrTABs);
      defs["NR_COHERENT_STOKES"] =
        lexical_cast<string>(itsParameters.nrStokes); // TODO: nrStokes and timeIntegrationFactor cannot differentiate between coh and incoh, while there are separate defines for coh and incoh. Correct?
      defs["INTEGRATION_SIZE"] =
        lexical_cast<string>(itsParameters.timeIntegrationFactor);
      defs["TIME_PARALLEL_FACTOR"] =
        lexical_cast<string>(itsParameters.timeParallelFactor);

      return defs;
    }

  }
}


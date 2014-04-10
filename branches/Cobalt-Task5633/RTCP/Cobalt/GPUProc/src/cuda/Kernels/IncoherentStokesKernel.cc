//# IncoherentStokesKernel.cc
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

#include "IncoherentStokesKernel.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <Common/lofar_complex.h>
#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    using boost::lexical_cast;
    using boost::format;

    string IncoherentStokesKernel::theirSourceFile = "IncoherentStokes.cu";
    string IncoherentStokesKernel::theirFunction = "incoherentStokes";

    IncoherentStokesKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrStokes(ps.settings.beamFormer.incoherentSettings.nrStokes),
      timeIntegrationFactor(
        ps.settings.beamFormer.incoherentSettings.timeIntegrationFactor)
    {
      nrChannelsPerSubband = 
        ps.settings.beamFormer.incoherentSettings.nrChannels;
      nrSamplesPerChannel =
        ps.settings.beamFormer.incoherentSettings.nrSamples;
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.IncoherentStokesKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_IncoherentStokesKernel.dat") % 
            ps.settings.observationID);
    }

    IncoherentStokesKernel::IncoherentStokesKernel(const gpu::Stream& stream,
                                                   const gpu::Module& module,
                                                   const Buffers& buffers,
                                                   const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      unsigned nrTimes = 
        params.nrSamplesPerChannel / params.timeIntegrationFactor;
      unsigned nrPasses = 
        (nrTimes + maxThreadsPerBlock - 1) / maxThreadsPerBlock;
      unsigned nrTimesPerPass = 
        (nrTimes + nrPasses - 1) / nrPasses;

      LOG_DEBUG_STR("nrTimes = " << nrTimes);
      LOG_DEBUG_STR("nrPasses = " << nrPasses);
      LOG_DEBUG_STR("nrTimesPerPass = " << nrTimesPerPass);

      setEnqueueWorkSizes(
        gpu::Grid(params.nrChannelsPerSubband, nrTimesPerPass * nrPasses),
        gpu::Block(1, nrTimesPerPass));

    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t
    KernelFactory<IncoherentStokesKernel>::
    bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case IncoherentStokesKernel::INPUT_DATA:
        return 
          (size_t) itsParameters.nrStations * NR_POLARIZATIONS * 
          itsParameters.nrSamplesPerChannel * 
          itsParameters.nrChannelsPerSubband * sizeof(std::complex<float>);
      case IncoherentStokesKernel::OUTPUT_DATA:
        return 
          (size_t) itsParameters.nrStokes * itsParameters.nrSamplesPerChannel / 
          itsParameters.timeIntegrationFactor * 
          itsParameters.nrChannelsPerSubband * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
    KernelFactory<IncoherentStokesKernel>::compileDefinitions() const
    {
Backtrace();
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["TIME_INTEGRATION_FACTOR"] = 
        lexical_cast<string>(itsParameters.timeIntegrationFactor);
      defs["NR_CHANNELS"] = 
        lexical_cast<string>(itsParameters.nrChannelsPerSubband);
      defs["NR_INCOHERENT_STOKES"] = 
        lexical_cast<string>(itsParameters.nrStokes);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);
      defs["NR_STATIONS"] = 
        lexical_cast<string>(itsParameters.nrStations);
      return defs;
    }

  }
}


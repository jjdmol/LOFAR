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

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "BeamFormerKernel.h"

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <GPUProc/global_defines.h>

using boost::lexical_cast;

namespace LOFAR
{
  namespace Cobalt
  {
    string BeamFormerKernel::theirSourceFile = "BeamFormer.cu";
    string BeamFormerKernel::theirFunction = "beamFormer";

    BeamFormerKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrSAPs(ps.settings.beamFormer.SAPs.size()),
      nrTABs(ps.settings.beamFormer.maxNrTABsPerSAP()),
      weightCorrection(1.0f),  // TODO: Add a key to the parset to specify this
      subbandBandwidth(ps.settings.subbandWidth()),
      dumpBuffers(false)  // TODO: Add a key to the parset to specify this
    {
      // override the correlator settings with beamformer specifics
      nrChannelsPerSubband = ps.settings.beamFormer.coherentSettings.nrChannels;
      nrSamplesPerChannel  = ps.settings.beamFormer.coherentSettings.nrSamples(ps.nrSamplesPerSubband());
    }

    BeamFormerKernel::BeamFormerKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction))
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, buffers.beamFormerDelays);

      size_t maxChannelParallisation = std::min(params.nrChannelsPerSubband, maxThreadsPerBlock / NR_POLARIZATIONS / params.nrTABs);

      ASSERT(params.nrChannelsPerSubband % maxChannelParallisation == 0);

      globalWorkSize = gpu::Grid(NR_POLARIZATIONS, 
                                 params.nrTABs, 
                                 params.nrChannelsPerSubband);
      localWorkSize = gpu::Block(NR_POLARIZATIONS, 
                                 params.nrTABs, 
                                 maxChannelParallisation);

#if 0
      size_t nrDelaysBytes = bufferSize(ps, BEAM_FORMER_DELAYS);
      size_t nrSampleBytesPerPass = bufferSize(ps, INPUT_DATA);
      size_t nrComplexVoltagesBytesPerPass = bufferSize(ps, OUTPUT_DATA);

      size_t count = 
        params.nrChannelsPerSubband * params.nrSamplesPerChannel * NR_POLARIZATIONS;
      unsigned nrPasses = std::max((params.nrStations + 6) / 16, 1U);

      nrOperations = count * params.nrStations * params.nrTABs * 8;
      nrBytesRead = 
        nrDelaysBytes + nrSampleBytesPerPass + (nrPasses - 1) * 
        nrComplexVoltagesBytesPerPass;
      nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
#endif
    }

    void BeamFormerKernel::enqueue(gpu::Stream &queue, PerformanceCounter &counter,
                                   float subbandFrequency, unsigned SAP)
    {
      setArg(3, subbandFrequency);
      setArg(4, SAP);
      Kernel::enqueue(queue, counter);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t 
    KernelFactory<BeamFormerKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case BeamFormerKernel::INPUT_DATA: 
        return
          itsParameters.nrChannelsPerSubband * itsParameters.nrSamplesPerChannel *
          NR_POLARIZATIONS * itsParameters.nrStations * sizeof(std::complex<float>);
      case BeamFormerKernel::OUTPUT_DATA:
        return
          itsParameters.nrChannelsPerSubband * itsParameters.nrSamplesPerChannel *
          NR_POLARIZATIONS * itsParameters.nrTABs * sizeof(std::complex<float>);
      case BeamFormerKernel::BEAM_FORMER_DELAYS:
        return 
          itsParameters.nrSAPs * itsParameters.nrStations * itsParameters.nrTABs *
          sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    
    
    template<> CompileDefinitions
    KernelFactory<BeamFormerKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["NR_SAPS"] =
        lexical_cast<string>(itsParameters.nrSAPs);
      defs["NR_TABS"] =
        lexical_cast<string>(itsParameters.nrTABs);
      defs["WEIGHT_CORRECTION"] =
        str(boost::format("%.7ff") % itsParameters.weightCorrection);
      defs["SUBBAND_BANDWIDTH"] =
        str(boost::format("%.7ff") % itsParameters.subbandBandwidth);

      return defs;
    }
  }
}


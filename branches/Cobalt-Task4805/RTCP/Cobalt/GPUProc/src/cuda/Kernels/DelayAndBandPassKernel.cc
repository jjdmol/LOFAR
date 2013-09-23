//# DelayAndBandPassKernel.cc
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

#include "DelayAndBandPassKernel.h"

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <GPUProc/global_defines.h>
#include <GPUProc/BandPass.h>

#include <fstream>

using boost::lexical_cast;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    string DelayAndBandPassKernel::theirSourceFile = "DelayAndBandPass.cu";
    string DelayAndBandPassKernel::theirFunction = "applyDelaysAndCorrectBandPass";

    DelayAndBandPassKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrBitsPerSample(ps.settings.nrBitsPerSample),
      nrBytesPerComplexSample(ps.nrBytesPerComplexSample()),
      nrSAPs(ps.settings.SAPs.size()),
      delayCompensation(ps.settings.delayCompensation.enabled),
      correctBandPass(ps.settings.corrections.bandPass),
      transpose(correctBandPass), // sane for correlator; bf redefines
      subbandBandwidth(ps.settings.subbandWidth())
    {
      dumpBuffers = true; // TODO: Add a key to the parset to specify this
    }

    DelayAndBandPassKernel::DelayAndBandPassKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction), params.dumpBuffers),
      itsBuffers(buffers)
    {
      ASSERT(params.nrChannelsPerSubband % 16 == 0 || params.nrChannelsPerSubband == 1);
      ASSERT(params.nrSamplesPerChannel % 16 == 0);

      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(4, buffers.delaysAtBegin);
      setArg(5, buffers.delaysAfterEnd);
      setArg(6, buffers.phaseOffsets);
      setArg(7, buffers.bandPassCorrectionWeights);

      globalWorkSize = gpu::Grid(256, params.nrChannelsPerSubband == 1 ? 1 : params.nrChannelsPerSubband / 16, params.nrStations);
      localWorkSize = gpu::Block(256, 1, 1);

      size_t nrSamples = params.nrStations * params.nrChannelsPerSubband * params.nrSamplesPerChannel * NR_POLARIZATIONS;
      nrOperations = nrSamples * 12;
      nrBytesRead = nrBytesWritten = nrSamples * sizeof(std::complex<float>);

      // Initialise bandpass correction weights
      if (params.correctBandPass)
      {
        gpu::HostMemory bpWeights(stream.getContext(), buffers.bandPassCorrectionWeights.size());
        BandPass::computeCorrectionFactors(bpWeights.get<float>(), params.nrChannelsPerSubband);
        stream.writeBuffer(buffers.bandPassCorrectionWeights, bpWeights, true);
      }
    }


    void DelayAndBandPassKernel::enqueue(PerformanceCounter &counter, float subbandFrequency, size_t SAP)
    {
      setArg(2, subbandFrequency);
      setArg(3, SAP);
      Kernel::enqueue(counter);
    }

    void DelayAndBandPassKernel::dumpBuffers() const
    {
      LOG_INFO("Dumping output buffer");
      gpu::HostMemory buf(itsBuffers.output.fetch());
      std::ofstream ofs("DelayAndBandPassKernel_OutputBuffer.raw",
                        std::ios::binary);
      ofs.write(buf.get<char>(), buf.size());
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t 
    KernelFactory<DelayAndBandPassKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case DelayAndBandPassKernel::INPUT_DATA: 
        if (itsParameters.nrChannelsPerSubband == 1)
          return 
            itsParameters.nrStations * NR_POLARIZATIONS * 
            itsParameters.nrSamplesPerSubband *
            itsParameters.nrBytesPerComplexSample;
        else
          return 
            itsParameters.nrStations * NR_POLARIZATIONS * 
            itsParameters.nrSamplesPerSubband * sizeof(std::complex<float>);
      case DelayAndBandPassKernel::OUTPUT_DATA:
        return
          itsParameters.nrStations * NR_POLARIZATIONS * 
          itsParameters.nrSamplesPerSubband * sizeof(std::complex<float>);
      case DelayAndBandPassKernel::DELAYS:
        return 
          itsParameters.nrSAPs * itsParameters.nrStations * 
          NR_POLARIZATIONS * sizeof(double);
      case DelayAndBandPassKernel::PHASE_OFFSETS:
        return
          itsParameters.nrStations * NR_POLARIZATIONS * sizeof(double);
      case DelayAndBandPassKernel::BAND_PASS_CORRECTION_WEIGHTS:
        return
          itsParameters.nrChannelsPerSubband * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
    KernelFactory<DelayAndBandPassKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["NR_BITS_PER_SAMPLE"] =
        lexical_cast<string>(itsParameters.nrBitsPerSample);
      defs["NR_SAPS"] =
        lexical_cast<string>(itsParameters.nrSAPs);
      defs["SUBBAND_BANDWIDTH"] =
        str(format("%.7f") % itsParameters.subbandBandwidth);

      if (itsParameters.delayCompensation) {
        defs["DELAY_COMPENSATION"] = "1";
      }

      if (itsParameters.correctBandPass) {
        defs["BANDPASS_CORRECTION"] = "1";
      }

      if (itsParameters.transpose) {
        defs["DO_TRANSPOSE"] = "1";
      }

      return defs;
    }
  }
}

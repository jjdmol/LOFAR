//# FIR_FilterKernel.cc
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

#include "FIR_FilterKernel.h"
#include <GPUProc/global_defines.h>

#include <boost/lexical_cast.hpp>
#include <complex>

using namespace std;
using boost::lexical_cast;

namespace LOFAR
{
  namespace Cobalt
  {
    string FIR_FilterKernel::theirSourceFile = "FIR_Filter.cu";
    string FIR_FilterKernel::theirFunction = "FIR_filter";

    FIR_FilterKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrBitsPerSample(ps.nrBitsPerSample()),
      nrBytesPerComplexSample(ps.nrBytesPerComplexSample()),
      nrHistorySamples(ps.nrHistorySamples()),
      nrPPFTaps(ps.nrPPFTaps())
    {
    }

    FIR_FilterKernel::FIR_FilterKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction))
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, buffers.filterWeights);

      size_t maxNrThreads = 
        getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);

      unsigned totalNrThreads = 
        params.nrChannelsPerSubband * params.nrPolarizations * 2;
      unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;

      globalWorkSize = gpu::Grid(totalNrThreads, params.nrStations);
      localWorkSize = gpu::Block(totalNrThreads / nrPasses, 1);

      size_t nrSamples = 
        params.nrStations * params.nrChannelsPerSubband * 
        params.nrPolarizations;

      nrOperations = 
        nrSamples * params.nrSamplesPerChannel * params.nrPPFTaps * 2 * 2;

      nrBytesRead = 
        nrSamples * (params.nrPPFTaps - 1 + params.nrSamplesPerChannel) * 
        params.nrBytesPerComplexSample;

      nrBytesWritten = 
        nrSamples * params.nrSamplesPerChannel * sizeof(std::complex<float>);

      // Note that these constant weights are now (unnecessarily) stored on the
      // device for every workqueue. A single copy per device could be used, but
      // first verify that the device platform still allows workqueue overlap.
      FilterBank filterBank(true, params.nrPPFTaps, 
                            params.nrChannelsPerSubband, KAISER);
      filterBank.negateWeights();

      gpu::HostMemory firWeights(stream.getContext(), buffers.filterWeights.size());
      std::memcpy(firWeights.get<void>(), filterBank.getWeights().origin(),
                  firWeights.size());
      stream.writeBuffer(buffers.filterWeights, firWeights, true);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<>
    KernelFactory<FIR_FilterKernel>::KernelFactory(const Parset& ps) :
      itsParameters(ps)
    {
      itsPTX = createPTX(FIR_FilterKernel::theirSourceFile,
                         compileDefinitions(),
                         compileFlags());
    }

    template<>
    FIR_FilterKernel*
    KernelFactory<FIR_FilterKernel>::create(const gpu::Stream& stream,
                                            const Buffers& buffers) const
    {
      return new FIR_FilterKernel(
        stream, createModule(stream.getContext(), 
                             FIR_FilterKernel::theirSourceFile,
                             itsPTX), 
        buffers, itsParameters);
    }

    template<> size_t 
    KernelFactory<FIR_FilterKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case FIR_FilterKernel::INPUT_DATA: 
        return
          (itsParameters.nrHistorySamples + itsParameters.nrSamplesPerSubband) *
          itsParameters.nrStations * itsParameters.nrPolarizations * 
          itsParameters.nrBytesPerComplexSample;
      case FIR_FilterKernel::OUTPUT_DATA:
        return
          itsParameters.nrSamplesPerSubband * itsParameters.nrStations * 
          itsParameters.nrPolarizations * sizeof(std::complex<float>);
      case FIR_FilterKernel::FILTER_WEIGHTS:
        return 
          itsParameters.nrChannelsPerSubband * itsParameters.nrPPFTaps *
          sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
    KernelFactory<FIR_FilterKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["NR_BITS_PER_SAMPLE"] =
        lexical_cast<string>(itsParameters.nrBitsPerSample);
      defs["NR_TAPS"] =
        lexical_cast<string>(itsParameters.nrPPFTaps);
      return defs;
    }

    template<> CompileFlags
    KernelFactory<FIR_FilterKernel>::compileFlags() const
    {
      CompileFlags flags;
      return flags;
    }

  }
}

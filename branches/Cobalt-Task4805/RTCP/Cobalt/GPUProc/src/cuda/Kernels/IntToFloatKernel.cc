//# IntToFloatKernel.cc
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

#include "IntToFloatKernel.h"

#include <boost/lexical_cast.hpp>

#include <Common/lofar_complex.h>

#include <GPUProc/global_defines.h>

using boost::lexical_cast;

namespace LOFAR
{
  namespace Cobalt
  {
    string IntToFloatKernel::theirSourceFile = "IntToFloat.cu";
    string IntToFloatKernel::theirFunction = "intToFloat";

    IntToFloatKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrBitsPerSample(ps.settings.nrBitsPerSample),
      nrBytesPerComplexSample(ps.nrBytesPerComplexSample()),
      nrTAPs(ps.nrPPFTaps()),
      dumpBuffers(false)  // TODO: Add a key to the parset to specify this
    {
    }

    IntToFloatKernel::IntToFloatKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction))
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      size_t maxNrThreads;
      maxNrThreads = getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);
      globalWorkSize = gpu::Grid(maxNrThreads, params.nrStations);
      localWorkSize = gpu::Block(maxNrThreads, 1);

      size_t nrSamples = params.nrStations * params.nrChannelsPerSubband * NR_POLARIZATIONS;
      nrOperations = nrSamples * 2;
      nrBytesRead = nrSamples * 2 * params.nrBitsPerSample / 8;
      nrBytesWritten = nrSamples * sizeof(std::complex<float>);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t 
    KernelFactory<IntToFloatKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case IntToFloatKernel::INPUT_DATA:
        return
          itsParameters.nrStations * NR_POLARIZATIONS * 
          itsParameters.nrSamplesPerSubband * itsParameters.nrBytesPerComplexSample;
      case IntToFloatKernel::OUTPUT_DATA:
        return
          itsParameters.nrStations * NR_POLARIZATIONS * 
          itsParameters.nrSamplesPerSubband * sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
    KernelFactory<IntToFloatKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["NR_BITS_PER_SAMPLE"] =
        lexical_cast<string>(itsParameters.nrBitsPerSample);
      defs["NR_TAPS"] =
        lexical_cast<string>(itsParameters.nrTAPs);
      return defs;
    }

  }
}


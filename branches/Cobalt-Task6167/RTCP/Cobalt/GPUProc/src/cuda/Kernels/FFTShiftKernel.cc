//# FFTShiftKernel.cc
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

#include "FFTShiftKernel.h"

#include <GPUProc/gpu_utils.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>
#include <Common/lofar_complex.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <fstream>

using boost::lexical_cast;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    string FFTShiftKernel::theirSourceFile = "FFTShift.cu";
    string FFTShiftKernel::theirFunction = "FFTShift";

    FFTShiftKernel::Parameters::Parameters(const Parset& ps, unsigned nrSTABs, unsigned nrChannels, const std::string &name):
      Kernel::Parameters(name),
      nrSTABs(nrSTABs),

      nrChannels(nrChannels),
      nrSamplesPerChannel(ps.settings.blockSize / nrChannels)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.FFTShiftKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_FFTShiftKernel.dat") % 
            ps.settings.observationID);
    }


    size_t FFTShiftKernel::Parameters::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case FFTShiftKernel::INPUT_DATA:  // fall tru
      case FFTShiftKernel::OUTPUT_DATA:
        return (size_t)nrSTABs * NR_POLARIZATIONS *
          nrChannels * nrSamplesPerChannel *
          sizeof(std::complex<float>);
          
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    FFTShiftKernel::FFTShiftKernel(const gpu::Stream& stream,
                                   const gpu::Module& module,
                                   const Buffers& buffers,
                                   const Parameters& params) :
      CompiledKernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.input);

      // Number of samples per channel must be even
      ASSERT(params.nrSamplesPerChannel % 2 == 0);

      size_t nrSamples = 
        params.nrSTABs * NR_POLARIZATIONS *
        params.nrChannels * params.nrSamplesPerChannel;

      // The total number of samples must be divisible by the maximum number of
      // threads per block (typically 1024).
      ASSERT(nrSamples % maxThreadsPerBlock == 0);

      setEnqueueWorkSizes(
        gpu::Grid(nrSamples),   // use of grid.x only is safe in practice
        gpu::Block(maxThreadsPerBlock));

    }

    //--------  Template specializations for KernelFactory  --------//

    template<> CompileDefinitions
      KernelFactory<FFTShiftKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_CHANNELS"] = lexical_cast<string>(itsParameters.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      return defs;
    }

  }
}

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

#include "FFTShiftKernel.h"

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_utils.h>
#include <CoInterface/BlockID.h>
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

    FFTShiftKernel::Parameters::Parameters(const Parset& ps,
      unsigned channels):
      Kernel::Parameters(ps)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.FFTShiftKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_FFTShiftKernel.dat") % 
            ps.settings.observationID);
      nrChannels = channels;


      cout << "$#######nr nrStations:" << nrStations << endl;
    }

    FFTShiftKernel::FFTShiftKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      unsigned maxNrThreads;
      maxNrThreads = getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);

      cout << "************************ params.nrSamplesPerSubband:" << params.nrSamplesPerSubband << endl;
      setEnqueueWorkSizes( gpu::Grid(params.nrSamplesPerSubband ,
                           params.nrStations,
                           params.nrChannels),
                           gpu::Block(256, 1, 1) );

      unsigned nrSamples = params.nrStations * params.nrChannelsPerSubband * NR_POLARIZATIONS;
      nrOperations = (size_t) nrSamples * 2;
      nrBytesRead = (size_t) nrSamples * 2 * params.nrBitsPerSample / 8;
      nrBytesWritten = (size_t) nrSamples * sizeof(std::complex<float>);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t 
      KernelFactory<FFTShiftKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case FFTShiftKernel::INPUT_DATA:

        cout << "**************************************" << endl
          << (size_t)itsParameters.nrStations << endl
          << NR_POLARIZATIONS << endl
          << itsParameters.nrChannels << endl
          << itsParameters.nrSamplesPerSubband << endl;
        cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;

        return (size_t)itsParameters.nrStations * NR_POLARIZATIONS *
          itsParameters.nrChannels *
            itsParameters.nrSamplesPerSubband *
            sizeof(std::complex<float>);
          
      case FFTShiftKernel::OUTPUT_DATA:
        return  (size_t)itsParameters.nrStations * NR_POLARIZATIONS *
          itsParameters.nrChannels *
          itsParameters.nrSamplesPerSubband *
          sizeof(std::complex<float>);
          
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
      KernelFactory<FFTShiftKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      return defs;
    }

  }
}

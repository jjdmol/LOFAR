//# BeamFormerTransposeKernel.cc
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

#include "CoherentStokesTransposeKernel.h"

#include <GPUProc/gpu_utils.h>
#include <CoInterface/Align.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>
#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <fstream>

using boost::lexical_cast;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    string CoherentStokesTransposeKernel::theirSourceFile = "CoherentStokesTranspose.cu";
    string CoherentStokesTransposeKernel::theirFunction = "coherentStokesTranspose";

    CoherentStokesTransposeKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters("coherentStokesTranspose"),
      nrChannels(ps.settings.beamFormer.nrHighResolutionChannels),
      nrSamplesPerChannel(ps.settings.blockSize / nrChannels),

      nrTABs(ps.settings.beamFormer.maxNrCoherentTABsPerSAP())
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.CoherentStokesTransposeKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_CoherentStokesTransposeKernel.dat") % 
            ps.settings.observationID);
    }


    size_t CoherentStokesTransposeKernel::Parameters::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case CoherentStokesTransposeKernel::INPUT_DATA: 
      case CoherentStokesTransposeKernel::OUTPUT_DATA:
        return
          (size_t) nrChannels * nrSamplesPerChannel * 
            NR_POLARIZATIONS * nrTABs * sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    CoherentStokesTransposeKernel::
    CoherentStokesTransposeKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      CompiledKernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      ASSERT(params.nrSamplesPerChannel > 0);
      ASSERT(params.nrTABs > 0);
      ASSERT(params.nrChannels  % 16 == 0);

      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setEnqueueWorkSizes( gpu::Grid(align(params.nrTABs, 16),
                                     params.nrChannels,
                                     params.nrSamplesPerChannel),
                           gpu::Block(16, 16, 1) );

      nrOperations = 0;
      nrBytesRead = nrBytesWritten =
        (size_t) params.nrTABs * NR_POLARIZATIONS * params.nrChannels * 
        params.nrSamplesPerChannel * sizeof(std::complex<float>);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> CompileDefinitions
    KernelFactory<CoherentStokesTransposeKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_CHANNELS"] = lexical_cast<string>(itsParameters.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      defs["NR_TABS"] =
        lexical_cast<string>(itsParameters.nrTABs);

      return defs;
    }
  }
}


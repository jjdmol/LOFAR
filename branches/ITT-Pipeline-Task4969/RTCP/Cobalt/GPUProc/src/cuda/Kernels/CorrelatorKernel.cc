//# CorrelatorKernel.cc
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

#include "CorrelatorKernel.h"

#include <GPUProc/gpu_utils.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>
#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>

using boost::format;
using boost::lexical_cast;

namespace LOFAR
{
  namespace Cobalt
  {
    string CorrelatorKernel::theirSourceFile = "Correlator.cu";
    string CorrelatorKernel::theirFunction = "correlate";

    CorrelatorKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters("correlator"),
      nrStations(ps.settings.antennaFields.size()),
      // For Cobalt (= up to 80 antenna fields), the 2x2 kernel gives the best
      // performance.
      nrStationsPerThread(2),

      nrChannels(ps.settings.correlator.nrChannels),
      nrSamplesPerChannel(ps.settings.correlator.nrSamplesPerChannel)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.CorrelatorKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_CorrelatorKernel.dat") % 
            ps.settings.observationID);
    }

    unsigned CorrelatorKernel::Parameters::nrBaselines() const {
      return nrStations * (nrStations + 1) / 2;
    }


    size_t CorrelatorKernel::Parameters::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case CorrelatorKernel::INPUT_DATA:
        return
          (size_t) nrChannels * nrSamplesPerChannel * nrStations * 
            NR_POLARIZATIONS * sizeof(std::complex<float>);
      case CorrelatorKernel::OUTPUT_DATA:
        return 
          (size_t) nrBaselines() * nrChannels * 
            NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
      default: 
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    CorrelatorKernel::CorrelatorKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      CompiledKernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      unsigned preferredMultiple = 64; // FOR NVIDIA CUDA, there is no call to get this. Could check what the NV OCL pf says, but set to 64 for now. Generalize later (TODO).

      // Knowing the number of stations/thread, we can divide up our baselines
      // in bigger blocks (of stationsPerThread each).
      unsigned nrMacroStations = ceilDiv(params.nrStations, params.nrStationsPerThread);
      unsigned nrBlocks = nrMacroStations * (nrMacroStations + 1) / 2;

      unsigned nrPasses = ceilDiv(nrBlocks, maxThreadsPerBlock);
      unsigned nrThreads = ceilDiv(nrBlocks, nrPasses);
      nrThreads = align(nrThreads, preferredMultiple);

      //LOG_DEBUG_STR("nrBlocks = " << nrBlocks << ", nrPasses = " << nrPasses << ", preferredMultiple = " << preferredMultiple << ", nrThreads = " << nrThreads);

      unsigned nrUsableChannels = std::max(params.nrChannels - 1, 1U);
      setEnqueueWorkSizes( gpu::Grid(nrPasses * nrThreads, nrUsableChannels),
                           gpu::Block(nrThreads, 1) );

      nrOperations = (size_t) nrUsableChannels * params.nrBaselines() * params.nrSamplesPerChannel * 32;
      nrBytesRead = (size_t) nrPasses * params.nrStations * nrUsableChannels * params.nrSamplesPerChannel * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) params.nrBaselines() * nrUsableChannels * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }

    //--------  Template specializations for KernelFactory  --------//


    template<> CompileDefinitions
    KernelFactory<CorrelatorKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_STATIONS"] = lexical_cast<string>(itsParameters.nrStations);
      defs["NR_STATIONS_PER_THREAD"] = lexical_cast<string>(itsParameters.nrStationsPerThread);

      defs["NR_CHANNELS"] = lexical_cast<string>(itsParameters.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      return defs;
    }
  }
}


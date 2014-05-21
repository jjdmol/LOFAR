//# CoherentStokesKernel.cc
//# Copyright (C) 2012-2014  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "CoherentStokesKernel.h"

#include <utility>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Align.h>
#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_utils.h>

namespace LOFAR
{
  namespace Cobalt
  {
    using boost::lexical_cast;
    using boost::format;

    string CoherentStokesKernel::theirSourceFile = "CoherentStokes.cu";
    string CoherentStokesKernel::theirFunction = "coherentStokes";

    CoherentStokesKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters(ps),
      nrTABs(ps.settings.beamFormer.maxNrTABsPerSAP()),
      nrStokes(ps.settings.beamFormer.coherentSettings.nrStokes),
      outputComplexVoltages(ps.settings.beamFormer.coherentSettings.type == STOKES_XXYY),
      timeIntegrationFactor(
        ps.settings.beamFormer.coherentSettings.timeIntegrationFactor)
     {
      nrChannelsPerSubband = ps.settings.beamFormer.coherentSettings.nrChannels;
      nrSamplesPerChannel  = ps.settings.beamFormer.coherentSettings.nrSamples;

      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.CoherentStokesKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_CoherentStokesKernel.dat") % 
            ps.settings.observationID);
    }


    CoherentStokesKernel::CoherentStokesKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      ASSERT(params.timeIntegrationFactor > 0);
      ASSERT(params.nrSamplesPerChannel % params.timeIntegrationFactor == 0);
      ASSERT(params.nrStokes == 1 || params.nrStokes == 4);

      setArg(0, buffers.output);
      setArg(1, buffers.input);

      const gpu::Device device(_context.getDevice());

      const unsigned defaultNrChannelsPerBlock = 16;
      const unsigned defaultNrTABsPerBlock     = 16;

      const unsigned minNrChannelsPerBlock  = std::min(params.nrChannelsPerSubband,
                                                      defaultNrChannelsPerBlock);
      const unsigned minTimeParallelThreads = 1;
      const unsigned minNrTABsPerBlock      = std::min(params.nrTABs,
                                                       defaultNrTABsPerBlock);

      const gpu::Block maxLocalWorkSize     = device.getMaxBlockDims();
      const unsigned maxNrChannelsPerBlock  = std::min(params.nrChannelsPerSubband,
                                                       maxLocalWorkSize.x);
      const unsigned maxTimeParallelThreads = 
              std::min(params.nrSamplesPerChannel / params.timeIntegrationFactor,
                                                       maxLocalWorkSize.y);
      const unsigned maxNrTABsPerBlock      = std::min(params.nrTABs,
                                                       maxLocalWorkSize.z);

      // Generate possible execution configs and their (predicted) occupancy.
      // Preferred: >=64 thr/blk, max occupancy, smallest blk size (max #blks).
      gpu::ExecConfig selectedConfig;
      double maxOccupancy = 0.0;
      for (unsigned nrTABsPerBlock = minNrTABsPerBlock;
           nrTABsPerBlock <= maxNrTABsPerBlock; 
           nrTABsPerBlock = align(++nrTABsPerBlock, defaultNrTABsPerBlock)) {
        for (unsigned nrTimeParallelThreadsPerBlock = minTimeParallelThreads, incFactor = 2;
             nrTimeParallelThreadsPerBlock <= maxTimeParallelThreads;
             nrTimeParallelThreadsPerBlock *= incFactor = nextFactor(incFactor, maxTimeParallelThreads)) {
          for (unsigned nrChannelsPerBlock = minNrChannelsPerBlock;
               nrChannelsPerBlock <= maxNrChannelsPerBlock;
               nrChannelsPerBlock = align(++nrChannelsPerBlock, defaultNrChannelsPerBlock)) {

            // Grid dims in terms of #threads (Note: i.e. OpenCL semantics!)
            unsigned nrChannelThreads      = align(params.nrChannelsPerSubband,
                                                   nrChannelsPerBlock);
            unsigned nrTimeParallelThreads = align(maxTimeParallelThreads,
                                                   nrTimeParallelThreadsPerBlock);
            unsigned nrTABsThreads         = align(params.nrTABs, nrTABsPerBlock);
            gpu::Grid  grid (nrChannelThreads,   nrTimeParallelThreads,         nrTABsThreads);
            gpu::Block block(nrChannelsPerBlock, nrTimeParallelThreadsPerBlock, nrTABsPerBlock);

            // Filter configs that fall outside device caps (e.g. max threads per block).
            // Correct by construction is hard and risky.
            string errMsgs;
            setEnqueueWorkSizes(grid, block, &errMsgs);
            if (errMsgs.empty()) {
              double occupancy = predictMultiProcOccupancy();
              if (occupancy > maxOccupancy) {
                selectedConfig.grid  = grid;
                selectedConfig.block = block;
                maxOccupancy = occupancy;
                itsTimeParallelFactor = nrTimeParallelThreads;
              }
            } else {
              LOG_DEBUG_STR("Skipping invalid exec config for CoherentStokes: " << errMsgs);
            }

          }

          // The juggle with time par factors cannot be made correct for this corner-case,
          // as the smallest prime factor is 2. Consider such a config once, not inf times.
          if (maxTimeParallelThreads == 1)
            break;
        }
      }
      ASSERT(maxOccupancy > 0.0);

      ASSERTSTR(params.nrSamplesPerChannel % itsTimeParallelFactor == 0,
             "itsTimeParallelFactor=" << itsTimeParallelFactor);
      ASSERTSTR(params.nrSamplesPerChannel % (params.timeIntegrationFactor * itsTimeParallelFactor) == 0,
             "itsTimeParallelFactor=" << itsTimeParallelFactor);

      setEnqueueWorkSizes(selectedConfig.grid, selectedConfig.block);
      LOG_INFO_STR("Exec config for CoherentStokes has a (predicted) occupancy of " << maxOccupancy);

      // The itsTimeParallelFactor immediate kernel arg must outlive kernel runs.
      setArg(2, itsTimeParallelFactor); // could be a kernel define, but not yet known at kernel compilation


      nrOperations = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * (params.nrStokes == 1 ? 8 : 20 + 2.0 / params.timeIntegrationFactor);
      nrBytesRead = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) params.nrTABs * params.nrStokes * params.nrSamplesPerChannel / params.timeIntegrationFactor * params.nrChannelsPerSubband * sizeof(float);
    }

    unsigned CoherentStokesKernel::nextFactor(unsigned factor, unsigned maxFactor) const
    {
      // find the next factor value of maxFactor
      while (maxFactor % factor != 0) {
        factor += 1; // could skip non-primes, but can't gain back the effort
      }

      return factor;
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t
    KernelFactory<CoherentStokesKernel>::bufferSize(BufferType bufferType) const
    {

      switch (bufferType) {
      case CoherentStokesKernel::INPUT_DATA:
        return
          (size_t) itsParameters.nrChannelsPerSubband * itsParameters.nrSamplesPerChannel *
            NR_POLARIZATIONS * itsParameters.nrTABs * sizeof(std::complex<float>);

      case CoherentStokesKernel::OUTPUT_DATA:
        return 
          (size_t) itsParameters.nrTABs * itsParameters.nrStokes * itsParameters.nrSamplesPerChannel /
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
      defs["COMPLEX_VOLTAGES"] =
        itsParameters.outputComplexVoltages ? "1" : "0";
      defs["NR_COHERENT_STOKES"] =
        lexical_cast<string>(itsParameters.nrStokes);
      defs["TIME_INTEGRATION_FACTOR"] =
        lexical_cast<string>(itsParameters.timeIntegrationFactor);

      return defs;
    }

  }
}


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
#include <CoInterface/fpequals.h>
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


      // The following code paragraphs attempt to determine a reasonable
      // execution configuration for any #channels, timeParallelFactor, #TABs.
      //
      // We first produce viable configs combinatorically (starting with the
      // max and min parallelization dim constants), and
      // then filter these configs using some heuristics (no compilation or run).
      // We end up with 1 "best" config that we then apply after some assertions.
      // The used heuristics are crude. Lots of room for improvement.
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

      // Generate viable execution configurations.
      // Step through each of the 3 parallelization dims and verify if supp by hw.
      // Store candidates along with their timeParallelFactor to be passed as kernel arg.
      using std::vector;
      vector<CoherentStokesExecConfig> configs;
      for (unsigned nrTABsPerBlock = minNrTABsPerBlock;
           nrTABsPerBlock <= maxNrTABsPerBlock; 
           nrTABsPerBlock = align(++nrTABsPerBlock, defaultNrTABsPerBlock)) {
        for (unsigned nrTimeParallelThreadsPerBlock = minTimeParallelThreads;
             /* exit cond at the end */ ;
             nrTimeParallelThreadsPerBlock *= smallestFactorOf(maxTimeParallelThreads /
                                                               nrTimeParallelThreadsPerBlock)) {
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
            string errMsgs;
            setEnqueueWorkSizes(grid, block, &errMsgs);
            if (errMsgs.empty()) {
              CoherentStokesExecConfig ec;
              ec.grid  = grid;
              ec.block = block;
              ec.nrTimeParallelThreads = nrTimeParallelThreads;
              configs.push_back(ec);
            } else {
              LOG_DEBUG_STR("Skipping unsupported CoherentStokes exec config: " << errMsgs);
            }

          }

          // Loop exit check after (not before) considering a config, esp. for
          // maxTimeParallelThreads == 1 and other cases of smallestFactorOf(1).
          if (nrTimeParallelThreadsPerBlock == maxTimeParallelThreads)
            break;
        }
      }
      ASSERT(!configs.empty());

      // Select config to use by narrowing down a few times based on some heuristics.
      // Each time, the surviving exec configs are copied to a new vector.
      // The first filter prefers >=64 threads per block.
      // Thereafter, we prefer high occupancy, and max #blks (= smallest blk size).
      vector<CoherentStokesExecConfig> configsMinBlockSize;
      unsigned minThreadsPerBlock = 64;
      const unsigned warpSize = device.getAttribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE);
      while (minThreadsPerBlock >= warpSize) {
        for (vector<CoherentStokesExecConfig>::const_iterator it = configs.begin();
             it != configs.end(); ++it) {
          if (it->block.x * it->block.y * it->block.z >= minThreadsPerBlock) {
            configsMinBlockSize.push_back(*it);
          }
        }

        if (!configsMinBlockSize.empty()) {
          break;
        }
        minThreadsPerBlock /= 2;
      }
      if (configsMinBlockSize.empty()) {
        configsMinBlockSize = configs; // tough luck
      }

      // High occupancy heuristic.
      vector<CoherentStokesExecConfig> configsMaxOccupancy;
      double maxOccupancy = 0.0;
      for (vector<CoherentStokesExecConfig>::const_iterator it = configsMinBlockSize.begin();
           it != configsMinBlockSize.end(); ++it) {
        setEnqueueWorkSizes(it->grid, it->block);
        double occupancy = predictMultiProcOccupancy();
        if (fpEquals(occupancy, maxOccupancy, 0.05)) { // 0.05 occ diff is meaningless for sure
          configsMaxOccupancy.push_back(*it);
        } else if (occupancy > maxOccupancy) {
          maxOccupancy = occupancy;
          configsMaxOccupancy.clear();
          configsMaxOccupancy.push_back(*it);
        }
      }

      // Within earlier constraints, prefer min block size to maximize #blocks.
      CoherentStokesExecConfig selectedConfig;
      unsigned minBlockSizeSeen = maxLocalWorkSize.x * maxLocalWorkSize.y * maxLocalWorkSize.z + 1;
      for (vector<CoherentStokesExecConfig>::const_iterator it = configsMaxOccupancy.begin();
           it != configsMaxOccupancy.end(); ++it) {
        unsigned blockSize = it->block.x * it->block.y * it->block.z;
        if (blockSize < minBlockSizeSeen) {
          minBlockSizeSeen = blockSize;
          selectedConfig = *it;
        }
        LOG_DEBUG_STR("Coherent Stokes exec config candidate (last round): " << *it);
      }

      itsTimeParallelFactor = selectedConfig.nrTimeParallelThreads;
      ASSERTSTR(params.nrSamplesPerChannel % itsTimeParallelFactor == 0,
             "itsTimeParallelFactor=" << itsTimeParallelFactor);
      ASSERTSTR(params.nrSamplesPerChannel % (params.timeIntegrationFactor * itsTimeParallelFactor) == 0,
             "itsTimeParallelFactor=" << itsTimeParallelFactor);
      setEnqueueWorkSizes(selectedConfig.grid, selectedConfig.block);
      LOG_INFO_STR("Coherent Stokes exec config has a (predicted) occupancy of " << maxOccupancy); // off by max 0.05...

      // The itsTimeParallelFactor immediate kernel arg must outlive kernel runs.
      setArg(2, itsTimeParallelFactor); // could be a kernel define, but not yet known at kernel compilation


      nrOperations = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * (params.nrStokes == 1 ? 8 : 20 + 2.0 / params.timeIntegrationFactor);
      nrBytesRead = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) params.nrTABs * params.nrStokes * params.nrSamplesPerChannel / params.timeIntegrationFactor * params.nrChannelsPerSubband * sizeof(float);
    }

    std::ostream& operator<<(std::ostream& os,
            const CoherentStokesKernel::CoherentStokesExecConfig& execConfig)
    {
      os << "{" << execConfig.grid << ", " << execConfig.block <<
            ", " << execConfig.dynSharedMemSize <<
            ", " << execConfig.nrTimeParallelThreads << "}";
      return os;
    }

    unsigned CoherentStokesKernel::smallestFactorOf(unsigned n) const
    {
      if (n % 2 == 0) {
        return 2;
      }

      unsigned sqrtn = sqrt(n+1); // +1: avoid e.g. sqrt(25)=4.999...
      for (unsigned i = 3; i <= sqrtn; i += 2) {
        if (n % i == 0) {
          return i;
        }
      }

      return n;
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


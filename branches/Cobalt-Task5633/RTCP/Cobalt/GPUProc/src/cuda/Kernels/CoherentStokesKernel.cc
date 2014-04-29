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
      ASSERT(params.nrStokes == 1 || params.nrStokes == 4);

      setArg(0, buffers.output);
      setArg(1, buffers.input);

      timeParallelFactor = module.getContext().getDevice().getMaxThreadsPerBlock() / params.nrChannelsPerSubband;
      if (params.nrSamplesPerChannel < timeParallelFactor)
        timeParallelFactor = 1;


      setEnqueueWorkSizes(gpu::Grid (params.nrChannelsPerSubband, timeParallelFactor, params.nrTABs),
                          gpu::Block(params.nrChannelsPerSubband, timeParallelFactor, 1));

      // The timeParallelFactor immediate kernel arg must outlive kernel runs.
      setArg(2, timeParallelFactor);
#if 0
      // Process 16 channels and 16 TABs per block, unless fewer needed. Use kernel checks to skip unneeded work.
      const unsigned maxNrChannelsPerBlock = 16;
      const unsigned maxNrTABsPerBlock     = 16;

      // block dims
      const unsigned nrChannelsPerBlock = std::min(params.nrChannelsPerSubband, maxNrChannelsPerBlock);
      unsigned nrTABsPerBlock           = std::min(params.nrTABs,               maxNrTABsPerBlock);

      // grid dims in terms of #threads (OpenCL semantics)
      const unsigned nrChannelThreads = align(params.nrChannelsPerSubband, nrChannelsPerBlock);
      unsigned nrTABsThreads          = align(params.nrTABs,               nrTABsPerBlock);

      // With few channels use more time parallellism.
      // Don't do that with TABs just yet. #TABs may not be a power of 2.
      // Ensure no time parallel boundary falls within an integration step.
      const unsigned maxNrIntegrationsMultiple = params.nrSamplesPerChannel / params.timeIntegrationFactor;
      const unsigned maxTimeParallelFactor = module.getContext().getDevice().getMaxThreadsPerBlock() /
                                             (maxNrTABsPerBlock * nrChannelsPerBlock);
      ASSERT(maxNrIntegrationsMultiple > 0);
      ASSERT(maxTimeParallelFactor > 0);
      unsigned timeParallelFactor = gcd(maxNrIntegrationsMultiple, maxTimeParallelFactor);

      // 1st order (expected)
      gpu::Block block(nrChannelsPerBlock, timeParallelFactor, nrTABsPerBlock);
      gpu::Grid  grid (nrChannelThreads,   timeParallelFactor, nrTABsThreads);
      setEnqueueWorkSizes(grid, block);

      // If we end up with few (large) blocks, tone down the block size,
      // preferably in the TAB dim, else in the time dim.
      // May not be possible with small problem sizes; doesn't matter.
      const unsigned minBlockSize = 64; // don't go smaller
      const unsigned expectedBlockSize = block.x * block.y * block.z;
      const unsigned acceptableFewerBlockSizeFactor = std::max(1U, expectedBlockSize / minBlockSize);

      const unsigned expectedNrBlocks = (grid.x / block.x) * (grid.y / block.y) * (grid.z / block.z);
      const unsigned desiredExtraNrBlocksFactor = std::max( 1U,
          (2 * module.getContext().getDevice().getMultiProcessorCount() + expectedNrBlocks - 1) / expectedNrBlocks ); // K10 MPCount=8

      const unsigned triedTransferFactor        = std::min(acceptableFewerBlockSizeFactor, desiredExtraNrBlocksFactor);
      const unsigned transferTABsThreadsFactor  = gcd(triedTransferFactor, nrTABsPerBlock);
      const unsigned remainingTriedTransferFactor = triedTransferFactor / transferTABsThreadsFactor;
      const unsigned transferTimeParallelFactor = gcd(remainingTriedTransferFactor, timeParallelFactor);

      // apply transferTABsThreadsFactor and transferTimeParallelFactor (may be 1)
      nrTABsPerBlock     /= transferTABsThreadsFactor;
      timeParallelFactor /= transferTimeParallelFactor;
      nrTABsThreads = align(params.nrTABs, nrTABsPerBlock);

      ASSERT(params.nrSamplesPerChannel % timeParallelFactor == 0);
      ASSERT(params.nrSamplesPerChannel % (params.timeIntegrationFactor * timeParallelFactor) == 0);

      setArg(2, timeParallelFactor); // could be a kernel define, but not yet known at kernel compilation

      // 2nd order (final)
      setEnqueueWorkSizes(grid, block);
#endif

      nrOperations = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * (params.nrStokes == 1 ? 8 : 20 + 2.0 / params.timeIntegrationFactor);
      nrBytesRead = (size_t) params.nrChannelsPerSubband * params.nrSamplesPerChannel * params.nrTABs * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) params.nrTABs * params.nrStokes * params.nrSamplesPerChannel / params.timeIntegrationFactor * params.nrChannelsPerSubband * sizeof(float);
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


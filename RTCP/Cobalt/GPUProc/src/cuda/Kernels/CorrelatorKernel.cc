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

#include <vector>
#include <algorithm>

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <CoInterface/Align.h>
#include <CoInterface/Exceptions.h>

#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {

#if !defined USE_NEW_CORRELATOR
    CorrelatorKernel::CorrelatorKernel(const Parset &ps, 
                                       gpu::Context &context,
                                       gpu::DeviceMemory &devVisibilities,
                                       gpu::DeviceMemory &devCorrectedData)
      :
# if defined USE_4X4
      Kernel(ps, context, "Correlator.cu", "correlate_4x4")
# elif defined USE_3X3
      Kernel(ps, context, "Correlator.cu", "correlate_3x3")
# elif defined USE_2X2
      Kernel(ps, context, "Correlator.cu", "correlate_2x2")
# else
      Kernel(ps, context, "Correlator.cu", "correlate")
# endif
    {
      init(devVisibilities, devCorrectedData);
    }

    CorrelatorKernel::CorrelatorKernel(const Parset &ps, 
                                       gpu::Module &module,
                                       gpu::DeviceMemory &devVisibilities,
                                       gpu::DeviceMemory &devCorrectedData)
      :
# if defined USE_4X4
      Kernel(ps, module, "correlate_4x4")
# elif defined USE_3X3
      Kernel(ps, module, "correlate_3x3")
# elif defined USE_2X2
      Kernel(ps, module, "correlate_2x2")
# else
      Kernel(ps, module, "correlate")
# endif
    {
      init(devVisibilities, devCorrectedData);
    }

    void CorrelatorKernel::init(gpu::DeviceMemory &devVisibilities,
                                gpu::DeviceMemory &devCorrectedData)
    {
      setArg(0, devVisibilities);
      setArg(1, devCorrectedData);

      size_t maxNrThreads, preferredMultiple;
      maxNrThreads = getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);

      //std::vector<cl_context_properties> properties;
      //if (gpu::Platform((cl_platform_id) properties[1]).getInfo<CL_PLATFORM_NAME>() == "AMD Accelerated Parallel Processing") {
      gpu::Platform pf; // Redecl not so great. Generalize for OpenCL later, then remove prev commented lines
      if (pf.getName() == "AMD Accelerated Parallel Processing") {
        preferredMultiple = 256;
      } else {
        preferredMultiple = 64; // FOR NVIDIA CUDA, there is no call to get this. Could check what the NV OCL pf says, but set to 64 for now. Generalize later.
      }

# if defined USE_4X4
      unsigned quartStations = (ps.nrStations() + 2) / 4;
      unsigned nrBlocks = quartStations * (quartStations + 1) / 2;
# elif defined USE_3X3
      unsigned thirdStations = (ps.nrStations() + 2) / 3;
      unsigned nrBlocks = thirdStations * (thirdStations + 1) / 2;
# elif defined USE_2X2
      unsigned halfStations = (ps.nrStations() + 1) / 2;
      unsigned nrBlocks = halfStations * (halfStations + 1) / 2;
# else
      unsigned nrBlocks = ps.nrBaselines();
# endif
      unsigned nrPasses = (nrBlocks + maxNrThreads - 1) / maxNrThreads;
      unsigned nrThreads = (nrBlocks + nrPasses - 1) / nrPasses;
      nrThreads = (nrThreads + preferredMultiple - 1) / preferredMultiple * preferredMultiple;
      //LOG_DEBUG_STR("nrBlocks = " << nrBlocks << ", nrPasses = " << nrPasses << ", preferredMultiple = " << preferredMultiple << ", nrThreads = " << nrThreads);

      unsigned nrUsableChannels = std::max(ps.nrChannelsPerSubband() - 1, 1U);
      globalWorkSize = gpu::Grid(nrPasses * nrThreads, nrUsableChannels);
      localWorkSize = gpu::Block(nrThreads, 1);

      nrOperations = (size_t) nrUsableChannels * ps.nrBaselines() * ps.nrSamplesPerChannel() * 32;
      nrBytesRead = (size_t) nrPasses * ps.nrStations() * nrUsableChannels * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrBaselines() * nrUsableChannels * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }

#else


    CorrelatorKernel::CorrelatorKernel(const Parset &ps, 
                                       gpu::Context &context,
                                       gpu::DeviceMemory &devVisibilities,
                                       gpu::DeviceMemory &devCorrectedData)
      :
# if defined USE_2X2
      Kernel(ps, context, "NewCorrelator.cu", "correlate")
# else
#  error not implemented
# endif
    {
      setArg(0, devVisibilities);
      setArg(1, devCorrectedData);

      unsigned nrRectanglesPerSide = (ps.nrStations() - 1) / (2 * 16);
      unsigned nrRectangles = nrRectanglesPerSide * (nrRectanglesPerSide + 1) / 2;
      //LOG_DEBUG_STR("nrRectangles = " << nrRectangles);

      unsigned nrBlocksPerSide = (ps.nrStations() + 2 * 16 - 1) / (2 * 16);
      unsigned nrBlocks = nrBlocksPerSide * (nrBlocksPerSide + 1) / 2;
      //LOG_DEBUG_STR("nrBlocks = " << nrBlocks);

      unsigned nrUsableChannels = std::max(ps.nrChannelsPerSubband() - 1, 1U);
      globalWorkSize = gpu::Grid(16 * 16, nrBlocks, nrUsableChannels);
      localWorkSize = gpu::Block(16 * 16, 1, 1);

      // FIXME
      //nrOperations   = (size_t) (32 * 32) * nrRectangles * nrUsableChannels * ps.nrSamplesPerChannel() * 32;
      nrOperations = (size_t) ps.nrBaselines() * ps.nrSamplesPerSubband() * 32;
      nrBytesRead = (size_t) (32 + 32) * nrRectangles * nrUsableChannels * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) (32 * 32) * nrRectangles * nrUsableChannels * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }

    CorrelateRectangleKernel::
    CorrelateRectangleKernel(const Parset &ps,
                             gpu::Context &context,
                             gpu::DeviceMemory &devVisibilities,
                             gpu::DeviceMemory &devCorrectedData)
      :
# if defined USE_2X2
      Kernel(ps, context, "NewCorrelator.cu", "correlateRectangleKernel")
# else
#  error not implemented
# endif
    {
      setArg(0, devVisibilities);
      setArg(1, devCorrectedData);

      unsigned nrRectanglesPerSide = (ps.nrStations() - 1) / (2 * 16);
      unsigned nrRectangles = nrRectanglesPerSide * (nrRectanglesPerSide + 1) / 2;
      LOG_DEBUG_STR("nrRectangles = " << nrRectangles);

      unsigned nrUsableChannels = std::max(ps.nrChannelsPerSubband() - 1, 1U);
      globalWorkSize = gpu::Grid(16 * 16, nrRectangles, nrUsableChannels);
      localWorkSize = gpu::Block(16 * 16, 1, 1);

      nrOperations = (size_t) (32 * 32) * nrRectangles * nrUsableChannels * ps.nrSamplesPerChannel() * 32;
      nrBytesRead = (size_t) (32 + 32) * nrRectangles * nrUsableChannels * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) (32 * 32) * nrRectangles * nrUsableChannels * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }


    CorrelateTriangleKernel::
    CorrelateTriangleKernel(const Parset &ps,
                            gpu::Context &context,
                            gpu::DeviceMemory &devVisibilities,
                            gpu::DeviceMemory &devCorrectedData)
      :
# if defined USE_2X2
      Kernel(ps, context, "NewCorrelator.cu", "correlateTriangleKernel")
# else
#  error not implemented
# endif
    {
      setArg(0, devVisibilities);
      setArg(1, devCorrectedData);

      unsigned nrTriangles = (ps.nrStations() + 2 * 16 - 1) / (2 * 16);
      unsigned nrMiniBlocksPerSide = 16;
      unsigned nrMiniBlocks = nrMiniBlocksPerSide * (nrMiniBlocksPerSide + 1) / 2;
      size_t preferredMultiple;
      preferredMultiple = 64; // FOR NVIDIA CUDA, there is no call to get this. Could check what the NV OCL pf says, but set to 64 for now. Generalize later.
      unsigned nrThreads = align(nrMiniBlocks, preferredMultiple);

      LOG_DEBUG_STR("nrTriangles = " << nrTriangles << ", nrMiniBlocks = " << nrMiniBlocks << ", nrThreads = " << nrThreads);

      unsigned nrUsableChannels = std::max(ps.nrChannelsPerSubband() - 1, 1U);
      globalWorkSize = gpu::Grid(nrThreads, nrTriangles, nrUsableChannels);
      localWorkSize = gpu::Block(nrThreads, 1, 1);

      nrOperations = (size_t) (32 * 32 / 2) * nrTriangles * nrUsableChannels * ps.nrSamplesPerChannel() * 32;
      nrBytesRead = (size_t) 32 * nrTriangles * nrUsableChannels * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) (32 * 32 / 2) * nrTriangles * nrUsableChannels * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }

#endif

    size_t CorrelatorKernel::bufferSize(const Parset& ps, BufferType bufferType)
    {
      switch (bufferType) {
      case INPUT_DATA:
        return
          ps.nrSamplesPerSubband() * ps.nrStations() * 
          NR_POLARIZATIONS * sizeof(std::complex<float>);
      case OUTPUT_DATA:
        return 
          ps.nrBaselines() * ps.nrChannelsPerSubband() * 
          NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
      default: 
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

  }
}


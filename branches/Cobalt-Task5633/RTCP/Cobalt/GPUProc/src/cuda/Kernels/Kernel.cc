//# Kernel.cc
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

#include <ostream>
#include <sstream>
#include <boost/format.hpp>
#include <cuda_runtime.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/PerformanceCounter.h>
#include <CoInterface/Parset.h>
#include <CoInterface/BlockID.h>
#include <Common/LofarLogger.h>

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {
    Kernel::Parameters::Parameters(const Parset& ps) :
      nrStations(ps.nrStations()),
      nrChannelsPerSubband(ps.nrChannelsPerSubband()),
      nrSamplesPerChannel(ps.nrSamplesPerChannel()),
      nrSamplesPerSubband(ps.nrSamplesPerSubband()),
      nrPolarizations(NR_POLARIZATIONS),
      dumpBuffers(false)
    {
    }

    Kernel::~Kernel()
    {
    }

    Kernel::Kernel(const gpu::Stream& stream, 
                   const gpu::Function& function,
                   const Buffers &buffers,
                   const Parameters &params)
      : 
      gpu::Function(function),
      itsCounter(stream.getContext()),
      maxThreadsPerBlock(
        stream.getContext().getDevice().getMaxThreadsPerBlock()),
      itsStream(stream),
      itsBuffers(buffers),
      itsParameters(params)
    {
      LOG_INFO_STR(
        "Function " << function.name() << ":" << 
        "\n  nr. of registers used : " <<
        getAttribute(CU_FUNC_ATTRIBUTE_NUM_REGS) <<
        "\n  nr. of bytes of shared memory used (static) : " <<
        getAttribute(CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES)
      );
    }

    void Kernel::setEnqueueWorkSizes(gpu::Grid globalWorkSize, 
                                     gpu::Block localWorkSize)
    {
      gpu::Grid grid;
      ostringstream errMsgs;

      // Enforce by the hardware supported work sizes to see errors early

      gpu::Block maxLocalWorkSize = 
        itsStream.getContext().getDevice().getMaxBlockDims();
      if (localWorkSize.x > maxLocalWorkSize.x ||
          localWorkSize.y > maxLocalWorkSize.y ||
          localWorkSize.z > maxLocalWorkSize.z)
        errMsgs << "  - localWorkSize must be at most " << maxLocalWorkSize 
                << endl;

      if (localWorkSize.x * localWorkSize.y * localWorkSize.z > 
          maxThreadsPerBlock)
        errMsgs << "  - localWorkSize total must be at most " 
                << maxThreadsPerBlock << " threads/block" << endl;

      // globalWorkSize may (in theory) be all zero (no work), so allow.
      // Do reject an all zero localWorkSize. We need to mod or div by it.
      if (localWorkSize.x == 0 || 
          localWorkSize.y == 0 ||
          localWorkSize.z == 0) {
        errMsgs << "  - localWorkSize dimensions must be non-zero" << endl;
      } else {
        if (globalWorkSize.x % localWorkSize.x != 0 ||
            globalWorkSize.y % localWorkSize.y != 0 ||
            globalWorkSize.z % localWorkSize.z != 0)
          errMsgs << "  - globalWorkSize must divide localWorkSize" << endl;
        grid = gpu::Grid(globalWorkSize.x / localWorkSize.x,
                         globalWorkSize.y / localWorkSize.y,
                         globalWorkSize.z / localWorkSize.z);

        gpu::Grid maxGridWorkSize =
          itsStream.getContext().getDevice().getMaxGridDims();
        if (grid.x > maxGridWorkSize.x ||
            grid.y > maxGridWorkSize.y ||
            grid.z > maxGridWorkSize.z)
          errMsgs << "  - globalWorkSize / localWorkSize must be at most "
                  << maxGridWorkSize << endl;
      }

      string errStr(errMsgs.str());
      if (!errStr.empty())
        THROW(gpu::GPUException,
              "setEnqueueWorkSizes(): unsupported globalWorkSize " <<
              globalWorkSize << " and/or localWorkSize " << localWorkSize <<
              " selected:" << endl << errStr);

      LOG_DEBUG_STR("CUDA Grid size: " << grid);
      LOG_DEBUG_STR("CUDA Block size: " << localWorkSize);

      itsGridDims = grid;
      itsBlockDims = localWorkSize;
    }

    unsigned Kernel::getNrBlocksPerMultiProc(unsigned dynSharedMemBytes) const
    {
      // See NVIDIA's CUDA_Occupancy_Calculator.xls
      // TODO: Take warp allocation granularity into account. (Or only use a multiple of 32.)
      const gpu::Device device(_context.getDevice());
      const unsigned computeCapMajor = device.getAttribute(CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR);
      const unsigned warpSize = device.getAttribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE);
      unsigned factor;

      // #blocks regardless of kernel
      /*const */unsigned maxBlocksPerMultiProc; // no device.getAttribute() to retrieve it
      switch (computeCapMajor) {
        case 1:
        case 2:  maxBlocksPerMultiProc =  8; break;
        case 3:  maxBlocksPerMultiProc = 16; break;
        default: maxBlocksPerMultiProc = 16; break; // guess; unknown for future hardware
      }
      factor = maxBlocksPerMultiProc;

      // take block size into account
      unsigned nrThreadsPerBlock = itsBlockDims.x * itsBlockDims.y * itsBlockDims.z;
      nrThreadsPerBlock = (nrThreadsPerBlock + warpSize - 1) & ~(warpSize - 1); // assumes warpSize is a pow of 2
      const unsigned maxThreadsPerMP = device.getAttribute(CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR);
      unsigned threadsFactor = maxThreadsPerMP / nrThreadsPerBlock;
      if (threadsFactor < factor)
        factor = threadsFactor;

      // number of registers
      unsigned nrRegsPerThread = getAttribute(CU_FUNC_ATTRIBUTE_NUM_REGS);
      if (nrRegsPerThread > 0) {
        unsigned nrRegsPerBlock;
        // sm_1x devices apply the reg gran per block. Newer devices apply it per warp.
        if (computeCapMajor == 1) {
          const unsigned computeCapMinor = device.getAttribute(CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR);
          nrRegsPerBlock = nrRegsPerThread * nrThreadsPerBlock;
          unsigned regsGranularity = computeCapMinor <= 1 ? 256 : 512;
          nrRegsPerBlock = (nrRegsPerBlock + regsGranularity - 1) & ~(regsGranularity - 1); // assumes regsGranularity is a pow of 2
        } else {
          unsigned nrRegsPerWarp = nrRegsPerThread * warpSize;
          unsigned regsGranularity;
          switch (computeCapMajor) {
            case 2:  regsGranularity = 128; break;
            case 3:  regsGranularity = 256; break;
            default: regsGranularity = 256; break; // guess; unknown for future hardware
          }
          nrRegsPerWarp = (nrRegsPerWarp + regsGranularity - 1) & ~(regsGranularity - 1); // assumes regsGranularity is a pow of 2
          unsigned nrWarpsPerBlock = nrThreadsPerBlock / warpSize;
          nrRegsPerBlock = nrRegsPerWarp * nrWarpsPerBlock;
        }
        const unsigned devNrRegs = device.getAttribute(CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK);
        unsigned regsFactor = devNrRegs / nrRegsPerBlock;
        if (regsFactor < factor)
          factor = regsFactor;
      }

      // shared memory size
      size_t shMem = getAttribute(CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES) + dynSharedMemBytes;
      if (shMem > 0) {
        size_t shMemGranularity;
        switch (computeCapMajor) {
          case 1:  shMemGranularity = 512; break;
          case 2:  shMemGranularity = 128; break;
          case 3:  shMemGranularity = 256; break;
          default: shMemGranularity = 256; break; // guess; unknown for future hardware
        }
        shMem = (shMem + shMemGranularity - 1) & ~(shMemGranularity - 1); // assumes shMemGranularity is a pow of 2
        const size_t devShMem = device.getAttribute(CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK);
        unsigned shMemFactor = devShMem / shMem;
        if (shMemFactor < factor)
          factor = shMemFactor;
      }

      return factor;
    }

    double Kernel::predictMultiProcOccupancy(unsigned dynSharedMemBytes) const
    {
      const gpu::Device device(_context.getDevice());

      const unsigned warpSize = device.getAttribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE);
      unsigned nrThreadsPerBlock = itsBlockDims.x * itsBlockDims.y * itsBlockDims.z;
      unsigned nrWarpsPerBlock = (nrThreadsPerBlock + warpSize - 1) / warpSize;
      unsigned nrBlocksPerMP = getNrBlocksPerMultiProc(dynSharedMemBytes);
      unsigned nrWarps = nrBlocksPerMP * nrWarpsPerBlock;

      const unsigned maxThreadsPerMP = device.getAttribute(CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR);
      unsigned maxNrWarpsPerMP = maxThreadsPerMP / warpSize;

      return static_cast<double>(nrWarps) / maxNrWarpsPerMP;
    }

    void Kernel::enqueue(const BlockID &blockId) const
    {
      itsStream.recordEvent(itsCounter.start);
      itsStream.launchKernel(*this, itsGridDims, itsBlockDims);
      itsStream.recordEvent(itsCounter.stop);

      if (itsParameters.dumpBuffers && blockId.block >= 0) {
        itsStream.synchronize();
        dumpBuffers(blockId);
      }
    }

    void Kernel::dumpBuffers(const BlockID &blockId) const
    {
      dumpBuffer(itsBuffers.output,
                 str(boost::format(itsParameters.dumpFilePattern) %
                     blockId.globalSubbandIdx %
                     blockId.block));
    }

    PerformanceCounter &Kernel::getCounter()
    {
      return itsCounter;
    }

  }
}


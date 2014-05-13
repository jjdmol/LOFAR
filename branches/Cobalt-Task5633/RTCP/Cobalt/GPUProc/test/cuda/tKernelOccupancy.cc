//# tKernelOccupancy.cc: test occupancy prediction for CUDA kernels
//# Copyright (C) 2014  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <cstring>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Parset.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/global_defines.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace LOFAR::Cobalt;

// Note: To compute (predicted) occupancy, we don't need to run any kernels.

struct MyKernel : Kernel {
  MyKernel(const gpu::Stream& stream, const gpu::Module& module,
           const string& fname,
           const Buffers& buffers, const Parameters& parameters)
  : Kernel(stream, gpu::Function(module, fname), buffers, parameters) { }

  struct Parameters : Kernel::Parameters {
    Parameters(const Parset& ps) : Kernel::Parameters(ps) { }
  };

  struct Buffers : Kernel::Buffers {
    Buffers(const gpu::DeviceMemory& inout) : Kernel::Buffers(inout, inout) { }
  };

  // for the test, expose some protected members from Kernel
  using Kernel::setEnqueueWorkSizes;
  using Kernel::predictMultiProcOccupancy;
};

int blkLimit(MyKernel& kernel) {
  LOG_INFO("\nTest 1: blkLimit");

  gpu::Grid grid(16);
  gpu::Block block; // 1, 1, 1
  kernel.setEnqueueWorkSizes(grid, block); // occupancy deps on grid/block, so call this first
  double occ = kernel.predictMultiProcOccupancy();
  LOG_INFO_STR("predicted occupancy at blk.x=" << block.x << " is " << occ);
  double prevOcc, maxOcc = occ;

  for (unsigned x = 32; x <= 512; x += 32) {
    prevOcc = occ;

    block.x = x;
    grid.x = 16 * block.x; // only make sure it divs block.x
    kernel.setEnqueueWorkSizes(grid, block); // occupancy deps on grid/block, so call this first
    occ = kernel.predictMultiProcOccupancy();
    LOG_INFO_STR("predicted occupancy at blk.x=" << block.x << " is " << occ);

    if (maxOcc < occ)
      maxOcc = occ;

    // With few regs and no shmem, occ should rise to 1.0, then fluctuate a few times between 0.5+ and 1.0.
    if (maxOcc < 1.0 && occ < prevOcc) {
      LOG_ERROR_STR("as we increase block size, occupancy cannot drop until we have seen an occ of 1.0: " << prevOcc << " -> " << occ);
      return 1;
    }
    if (maxOcc == 1.0 && occ < 0.5) {
      LOG_ERROR_STR("as we increase block size, after reaching an occupancy of 1.0, occupancy cannot drop below 0.5: " << occ);
      return 1;
    }
  }

  if (maxOcc != 1.0) {
    LOG_ERROR_STR("an occupancy of 1.0 must be reachable");
    return 1;
  }

  return 0;
}

int regsLimit(MyKernel& kernel) {
  LOG_INFO("\nTest 2: regsLimit");

  gpu::Grid grid(16);
  gpu::Block block;
  kernel.setEnqueueWorkSizes(grid, block);
  double occ = kernel.predictMultiProcOccupancy();
  LOG_INFO_STR("predicted occupancy at blk.x=" << block.x << " is " << occ);
  double maxOcc = occ;

  for (unsigned x = 32; x <= 512; x += 32) {
    block.x = x;
    grid.x = 16 * block.x; // only make sure it divs block.x
    kernel.setEnqueueWorkSizes(grid, block); // occupancy deps on grid/block, so call this first
    occ = kernel.predictMultiProcOccupancy();
    LOG_INFO_STR("predicted occupancy at blk.x=" << block.x << " is " << occ);

    if (maxOcc < occ)
      maxOcc = occ;
  }

  if (maxOcc == 1.0) {
    LOG_ERROR_STR("an occupancy of 1.0 should not be reachable since reg pressure is intended to be too high");
    return 1;
  }

  return 0;
}

int shmemLimit(MyKernel& kernel) {
  LOG_INFO("\nTest 3: shmemLimit");

  gpu::Grid grid(16 * 1024);
  gpu::Block block(64);
  kernel.setEnqueueWorkSizes(grid, block);
  double occ1 = kernel.predictMultiProcOccupancy();
  LOG_INFO_STR("predicted occupancy is " << occ1);

  block.x = 256;
  kernel.setEnqueueWorkSizes(grid, block);
  double occ2 = kernel.predictMultiProcOccupancy();
  LOG_INFO_STR("predicted occupancy is " << occ2);

  // shmem is per block, so with a larger block, we have more warps active.
  // (Up till some point; 64 vs 256 seems to be reasonable to compare.)
  if (!(occ1 < occ2)) {
    LOG_ERROR_STR("for this shmem limited kernel, a block size of 256 should give higher occupancy than 64");
    return 1;
  }

  return 0;
}

int main() {
  INIT_LOGGER("tKernelOccupancy");

  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    return 3;
  }

  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  string srcFilename("tKernelOccupancy.in_.cu");
  Parset ps("tKernelOccupancy.parset");

  // Get default parameters for the compiler
  CompileFlags flags = defaultCompileFlags();
  CompileDefinitions definitions = defaultCompileDefinitions();

  string ptx = createPTX(srcFilename, definitions, flags, devices);
  gpu::Module module(createModule(ctx, srcFilename, ptx));
  LOG_INFO_STR("Succesfully compiled '" << srcFilename << "'");

  gpu::Stream stream(ctx);
  gpu::DeviceMemory inout(ctx, 0);
  MyKernel::Buffers    bufs(inout);
  MyKernel::Parameters para(ps);
  MyKernel blkLimitKernel  (stream, module, "blkLimit",   bufs, para);
  MyKernel regsLimitKernel (stream, module, "regsLimit",  bufs, para);
  MyKernel shmemLimitKernel(stream, module, "shmemLimit", bufs, para);

  int err = 0;
  err |= blkLimit(blkLimitKernel);
  err |= regsLimit(regsLimitKernel);
  err |= shmemLimit(shmemLimitKernel);
  return err;
}


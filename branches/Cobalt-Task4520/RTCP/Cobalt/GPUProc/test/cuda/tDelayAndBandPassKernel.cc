//# tDelayAndBandPassKernel.cc: test Kernels/DelayAndBandPassKernel class
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main() {
  INIT_LOGGER("tDelayAndBandPassKernel");

  // Set up gpu environment
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
  gpu::Stream stream(ctx);

  Parset ps("tDelayAndBandPassKernel.in_parset");
  string srcFilename("DelayAndBandPass.cu");

  // Get default parameters for the compiler
  flags_type flags = defaultFlags();
  Kernel::definitions_type definitions(Kernel::compileDefinitions(ps));

  string ptx = createPTX(devices, srcFilename, flags, definitions);
  gpu::Module module(createModule(ctx, srcFilename, ptx));
  cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  gpu::DeviceMemory 
    inputData(ctx, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::INPUT_DATA)),
    filteredData(ctx, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::OUTPUT_DATA)),
    delaysAtBegin(ctx, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::DELAYS)),
    delaysAfterEnd(ctx, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::DELAYS)),
    phaseOffsets(ctx, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::PHASE_OFFSETS)),
    bandPassCorrectionWeights(ctx, DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::BAND_PASS_CORRECTION_WEIGHTS));

  DelayAndBandPassKernel kernel(ps, module, inputData, filteredData, 
                                delaysAtBegin, delaysAfterEnd, phaseOffsets, 
                                bandPassCorrectionWeights);

  unsigned subband = 0;
  kernel.enqueue(stream, subband);
  stream.synchronize();

  return 0;
}


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
#include <GPUProc/Buffers.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main() {
  INIT_LOGGER("tDelayAndBandPassKernel");

  // Set up gpu environment
  gpu::Platform pf;
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream stream(ctx);

  Parset ps("tDelayAndBandPassKernel.parset.in");
  string srcFilename("DelayAndBandPass.cu");

  // Get default parameters for the compiler
  flags_type flags = defaultFlags();
  definitions_type definitions = defaultDefinitions(ps);

  string ptx = createPTX(devices, srcFilename, flags, definitions);
  gpu::Module module(createModule(ctx, srcFilename, ptx));
  cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  WorkQueueInputData::DeviceBuffers devInput(ps.nrBeams(),
                ps.nrStations(),
                NR_POLARIZATIONS,
                ps.nrHistorySamples() + ps.nrSamplesPerSubband(),
                ps.nrBytesPerComplexSample(),
                stream);

  MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()],
                                stream,
                                CL_MEM_WRITE_ONLY,
                                CL_MEM_READ_ONLY);

  DeviceBuffer devFilteredData(stream,
                      CL_MEM_READ_WRITE,

                      // reserve enough space for the output of the
                      // firFilterKernel,
                      std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>),
                      // and the correlatorKernel.
                      ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>)));

  stream.synchronize();

  DelayAndBandPassKernel kernel(ps, module, 
                             devInput.inputSamples,
                             devFilteredData,
                             devInput.delaysAtBegin,
                             devInput.delaysAfterEnd,
                             devInput.phaseOffsets,
                             bandPassCorrectionWeights);

  kernel.enqueue(stream, 0);
  stream.synchronize();

  return 0;
}


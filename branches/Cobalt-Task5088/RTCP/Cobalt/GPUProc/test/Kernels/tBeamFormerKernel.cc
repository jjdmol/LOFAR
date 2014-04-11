//# tBeamFormerKernel.cc
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

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <GPUProc/SubbandProcs/BeamFormerFactories.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>
#include <Common/LofarLogger.h>

#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "KernelTestHelpers.h"

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;

int main(int argc, char *argv[])
{
  const char * testName = "tBeamFormerKernel";
  INIT_LOGGER(testName);
  // parse command line arguments to parset
  Parset ps;
  parseCommandlineParameters(argc, argv, ps, testName);
  
  // Set up gpu environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " GPU devices" << endl;
  }
  catch (gpu::GPUException& e) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream stream(ctx);

  // Create the factory
  KernelFactory<BeamFormerKernel> factory(BeamFormerFactories::beamFormerParams(ps));

  DeviceMemory devDelaysMemory(ctx, factory.bufferSize(BeamFormerKernel::BEAM_FORMER_DELAYS)),
    devBandPassCorrectedMemory(ctx, factory.bufferSize(BeamFormerKernel::INPUT_DATA)),
    devComplexVoltagesMemory(ctx, factory.bufferSize(BeamFormerKernel::OUTPUT_DATA));


  BeamFormerKernel::Buffers buffers(devBandPassCorrectedMemory, 
                                    devComplexVoltagesMemory,
                                     devDelaysMemory);
  
  // kernel
  auto_ptr<BeamFormerKernel> kernel(factory.create(stream, buffers));

  float subbandFreq = 60e6f;
  unsigned sap = 0;

  BlockID blockId;
  // run
  kernel->enqueue(blockId, subbandFreq, sap);
  stream.synchronize();

  return 0;
}

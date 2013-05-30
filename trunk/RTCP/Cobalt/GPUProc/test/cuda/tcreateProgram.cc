//# tcreateProgram.cc: test CUDA kernel runtime compilation from src file
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

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <GPUProc/global_defines.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main(int argc, char *argv[]) {
  INIT_LOGGER(argv[0]);
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <L12345.parset> <kernel.cu>" << endl;
    return 1;
  }

  // Create the gpu parts needed for running a kernel
  gpu::Platform pf;
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);

  // Open inputs
  Parset ps(argv[1]);
  string srcFilename(argv[2]);

  // Collect inputs from the parste and assign them to CudaRuntimeCompiler
  // input_types.
  flags_type flags = defaultFlags();
  definitions_type definitions = defaultDefinitions(ps);

  string ptx = createPTX(devices, srcFilename, flags, definitions);
  gpu::Module module(createModule(ctx, srcFilename, ptx));
  cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  return 0;
}


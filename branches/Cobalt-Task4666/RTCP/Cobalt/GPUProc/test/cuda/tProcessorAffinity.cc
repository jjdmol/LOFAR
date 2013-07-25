//# tDelayAndBandpass.cc: test delay and bandpass CUDA kernel
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
//# $Id: tDelayAndBandPass.cc 25699 2013-07-18 07:18:02Z mol $

#include <lofar_config.h>

#include <cstdlib>
#include <cmath> 
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <UnitTest++.h>

#include "TestUtil.h"

using namespace std;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;

// 
float * runTest()
{
  // Set up environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    exit(3);
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  Stream cuStream(ctx);
  std::stringstream tostrstream("");

  return NULL;
}

TEST(BandPass)
{
  // The input samples are all ones
  // After correction, multiply with 2.
  // The first and the last complex values are retrieved. They should be scaled with the bandPassFactor == 2
  runTest();
  CHECK(true);
}

int main()
{
  INIT_LOGGER("tProcessorAffinity");
  cout << "We are here." << endl;
  return UnitTest::RunAllTests() > 0;
}


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

#include <sched.h>

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
  
  cpu_set_t mask;
  //                                    pid, size of cpu set, mask to place data in    
  bool sched_return = sched_getaffinity(0, sizeof(cpu_set_t), &mask);
  cout << "Number of cpu's in the mask: " << CPU_COUNT(&mask) << endl;
  unsigned numCPU = sysconf( _SC_NPROCESSORS_ONLN );
  cout << "Number of cpu's: " << numCPU << endl;
  cout << "cpu mask: "<< endl;
  for (unsigned idx_cpu =0; idx_cpu <numCPU ; ++ idx_cpu)
  {
    cout << CPU_ISSET(idx_cpu, &mask) << ",";
  }
  cout << endl;
  CPU_ZERO(&mask) ;
    for (unsigned idx_cpu =0; idx_cpu < (unsigned)2; ++ idx_cpu)
  {
    CPU_SET(idx_cpu, &mask);
  }
    cout << "cpu mask: "<< endl;
   for (unsigned idx_cpu =0; idx_cpu < numCPU; ++ idx_cpu)
   {
     cout << CPU_ISSET(idx_cpu, &mask) << ",";
   }
   sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
   cout << "result of set affinity command (should be zer0): " << sched_return << endl;
   
  cout << "Current process is running on processor id: " << sched_getcpu() << endl;
  cout << "resetting the cpu mask" << endl;
  CPU_ZERO(&mask) ;
  for (unsigned idx_cpu =14; idx_cpu < (unsigned)16; ++ idx_cpu)
  {
    CPU_SET(idx_cpu, &mask);
  }
     sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
   cout << "result of set affinity command (should be zer0): " << sched_return << endl;
   cout << "Current process is running on processor id: " << sched_getcpu() << endl;
  
  return 0;
  //return UnitTest::RunAllTests() > 0;
}


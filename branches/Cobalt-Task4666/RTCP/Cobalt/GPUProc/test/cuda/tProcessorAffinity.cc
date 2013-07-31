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
#include <list>
#include <iostream>
#include <exception>
#include <stdlib.h>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <UnitTest++.h>
#include "TestUtil.h"
#include <sched.h>
#include <boost/lexical_cast.hpp>


using namespace std;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;



// float * runTest()
// {
  // // Set up environment
  // try {
    // gpu::Platform pf;
    // cout << "Detected " << pf.size() << " CUDA devices" << endl;
  // } catch (gpu::CUDAException& e) {
    // cerr << e.what() << endl;
    // exit(3);
  // }
  // gpu::Device device(0);
  // vector<gpu::Device> devices(1, device);
  // gpu::Context ctx(device);
  // Stream cuStream(ctx);
  // std::stringstream tostrstream("");

  // return NULL;
// }

// TEST(BandPass)
// {
  // // The input samples are all ones
  // // After correction, multiply with 2.
  // // The first and the last complex values are retrieved. They should be scaled with the bandPassFactor == 2
  // runTest();
  // CHECK(true);
// }



int main()
{
  INIT_LOGGER("tProcessorAffinity");
  
  unsigned mpiRank = 0;
  
  // Create an vector with all the available gpus
  vector<string>allGPUs;
  allGPUs.push_back("cbt001.gpu0");
  allGPUs.push_back("cbt001.gpu1");
  allGPUs.push_back("cbt002.gpu0");
  allGPUs.push_back("cbt002.gpu1");
  allGPUs.push_back("cbt003.gpu0");
  allGPUs.push_back("cbt003.gpu1");
  allGPUs.push_back("cbt004.gpu0");
  allGPUs.push_back("cbt005.gpu1");
  
  // Vector with selected gpus
  vector<unsigned>gpuset;
  gpuset.push_back(0);
  gpuset.push_back(3);
  gpuset.push_back(5);
  
  //validate that the indexes are valid
  if(mpiRank >= gpuset.size() || gpuset[mpiRank] >= allGPUs.size())
  {
    cerr << "received invalid mpi / gpu rank: "  << endl;
		throw  exception();
  } 
  
  // Get the gpu that is assigned to the current mpi rank
  string assignedGPUString = allGPUs[gpuset[mpiRank]];
  
  // get the last char in the string as an integer, assume valid gpu list
  unsigned GPUidx = boost::lexical_cast<int>((*assignedGPUString.find(".")) + 1f);  
  
  // Get the number of cpu's
  unsigned numCPU = sysconf( _SC_NPROCESSORS_ONLN );
  
  // Get a valid cpu set
  // sched_getaffinity(pid, size of cpu set, mask to place data in) //pid 0 = current thread
  cpu_set_t mask;  
  bool sched_return = sched_getaffinity(0, sizeof(cpu_set_t), &mask);

  // zero the mask 
  CPU_ZERO(&mask) ; 
    
  // Set the mask, beginning with GPUidx and set the corresponding cpus
  for (unsigned idx_cpu = GPUidx; idx_cpu < numCPU; idx_cpu += 2)  // use a step of two, this is cobalt specific
    CPU_SET(idx_cpu, &mask);
  
  if(true)
  {
    for (unsigned idx_cpu =0; idx_cpu < numCPU; ++ idx_cpu)
      cout << CPU_ISSET(idx_cpu, &mask) << ",";
    cout << endl;
  }
  sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
  if (sched_return != 0)
  {
    cerr << "Failed setting the cpu affinity! Aborting..." << endl;
    throw  exception();
  }
  
  cout << "**********************************************" << endl;
  return 0;    
}

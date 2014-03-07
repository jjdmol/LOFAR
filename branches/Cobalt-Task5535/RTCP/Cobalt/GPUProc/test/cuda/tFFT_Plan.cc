//# tFFT_Plan.cc: test the FFT_Plan class
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

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <fftw3.h>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <CoInterface/BlockID.h>
#include <GPUProc/cuda/Kernels/FFT_Plan.h>  // Import the 
#include <GPUProc/PerformanceCounter.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::Cobalt;


void testDifferentArgumentsDifferentPlan(gpu::Context ctx, gpu::Stream stream)
{
  // **************************************************
  // Test 1: Two fft plans with different inputs should result in 
  // different interal plans 
  // From docs:
  // cufftHandle is a handle type used to store and access CUFFT plans.
  // typedef int cufftHandle;
  FFT_Plan plan1(ctx, stream, 20, 20);
  FFT_Plan plan2(ctx, stream,  20, 40);
  assert(plan1.plan != plan2.plan);
}

void testSameArgumentsSamePlan(gpu::Context ctx, gpu::Stream stream)
{
  // **************************************************
  // Test 2: Two fft plans with same inputs should result in 
  // same interal plans 
  // From docs:
  // cufftHandle is a handle type used to store and access CUFFT plans.
  // typedef int cufftHandle;
  FFT_Plan plan1(ctx, stream, 20, 20);
  FFT_Plan plan2(ctx, stream,  20, 20);
  assert(plan1.plan == plan2.plan);
}

void testSameArgumentsPlanValidAfterDestruction(gpu::Context ctx, gpu::Stream stream)
{
  // **************************************************
  // Test 3: Two fft plans with same inputs should result in 
  // same interal plans 
  // Then delete the internal plan: this should succeed:
  // It should not have been delete with the first delete
  FFT_Plan plan1(ctx, stream, 20, 20);
  FFT_Plan*plan2 = new FFT_Plan(ctx, stream, 20, 20); // plan with the same parameters
  assert(plan1.plan == plan2->plan);

  // now delete the second plan and assert the first is still valid
  delete plan2;
  
  // manually destroy the underlaying handle
  cufftResult_t  firstdel = cufftDestroy(plan1.plan);
  assert(firstdel == cudaSuccess); // asure del was a succes: The handle was valid!!!

  cufftResult_t  seconddel = cufftDestroy(plan1.plan);
  assert(seconddel != cudaSuccess); // the destruction should fail.
}


void testSameArgumentsPlanAllDestroyedCorrect(gpu::Context ctx, gpu::Stream stream)
{
  // **************************************************
  // Test 3: Two fft plans with same inputs should result in 
  // same interal plans 
  // Then delete both the plans. The internal cufft plan should be destroyed also
  FFT_Plan* plan1 = new FFT_Plan(ctx, stream, 20, 20);
  FFT_Plan* plan2 = new FFT_Plan(ctx, stream, 20, 20); // plan with the same parameters
  assert(plan1->plan == plan2->plan);
  cufftHandle planHandle = plan1->plan;
  // now delete the second plan and assert the first is still valid
  delete plan2;
  delete plan1;


  // manually destroy the underlaying handle
  cufftResult_t  deleteReturnvalue = cufftDestroy(planHandle);
  assert(deleteReturnvalue != cudaSuccess); // the plan should be destroyed after
  //both the plans are destructed 

}


int main() {
  INIT_LOGGER("tFFT");

  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  }
  catch (gpu::CUDAException& e) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 0;
  }
  gpu::Device device(0);
  gpu::Context ctx(device);
  gpu::Stream stream(ctx);
 
  testDifferentArgumentsDifferentPlan(ctx, stream);
  testSameArgumentsSamePlan(ctx, stream);
  testSameArgumentsPlanValidAfterDestruction(ctx, stream);
  testSameArgumentsPlanAllDestroyedCorrect(ctx, stream);

  
  return 0;
}


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
//# $Id$

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
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <UnitTest++.h>

#include "TestUtil.h"

using namespace std;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;

// 
float * runTest(unsigned NR_BITS_PER_SAMPLE = 16, 
                char char_default_value = 0, // will be used depending NR_BITS_PER_SAMPLE
                short short_default_value = 0)
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

  string kernelPath = "IntToFloat.cu";  //The test copies the kernel to the current dir (also the complex header, needed for compilation)

  // ****************************************
  // Compile to ptx  
  // Get an instantiation of the default parameters
  definitions_type definitions = Kernel::definitions_type();
  flags_type flags = defaultFlags();

  // Set op string string pairs to be provided to the compiler as defines
  definitions["NR_STATIONS"] = "2";
  unsigned NR_STATIONS = 2;
  definitions["NR_SAMPLES_PER_SUBBAND"] = "1024";
  unsigned NR_SAMPLES_PER_SUBBAND = 1024;
  
  tostrstream << NR_BITS_PER_SAMPLE;
  definitions["NR_BITS_PER_SAMPLE"] = tostrstream.str();
  definitions["NR_POLARIZATIONS"] = "2";
  unsigned NR_POLARIZATIONS = 2;
  definitions["COMPLEX"] = "2";
  unsigned COMPLEX = 2;
  string ptx = createPTX(devices, kernelPath, flags, definitions);
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  Function  hKernel(module, "intToFloat");  // c function this no argument overloading

  // *************************************************************
  // Create the data arrays  
  // remember to use the correct size depending on the input type
  size_t sizeSampledData;
  if (NR_BITS_PER_SAMPLE==16)
    sizeSampledData = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * sizeof(short);
  else
      sizeSampledData = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * sizeof(char);
      
  DeviceMemory DevSampledMemory(ctx, sizeSampledData);
  HostMemory rawSampledData = getInitializedArray(ctx, 0, (short)0); // use bogus value: no default constructor..
  if (NR_BITS_PER_SAMPLE==16)
    rawSampledData = getInitializedArray(ctx, sizeSampledData, (short)short_default_value);
  else
    rawSampledData = getInitializedArray(ctx, sizeSampledData, (char)char_default_value);  
  cuStream.writeBuffer(DevSampledMemory, rawSampledData);

  // Output is always float
  size_t sizeConvertedData = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * sizeof(float);
  DeviceMemory DevConvertedMemory(ctx, sizeConvertedData);
  HostMemory rawConvertedData = getInitializedArray(ctx, sizeConvertedData, 42.0f); 
  cuStream.writeBuffer(DevConvertedMemory, rawConvertedData);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(1, DevSampledMemory);
  hKernel.setArg(0, DevConvertedMemory);

  // assign to gpu_wrapper objects
  Grid globalWorkSize(1, NR_STATIONS, 1);  
  Block localWorkSize(256, 1,1); 

  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished
  
  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawConvertedData, DevConvertedMemory);
  cuStream.synchronize(); //assure copy from device is done
  
  // *************************************
  // Create the return values
  float *firstAndLastComplex = new float[4];
  // Return the first complex
  firstAndLastComplex[0] = rawConvertedData.get<float>()[0];
  firstAndLastComplex[1] = rawConvertedData.get<float>()[1];
  //return the last complex number
  firstAndLastComplex[2] = rawConvertedData.get<float>()[(sizeConvertedData / sizeof(float)) - 2];
  firstAndLastComplex[3] = rawConvertedData.get<float>()[(sizeConvertedData / sizeof(float)) - 1];

  // *************************************
  // cleanup memory

  return firstAndLastComplex;
}

TEST(CornerCaseMinus128)
{
  // ***********************************************************
  // Test the corner case for 8 bit input, -128 should be clamped to -127
  float * results;
  results = runTest(8,     // use 8 bits per sample
                    -128,  // Use -128 as default input
                    0);    // not used in 8 bit samples

  CHECK_CLOSE(-127.0, results[0], 0.00000001);
  CHECK_CLOSE(-127.0, results[1], 0.00000001);
  CHECK_CLOSE(-127.0, results[2], 0.00000001);
  CHECK_CLOSE(-127.0, results[3], 0.00000001);

  delete[] results;
}

TEST(CornerCaseMinus128short)
{
  // ***********************************************************
  // The -128 to -127 clamp should not be applied to 16 bit samples
  float * results;
  results = runTest(16, // 16 bit samples
                    0,  // Not used in 16 bit
                    -128); // use -128 as input

  CHECK_CLOSE(-128.0, results[0], 0.00000001);
  CHECK_CLOSE(-128.0, results[1], 0.00000001);
  CHECK_CLOSE(-128.0, results[2], 0.00000001);
  CHECK_CLOSE(-128.0, results[3], 0.00000001);

  delete[] results;
}


TEST(CornerCaseMinus127)
{
  // ***********************************************************
  // Minus 127 should stay -128
  float * results;
  results = runTest(8, -127, 0);

  CHECK_CLOSE(-127.0, results[0], 0.00000001);
  CHECK_CLOSE(-127.0, results[1], 0.00000001);
  CHECK_CLOSE(-127.0, results[2], 0.00000001);
  CHECK_CLOSE(-127.0, results[3], 0.00000001);

  delete[] results;
}

TEST(IntToFloatSimple)
{
  // ***********************************************************
  // Test if 10 is converted
  float * results;
  results = runTest(8, 10, 0);

  CHECK_CLOSE(10.0, results[0], 0.00000001);
  CHECK_CLOSE(10.0, results[1], 0.00000001);
  CHECK_CLOSE(10.0, results[2], 0.00000001);
  CHECK_CLOSE(10.0, results[3], 0.00000001);

  delete[] results;
}

TEST(IntToFloatSimpleShort)
{
  // ***********************************************************
  // Test if 10 is converted
  float * results;
  results = runTest(16, 0, 2034);

  CHECK_CLOSE(2034.0, results[0], 0.00000001);
  CHECK_CLOSE(2034.0, results[1], 0.00000001);
  CHECK_CLOSE(2034.0, results[2], 0.00000001);
  CHECK_CLOSE(2034.0, results[3], 0.00000001);

  delete[] results;
}

int main()
{
  INIT_LOGGER("tIntToFloat");

  return UnitTest::RunAllTests() > 0;
}


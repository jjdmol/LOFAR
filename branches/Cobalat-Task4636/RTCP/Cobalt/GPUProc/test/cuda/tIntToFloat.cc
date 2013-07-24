//# tIntToFloat.cc: test Int2Float CUDA kernel
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
#include <cstring>
#include <cmath> 
#include <cassert>
#include <complex>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <limits>

#include <boost/lexical_cast.hpp>
#include <UnitTest++.h>

#include <Common/LofarLogger.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>

#include "TestUtil.h"

using namespace std;
using namespace LOFAR::Cobalt;

CompileDefinitions compileDefs;
gpu::Stream *stream;

// default compile definitions
const unsigned NR_STATIONS = 1;//2;
const unsigned NR_CHANNELS = 2;//64;
const unsigned NR_SAMPLES_PER_CHANNEL = 16;
const unsigned NR_SAMPLES_PER_SUBBAND = NR_SAMPLES_PER_CHANNEL * NR_CHANNELS;
const unsigned NR_BITS_PER_SAMPLE = 8;
const unsigned NR_POLARIZATIONS = 2;
const unsigned COMPLEX = 2;


// Initialize input AND output before calling runKernel().
// We copy both to the GPU, to make sure the final output is really from the kernel.
void runKernel(gpu::Function kfunc,
               gpu::HostMemory input,  size_t inputSize,
               gpu::HostMemory output, size_t outputSize)
{
  gpu::Context ctx(stream->getContext());

  gpu::DeviceMemory devInput (ctx, inputSize);
  gpu::DeviceMemory devOutput(ctx, outputSize);

  kfunc.setArg(0, devOutput);
  kfunc.setArg(1, devInput);

  gpu::Grid globalWorkSize(1, NR_STATIONS, 1);  
  gpu::Block localWorkSize(256, 1, 1); 

  // Overwrite devOutput, so result verification is more reliable.
  stream->writeBuffer(devOutput, output);

  stream->writeBuffer(devInput, input);
  stream->launchKernel(kfunc, globalWorkSize, localWorkSize);
  stream->readBuffer(output, devOutput);
  stream->synchronize(); // wait until transfer completes
}

gpu::Function initKernel(gpu::Context ctx, const CompileDefinitions& defs) {
  // Compile to ptx. Copies the kernel to the current dir
  // (also the complex header, needed for compilation).
  string kernelPath("IntToFloat.cu");
  CompileFlags flags(defaultCompileFlags());
  vector<gpu::Device> devices(1, gpu::Device(0));
  string ptx(createPTX(kernelPath, defs, flags, devices));
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  gpu::Function kfunc(module, "intToFloat");

  return kfunc;
}

template <typename T>
vector<float> runTest(T defaultVal)
{
  gpu::Context ctx(stream->getContext());

  // Apply test-specific parameters for mem alloc sizes and kernel compilation.
  size_t inputSize  = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * sizeof(T);
  size_t outputSize = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * sizeof(float);
  gpu::HostMemory input (getInitializedArray(ctx, inputSize , defaultVal));
  gpu::HostMemory output(getInitializedArray(ctx, outputSize, (T)0));

  unsigned nrBitsPerSample = 8 * sizeof(T);
  compileDefs["NR_BITS_PER_SAMPLE"] = boost::lexical_cast<string>(nrBitsPerSample);
  gpu::Function kfunc(initKernel(ctx, compileDefs));


  runKernel(kfunc, input, inputSize, output, outputSize);

  // Tests that use this function only check the first 4 output vals.
  const unsigned nrResultVals = 4;
  vector<float> outputrv(nrResultVals);
  memcpy(&outputrv[0], output.get<float>(), nrResultVals * sizeof(float));
  return outputrv;
}

template <typename T>
void initArray(T *ptr, size_t len)
{
  // Repeatedly write -128, -127, ..., 0, ... 127; or whatever is the max value range for T.
  T val = numeric_limits<T>::min();
  for (size_t i = 0; i < len; i++)
  {
    ptr[i] = val;
    if (val == numeric_limits<T>::max())
      val = numeric_limits<T>::min(); // wrap back to minimum
    else
      val += 1;
  }
}

// T is the same as for initArray(). Output of Int2Float kernel is always float.
// Note that the IntToFloat kernel also transposes: it outputs first all polX, then all polY.
template <typename T>
void checkTransposedArray(const float *p, size_t len)
{
  // The verification assumes that the nr of possible expected values is even. (e.g. not 1-complement)
  assert( ((numeric_limits<T>::max() - numeric_limits<T>::min() + 1) & 1) == 0 );


  // check a complex nr at a time
  complex<float> *ptr = (complex<float> *)p;
  len /= sizeof(complex<float>) / sizeof(float);

  float expectedVal = numeric_limits<T>::min();

  for (size_t t = 0; t < len / 2; t++)
  {
  for (size_t pol = 0; pol < NR_POLARIZATIONS; pol++)
  {
    // Verify alternatingly over polX, polY. I.e. in incremental expectedVal order (apart from wrapping).
    complex<float> v = ptr[pol * (len/2) + t];

    // In 8 bit mode, -128, if expressable, is converted and clamped to -127.0f.
    // Also, everything is scaled to the amplitude of 16 bit mode.
    // Don't bother with template specialization just for this.
    if (sizeof(T) == 1) // 8 bit mode
    {
      if (expectedVal == numeric_limits<T>::min())
      {
        CHECK_CLOSE((float)(256 * (expectedVal+1)), v.real(), 0.00000001);
      } else {
        CHECK_CLOSE((float)(256 * expectedVal), v.real(), 0.00000001);
      }
      CHECK_CLOSE((float)(256 * (expectedVal+1)), v.imag(), 0.00000001);
    } else { // 16 bit mode
      CHECK_CLOSE((float)expectedVal,     v.real(), 0.00000001);
      CHECK_CLOSE((float)(expectedVal+1), v.imag(), 0.00000001);
    }

    if (expectedVal == numeric_limits<T>::max() - 1)
      expectedVal = numeric_limits<T>::min(); // wrap back to minimum
    else
      expectedVal += 2;
  }
  }
}

void runTest2(unsigned nrBitsPerSample)
{
  gpu::Context ctx(stream->getContext());

  // Don't use the Kernel class helpers to retrieve buffer sizes,
  // because we test the kernel, not the Kernel class.
  size_t inputSize  = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * nrBitsPerSample / 8;
  size_t outputSize = NR_STATIONS * NR_SAMPLES_PER_SUBBAND * NR_POLARIZATIONS * COMPLEX * sizeof(float);
  gpu::HostMemory input (ctx, inputSize);
  gpu::HostMemory output(ctx, outputSize);

  // init input
  void *ptr = input.get<void>();
  if (nrBitsPerSample == 16)
    initArray((int16_t *)ptr, inputSize / sizeof(int16_t));
  else if (nrBitsPerSample == 8)
    initArray((int8_t  *)ptr, inputSize / sizeof(int8_t));
  else
    assert(nrBitsPerSample != 16 && nrBitsPerSample != 8);

  // set output for proper verification later
  memset(output.get<void>(), 0, outputSize);

  compileDefs["NR_BITS_PER_SAMPLE"] = boost::lexical_cast<string>(nrBitsPerSample);
  gpu::Function kfunc(initKernel(ctx, compileDefs));


  runKernel(kfunc, input, inputSize, output, outputSize);

  // check output
  float *ptr2 = output.get<float>();
  if (nrBitsPerSample == 16)
    checkTransposedArray<int16_t>(ptr2, outputSize / sizeof(float));
  else if (nrBitsPerSample == 8)
    checkTransposedArray<int8_t >(ptr2, outputSize / sizeof(float));
}


// Unit tests of value conversion
TEST(CornerCaseMinus128)
{
  // Test the corner case for 8 bit input, -128 should be clamped to scaled -127
  vector<float> results(runTest<signed char>(-128));

  const float scale = 256.0f;
  CHECK_CLOSE(scale * -127.0, results[0], 0.00000001);
  CHECK_CLOSE(scale * -127.0, results[1], 0.00000001);
  CHECK_CLOSE(scale * -127.0, results[2], 0.00000001);
  CHECK_CLOSE(scale * -127.0, results[3], 0.00000001);
}

TEST(CornerCaseMinus128short)
{
  // The -128 to -127 clamp should not be applied to 16 bit samples
  vector<float> results(runTest<short>(-128));

  CHECK_CLOSE(-128.0, results[0], 0.00000001);
  CHECK_CLOSE(-128.0, results[1], 0.00000001);
  CHECK_CLOSE(-128.0, results[2], 0.00000001);
  CHECK_CLOSE(-128.0, results[3], 0.00000001);
}


TEST(CornerCaseMinus127)
{
  // Minus 127 should stay -127
  vector<float> results(runTest<signed char>(-127));

  const float scale = 256.0f;
  CHECK_CLOSE(scale * -127.0, results[0], 0.00000001);
  CHECK_CLOSE(scale * -127.0, results[1], 0.00000001);
  CHECK_CLOSE(scale * -127.0, results[2], 0.00000001);
  CHECK_CLOSE(scale * -127.0, results[3], 0.00000001);
}

TEST(IntToFloatSimple)
{
  // Test if 10 is converted
  vector<float> results(runTest<signed char>(10));

  const float scale = 256.0f;
  CHECK_CLOSE(scale * 10.0, results[0], 0.00000001);
  CHECK_CLOSE(scale * 10.0, results[1], 0.00000001);
  CHECK_CLOSE(scale * 10.0, results[2], 0.00000001);
  CHECK_CLOSE(scale * 10.0, results[3], 0.00000001);
}

TEST(IntToFloatSimpleShort)
{
  // Test if 2034 is converted
  vector<float> results(runTest<short>(2034));

  CHECK_CLOSE(2034.0, results[0], 0.00000001);
  CHECK_CLOSE(2034.0, results[1], 0.00000001);
  CHECK_CLOSE(2034.0, results[2], 0.00000001);
  CHECK_CLOSE(2034.0, results[3], 0.00000001);
}

// These tests use different values and as such also the transpose.
TEST(AllVals8)
{
  runTest2(8);
}
TEST(AllVals16)
{
  runTest2(16);
}

void initDefaultCompileDefinitions(CompileDefinitions& defs)
{
  defs["NR_STATIONS"]            = boost::lexical_cast<string>(NR_STATIONS);
  defs["NR_CHANNELS"]            = boost::lexical_cast<string>(NR_CHANNELS);
  defs["NR_SAMPLES_PER_CHANNEL"] = boost::lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  defs["NR_SAMPLES_PER_SUBBAND"] = boost::lexical_cast<string>(NR_SAMPLES_PER_SUBBAND);
  defs["NR_BITS_PER_SAMPLE"]     = boost::lexical_cast<string>(NR_BITS_PER_SAMPLE);
  defs["NR_POLARIZATIONS"]       = boost::lexical_cast<string>(NR_POLARIZATIONS);
  defs["COMPLEX"]                = boost::lexical_cast<string>(COMPLEX);
}

gpu::Stream initDevice()
{
  // Set up device (GPU) environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    exit(3); // test skipped
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream cuStream(ctx);

  return cuStream;
}

int main()
{
  INIT_LOGGER("tIntToFloat");

  // init global(s): compile defs, device, context/stream.
  initDefaultCompileDefinitions(compileDefs);
  gpu::Stream strm(initDevice());
  stream = &strm;

  int exitStatus = UnitTest::RunAllTests();
  return exitStatus > 0 ? 1 : 0;
}


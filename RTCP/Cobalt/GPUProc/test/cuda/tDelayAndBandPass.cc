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
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <UnitTest++.h>

#include "TestUtil.h"

using namespace std;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;

// 
float * runTest(float bandPassFactor,
                float frequency = 0.0,
                float subbandWidth = 0.0,
                bool delayCompensation = false, 
                float delayBegin = 0.0,
                float delayEnd = 0.0,
                float PhaseOffset = 0.0)
{
  // Set up environment
  gpu::Platform pf;
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  Stream cuStream(ctx);
  std::stringstream tostrstream("");

  string kernelPath = "DelayAndBandPass.cu";  //The test copies the kernel to the current dir (also the complex header, needed for compilation)
 
  // Get an instantiation of the default parameters
  definitions_type definitions = defaultDefinitions();
  flags_type flags = defaultFlags();

  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NR_CHANNELS"] = "16";
  unsigned NR_CHANNELS = 16;
  definitions["NR_STATIONS"] = "2";
  unsigned NR_STATIONS = 2;
  definitions["NR_SAMPLES_PER_CHANNEL"] = "64";
  unsigned NR_SAMPLES_PER_CHANNEL = 64;
  definitions["NR_SAMPLES_PER_SUBBAND"] = "1024";
  unsigned NR_SAMPLES_PER_SUBBAND = 1024;
  definitions["NR_BITS_PER_SAMPLE"] = "8";
  unsigned NR_BITS_PER_SAMPLE = 8;
  definitions["NR_POLARIZATIONS"] = "2";
  unsigned NR_POLARIZATIONS = 2;
  definitions["NR_BEAMS"] = "8";
  unsigned NR_BEAMS = 8;
  definitions["USE_CUDA"] = "1";
  definitions["COMPLEX"] = "2";
  unsigned COMPLEX = 2;
  tostrstream << subbandWidth;
  definitions["SUBBAND_BANDWIDTH"] = tostrstream.str();
  tostrstream.clear();
  float SUBBAND_BANDWIDTH = subbandWidth;
  definitions["BANDPASS_CORRECTION"] = "1";
  if (delayCompensation)
    definitions["DELAY_COMPENSATION"] = "1";
  string ptx = createPTX(devices, kernelPath, flags, definitions);
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  Function  hKernel(module, "applyDelaysAndCorrectBandPass");  // c function this no argument overloading

  // *************************************************************
  // Create the data arrays  
  size_t sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX * sizeof(float);
  DeviceMemory DevFilteredMemory(ctx, sizeFilteredData);
  HostMemory rawFilteredData = getInitializedArray(ctx, sizeFilteredData, 1.0f);
  cuStream.writeBuffer(DevFilteredMemory, rawFilteredData);

  size_t sizeCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX * sizeof(float);
  DeviceMemory DevCorrectedMemory(ctx, sizeCorrectedData);
  HostMemory rawCorrectedData = getInitializedArray(ctx, sizeCorrectedData, 42.0f); 
  cuStream.writeBuffer(DevCorrectedMemory, rawCorrectedData);

  size_t sizeDelaysAtBeginData = NR_STATIONS * NR_BEAMS * 2 * sizeof(float);  
  DeviceMemory DevDelaysAtBeginMemory(ctx, sizeDelaysAtBeginData);
  HostMemory rawDelaysAtBeginData = getInitializedArray(ctx, sizeDelaysAtBeginData, delayBegin);
  cuStream.writeBuffer(DevDelaysAtBeginMemory, rawDelaysAtBeginData);
    
  size_t sizeDelaysAfterEndData = NR_STATIONS * NR_BEAMS * 2 * sizeof(float); 
  DeviceMemory DevDelaysAfterEndMemory(ctx, sizeDelaysAfterEndData);
  HostMemory rawDelaysAfterEndData = getInitializedArray(ctx, sizeDelaysAfterEndData, delayEnd);
  cuStream.writeBuffer(DevDelaysAfterEndMemory, rawDelaysAfterEndData);
    
  size_t sizePhaseOffsetData = NR_STATIONS * 2*sizeof(float); 
  DeviceMemory DevPhaseOffsetMemory(ctx, sizePhaseOffsetData);
  HostMemory rawPhaseOffsetData = getInitializedArray(ctx, sizePhaseOffsetData, PhaseOffset);
  cuStream.writeBuffer(DevPhaseOffsetMemory, rawPhaseOffsetData);

  size_t sizebandPassFactorsData = NR_CHANNELS * sizeof(float);
  DeviceMemory DevbandPassFactorsMemory(ctx, sizebandPassFactorsData);
  HostMemory rawbandPassFactorsData = getInitializedArray(ctx, sizebandPassFactorsData, bandPassFactor);
  cuStream.writeBuffer(DevbandPassFactorsMemory, rawbandPassFactorsData);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, DevCorrectedMemory);
  hKernel.setArg(1, DevFilteredMemory);
  hKernel.setArg(2, frequency);
  hKernel.setArg(3, 0U);
  hKernel.setArg(4, DevDelaysAtBeginMemory);
  hKernel.setArg(5, DevDelaysAfterEndMemory);
  hKernel.setArg(6, DevPhaseOffsetMemory);
  hKernel.setArg(7, DevbandPassFactorsMemory);

  // Calculate the number of threads in total and per blovk
  int nrChannelsPerSubband = NR_CHANNELS;
  int nrStations = NR_STATIONS; 
  int MAXNRCUDATHREADS = 1024;//doet moet nog opgevraagt worden en niuet als magish getal
  size_t maxNrThreads = MAXNRCUDATHREADS;
  unsigned totalNrThreads = nrChannelsPerSubband * NR_POLARIZATIONS * 2; //ps.nrChannelsPerSubband()
  unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
  // assign to gpu_wrapper objects
  Grid globalWorkSize(1, NR_CHANNELS == 1? 1: NR_CHANNELS/16, NR_STATIONS);  
  Block localWorkSize(256, 1,1); 

  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished
  
  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawCorrectedData, DevCorrectedMemory);
  cuStream.synchronize(); //assure copy from device is done
  
  // *************************************
  // Create the return values
  float *firstAndLastComplex = new float[4];
  // Return the first complex
  firstAndLastComplex[0] = rawCorrectedData.get<float>()[0];
  firstAndLastComplex[1] = rawCorrectedData.get<float>()[1];
  //return the last complex number
  firstAndLastComplex[2] = rawCorrectedData.get<float>()[(sizeCorrectedData / sizeof(float)) - 2];
  firstAndLastComplex[3] = rawCorrectedData.get<float>()[(sizeCorrectedData / sizeof(float)) - 1];

  // *************************************
  // cleanup memory

  return firstAndLastComplex;
}

TEST(BandPass)
{
  // ***********************************************************
  // Test if the bandpass correction factor is applied correctly in isolation
  float bandPassFactor = 2.0;
  float * results;

  // The input samples are all ones
  // After correction, multiply with 2.
  // The first and the last complex values are retrieved. They should be scaled with the bandPassFactor == 2
  results = runTest(bandPassFactor);

  CHECK_CLOSE(2.0, results[0], 0.00001);
  CHECK_CLOSE(2.0, results[1], 0.00001);
  CHECK_CLOSE(2.0, results[2], 0.00001);
  CHECK_CLOSE(2.0, results[3], 0.00001);

  delete[] results;
}

TEST(PhaseOffsets)
{
  float * results;

  //**********************************************************************
  // Delaycompensation but only for the phase ofsets:
  // All computations the drop except the phase ofset of 1,0 which is fed into a cosisin
  // cosisin(pi) = -1
  results = runTest(1.0,   // bandpass factor
                    1.0,   // frequency
                    1.0,   
                    true,  // delayCompensation
                    0.0,   // delays begin  
                    0.0,   // delays end
                    M_PI); // phase offsets

  CHECK_CLOSE(-1.0, results[0], 0.00001);
  CHECK_CLOSE(-1.0, results[1], 0.00001);
  CHECK_CLOSE(-1.0, results[2], 0.00001);
  CHECK_CLOSE(-1.0, results[3], 0.00001);

  delete[] results;
}

SUITE(DelayCompensation)
{
  TEST(ConstantDelay)
  {
    float * results;

    //****************************************************************************
    // delays  begin and end both 1 no phase offset frequency 1 width 1
    // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
    //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
    // cosisin(-3.14159+0 i) == -1
    results = runTest(1.0,   // bandpass factor
                      1.0,   // frequency
                      1.0,   
                      true,  // delayCompensation
                      1.0,   // delays begin  
                      1.0,   // delays end
                      0.0);  // phase offsets

    CHECK_CLOSE(-1.0, results[0], 0.00001);
    CHECK_CLOSE(-1.0, results[1], 0.00001);
    CHECK_CLOSE(-1.0, results[2], 0.00001);
    CHECK_CLOSE(-1.0, results[3], 0.00001);

    delete[] results;
  }

  TEST(SlopedDelay)
  {
    float * results;

    //****************************************************************************
    // delays  begin 1 and end 0 no phase offset frequency 1 width 1
    // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
    //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
    // cosisin(-3.14159+0 i) == -1
    // The later sets of samples are calculate as:
    // vX = vX * dvX;  The delays are multiplied because we are calculating with exponents
    // Ask john Romein for more details
    results = runTest(1.0,   // bandpass factor
                      1.0,   // frequency
                      1.0,   
                      true,  // delayCompensation
                      1.0,   // delays begin  
                      0.0,   // delays end
                      0.0);  // phase offsets

    CHECK_CLOSE(-1.0,     results[0], 0.00001);
    CHECK_CLOSE(-1.0,     results[1], 0.00001);
    CHECK_CLOSE(1.047860, results[2], 0.00001);
    CHECK_CLOSE(0.949728, results[3], 0.00001);

    delete[] results;
  }
}

TEST(AllAtOnce)
{
  float * results;

  //****************************************************************************
  // delays  begin 1 and end 0 no phase offset frequency 1 width 1
  // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
  //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
  // cosisin(-3.14159+0 i) == -1
  // The later sets of samples are calculate as:
  // vX = vX * dvX;  The delays are multiplied because we are calculating with exponents
  // Ask john Romein for more details
  // In this test the phase offsets are also compensated
  results = runTest(2.0,   // bandpass factor (weights == 2)
                    1.0,   // frequency
                    1.0,   
                    true,  // delayCompensation
                    1.0,   // delays begin  
                    0.0,   // delays end
                    1.0);  // phase offsets (correct with e = 2.71828)

  CHECK_CLOSE( 0.602337, results[0], 0.00001);
  CHECK_CLOSE(-2.763550, results[1], 0.00001);
  CHECK_CLOSE(-0.466011, results[2], 0.00001);
  CHECK_CLOSE( 2.789770, results[3], 0.00001);

  delete[] results;
}

int main()
{
  INIT_LOGGER("tDelayAndBandPass");

  return UnitTest::RunAllTests() > 0;
}


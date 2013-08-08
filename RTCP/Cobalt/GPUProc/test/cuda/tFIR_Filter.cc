//# FIR_Filter.cc
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

#include "tFIR_Filter.h"

#include <cstdlib> 
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

#include "../TestUtil.h"

using namespace std;
using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace LOFAR::Cobalt::gpu;

using boost::lexical_cast;

int test()
{
  bool testOk = true;

  string kernelFile = "FIR_Filter.cu";
  string function = "FIR_filter";

  // Get an instantiation of the default parameters
  CompileDefinitions definitions = CompileDefinitions();
  CompileFlags flags = defaultCompileFlags();

  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NR_TAPS"] = lexical_cast<string>(NR_TAPS);
  definitions["NR_STABS"] = lexical_cast<string>(NR_STATIONS);
  definitions["NR_CHANNELS"] = lexical_cast<string>(NR_CHANNELS);
  definitions["NR_SAMPLES_PER_CHANNEL"] = lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  definitions["NR_POLARIZATIONS"] = lexical_cast<string>(NR_POLARIZATIONS);
  definitions["COMPLEX"] = lexical_cast<string>(COMPLEX);
  definitions["NR_BITS_PER_SAMPLE"] = lexical_cast<string>(NR_BITS_PER_SAMPLE);

  // Create a default context
  Platform pf;
  Device device(0);
  Context ctx(device);
  Stream stream(ctx);
  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(kernelFile, definitions, flags, devices);
  Module module(createModule(ctx, kernelFile, ptx));
  Function hKernel(module, function);

  // Create the needed data
  unsigned sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
  HostMemory rawFilteredData = getInitializedArray(ctx, sizeFilteredData * sizeof(float), 0.0f);

  unsigned sizeSampledData = NR_STATIONS * (NR_TAPS - 1 + NR_SAMPLES_PER_CHANNEL) * NR_CHANNELS * NR_POLARIZATIONS * COMPLEX;               
  HostMemory rawInputSamples = getInitializedArray(ctx, sizeSampledData * sizeof(signed char), char(0));

  unsigned sizeWeightsData = NR_CHANNELS * NR_TAPS;
  HostMemory rawFirWeights = getInitializedArray(ctx, sizeWeightsData * sizeof(float), 0.0f);

  // Data on the gpu
  DeviceMemory devFilteredData(ctx, sizeFilteredData * sizeof(float));
  DeviceMemory devSampledData(ctx, sizeSampledData * sizeof(float));
  DeviceMemory devFirWeights(ctx, sizeWeightsData * sizeof(float));

  unsigned station, sample, ch, pol;

  // Test 1: Single impulse test on single non-zero weight
  station = ch = pol = 0;
  sample = NR_TAPS - 1; // skip FIR init samples
  rawFirWeights.get<float>()[0] = 2.0f;
  MultiDimArray<signed char, 5> inputSamplesArr(boost::extents[NR_STATIONS][NR_SAMPLES_PER_CHANNEL + (NR_TAPS - 1)][NR_CHANNELS][NR_POLARIZATIONS][COMPLEX]
, rawInputSamples.get<signed char>(), false);
  inputSamplesArr[station][sample][ch][pol][0] = 3;

  // Copy input vectors from host memory to GPU buffers.
  stream.writeBuffer(devFilteredData, rawFilteredData, true);
  stream.writeBuffer(devSampledData, rawInputSamples, true);
  stream.writeBuffer(devFirWeights, rawFirWeights, true);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, devFilteredData);
  hKernel.setArg(1, devSampledData);
  hKernel.setArg(2, devFirWeights);

  // Calculate the number of threads in total and per blovk
  int MAXNRCUDATHREADS = 1024;//doet moet nog opgevraagt worden en niuet als magish getal
  size_t maxNrThreads = MAXNRCUDATHREADS;
  unsigned totalNrThreads = NR_CHANNELS * NR_POLARIZATIONS * 2; //ps.nrChannelsPerSubband()
  unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
  
  Grid globalWorkSize(nrPasses, NR_STATIONS); 
  Block localWorkSize(totalNrThreads / nrPasses, 1); 

  // Run the kernel
  stream.synchronize();
  stream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  stream.synchronize();

  stream.readBuffer(rawFilteredData, devFilteredData, true);

  // Expected output: St0, pol0, ch0, sampl0: 6. The rest all 0.
  // However, in modes other than 16 bit mode, all amplitudes are scaled to match 16 bit mode.
  // For 8 bit mode, this means *256.
  unsigned scale = 1;
  if (NR_BITS_PER_SAMPLE != 16)
    scale = 256;
  if (rawFilteredData.get<float>()[0] != 6.0f * scale) 
  {
    std::cerr << "FIR_FilterTest 1: Expected at idx 0: " << 6 * scale << "; got: " << rawFilteredData.get<float>()[0] << std::endl;

    testOk = false;
  }
  std::cerr << "Weights returned " << rawFilteredData.get<float>()[0] << std::endl;

  const size_t nrExpectedZeros = sizeFilteredData - 1;
  size_t nrZeros = 0;
  for (unsigned i = 1; i < sizeFilteredData; i++) 
    if (rawFilteredData.get<float>()[i] == 0.0f) {
      nrZeros ++;
    } else {
      std::cerr << "filteredData[" << i << "] = " << rawFilteredData.get<float>()[i] << std::endl;
    }

  if (nrZeros != nrExpectedZeros) 
  {
    std::cerr << "FIR_FilterTest 1: Unexpected non-zero(s). Only " << nrZeros << " zeros out of " << nrExpectedZeros << std::endl;
    testOk = false;
  }

  return testOk ? 0 : 1;
}

int main()
{
  INIT_LOGGER("tFIR_Filter");
  return test() > 0;
}


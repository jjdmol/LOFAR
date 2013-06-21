//# tBeamFormer.cc: test BeamFormer CUDA kernel
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

#include <cstdlib>  // for rand()
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>

#include "TestUtil.h"

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using LOFAR::Exception;

// The tests succeeds for different values of stations, channels, samples and tabs.
unsigned NR_STATIONS = 4;  
unsigned NR_CHANNELS = 4;
unsigned NR_SAMPLES_PER_CHANNEL = 4;
unsigned NR_TABS = 4;
unsigned NR_POLARIZATIONS = 2;
unsigned COMPLEX = 2;


// Create the data arrays
size_t lengthWeightsData = NR_STATIONS * NR_CHANNELS * NR_TABS * COMPLEX ;
size_t lengthBandPassCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX;
size_t lengthComplexVoltagesData = NR_CHANNELS* NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS * COMPLEX;



void exit_with_print(float *outputOnHostPtr)
{
  // Plot the output of the kernel in a readable manner
  unsigned size_channel = NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS * COMPLEX;
  unsigned size_sample = NR_TABS * NR_POLARIZATIONS * COMPLEX;
  unsigned size_tab = NR_POLARIZATIONS * COMPLEX;
  for (unsigned idx_channel = 0; idx_channel < NR_CHANNELS; ++ idx_channel)
  {
    cout << "idx_channel: " << idx_channel << endl;
    for (unsigned idx_samples = 0; idx_samples < NR_SAMPLES_PER_CHANNEL; ++ idx_samples)
    
    {
      cout << "idx_samples " << idx_samples << ": ";
      for (unsigned idx_tab = 0; idx_tab < NR_TABS; ++ idx_tab)
      {
        unsigned index_data_base = size_channel * idx_channel + 
                                   size_sample * idx_samples +
                                   size_tab * idx_tab ;

        cout << "(" << outputOnHostPtr[index_data_base] << ", " 
             << outputOnHostPtr[index_data_base +1] << ")-" 
             << "(" << outputOnHostPtr[index_data_base + 2] 
             << ", " << outputOnHostPtr[index_data_base +3] << ")  ";
      }
      cout << endl;
    }
    cout << endl;
  }
  
  exit(-1);
}


HostMemory runTest(gpu::Context ctx,
                   Stream cuStream,
                   float * weightsData,
                   float * bandPassCorrectedData,
                   float * complexVoltagesData,
                   string function)
{
  string kernelFile = "BeamFormer.cu";

  cout << "\n==== runTest: function = " << function << " ====\n" << endl;

  // Get an instantiation of the default parameters
  CompileFlags flags = CompileFlags();
  CompileDefinitions definitions = CompileDefinitions();
  
  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NVIDIA_CUDA"] = "";
  definitions["NR_STATIONS"] = lexical_cast<string>(NR_STATIONS);
  definitions["NR_CHANNELS"] = lexical_cast<string>(NR_CHANNELS);
  definitions["NR_SAMPLES_PER_CHANNEL"] = lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  definitions["NR_TABS"] = lexical_cast<string>(NR_TABS);
  definitions["NR_POLARIZATIONS"] = lexical_cast<string>(NR_POLARIZATIONS);
  definitions["COMPLEX"] = lexical_cast<string>(COMPLEX);
  

  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(kernelFile, flags, definitions, devices);
  gpu::Module module(createModule(ctx, kernelFile, ptx));
  Function hKernel(module, function);   // c function this no argument overloading

  // *************************************************************
  // Create the data arrays
  size_t sizeWeightsData = lengthWeightsData* sizeof(float);
  DeviceMemory devWeightsMemory(ctx, sizeWeightsData);
  HostMemory rawWeightsData = getInitializedArray(ctx, sizeWeightsData, 1.0f);
  float *rawWeightsPtr = rawWeightsData.get<float>();
  for (unsigned idx = 0; idx < lengthWeightsData; ++idx)
    rawWeightsPtr[idx] = weightsData[idx];
  cuStream.writeBuffer(devWeightsMemory, rawWeightsData);

  size_t sizeBandPassCorrectedData= lengthBandPassCorrectedData * sizeof(float);
  DeviceMemory devBandPassCorrectedMemory(ctx, sizeBandPassCorrectedData);
  HostMemory rawBandPassCorrectedData = getInitializedArray(ctx, sizeBandPassCorrectedData, 2.0f);
  float *rawBandPassCorrectedPtr = rawBandPassCorrectedData.get<float>();
  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
    rawBandPassCorrectedPtr[idx] = bandPassCorrectedData[idx];
  cuStream.writeBuffer(devBandPassCorrectedMemory, rawBandPassCorrectedData);

  size_t sizeComplexVoltagesData = lengthComplexVoltagesData * sizeof(float);
  DeviceMemory devComplexVoltagesMemory(ctx, sizeComplexVoltagesData);
  HostMemory rawComplexVoltagesData = getInitializedArray(ctx, sizeComplexVoltagesData, 3.0f);
  float *rawComplexVoltagesPtr = rawComplexVoltagesData.get<float>();
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
    rawComplexVoltagesPtr[idx] = complexVoltagesData[idx];
  // Write output content.
  cuStream.writeBuffer(devComplexVoltagesMemory, rawComplexVoltagesData);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, devComplexVoltagesMemory);
  hKernel.setArg(1, devBandPassCorrectedMemory);
  hKernel.setArg(2, devWeightsMemory);

  // Calculate the number of threads in total and per block
  Grid globalWorkSize(1, 1, 1);
  Block localWorkSize(NR_POLARIZATIONS, NR_TABS, NR_CHANNELS);

  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished

  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawComplexVoltagesData, devComplexVoltagesMemory);
  cuStream.synchronize(); //assure copy from device is done

  return rawComplexVoltagesData; 
}

Exception::TerminateHandler t(Exception::terminate);


int main()
{
  INIT_LOGGER("tBeamFormer");
  const char* function = "beamFormer";
  try 
  {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } 
  catch (gpu::CUDAException& e) 
  {
    cerr << e.what() << endl;
    return 3;
  }

  // Create a default context
  gpu::Device device(0);
  gpu::Context ctx(device);
  Stream cuStream(ctx);

  // Define the input and output arrays
  // ************************************************************* 
  float * complexVoltagesData = new float[lengthComplexVoltagesData];
  float * bandPassCorrectedData = new float[lengthBandPassCorrectedData];
  float * weightsData= new float[lengthWeightsData];
  float * outputOnHostPtr;

  // ***********************************************************
  // Baseline test: If all weight data is zero the output should be zero
  // The output array is initialized with 42s
  cout << "test 1" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 42.0f;
    complexVoltagesData[idx * 2 + 1] = 42.0f;
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx * 2] = 1.0f;
    bandPassCorrectedData[idx * 2+ 1] = 1.0f;
  }

  for (unsigned idx = 0; idx < lengthWeightsData/2; ++idx)
  {
    weightsData[idx * 2] = 0.0f;
    weightsData[idx * 2 + 1] = 0.0f;
  }

  HostMemory outputOnHost = runTest(ctx, cuStream, weightsData,
    bandPassCorrectedData,complexVoltagesData, function);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost.get<float>();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    if (outputOnHostPtr[idx] != 0.0f)
    {
      cerr << "The data returned by the kernel should be all zero: All weights are zero";
      exit_with_print(outputOnHostPtr);
    }
    
  // ***********************************************************
  // Baseline test 2: If all input data is zero the output should be zero while the wheights are non zero
  // The output array is initialized with 42s
  cout << "test 2" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 42.0f;
    complexVoltagesData[idx * 2 + 1] = 42.0f;
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx * 2] = 0.0f;
    bandPassCorrectedData[idx * 2+ 1] = 0.0f;  }

  for (unsigned idx = 0; idx < lengthWeightsData/2; ++idx)
  {
    weightsData[idx * 2] = 1.0f;
    weightsData[idx * 2 + 1] = 1.0f;
  }

  HostMemory outputOnHost2 = runTest(ctx, cuStream, weightsData,
    bandPassCorrectedData,complexVoltagesData, function);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost2.get<float>();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    if (outputOnHostPtr[idx] != 0.0f)
    {
      cerr << "The data returned by the kernel should be all zero: All inputs are zero";
      exit_with_print(outputOnHostPtr);
    }
    

  // ***********************************************************
  // Test 3: all inputs 1 (including imag)
  // with only the real weight set to 1.
  // all outputs should be 4
  cout << "test 3" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 42.0f;
    complexVoltagesData[idx * 2 + 1] = 42.0f;
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx * 2] = 1.0f;
    bandPassCorrectedData[idx * 2+ 1] = 1.0f;  }

  for (unsigned idx = 0; idx < lengthWeightsData/2; ++idx)
  {
    weightsData[idx * 2] = 1.0f;
    weightsData[idx * 2 + 1] = 0.0f;
  }

  HostMemory outputOnHost3 = runTest(ctx, cuStream, weightsData,
    bandPassCorrectedData,complexVoltagesData, function);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost3.get<float>();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    if (outputOnHostPtr[idx] != NR_STATIONS * 1.0f)
    {
      cerr << "all the data returned by the kernel should be (" << NR_STATIONS << ", " << NR_STATIONS << ")" ;
      exit_with_print(outputOnHostPtr);
    }
    
  // ***********************************************************
  // Test 4: all inputs 1 (including imag)
  // with only the real weight set to 1 and imag also 1
  // all outputs should be (0,8) 
  // (1 , i) * (1, i) == 1 * 1 + i * i + 1 * i + 1 * i= 1 -1 +2i = 2i
  // times 4 stations is (0.8)
  cout << "test 4" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 42.0f;
    complexVoltagesData[idx * 2 + 1] = 42.0f;
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx * 2] = 1.0f;
    bandPassCorrectedData[idx * 2+ 1] = 1.0f;  }

  for (unsigned idx = 0; idx < lengthWeightsData/2; ++idx)
  {
    weightsData[idx * 2] = 1.0f;
    weightsData[idx * 2 + 1] = 1.0f;
  }

  HostMemory outputOnHost4 = runTest(ctx, cuStream, weightsData,
    bandPassCorrectedData,complexVoltagesData, function);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost4.get<float>();
  for (size_t idx = 0; idx < lengthComplexVoltagesData/2; ++idx)
  {
    if (outputOnHostPtr[idx  * 2] != 0.0f)
    {
      cerr << "The REAL data returned by the kernel should be all zero: both input and weights are (1, i)";
      exit_with_print(outputOnHostPtr);
    }
    if (outputOnHostPtr[idx * 2 + 1] != NR_STATIONS * 2 * 1.0f)
    {
      cerr << "The imag data should be all " << NR_STATIONS * 2 * 1.0f;
      exit_with_print(outputOnHostPtr);
    } 
  }

  delete [] complexVoltagesData;
  delete [] bandPassCorrectedData;
  delete [] weightsData;

  return 0;
}


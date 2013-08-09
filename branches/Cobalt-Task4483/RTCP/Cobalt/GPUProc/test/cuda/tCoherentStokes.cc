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
//# $Id: tBeamFormer.cc 25747 2013-07-24 13:12:39Z klijn $

#include <lofar_config.h>

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
unsigned NR_CHANNELS = 16;
unsigned NR_SAMPLES_PER_CHANNEL = 128;
unsigned TIME_PARALLEL_FACTOR = 2;  // cannot be more then NR_SAMPLES_PER_CHANNEL / INTEGRATION_SIZE
unsigned NR_TABS = 1; 
unsigned NR_POLARIZATIONS = 2;
unsigned COMPLEX = 2;
unsigned NR_COHERENT_STOKES = 4;
unsigned INTEGRATION_SIZE = 64;
unsigned STOKES_INTEGRATION_SAMPLES = NR_SAMPLES_PER_CHANNEL / INTEGRATION_SIZE;




// Create the data arrays
size_t lengthInputData = NR_TABS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
size_t lengthOutputData = NR_TABS * NR_COHERENT_STOKES *  (NR_SAMPLES_PER_CHANNEL / INTEGRATION_SIZE) * NR_CHANNELS;

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
  
  exit(1);
}

HostMemory runTest(Context ctx,
                   Stream cuStream,
                   float * inputData,
                   float * outputData,
                   string function)
{
  string kernelFile = "CoherentStokes.cu";

  cout << "\n==== runTest: function = " << function << " ====\n" << endl;

  // Get an instantiation of the default parameters
  CompileFlags flags = CompileFlags();
  CompileDefinitions definitions = CompileDefinitions();
  
  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NVIDIA_CUDA"] = "";
  definitions["NR_CHANNELS"] = lexical_cast<string>(NR_CHANNELS);
  definitions["NR_SAMPLES_PER_CHANNEL"] = lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  definitions["NR_TABS"] = lexical_cast<string>(NR_TABS);
  definitions["NR_POLARIZATIONS"] = lexical_cast<string>(NR_POLARIZATIONS);
  definitions["COMPLEX"] = lexical_cast<string>(COMPLEX);
  definitions["NR_COHERENT_STOKES"] = lexical_cast<string>(NR_COHERENT_STOKES);
  definitions["STOKES_INTEGRATION_SAMPLES"] = lexical_cast<string>(STOKES_INTEGRATION_SAMPLES);
  definitions["INTEGRATION_SIZE"] = lexical_cast<string>(INTEGRATION_SIZE);
  definitions["TIME_PARALLEL_FACTOR"] = lexical_cast<string>(TIME_PARALLEL_FACTOR);

  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(kernelFile, definitions, flags, devices);
  Module module(createModule(ctx, kernelFile, ptx));
  Function hKernel(module, function);   

  // *************************************************************
  // Create the data arrays
  
  size_t sizeInputData= lengthInputData * sizeof(float);
  DeviceMemory devInputMemory(ctx, sizeInputData);
  HostMemory rawInputData = getInitializedArray(ctx, sizeInputData, 2.0f);
  float *rawInputPtr = rawInputData.get<float>();
  for (unsigned idx = 0; idx < lengthInputData; ++idx)
    rawInputPtr[idx] = inputData[idx];
  cuStream.writeBuffer(devInputMemory, rawInputData);

  size_t sizeOutputData = lengthOutputData * sizeof(float);
  DeviceMemory devOutputMemory(ctx, sizeOutputData);
  HostMemory rawOutputData = getInitializedArray(ctx, sizeOutputData, 3.0f);
  float *rawOutputPtr = rawOutputData.get<float>();
  for (unsigned idx = 0; idx < lengthOutputData; ++idx)
    rawOutputPtr[idx] = outputData[idx];
  // Write output content.
  cuStream.writeBuffer(devOutputMemory, rawOutputData);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, devOutputMemory);
  hKernel.setArg(1, devInputMemory);
  
  // Calculate the number of threads in total and per block
  Grid globalWorkSize(NR_CHANNELS, TIME_PARALLEL_FACTOR, NR_TABS);
  Block localWorkSize(NR_CHANNELS, TIME_PARALLEL_FACTOR, NR_TABS);
    
  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished

  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawOutputData, devOutputMemory);
  cuStream.synchronize(); //assure copy from device is done

  return rawOutputData; 
}

Exception::TerminateHandler t(Exception::terminate);


int main()
{

  INIT_LOGGER("tCoherentStokes");
  const char* function = "coherentStokes";
  try 
  {
    Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } 
  catch (CUDAException& e) 
  {
    cerr << e.what() << endl;
    return 3;
  }

  // Create a default context
  Device device(0);
  Context ctx(device);
  Stream cuStream(ctx);

  // Define the input and output arrays
  // ************************************************************* 
  float * intputData = new float[lengthInputData];
  float * outputData = new float[lengthOutputData];
  
  float * outputOnHostPtr;

  // ***********************************************************
  // Baseline test: If all weight data is zero the output should be zero
  // The output array is initialized with 42s
  cout << "test 1" << endl;
  for (unsigned idx = 0; idx < lengthInputData; ++idx)
  {
    intputData[idx] = 0.0f;
  }

  for (unsigned idx = 0; idx < lengthOutputData / 2; ++idx)
  {
    outputData[idx * 2] = 1.0f;
    outputData[idx * 2+ 1] = 1.0f;
  }


  HostMemory outputOnHost = runTest(ctx, cuStream, 
    intputData, outputData, function);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost.get<float>();
  for (size_t idx = 0; idx < lengthOutputData; ++idx)
    cout << outputOnHostPtr[idx] << ',';

  delete [] intputData;
  delete [] outputData;

  return 0;
}

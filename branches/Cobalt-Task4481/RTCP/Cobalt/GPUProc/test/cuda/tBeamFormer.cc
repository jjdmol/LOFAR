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

#include <cstdlib>  // for rand()
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <curand>

#include "TestUtil.h"

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using LOFAR::Exception;

unsigned NR_INPUTS = 4;
unsigned NR_NEURONS = 16;  // 16 * 2 == width of thread wave
unsigned NR_TIMESTEPS = 3;
unsigned COMPLEX = 2;
unsigned NR_BASENETWORKS = 1;
unsigned NR_SOLUTION_PARALEL = 128;


// Create the data arrays
size_t lengthInputData = NR_INPUTS * NR_TIMESTEPS * COMPLEX ;
size_t lengthInputsWeightsData = NR_INPUTS * NR_NEURONS * COMPLEX;
size_t lengthWeightsData = NR_BASENETWORKS * NR_NEURONS * NR_NEURONS *  COMPLEX;
size_t lengthActivationsData = NR_BASENETWORKS * NR_NEURONS * NR_NEURONS * NR_SOLUTION_PARALEL * COMPLEX;

HostMemory runTest(gpu::Context ctx,
                   Stream cuStream,
                   float * inputData,
                   float * inputsWeightsData,
                   float * weightsData,
                   string function)
{
  string kernelFile = "Echo.cu";

  cout << "\n==== runTest: function = " << function << " ====\n" << endl;

  // Get an instantiation of the default parameters
  definitions_type definitions = defaultDefinitions();
  flags_type flags = defaultFlags();

  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NVIDIA_CUDA"] = "";
  definitions["NR_INPUTS"] = lexical_cast<string>(NR_INPUTS);
  definitions["NR_NEURONS"] = lexical_cast<string>(NR_NEURONS);
  definitions["NR_TIMESTEPS"] = lexical_cast<string>(NR_TIMESTEPS);
  definitions["NR_BASENETWORKS"] = lexical_cast<string>(NR_BASENETWORKS);
  definitions["NR_SOLUTION_PARALEL"] = lexical_cast<string>(NR_SOLUTION_PARALEL);
  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(devices, kernelFile, flags, definitions);
  gpu::Module module(createModule(ctx, kernelFile, ptx));
  Function hKernel(module, function);   // c function this no argument overloading

  // *************************************************************
  // Create the data arrays
  size_t sizeInputData = lengthInputData* sizeof(float);
  DeviceMemory devInputMemory(ctx, sizeInputData);
  HostMemory rawInputData = getInitializedArray(ctx, sizeInputData, 1.0f);
  float *rawInputPtr = rawInputData.get<float>();
  for (unsigned idx = 0; idx < lengthInputData; ++idx)
    rawInputPtr[idx] = inputData[idx];
  cuStream.writeBuffer(devInputMemory, rawInputData);

  size_t sizeInputsWeightsData = lengthInputsWeightsData * sizeof(float);
  DeviceMemory devInputsWeightsMemory(ctx, sizeInputsWeightsData);
  HostMemory rawInputsWeightsData = getInitializedArray(ctx, sizeInputsWeightsData, 2.0f);
  float *rawInputsWeightsPtr = rawInputsWeightsData.get<float>();
  for (unsigned idx = 0; idx < lengthInputsWeightsData; ++idx)
    rawInputsWeightsPtr[idx] = inputsWeightsData[idx];
  cuStream.writeBuffer(devInputsWeightsMemory, rawInputsWeightsData);


  size_t sizeWeightsData = lengthWeightsData * sizeof(float);
  DeviceMemory devWeightsMemory(ctx, sizeWeightsData);
  HostMemory rawWeightsData = getInitializedArray(ctx, sizeWeightsData, 3.0f);
  float *rawWeightsPtr = rawWeightsData.get<float>();
  for (unsigned idx = 0; idx < lengthWeightsData; ++idx)
  rawWeightsPtr[idx] = weightsData[idx];
  cuStream.writeBuffer(devWeightsMemory, rawWeightsData);

  size_t sizeActivationsData = lengthActivationsData* sizeof(float);
  DeviceMemory devActivationsMemory(ctx, sizeActivationsData);
  HostMemory rawActivationsData = getInitializedArray(ctx, sizeActivationsData, 42.0f);
  cuStream.writeBuffer(devActivationsMemory, rawActivationsData);

  curandState* devStates;
  cudaMalloc ( &devStates, N*sizeof( curandState ) );

  // ****************************************************************************
  // Run the kernel on the created data

  hKernel.setArg(0, devActivationsMemory);
  hKernel.setArg(1, devInputMemory);
  hKernel.setArg(2, devInputsWeightsMemory);
  hKernel.setArg(3, devWeightsMemory);
  hKernel.setArg(4, devStates);

  // Calculate the number of threads in total and per blovk
  //Grid globalWorkSize(nrPasses, nrUsableChannels, 1);
  //Block localWorkSize(nrThreads, 1,1);

  Grid globalWorkSize(1, 1, 1);
  Block localWorkSize(1, 1, 1);

  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished

  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawActivationsData, devActivationsMemory);
  cuStream.synchronize(); //assure copy from device is done

  return rawActivationsData; //TODO CORRECT THIS OUTPUT!!!
}

Exception::TerminateHandler t(Exception::terminate);

int main()
{
  INIT_LOGGER("tCorrelator");

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

  // Define dependend paramters
  // *************************************************************
  // Create data members
  float * inputData = new float[lengthInputData];
  for (unsigned idx = 0; idx < lengthInputData / 2; ++idx)
  {
    inputData[idx * 2] = 1.0f;
    inputData[idx * 2 + 1] = 0.0f;
  }
  float * inputsWeightsData = new float[lengthInputsWeightsData];
  for (unsigned idx = 0; idx < lengthInputsWeightsData / 2; ++idx)
  {
    inputsWeightsData[idx * 2] = 2.0f;
    inputsWeightsData[idx * 2+ 1] = 0.0f;
  }
  float * weightsData= new float[lengthWeightsData];
  for (unsigned idx = 0; idx < lengthWeightsData/2; ++idx)
  {
    weightsData[idx * 2] = 5.0f;
    weightsData[idx * 2 + 1] = 0.0f;
  }
  float * activationsData = new float[lengthActivationsData];

  float * outputOnHostPtr;
  
  const char* function = "echo";

  // ***********************************************************
  // Baseline test: If all input data is zero the output should be zero
  // The output array is initialized with 42s
  HostMemory outputOnHost = runTest(ctx, cuStream, inputData,
    inputsWeightsData,weightsData, function);

  // Copy the output data to a local array
  outputOnHostPtr = outputOnHost.get<float>();
  for (size_t idx = 0; idx < lengthActivationsData; ++idx)
    activationsData[idx] = outputOnHostPtr[idx];

  cout << "Returned output: "  << endl;
  for (unsigned idx_neuron = 0; idx_neuron < NR_NEURONS; ++ idx_neuron)
  {
    cout << "(" << activationsData[idx_neuron*2] << ", " << activationsData[idx_neuron*2 + 1] << ")" ;
    for (unsigned idx_timestep = 1; idx_timestep < NR_TIMESTEPS; ++ idx_timestep)
    {
      cout << ", (" << activationsData[idx_timestep *NR_NEURONS * 2 + idx_neuron*2] << ", " 
           << activationsData[idx_timestep *NR_NEURONS * 2 + idx_neuron*2 + 1] << ") " ;
    }
    cout << endl;
  }
  


  delete [] inputData;
  delete [] inputsWeightsData;
  delete [] weightsData;
  delete [] activationsData;

  return 0;
}



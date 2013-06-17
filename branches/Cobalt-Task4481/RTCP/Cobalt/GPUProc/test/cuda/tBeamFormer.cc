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

#include "TestUtil.h"

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using LOFAR::Exception;

unsigned NR_STATIONS = 160;  // 16 * 2 == width of thread wave
unsigned NR_CHANNELS = 3;
unsigned NR_SAMPLES_PER_CHANNEL = 2;
unsigned NR_TABS = 2;
unsigned NR_POLARIZATIONS = 16;
unsigned NR_BEAMS = 16;
unsigned COMPLEX = 2;


// Create the data arrays
size_t lengthWeightsData = NR_STATIONS * NR_CHANNELS * NR_TABS * COMPLEX ;
size_t lengthBandPassCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX;
size_t lengthComplexVoltagesData = NR_CHANNELS* NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS * COMPLEX;

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
  definitions_type definitions = defaultDefinitions();
  flags_type flags = defaultFlags();

  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NVIDIA_CUDA"] = "";
  definitions["NR_STATIONS"] = lexical_cast<string>(NR_STATIONS);
  definitions["NR_CHANNELS"] = lexical_cast<string>(NR_CHANNELS);
  definitions["NR_SAMPLES_PER_CHANNEL"] = lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  definitions["NR_TABS"] = lexical_cast<string>(NR_TABS);
  definitions["NR_POLARIZATIONS"] = lexical_cast<string>(NR_POLARIZATIONS);
  definitions["NR_BEAMS"] = lexical_cast<string>(NR_BEAMS);
  definitions["COMPLEX"] = lexical_cast<string>(COMPLEX);
  

  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(devices, kernelFile, flags, definitions);
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
  cuStream.writeBuffer(devComplexVoltagesMemory, rawComplexVoltagesData);




  // ****************************************************************************
  // Run the kernel on the created data

  hKernel.setArg(0, devComplexVoltagesMemory);
  hKernel.setArg(1, devBandPassCorrectedMemory);
  hKernel.setArg(2, devWeightsMemory);

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
  cuStream.readBuffer(rawComplexVoltagesData, devComplexVoltagesMemory);
  cuStream.synchronize(); //assure copy from device is done

  return rawComplexVoltagesData; 
}

Exception::TerminateHandler t(Exception::terminate);



int main()
{
  INIT_LOGGER("tBeamFormer");

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
  float * complexVoltagesData = new float[lengthComplexVoltagesData];
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 42.0f;
    complexVoltagesData[idx * 2 + 1] = 0.0f;
  }

  float * bandPassCorrectedData = new float[lengthBandPassCorrectedData];
  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx * 2] = 2.0f;
    bandPassCorrectedData[idx * 2+ 1] = 0.0f;
  }
  float * weightsData= new float[lengthWeightsData];
  for (unsigned idx = 0; idx < lengthWeightsData/2; ++idx)
  {
    weightsData[idx * 2] = 5.0f;
    weightsData[idx * 2 + 1] = 0.0f;
  }
  
  const char* function = "beamFormer";

  // ***********************************************************
  // Baseline test: If all input data is zero the output should be zero
  // The output array is initialized with 42s
  HostMemory outputOnHost = runTest(ctx, cuStream, complexVoltagesData,
    bandPassCorrectedData,weightsData, function);

  // Copy the output data to a local array
  float * outputOnHostPtr = outputOnHost.get<float>();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    complexVoltagesData[idx] = outputOnHostPtr[idx];

  cout << "Returned output: "  << endl;

  cout << "(" <<  complexVoltagesData[0] << ", " <<  complexVoltagesData[1] << endl;
  //for (unsigned idx_neuron = 0; idx_neuron < 10; ++ idx_neuron)
  //{
  //  cout << "(" << activationsData[idx_neuron*2] << ", " << activationsData[idx_neuron*2 + 1] << ")" ;
  //  for (unsigned idx_timestep = 1; idx_timestep < NR_TIMESTEPS; ++ idx_timestep)
  //  {
  //    cout << ", (" << activationsData[idx_timestep *NR_NEURONS * 2 + idx_neuron*2] << ", " 
  //         << activationsData[idx_timestep *NR_NEURONS * 2 + idx_neuron*2 + 1] << ") " ;
  //  }
  //  cout << endl;
  //}
  


  delete [] complexVoltagesData;
  delete [] bandPassCorrectedData;
  delete [] weightsData;
  
  return 0;
}



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

#include <Common/Exception.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>

using namespace std;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using LOFAR::Exception;
#define NR_BASELINES     (NR_STATIONS * (NR_STATIONS + 1) / 2)

#define checkCuCall(func)                                               \
  do {                                                                  \
    CUresult result = func;                                             \
    if (result != CUDA_SUCCESS) {                                       \
      THROW (LOFAR::Cobalt::gpu::CUDAException,                         \
             # func << ": " << LOFAR::Cobalt::gpu::errorMessage(result)); \
    }                                                                   \
  } while(0)

// Helper function to get initialized memory
HostMemory getInitializedArray(gpu::Context &ctx, unsigned size, float defaultValue)
{
  HostMemory memory(ctx, size);
  float* createdArray =  memory.get<float>();
  for (unsigned idx = 0; idx < size; ++idx)
    createdArray[idx] = (float)defaultValue;
  return memory;
}

// 
HostMemory runTest(gpu::Context ctx,
                     Stream cuStream,
                     float * inputData)
{
  string kernelPath = "Correlator.cu";  //The test copies the kernel to the current dir (also the complex header, needed for compilation)
 
  // Get an instantiation of the default parameters
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();
  CudaRuntimeCompiler::flags_type flags = CudaRuntimeCompiler::defaultFlags();

  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NR_STATIONS"] = "2";
  unsigned NR_STATIONS = 2;
  definitions["NR_CHANNELS"] = "16";
  unsigned NR_CHANNELS = 16;
  definitions["NR_SAMPLES_PER_CHANNEL"] = "64";
  unsigned NR_SAMPLES_PER_CHANNEL = 64;
  definitions["NR_POLARIZATIONS"] = "2";
  unsigned NR_POLARIZATIONS = 2;
  definitions["COMPLEX"] = "2";
  unsigned COMPLEX = 2;

  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(devices, kernelPath, flags, definitions);
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  Function  hKernel(module, "correlate");  // c function this no argument overloading

  // *************************************************************
  // Create the data arrays  
  size_t sizeCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX * sizeof(float);
  DeviceMemory DevCorrectedMemory(ctx, sizeCorrectedData);
  HostMemory rawCorrectedData = getInitializedArray(ctx, sizeCorrectedData, 0.0);
  

  size_t sizeVisibilitiesData = NR_BASELINES * NR_CHANNELS * NR_POLARIZATIONS * NR_POLARIZATIONS * COMPLEX * sizeof(float);
  DeviceMemory DevVisibilitiesMemory(ctx, sizeVisibilitiesData);
  HostMemory rawVisibilitiesData = getInitializedArray(ctx, sizeVisibilitiesData, 42.0); 
  cuStream.writeBuffer(DevVisibilitiesMemory, rawVisibilitiesData);

  //copy the input received as argument to the input array
  float *rawDataPtr = rawCorrectedData.get<float>();
  for (unsigned idx = 0; idx < sizeCorrectedData; ++idx)
    rawDataPtr[idx] = inputData[idx];
  cuStream.writeBuffer(DevCorrectedMemory, rawCorrectedData);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, DevVisibilitiesMemory);
  hKernel.setArg(1, DevCorrectedMemory);
  
  // Calculate the number of threads in total and per blovk
  unsigned nrBlocks = NR_BASELINES;
  unsigned nrPasses = (nrBlocks + 1024 - 1) / 1024;
  unsigned nrThreads = (nrBlocks + nrPasses - 1) / nrPasses;
  unsigned nrUsableChannels = 15;
  Grid globalWorkSize(nrPasses, nrUsableChannels, 1);  
  Block localWorkSize(nrThreads, 1,1); 

  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished
  
  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawVisibilitiesData, DevVisibilitiesMemory);
  cuStream.synchronize(); //assure copy from device is done

  return rawVisibilitiesData;
}

Exception::TerminateHandler t(Exception::terminate);

int main()
{
  // seed random generator with zero
  srand (0);

  // Create a default context
  gpu::Platform pf;
  gpu::Device device(0);
  gpu::Context ctx(device);
  Stream cuStream(ctx);

  // Define type paramters
  unsigned NR_STATIONS = 2;
  unsigned NR_CHANNELS = 16;
  unsigned NR_SAMPLES_PER_CHANNEL = 64;
  unsigned NR_POLARIZATIONS = 2;
  unsigned COMPLEX = 2;

  // Define dependend paramters
  unsigned lengthInputData = NR_STATIONS * NR_CHANNELS * 
                             NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS *COMPLEX;
  unsigned lengthOutputData = NR_BASELINES * NR_CHANNELS * NR_POLARIZATIONS * NR_POLARIZATIONS * COMPLEX;

  // Create data members
  float * inputData = new float[lengthInputData];
  float * outputData = new float[lengthOutputData];
  float *outputOnHostPtr;
    
  // ***********************************************************
  // Baseline test: If all input data is zero the output should be zero
  // The output array is initialized with 42s
  for (unsigned idx = 0; idx < lengthInputData ; ++idx)
    inputData[idx] = 0;

  HostMemory outputOnHost = runTest(ctx, cuStream, inputData);
   
  // Copy the output data to a local array
  outputOnHostPtr = outputOnHost.get<float>();
  for (unsigned idx = 0; idx < lengthOutputData ; ++ idx)
    outputData[idx] = outputOnHostPtr[idx];

  // Now validate the outputdata
  for (unsigned idx_baseline = 0; idx_baseline < NR_BASELINES ; ++ idx_baseline)
  {
    cerr << "baseline: " << idx_baseline << endl;
    for (unsigned idx_channels = 0; idx_channels < NR_CHANNELS ; ++ idx_channels)
    {
      cerr << idx_channels << " : ";
      for (unsigned idx = 0; idx < 8; ++idx)
      {
        unsigned idx_in_output_data = idx_baseline * NR_CHANNELS * NR_POLARIZATIONS * NR_POLARIZATIONS * COMPLEX +
          idx_channels * NR_POLARIZATIONS * NR_POLARIZATIONS * COMPLEX +
          idx;
        cerr << outputData[idx_in_output_data] << ", " ;
        if ( idx_channels != 0)
          if (outputData[idx_in_output_data] != 0)
          {
            cerr << "Non zero number encountered while all input data was zero. Exit -1";
            delete [] inputData;
            delete [] outputData;
            return -1;
          }
      }
      cerr << endl;  
    }
  }

  // ***********************************************************
  // To test if the kernel is working we try to correlate a number of delayed
  // random initialize channels.
  // With zero delay the output should be the highest. A fringe :D 
  
  // 1. First create a random channel with a length that is large enough
  // It should be length NR_SAMPLES_PER_CHANNEL plus padding at both side to encompass the delay
  unsigned padding = 7; // We have 15 channels with content 2 * 7 delays + delay 0
  unsigned lengthRandomData = NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX + 2 * padding*4;
  float * randomInputData = new float[lengthRandomData];

  // Create the random signal
  for (unsigned idx = 0; idx < lengthRandomData ; ++idx)
    randomInputData[idx] = ((rand() % 1024  + 1) * 1.0 // make float
                            - 512.0) / 512.0;          // centre around zero and normalize
  
  // Fill the input array channels
  // channel 8 is the non delayed channel and should correlate
  for (unsigned idx_station = 0; idx_station < 2; ++ idx_station)
  {
    cerr << "idx_station: " << idx_station << endl;
    for (unsigned idx_channel = 0; idx_channel < 16; ++ idx_channel)
    {
      for (unsigned idx_datapoint = 0; idx_datapoint< NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX ; ++ idx_datapoint)
      {
        // In the input array step trough the channel
        unsigned idx_inputdata = 
            idx_station * 16 * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX +
            idx_channel * NR_SAMPLES_PER_CHANNEL *  NR_POLARIZATIONS * COMPLEX +
            idx_datapoint;
       
        // Pick from the random array the same number of samples
        // But with an offset depending on the channel number
        unsigned padding_offset = 32; 
        int padding = padding_offset - idx_channel * 4; // station 1 is filled with delayed signal

        // Station zero is anchored as the middle and is filled with the baseline signal
        if (idx_station == 0)
          padding = padding_offset - 8 * 4;

        // Get the index in the random signal array
        unsigned idx_randomdata = padding_offset + padding + idx_datapoint;

        //assign the signal;
        inputData[idx_inputdata] = randomInputData[idx_randomdata];
        if (idx_datapoint < 16)  // plot first part if the input signal for debugging purpose
          cerr << inputData[idx_inputdata] << " : ";
      }
      cerr << endl;
    }
  }

  // Run the kernel  
  outputOnHost = runTest(ctx, cuStream, inputData);
  
  // Copy the output data to a local array
  outputOnHostPtr = outputOnHost.get<float>();
  for (unsigned idx = 0; idx < lengthOutputData ; ++ idx)
    outputData[idx] = outputOnHostPtr[idx];

  // Target value for correlation channel  
  float targetValues[8] = {36.2332, 0, -7.83033, 3.32368, -7.83033, -3.32368, 42.246, 0};

  // print the contents of the output array for debugging purpose
  for (unsigned idx_baseline = 0; idx_baseline < NR_BASELINES ; ++ idx_baseline)
  {
    cerr << "baseline: " << idx_baseline << endl;
    for (unsigned idx_channels = 0; idx_channels < NR_CHANNELS ; ++ idx_channels)
    {
      cerr << idx_channels << " : ";
      for (unsigned idx = 0; idx < 8; ++idx)
      {
        unsigned idx_in_output_data = idx_baseline * NR_CHANNELS * 8 +
          idx_channels * 8 +
          idx;
        if ( idx_baseline == 1 && idx_channels == 8)
        {

          //validate that the correct value is found
          if ( abs(outputData[idx_in_output_data] - targetValues[idx]) > 0.0001)
          {
            cerr << "The correlated data found was not within an acceptable delta:" << endl
              << "Expected: " << outputData[idx_in_output_data] << endl
              << "Found: " << targetValues[idx] << endl
              << "Difference: " << outputData[idx_in_output_data] - targetValues[idx] 
              << "  Delta: " << 0.0001;

            delete [] inputData;
            delete [] outputData;
            return -1;
          }
        }
        cerr << outputData[idx_in_output_data] << ", " ;
      }
      cerr << endl;  
    }
  }
  // Validate that the channel 8 had fringes

  delete [] inputData;
  delete [] outputData;
  return 0;
}


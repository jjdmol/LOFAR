//# tTranspose.cc: test Transpose CUDA kernel
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

// The tests succeeds for different values of stations, channels, samples and tabs.

unsigned NR_CHANNELS = 16;
unsigned NR_SAMPLES_PER_CHANNEL = 32;
unsigned NR_TABS = 17;
unsigned NR_POLARIZATIONS = 2;
unsigned COMPLEX = 2;
bool CHANNEL_PARALLEL = false;

// Create the data arrays length
size_t lengthTransposedData =  NR_CHANNELS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_TABS * COMPLEX ;
size_t lengthComplexVoltagesData = NR_CHANNELS* NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS * COMPLEX;


void exit_with_print(float *complexVoltagesData, float *outputOnHostPtr)
{
  // Plot the output of the kernel in a readable manner
  // complex voltages
  for (unsigned idx_channel = 0; idx_channel < NR_CHANNELS; ++ idx_channel)
  {
    cout << endl << "idx_channel: " << idx_channel << endl;
    for(unsigned idx_samples = 0; idx_samples < NR_SAMPLES_PER_CHANNEL; ++idx_samples)
    {
      cout << "samples  " << idx_samples << ": ";
      for(unsigned idx_tabs = 0; idx_tabs < NR_TABS; ++ idx_tabs)
      {
        unsigned sample_base_idx = idx_channel * NR_SAMPLES_PER_CHANNEL * NR_TABS * 4 +
                              idx_samples * NR_TABS * 4 + 
                              idx_tabs * 4;

        cout << "<" << idx_tabs <<">" 
             << "(" << complexVoltagesData[sample_base_idx] << ", "
             << complexVoltagesData[sample_base_idx + 1] << ")-("
             << complexVoltagesData[sample_base_idx + 2] << ", " 
             << complexVoltagesData[sample_base_idx + 3] <<") ";
      }
      cout << endl;
    }
  }
  cout << endl << endl;
  // The the kernel functions output different arrays:
  // This if statement selects the correct print method
  if (CHANNEL_PARALLEL == false) 
  {
    cout << "not CHANNEL_PARALLEL" << endl;
    for(unsigned idx_tabs = 0; idx_tabs < NR_TABS; ++ idx_tabs)
    {
      cout << endl << "idx_tabs: " << idx_tabs << endl;
      for(unsigned idx_pol = 0; idx_pol < NR_POLARIZATIONS; ++idx_pol)
      {
        cout << "pol  " << idx_pol << ": ";
      
        for (unsigned idx_channel = 0; idx_channel < NR_CHANNELS; ++ idx_channel)
        {
          unsigned sample_base_idx = idx_tabs *  NR_POLARIZATIONS *  NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * COMPLEX +
                                idx_pol * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * COMPLEX + 
                                idx_channel * NR_SAMPLES_PER_CHANNEL * COMPLEX;
            cout << "<" << idx_channel << ">";
            for(unsigned idx_samples = 0; idx_samples < NR_SAMPLES_PER_CHANNEL; ++idx_samples)
            {
               cout << "(" << outputOnHostPtr[sample_base_idx + idx_samples * COMPLEX ] << ", "
                    << outputOnHostPtr[sample_base_idx + idx_samples * COMPLEX + 1] << ")" ;

            }
            cout << endl;
        }
        cout << endl;
      }
    }
  }
  else
  {
    cout << "CHANNEL_PARALLEL" << endl;
    for(unsigned idx_tabs = 0; idx_tabs < NR_TABS; ++ idx_tabs)
    {
      cout << endl << "idx_tabs: " << idx_tabs << endl;
      for(unsigned idx_pol = 0; idx_pol < NR_POLARIZATIONS; ++idx_pol)
      {
        cout << "pol " << idx_pol << ": ";
        for(unsigned idx_samples = 0; idx_samples < NR_SAMPLES_PER_CHANNEL; ++idx_samples)             
        {
          unsigned sample_base_idx = idx_tabs *  NR_POLARIZATIONS *  NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * COMPLEX +
                                idx_pol * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * COMPLEX + 
                                idx_samples * NR_CHANNELS * COMPLEX;
            cout << "<" << idx_samples << ">";
            for (unsigned idx_channel = 0; idx_channel < NR_CHANNELS; ++ idx_channel)
            {
               cout << "(" << outputOnHostPtr[sample_base_idx + idx_channel * COMPLEX ] << ", "
                    << outputOnHostPtr[sample_base_idx + idx_channel * COMPLEX + 1] << ")" ;
            }
        }
        cout << endl;
      }
    }
  }

  exit(1);
}


HostMemory runTest(gpu::Context ctx,
                   Stream cuStream,
                   float * transposedData,
                   float * complexVoltagesData,
                   string function)
{
  string kernelFile = "Transpose.cu";

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

  //Switch between the two function versions
  if (!CHANNEL_PARALLEL)
    definitions["POST_BEAMFORMER_KERNEL_TRANSPOSE"] = lexical_cast<string>("");

  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(devices, kernelFile, flags, definitions);
  gpu::Module module(createModule(ctx, kernelFile, ptx));
  Function hKernel(module, function);   // c function this no argument overloading

  // *************************************************************
  // Create the data arrays
  size_t sizeTransposedData = lengthTransposedData* sizeof(float);
  DeviceMemory devTransposedMemory(ctx, sizeTransposedData);
  HostMemory rawTransposedData = getInitializedArray(ctx, sizeTransposedData, 1.0f);
  float *rawTransposedPtr = rawTransposedData.get<float>();
  for (unsigned idx = 0; idx < lengthTransposedData; ++idx)
    rawTransposedPtr[idx] = transposedData[idx];
  cuStream.writeBuffer(devTransposedMemory, rawTransposedData);

  size_t sizeComplexVoltagesData= lengthComplexVoltagesData * sizeof(float);
  DeviceMemory devComplexVoltagesMemory(ctx, sizeComplexVoltagesData);
  HostMemory rawComplexVoltagesData = getInitializedArray(ctx, sizeComplexVoltagesData, 2.0f);
  float *rawComplexVoltagesPtr = rawComplexVoltagesData.get<float>();
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
    rawComplexVoltagesPtr[idx] = complexVoltagesData[idx];
  cuStream.writeBuffer(devComplexVoltagesMemory, rawComplexVoltagesData);


  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, devTransposedMemory);
  hKernel.setArg(1, devComplexVoltagesMemory);

  // Calculate the number of threads in total and per block
  // This depend on the parallization chosen: Of time then 
  //Grid globalWorkSize(NR_SAMPLES_PER_CHANNEL / 16, 1, 1);
  //Block localWorkSize(16, NR_TABS, NR_CHANNELS);
   //globalWorkSize = cl::NDRange(256, (ps.nrTABs(0) + 15) / 16, ps.nrSamplesPerChannel() / 16);
   //localWorkSize = cl::NDRange(256, 1, 1);
  Grid globalWorkSize(1, (NR_TABS + 15) / 16, NR_SAMPLES_PER_CHANNEL / 16);
  //Grid globalWorkSize(1, 1, 2);
  Block localWorkSize(256, 1, 1); // increasing x dim does not change anything
  // Run the kernel
  cuStream.synchronize(); // assure memory is copied
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);
  cuStream.synchronize(); // assure that the kernel is finished

  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawTransposedData, devTransposedMemory);
  cuStream.synchronize(); //assure copy from device is done

  return rawTransposedData; 
}

Exception::TerminateHandler t(Exception::terminate);


int main()
{
  INIT_LOGGER("tTranspose");
  const char* function = "transpose";
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
  float * transposedData= new float[lengthTransposedData];
  float * outputOnHostPtr;

  // ***********************************************************
  // Baseline test: If all weight data is zero the output should be zero
  // The output array is initialized with 42s
  cout << "test 1" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 0 ;
    complexVoltagesData[idx * 2 + 1] = 0;
  }

  for (unsigned idx = 0; idx < lengthTransposedData/2; ++idx)
  {
    transposedData[idx * 2] = 42.0f;
    transposedData[idx * 2 + 1] = 42.0f;
  }

  HostMemory outputOnHost = runTest(ctx, cuStream, transposedData,
    complexVoltagesData, function);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost.get<float>();
  for (size_t idx = 0; idx < lengthTransposedData; ++idx)
    if (outputOnHostPtr[idx] != 0.0f)
    {
      cerr << "The data returned by the kernel should be all zero: All Transposed are zero";
      exit_with_print(complexVoltagesData, outputOnHostPtr);
    }
    
  // ***********************************************************
  // Baseline test 2: perform the stanspose
  cout << "test 2" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = idx *2 * 1.0f;
    complexVoltagesData[idx * 2 + 1] = (idx * 2 + 1) * 1.0f;
  }

  for (unsigned idx = 0; idx < lengthTransposedData/2; ++idx)
  {
    transposedData[idx * 2] = 42.0f;
    transposedData[idx * 2 + 1] = 42.0f;
  }

  HostMemory outputOnHost2 = runTest(ctx, cuStream, transposedData,
    complexVoltagesData, function);

  // Validate the returned data array
  // Walk the data arrays and create linear data arrays with the correct content 
  outputOnHostPtr = outputOnHost2.get<float>();

//  exit_with_print(complexVoltagesData, outputOnHostPtr);
  //bool continue = true;
  //unsigned idx_in_data = 0;
  //for (size_t idx = 0; idx < NR_TABS * NR_POLARIZATIONS; ++idx)
  //{
  //  unsigned start_idx = idx * 2;

  //  for (size_t idy = 0; idy < lengthTransposedData / ; ++idy)
  //  {
  //    float idx_expected = 
  //  }
  //} 
  delete [] complexVoltagesData;
  delete [] transposedData;

  return 0;
}


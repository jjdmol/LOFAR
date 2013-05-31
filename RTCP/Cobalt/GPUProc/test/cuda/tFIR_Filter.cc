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

#include "tFIR_Filter.h"

#include <iostream>
#include <stdlib.h> 
#include <sstream>
#include <fstream>
#include <cuda.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;

int main()
{
  INIT_LOGGER("tFIR_Filter");
  char const *kernel_name = "FIR_Filter";
  const char *kernel_extention = ".cu";
  std::stringstream ss;
  ss << "nvcc " << kernel_name << kernel_extention
    << " -ptx"
    << " -DNR_STATIONS=" << NR_STATIONS
    << " -DNR_TAPS=" << NR_TAPS
    << " -DNR_SAMPLES_PER_CHANNEL=" << NR_SAMPLES_PER_CHANNEL
    << " -DNR_CHANNELS=" << NR_CHANNELS
    << " -DNR_POLARIZATIONS=" << NR_POLARIZATIONS
    << " -DCOMPLEX=" << COMPLEX
    << " -DNR_BITS_PER_SAMPLE=" << NR_BITS_PER_SAMPLE;
  std::string str = ss.str();

  //call system with the compiled string
  char const *CommandString= str.c_str();
  int return_value = system(  CommandString);
  std::cerr << "system call returned with status:"  << return_value << std::endl;


    CUresult cudaStatus;
  int cuda_device = 0;
  cudaError_t cuError;
  cudaDeviceProp deviceProp;

  cudaStatus = cuInit(0);
  if (cudaStatus != CUDA_SUCCESS) {
    std::cerr << " Failed intializion: " << cudaStatus << std::endl;
  }

  cuError = cudaSetDevice(0);
  if (cuError != cudaSuccess) {
    std::cerr << " Failed loading the device" << std::endl;
  }
  cuError =cudaGetDeviceProperties(&deviceProp, cuda_device);
  if (cuError != cudaSuccess) {
    std::cerr << " Failed loading cudaGetDeviceProperties" << std::endl;
  }

  std::cerr << "> Using CUDA device [" << cuda_device << " : " <<  deviceProp.name << std:: endl;


  // load the created module
  CUmodule     hModule  = 0;
  CUfunction   hKernel  = 0;

  std::fstream in("FIR_Filter.ptx", std::ios_base::in );
  std::stringstream sstr;
  sstr << in.rdbuf();
  cudaFree(0); // Hack to initialize the primary context. should use a proper api functions
  cudaStatus = cuModuleLoadDataEx(&hModule, sstr.str().c_str(), 0, 0, 0);
  if (cudaStatus != CUDA_SUCCESS) {
    std::cerr << " Failed loading the kernel module, status: " << cudaStatus <<std::endl;
  }



  // Get the entry point in the kernel
  cudaStatus = cuModuleGetFunction(&hKernel, hModule, "FIR_filter");
  if (cudaStatus != CUDA_SUCCESS)
  {
    std::cerr << " Failed loading the function entry point, status: " << cudaStatus <<std::endl;
  }

  cudaStream_t cuStream;
  cuError = cudaStreamCreate (&cuStream);
  if (cuError != cudaSuccess) {
    std::cerr << " Failed creating a stream: " << cuError << std::endl;
  }


  // Number of threads?
  int nrChannelsPerSubband = NR_CHANNELS;
  int nrStations = NR_STATIONS; 
  int MAXNRCUDATHREADS = 1024;//doet moet nog opgevraagt worden en niuet als magish getal
  size_t maxNrThreads = MAXNRCUDATHREADS;
  unsigned totalNrThreads = nrChannelsPerSubband * NR_POLARIZATIONS * 2; //ps.nrChannelsPerSubband()
  unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
  
  dim3 globalWorkSize(nrPasses, nrStations); 
  dim3 localWorkSize(totalNrThreads / nrPasses, 1); 




  cudaError_t cudaErrorStatus;
  bool testOk = true;
  //const size_t nrComplexComp = 2; // real, imag

  // Create the needed data
  unsigned sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
  float* rawFilteredData = new float[sizeFilteredData];
  for (unsigned idx = 0; idx < sizeFilteredData; ++idx)
  {
    rawFilteredData[idx] = 0;
  }

  FilteredDataType filteredData = reinterpret_cast<FilteredDataType>(rawFilteredData);

  unsigned sizeSampledData = NR_STATIONS * (NR_TAPS - 1 + NR_SAMPLES_PER_CHANNEL) * NR_CHANNELS * NR_POLARIZATIONS * COMPLEX;               
  SampleType * rawInputSamples = new SampleType[sizeSampledData];
  for (unsigned idx = 0; idx < sizeSampledData; ++idx)
  {
    rawInputSamples[idx] = 0;
  }
  SampledDataType inputSamples = reinterpret_cast<SampledDataType>(rawInputSamples);



  unsigned sizeWeightsData = NR_CHANNELS * 16;
  float * rawFirWeights = new float[sizeWeightsData];
  for (unsigned idx = 0; idx < sizeWeightsData; ++idx)
  {
    rawFirWeights[idx] = 0;
  }
  WeightsType firWeights = reinterpret_cast<WeightsType>(rawFirWeights);

  // Data on the gpu
  CUdeviceptr DevFilteredData = (CUdeviceptr)NULL;;
  CUdeviceptr DevSampledData = (CUdeviceptr)NULL;;
  CUdeviceptr DevFirWeights = (CUdeviceptr)NULL;;

  // CUdeviceptr d_data = (CUdeviceptr)NULL;


  // Allocate GPU buffers for three vectors (two input, one output)    .
  cudaStatus =   cuMemAlloc(&DevFilteredData, sizeFilteredData * sizeof(float));
  if (cudaStatus != CUDA_SUCCESS) {
    std::cerr << "memory allocation failed: " << cudaStatus << std::endl;
    throw "cudaMalloc failed!";
  }

  cudaStatus = cuMemAlloc(&DevSampledData, sizeSampledData * sizeof(SampleType));
  if (cudaStatus != CUDA_SUCCESS) {
    std::cerr << "memory allocation failed: " << cudaStatus << std::endl;
    throw "cudaMalloc failed!";
  }

  cudaStatus = cuMemAlloc(&DevFirWeights, sizeWeightsData * sizeof(float));
  if (cudaStatus != CUDA_SUCCESS) {
    std::cerr << "memory allocation failed: " << cudaStatus << std::endl;
    throw "cudaMalloc failed!";
  }    

  unsigned station, sample, ch, pol;

  // Test 1: Single impulse test on single non-zero weight
  station = ch = pol = 0;
  sample = NR_TAPS - 1; // skip FIR init samples
  (*firWeights)[0][0] = 2.0f;
  (*inputSamples)[station][sample][ch][pol] = 3;


  // Copy input vectors from host memory to GPU buffers.
  cudaStatus = cuMemcpyHtoD(DevFirWeights, rawFirWeights,
    sizeWeightsData * sizeof(float));
  if (cudaStatus != CUDA_SUCCESS) {
    fprintf(stderr, "cudaMemcpy failed!");
    throw "cudaMemcpy failed!";
  }

  cudaStatus = cuMemcpyHtoD(DevSampledData, rawInputSamples,
    sizeSampledData * sizeof(SampleType));
  if (cudaStatus != CUDA_SUCCESS) {
    fprintf(stderr, "cudaMemcpy failed!");
    throw "cudaMemcpy failed!";
  }

  cudaStatus = cuMemcpyHtoD(DevFilteredData, rawFilteredData,
    sizeFilteredData * sizeof(float));
  if (cudaStatus != CUDA_SUCCESS) {
    fprintf(stderr, "cudaMemcpy failed!");
    throw "cudaMemcpy failed!";
  }


  cudaErrorStatus = cudaDeviceSynchronize();
  if (cudaErrorStatus != cudaSuccess) {
    fprintf(stderr, "cudaDeviceSynchronize returned error code %d after sync of memory!\n", cudaErrorStatus);

  }


  void* kernel_func_args[3] = { &DevFilteredData,
                                &DevSampledData,
                                &DevFirWeights };

  // unsigned  sharedMemBytes = 512;

  cudaStatus = cuLaunchKernel( hKernel, globalWorkSize.x, globalWorkSize.y, globalWorkSize.z, 
    localWorkSize.x, localWorkSize.y, localWorkSize.z, 0, cuStream, kernel_func_args, NULL);
  if (cudaStatus != CUDA_SUCCESS)
  {
    std::cerr << " cuLaunchKernel " << cudaStatus <<std::endl;
  }

  cudaErrorStatus = cudaDeviceSynchronize();
  if (cudaErrorStatus != cudaSuccess) {
    fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching Kernel!\n", cudaErrorStatus);
    throw "cudaDeviceSynchronize returned error code after launching Kernel!\n";
  }

  // Copy output vector from GPU buffer to host memory.
  cudaStatus = cuMemcpyDtoH(filteredData, DevFilteredData,
    sizeFilteredData * sizeof(float));
  if (cudaStatus != CUDA_SUCCESS) {
    fprintf(stderr, "cudaMemcpy failed!");
    throw "cudaMemcpy failed!";
  }
  
  cudaErrorStatus = cudaDeviceSynchronize();
  // Expected output: St0, pol0, ch0, sampl0: 6. The rest all 0.
  if((*filteredData)[0][0][0][0][0] != 6.0f) 
  {
    std::cerr << "FIR_FilterTest 1: Expected at idx 0: 6; got: " << (*filteredData)[0][0][0][0][0] << std::endl;

    testOk = false;
  }
  std::cerr << "Weights returned " << (*filteredData)[0][0][0][0][0] << std::endl;

  const unsigned nrExpectedZeros = sizeFilteredData - 1;
  unsigned nrZeros = 0;
  for (unsigned i = 1; i < sizeFilteredData; i++) 
  {
    if (rawFilteredData[i] == 0.0f) 
    { 
      nrZeros += 1;
    }
  }
  if (nrZeros != nrExpectedZeros) 
  {
    std::cerr << "FIR_FilterTest 1: Unexpected non-zero(s). Only " << nrZeros << " zeros out of " << nrExpectedZeros << std::endl;
    testOk = false;
  }
  if ( !testOk)
    return -1;

  return 0;
}


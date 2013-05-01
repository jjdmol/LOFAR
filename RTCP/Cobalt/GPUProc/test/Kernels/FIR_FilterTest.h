//# FIR_FilterTest.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef GPUPROC_FIR_FILTERTEST_H
#define GPUPROC_FIR_FILTERTEST_H

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#define COMPLEX 2       // do not change

#define NR_BITS_PER_SAMPLE 8
#define NR_STATION_FILTER_TAPS  16
#define USE_NEW_CORRELATOR
#define NR_POLARIZATIONS         2 
#define NR_TAPS                 16

#define NR_STATIONS 20
#define NR_SAMPLES_PER_CHANNEL 100
#define NR_CHANNELS 64

#if NR_BITS_PER_SAMPLE == 16
typedef signed short SampleType;
#elif NR_BITS_PER_SAMPLE == 8
typedef signed char SampleType;
#else
#error unsupported NR_BITS_PER_SAMPLE
#endif

#include "lofar_config.h"

#include <iostream>
#include <string>
#include <sstream>

#include <iostream>
#include <stdlib.h> 
#include "FIR_FilterTest.h"
#include <sstream>
#include <fstream>
#include <cuda_runtime.h>
#include <cuda.h>
#include <exception>

extern cudaError_t FIR_filter_wrapper(float *DevFilteredData,
  float const *DevSampledData,
  float const *DevWeightsData);
namespace LOFAR
{
  namespace Cobalt
  {
    typedef SampleType (*SampledDataType)[NR_STATIONS][NR_TAPS - 1 + NR_SAMPLES_PER_CHANNEL][NR_CHANNELS][NR_POLARIZATIONS * COMPLEX];
    typedef float (*FilteredDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS][COMPLEX];
    typedef float (*WeightsType)[NR_CHANNELS][16];

    struct FIR_FilterTest 
    {
      void check(bool first, bool second)
      {
        if (first != second)
          throw "comparison failed";
      }

      FIR_FilterTest()
      {
        switch (NR_BITS_PER_SAMPLE) 
        {
        case 4: // TODO: move this case before UnitTest(...), which already fails to compile the kernel
          std::cerr << "4 bit mode not yet supported in this test" << std::endl;
          check(false, true);
          break;
        case 8:
          firTests<signed char>();
          break;
        case 16:
          firTests<short>();
          break;
        default:
          std::cerr << "unrecognized number of bits per sample type" << std::endl;
          check(false, true);
        }
      }

      template <typename SampleT>
      void firTests()
      {
        CUresult cudaStatus;
        int cuda_device = 0;
        cudaError_t cuError;
        cudaDeviceProp deviceProp;

        cudaStatus = cuInit(0);
        if (cudaStatus != CUDA_SUCCESS) {
          std::cerr << " Failed intializion" << std::endl;
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

        const char *kernel_name = "FIR_Filter";
        const char *kernel_extension = ".cu";
        std::stringstream ss;
        ss << "nvcc " << kernel_name << kernel_extension
          << " -ptx"
          << " -DNR_STATIONS=" << 10 
          << " -DNR_TAPS=" << 16
          << " -DNR_SAMPLES_PER_CHANNEL=" << 100 
          << " -DNR_CHANNELS=" << 20
          << " -DNR_POLARIZATIONS=" << 2
          << " -DCOMPLEX=" << 2
          << " -DNR_BITS_PER_SAMPLE=" << 16;
        std::string str = ss.str();

        //call system with the compiled string
        char const *CommandString= str.c_str();
        int return_value = system(  CommandString);
        std::cerr << "system call returned with status:"  << return_value << std::endl;

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
        unsigned totalNrThreads = nrChannelsPerSubband * NR_POLARIZATIONS * 2; //ps.nrChannelsPerSubband()
        dim3 globalWorkSize(totalNrThreads, nrStations); //ps.nrStations()

        int MAXNRCUDATHREADS = 512;
        size_t maxNrThreads = MAXNRCUDATHREADS;
        unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
        dim3 localWorkSize(totalNrThreads / nrPasses, 1); 




        cudaError_t cudaErrorStatus;
        bool testOk = true;
        //const size_t nrComplexComp = 2; // real, imag

        // Create the needed data
        unsigned sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
        float* rawFilteredData = new float[sizeFilteredData];
        for (unsigned idx = 0; idx < sizeFilteredData; ++idx)
        {
          rawFilteredData[idx] = 0.0;
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
          rawFirWeights[idx] = 0.0;
        }
        WeightsType firWeights = reinterpret_cast<WeightsType>(rawFirWeights);

        // Data on the gpu
        float *DevFilteredData;
        float *DevSampledData;
        float *DevFirWeights;

        return;
        // Allocate GPU buffers for three vectors (two input, one output)    .
        cudaErrorStatus = cudaMalloc((void**)&DevFilteredData, sizeFilteredData * sizeof(float));
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMalloc failed!");
          throw "cudaMalloc failed!";
        }

        cudaErrorStatus = cudaMalloc((void**)&DevSampledData, sizeSampledData * sizeof(SampleType));
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMalloc failed!");
          throw "cudaMalloc failed!";
        }

        cudaErrorStatus = cudaMalloc((void**)&DevFirWeights, sizeWeightsData * sizeof(float));
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMalloc failed!");
          throw "cudaMalloc failed!";
        }    


        //FIR_FilterKernel firFilterKernel(ps, queue, program, filteredData, inputSamples, firWeights);

        unsigned station, sample, ch, pol;

        // Test 1: Single impulse test on single non-zero weight
        station = ch = pol = 0;
        sample = NR_TAPS - 1; // skip FIR init samples
        (*firWeights)[0][0] = 2.0f;
        (*inputSamples)[station][sample][ch][pol] = 3;

        // Copy input vectors from host memory to GPU buffers.
        cudaErrorStatus = cudaMemcpy(DevFirWeights, rawFirWeights,
          sizeWeightsData * sizeof(float), cudaMemcpyHostToDevice);
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMemcpy failed!");
          throw "cudaMemcpy failed!";
        }

        cudaErrorStatus = cudaMemcpy(DevSampledData, rawInputSamples,
          sizeSampledData * sizeof(SampleType), cudaMemcpyHostToDevice);
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMemcpy failed!");
          throw "cudaMemcpy failed!";
        }

        cudaErrorStatus = cudaMemcpy(DevFilteredData, rawFilteredData,
          sizeFilteredData * sizeof(float), cudaMemcpyHostToDevice);
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMemcpy failed!");
          throw "cudaMemcpy failed!";
        }

        void* kernel_func_args[] = { (void*)&DevFilteredData,
                                     (void*)&DevSampledData,
                                     (void*)&DevFirWeights };

  

    unsigned  sharedMemBytes = 64;
    cudaStatus = cuLaunchKernel( hKernel, globalWorkSize.x, globalWorkSize.y, globalWorkSize.z, 
      localWorkSize.x, localWorkSize.y, localWorkSize.z, sharedMemBytes, cuStream, kernel_func_args,0);
    if (cudaStatus != CUDA_SUCCESS)
  {
    std::cerr << " cuLaunchKernel " << cudaStatus <<std::endl;
    }

        //// Launch a kernel on the GPU with one thread for each element.
     //   ::FIR_filter_wrapper(DevFilteredData, DevSampledData, DevFirWeights);

        // cudaDeviceSynchronize waits for the kernel to finish, and returns
        // any errors encountered during the launch.
        cudaErrorStatus = cudaDeviceSynchronize();
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching Kernel!\n", cudaErrorStatus);
          throw "cudaDeviceSynchronize returned error code after launching Kernel!\n";
        }

        // Copy output vector from GPU buffer to host memory.
        cudaErrorStatus = cudaMemcpy(filteredData, DevFilteredData,
          sizeFilteredData * sizeof(float), cudaMemcpyDeviceToHost);
        if (cudaErrorStatus != cudaSuccess) {
          fprintf(stderr, "cudaMemcpy failed!");
          throw "cudaMemcpy failed!";
        }

        // Expected output: St0, pol0, ch0, sampl0: 6. The rest all 0.
        if((*filteredData)[0][0][0][0][0] != 6.0f) 
        {
          // int maxSample = NR_SAMPLES_PER_CHANNEL;
          std::cerr << "FIR_FilterTest 1: Expected at idx 0: 6; got: " << (*filteredData)[0][0][0][0][0] << std::endl;
          testOk = false;
        }

        const unsigned nrExpectedZeros = sizeFilteredData - 1;
        unsigned nrZeros = 0;
        for (unsigned i = 1; i < sizeFilteredData; i++) 
        {
          if (rawFilteredData[i] == 0.0f) 
          { 
            nrZeros += 1;
          }
        }
        if (nrZeros == nrExpectedZeros) 
        {
          std::cout << "FIR_FilterTest 1: test OK" << std::endl;
        } 
        else 
        {
          std::cerr << "FIR_FilterTest 1: Unexpected non-zero(s). Only " << nrZeros << " zeros out of " << nrExpectedZeros << std::endl;
          testOk = false;
        }


        //// Test 2: Impulse train 2*NR_TAPS apart. All st, all ch, all pol.
        //for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //  for (unsigned tap = 0; tap < ps.nrPPFTaps(); tap++) {
        //    firWeights[ch][tap] = ch + tap;
        //  }
        //}

        //for (station = 0; station < ps.nrStations(); station++) {
        //  for (sample = ps.nrPPFTaps() - 1; sample < ps.nrPPFTaps() - 1 + ps.nrSamplesPerChannel(); sample += 2 * ps.nrPPFTaps()) {
        //    for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //      for (pol = 0; pol < NR_POLARIZATIONS; pol++) {
        //        inputSamples[station][sample][ch][pol][0] = station;
        //      }
        //    }
        //  }
        //}

        //firWeights.hostToDevice(CL_FALSE);
        //inputSamples.hostToDevice(CL_FALSE);
        //firFilterKernel.enqueue(queue, counter);
        //filteredData.deviceToHost(CL_TRUE);

        //// Expected output: sequences of (filterbank scaled by station nr, NR_TAPS zeros)
        //unsigned nrErrors = 0;
        //for (station = 0; station < ps.nrStations(); station++) {
        //  for (pol = 0; pol < NR_POLARIZATIONS; pol++) {
        //    unsigned s;
        //    for (sample = 0; sample < ps.nrSamplesPerChannel() / (2 * ps.nrPPFTaps()); sample += s) {
        //      for (s = 0; s < ps.nrPPFTaps(); s++) {
        //        for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //          if (filteredData[station][pol][sample + s][ch][0] != station * firWeights[ch][s]) {
        //            if (++nrErrors < 100) { // limit spam
        //              std::cerr << "2a.filtered[" << station << "][" << pol << "][" << sample + s << "][" << ch <<
        //                "][0] (sample=" << sample << " s=" << s << ") = " << filteredData[station][pol][sample + s][ch][0] << std::endl;
        //            }
        //          }
        //          if (filteredData[station][pol][sample + s][ch][1] != 0.0f) {
        //            if (++nrErrors < 100) {
        //              std::cerr << "2a imag non-zero: " << filteredData[station][pol][sample + s][ch][1] << std::endl;
        //            }
        //          }
        //        }
        //      }

        //      for (; s < 2 * ps.nrPPFTaps(); s++) {
        //        for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //          if (filteredData[station][pol][sample + s][ch][0] != 0.0f || filteredData[station][pol][sample + s][ch][1] != 0.0f) {
        //            if (++nrErrors < 100) {
        //              std::cerr << "2b.filtered[" << station << "][" << pol << "][" << sample + s << "][" << ch <<
        //                "][0] (sample=" << sample << " s=" << s << ") = " << filteredData[station][pol][sample + s][ch][0] <<
        //                ", " << filteredData[station][pol][sample + s][ch][1] << std::endl;
        //            }
        //          }
        //        }
        //      }
        //    }
        //  }
        //}
        //if (nrErrors == 0) {
        //  std::cout << "FIR_FilterTest 2: test OK" << std::endl;
        //} else {
        //  std::cerr << "FIR_FilterTest 2: " << nrErrors << " unexpected output values" << std::endl;
        //  testOk = false;
        //}


        //// Test 3: Scaled step test (scaled DC gain) on KAISER filterbank. Non-zero imag input.
        //FilterBank filterBank(true, ps.nrPPFTaps(), ps.nrChannelsPerSubband(), KAISER);
        //filterBank.negateWeights();         // not needed for testing, but as we use it
        ////filterBank.printWeights();

        //assert(firWeights.num_elements() == filterBank.getWeights().num_elements());
        //double* expectedSums = new double[ps.nrChannelsPerSubband()];
        //memset(expectedSums, 0, ps.nrChannelsPerSubband() * sizeof(double));
        //for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //  for (unsigned tap = 0; tap < ps.nrPPFTaps(); tap++) {
        //    firWeights[ch][tap] = filterBank.getWeights()[ch][tap];
        //    expectedSums[ch] += firWeights[ch][tap];
        //  }
        //}

        //for (station = 0; station < ps.nrStations(); station++) {
        //  for (sample = 0; sample < ps.nrPPFTaps() - 1 + ps.nrSamplesPerChannel(); sample++) {
        //    for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //      for (pol = 0; pol < NR_POLARIZATIONS; pol++) {
        //        inputSamples[station][sample][ch][pol][0] = 2; // real
        //        inputSamples[station][sample][ch][pol][1] = 3; // imag
        //      }
        //    }
        //  }
        //}

        //firWeights.hostToDevice(CL_FALSE);
        //inputSamples.hostToDevice(CL_FALSE);
        //firFilterKernel.enqueue(queue, counter);
        //filteredData.deviceToHost(CL_TRUE);

        //nrErrors = 0;
        //for (station = 0; station < ps.nrStations(); station++) {
        //  for (pol = 0; pol < NR_POLARIZATIONS; pol++) {
        //    for (sample = 0; sample < ps.nrSamplesPerChannel(); sample++) {
        //      for (ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
        //        // Expected sum must also be scaled by 2 and 3, because weights are real only.
        //        if (!fpEquals(filteredData[station][pol][sample][ch][0], 2 * expectedSums[ch])) {
        //          if (++nrErrors < 100) { // limit spam
        //            std::cerr << "3a.filtered[" << station << "][" << pol << "][" << sample << "][" << ch <<
        //              "][0] = " << filteredData[station][pol][sample][ch][0] << " 2*weight = " << 2 * expectedSums[ch] << std::endl;
        //          }
        //        }
        //        if (!fpEquals(filteredData[station][pol][sample][ch][1], 3 * expectedSums[ch])) {
        //          if (++nrErrors < 100) {
        //            std::cerr << "3b.filtered[" << station << "][" << pol << "][" << sample << "][" << ch <<
        //              "][1] = " << filteredData[station][pol][sample][ch][1] << " 3*weight = " << 3 * expectedSums[ch] << std::endl;
        //          }
        //        }
        //      }
        //    }
        //  }
        //}
        //delete[] expectedSums;
        //if (nrErrors == 0) {
        //  std::cout << "FIR_FilterTest 3: test OK" << std::endl;
        //} else {
        //  std::cerr << "FIR_FilterTest 3: " << nrErrors << " unexpected output values" << std::endl;
        //  testOk = false;
        //}


        //check(testOk, true);
      }
    };
  }
}

#endif


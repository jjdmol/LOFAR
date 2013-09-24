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

#include <cstdlib>
#include <cmath>
#include <complex>
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>

#include "../fpequals.h"
#include "../TestUtil.h"

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using LOFAR::Exception;

// The tests succeeds for different values of stations, channels, samples and tabs.
unsigned NR_STATIONS = 4;  
unsigned NR_CHANNELS = 4;
unsigned NR_SAMPLES_PER_CHANNEL = 4;
unsigned NR_SAPS = 2;
unsigned NR_TABS = 4;
unsigned NR_POLARIZATIONS = 2;


// Create the data arrays
size_t lengthDelaysData = NR_SAPS * NR_STATIONS * NR_TABS; // float
size_t lengthBandPassCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS; // complex<float>
size_t lengthComplexVoltagesData = NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS; // complex<float>


void exit_with_print(const complex<float> *outputOnHostPtr)
{
  // Plot the output of the kernel in a readable manner
  unsigned size_channel = NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS;
  unsigned size_sample = NR_TABS * NR_POLARIZATIONS;
  unsigned size_tab = NR_POLARIZATIONS;
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
        cout << outputOnHostPtr[index_data_base] <<          // X pol
                outputOnHostPtr[index_data_base + 1] << " "; // Y pol
      }
      cout << endl;
    }
    cout << endl;
  }
  
  std::exit(1);
}


HostMemory runTest(Context ctx,
                   Stream cuStream,
                   float * delaysData,
                   complex<float> * bandPassCorrectedData,
                   complex<float> * complexVoltagesData,
                   float subbandFrequency,
                   unsigned sap,
                   string function,
                   float weightCorrection,
                   double subbandBandwidth)
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
  definitions["NR_SAPS"] = lexical_cast<string>(NR_SAPS);
  definitions["NR_TABS"] = lexical_cast<string>(NR_TABS);
  definitions["NR_POLARIZATIONS"] = lexical_cast<string>(NR_POLARIZATIONS);
  definitions["WEIGHT_CORRECTION"] = str(boost::format("%.7ff") % weightCorrection);
  definitions["SUBBAND_BANDWIDTH"] = str(boost::format("%.7f") % subbandBandwidth);

  vector<Device> devices(1, ctx.getDevice());
  string ptx = createPTX(kernelFile, definitions, flags, devices);
  Module module(createModule(ctx, kernelFile, ptx));
  Function hKernel(module, function);   // c function this no argument overloading

  // *************************************************************
  // Create the data arrays
  size_t sizeDelaysData = lengthDelaysData * sizeof(float);
  DeviceMemory devDelaysMemory(ctx, sizeDelaysData);
  HostMemory rawDelaysData = getInitializedArray(ctx, sizeDelaysData, 1.0f);
  float *rawDelaysPtr = rawDelaysData.get<float>();
  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
    rawDelaysPtr[idx] = delaysData[idx];
  cuStream.writeBuffer(devDelaysMemory, rawDelaysData);

  size_t sizeBandPassCorrectedData = lengthBandPassCorrectedData * sizeof(complex<float>);
  DeviceMemory devBandPassCorrectedMemory(ctx, sizeBandPassCorrectedData);
  HostMemory rawBandPassCorrectedData = getInitializedArray(ctx, sizeBandPassCorrectedData, 2.0f);
  complex<float> *rawBandPassCorrectedPtr = rawBandPassCorrectedData.get<complex<float> >();
  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
    rawBandPassCorrectedPtr[idx] = bandPassCorrectedData[idx];
  cuStream.writeBuffer(devBandPassCorrectedMemory, rawBandPassCorrectedData);

  size_t sizeComplexVoltagesData = lengthComplexVoltagesData * sizeof(complex<float>);
  DeviceMemory devComplexVoltagesMemory(ctx, sizeComplexVoltagesData);
  HostMemory rawComplexVoltagesData = getInitializedArray(ctx, sizeComplexVoltagesData, 3.0f);
  complex<float> *rawComplexVoltagesPtr = rawComplexVoltagesData.get<complex<float> >();
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
    rawComplexVoltagesPtr[idx] = complexVoltagesData[idx];
  // Write output content.
  cuStream.writeBuffer(devComplexVoltagesMemory, rawComplexVoltagesData);

  // ****************************************************************************
  // Run the kernel on the created data
  hKernel.setArg(0, devComplexVoltagesMemory);
  hKernel.setArg(1, devBandPassCorrectedMemory);
  hKernel.setArg(2, devDelaysMemory);
  hKernel.setArg(3, subbandFrequency);
  hKernel.setArg(4, sap);

  // Calculate the number of threads in total and per block
  Grid globalWorkSize(1, 1, 1);
  Block localWorkSize(NR_POLARIZATIONS, NR_TABS, NR_CHANNELS);

  // Run the kernel
  cuStream.launchKernel(hKernel, globalWorkSize, localWorkSize);

  // Copy output vector from GPU buffer to host memory.
  cuStream.readBuffer(rawComplexVoltagesData, devComplexVoltagesMemory, true);

  return rawComplexVoltagesData; 
}

Exception::TerminateHandler t(Exception::terminate);


int main()
{
  INIT_LOGGER("tBeamFormer");
  const char function[] = "beamFormer";

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
  complex<float>* complexVoltagesData = new complex<float>[lengthComplexVoltagesData];
  complex<float>* bandPassCorrectedData = new complex<float>[lengthBandPassCorrectedData];
  float * delaysData= new float[lengthDelaysData];
  complex<float>* outputOnHostPtr;

  complex<float> refVal;

  float subbandFrequency = 1.5e8f; // Hz
  unsigned sap = 0;

  float weightCorrection = 1.0f;
  double subbandBandwidth = 200e3; // Hz

  // ***********************************************************
  // Baseline test: If all delays data is zero and input (1, 1) (and weightCorrection=1),
  // then the output must be all (NR_STATIONS, NR_STATIONS). (See below and/or kernel why.)
  // The output array is initialized with 42s
  cout << "test 1" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    complexVoltagesData[idx].real(42.0f);
    complexVoltagesData[idx].imag(42.0f);
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
  {
    bandPassCorrectedData[idx].real(1.0f);
    bandPassCorrectedData[idx].imag(1.0f);
  }

  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
  {
    delaysData[idx] = 0.0f;
  }

  HostMemory outputOnHost1 = runTest(ctx, cuStream, delaysData,
                                     bandPassCorrectedData,
                                     complexVoltagesData,
                                     subbandFrequency, sap,
                                     function,
                                     weightCorrection, subbandBandwidth);

  /*
   * Test 1 Reference output calculation:
   * The kernel calculates the beamformer weights per station from the delays (and the weightCorrection).
   * Since delays is 0, phi will be -M_PI * delay * channel_frequency = 0.
   * The complex weight is then (cos(phi), sin(phi)) * weightCorrection = (1, 0) * 1.
   * The samples (all (1, 1)) are then complex multiplied with the weights and summed for all (participating) stations.
   * The complex mul gives (1, 1). Summing NR_STATIONS of samples gives (NR_STATIONS, NR_STATIONS) for all samples.
   */
  refVal.real((float)NR_STATIONS);
  refVal.imag((float)NR_STATIONS);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost1.get<complex<float> >();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    if (!fpEquals(outputOnHostPtr[idx], refVal))
    {
      cerr << "The data returned by the kernel should be all (NR_STATIONS, NR_STATIONS): All input is (1, 1) and all delays are zero.";
      exit_with_print(outputOnHostPtr);
    }
  }

  // ***********************************************************
  // Baseline test 2: If all input data is zero the output should be zero while the delays are non zero
  // The output array is initialized with 42s
  cout << "test 2" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    complexVoltagesData[idx].real(42.0f);
    complexVoltagesData[idx].imag(42.0f);
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
  {
    bandPassCorrectedData[idx].real(0.0f);
    bandPassCorrectedData[idx].imag(0.0f);
  }

  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
  {
    delaysData[idx] = 1.0f;
  }

  HostMemory outputOnHost2 = runTest(ctx, cuStream, delaysData,
                                     bandPassCorrectedData,
                                     complexVoltagesData,
                                     subbandFrequency, sap,
                                     function,
                                     weightCorrection, subbandBandwidth);

  refVal.real(0.0f);
  refVal.imag(0.0f);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost2.get<complex<float> >();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    if (!fpEquals(outputOnHostPtr[idx], refVal))
    {
      cerr << "The data returned by the kernel should be all zero: All inputs are zero";
      exit_with_print(outputOnHostPtr);
    }


  // ***********************************************************
  // Test 3: all inputs 1 (including imag)
  // with only the delays set to 1e-6.
  // All outputs should have the val of (NR_STATIONS, NR_STATIONS).
  cout << "test 3" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    complexVoltagesData[idx].real(42.0f);
    complexVoltagesData[idx].imag(42.0f);
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
  {
    bandPassCorrectedData[idx].real(1.0f);
    bandPassCorrectedData[idx].imag(1.0f);
  }

  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
  {
    delaysData[idx] = 1.0e-6f;
  }

  HostMemory outputOnHost3 = runTest(ctx, cuStream, delaysData,
                                     bandPassCorrectedData,
                                     complexVoltagesData,
                                     subbandFrequency, sap,
                                     function,
                                     weightCorrection, subbandBandwidth);

  refVal.real((float)NR_STATIONS);
  refVal.imag((float)NR_STATIONS);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost3.get<complex<float> >();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    if (!fpEquals(outputOnHostPtr[idx], refVal))
    {
      cerr << "all the data returned by the kernel should be (" << NR_STATIONS << ", " << NR_STATIONS << ")" ;
      exit_with_print(outputOnHostPtr);
    }
    

#if 0
  // ***********************************************************
  // Test 4: all inputs 1 (including imag)
  // with only the real delay set to 1 and imag also 1
  // all outputs should be (0,8) 
  // (1 , i) * (1, i) == 1 * 1 + i * i + 1 * i + 1 * i= 1 -1 +2i = 2i
  // times 4 stations is (0,8)
  cout << "test 4" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    complexVoltagesData[idx].real(42.0f);
    complexVoltagesData[idx].imag(42.0f);
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
  {
    bandPassCorrectedData[idx].real(1.0f);
    bandPassCorrectedData[idx].imag(1.0f);
  }

  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
  {
    delaysData[idx] = 1.0f;
  }

  HostMemory outputOnHost4 = runTest(ctx, cuStream, delaysData,
                                     bandPassCorrectedData,
                                     complexVoltagesData,
                                     subbandFrequency, sap,
                                     function,
                                     weightCorrection, subbandBandwidth);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost4.get<complex<float> >();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    if (outputOnHostPtr[idx].real() != 0.0f)
    {
      cerr << "The REAL data returned by the kernel should be all zero: both input and delays are (1, i)";
      exit_with_print(outputOnHostPtr);
    }
    if (outputOnHostPtr[idx].imag() != NR_STATIONS * 2 * 1.0f)
    {
      cerr << "The imag data should be all " << NR_STATIONS * 2 * 1.0f;
      exit_with_print(outputOnHostPtr);
    } 
  }
    // ***********************************************************
  // Test 5: all inputs 1 (including imag)
  // with only the real delay set to 1.
  // The global delay correction is set to 2.01 so
  // all outputs should be 4 * 2.01 = 8.04
  cout << "test 5" << endl;
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
  {
    complexVoltagesData[idx].real(42.0f);
    complexVoltagesData[idx].imag(42.0f);
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx].real(1.0f);
    bandPassCorrectedData[idx].imag(1.0f);
  }

  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
  {
    delaysData[idx] = 1.0f;
  }

  HostMemory outputOnHost5 = runTest(ctx, cuStream, delaysData,
                                     bandPassCorrectedData,
                                     complexVoltagesData,
                                     subbandFrequency, sap,
                                     function,
                                     2.01f, subbandBandwidth);

  // Validate the returned data array
  outputOnHostPtr = outputOnHost5.get<complex<float> >();
  for (size_t idx = 0; idx < lengthComplexVoltagesData; ++idx)
    if (outputOnHostPtr[idx].real() != NR_STATIONS * 1.0f * 2.01f)
    {
      cerr << "all the data returned by the kernel should be (" << NR_STATIONS << ", " << NR_STATIONS << ")" ;
      exit_with_print(outputOnHostPtr);
    }
#endif    

  delete [] complexVoltagesData;
  delete [] bandPassCorrectedData;
  delete [] delaysData;

  return 0;
}


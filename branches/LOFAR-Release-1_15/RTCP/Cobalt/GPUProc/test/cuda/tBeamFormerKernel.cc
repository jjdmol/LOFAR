//# tBeamFormerKernel.cc: test Kernels/BeamFormerKernel class
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

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include "TestUtil.h"

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
int main() {
  INIT_LOGGER("tBeamFormerKernel");

  // Set up gpu environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    return 3;
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream stream(ctx);

  Parset ps("tBeamFormerKernel.in_parset");

  // Get default parameters for the compiler
  CompileFlags flags = CompileFlags();
  
  // Calculate bandpass weights and transfer to the device.
  // *************************************************************
  size_t lengthWeightsData = 
    BeamFormerKernel::bufferSize(ps, BeamFormerKernel::BEAM_FORMER_WEIGHTS) / 
    sizeof(float);
  size_t lengthBandPassCorrectedData =
    BeamFormerKernel::bufferSize(ps, BeamFormerKernel::INPUT_DATA) /
    sizeof(float);
  size_t lengthComplexVoltagesData = 
    BeamFormerKernel::bufferSize(ps, BeamFormerKernel::OUTPUT_DATA) / 
    sizeof(float);

  // Define the input and output arrays
  // ************************************************************* 
  float * complexVoltagesData = new float[lengthComplexVoltagesData];
  float * bandPassCorrectedData = new float[lengthBandPassCorrectedData];
  float * weightsData= new float[lengthWeightsData];

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

  size_t sizeWeightsData = lengthWeightsData * sizeof(float);
  DeviceMemory devWeightsMemory(ctx, sizeWeightsData);
  HostMemory rawWeightsData = getInitializedArray(ctx, sizeWeightsData, 1.0f);
  float *rawWeightsPtr = rawWeightsData.get<float>();
  for (unsigned idx = 0; idx < lengthWeightsData; ++idx)
    rawWeightsPtr[idx] = weightsData[idx];
  stream.writeBuffer(devWeightsMemory, rawWeightsData);

  size_t sizeBandPassCorrectedData = 
    lengthBandPassCorrectedData * sizeof(float);
  DeviceMemory devBandPassCorrectedMemory(ctx, sizeBandPassCorrectedData);
  HostMemory rawBandPassCorrectedData = 
    getInitializedArray(ctx, sizeBandPassCorrectedData, 2.0f);
  float *rawBandPassCorrectedPtr = rawBandPassCorrectedData.get<float>();
  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
    rawBandPassCorrectedPtr[idx] = bandPassCorrectedData[idx];
  stream.writeBuffer(devBandPassCorrectedMemory, rawBandPassCorrectedData);

  size_t sizeComplexVoltagesData = lengthComplexVoltagesData * sizeof(float);
  DeviceMemory devComplexVoltagesMemory(ctx, sizeComplexVoltagesData);
  HostMemory rawComplexVoltagesData = 
    getInitializedArray(ctx, sizeComplexVoltagesData, 3.0f);
  float *rawComplexVoltagesPtr = rawComplexVoltagesData.get<float>();
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
    rawComplexVoltagesPtr[idx] = complexVoltagesData[idx];

  // Write output content.
  stream.writeBuffer(devComplexVoltagesMemory, rawComplexVoltagesData);

  BeamFormerKernel kernel(ps, ctx, 
                          devComplexVoltagesMemory,
                          devBandPassCorrectedMemory,
                          devWeightsMemory);

  kernel.enqueue(stream);
  stream.synchronize();

  return 0;
}


//# tDelayAndBandPassKernel.cc: test Kernels/DelayAndBandPassKernel class
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

using namespace std;
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
  string srcFilename("BeamFormer.cu");

  // Get default parameters for the compiler
  flags_type flags = defaultFlags();
  definitions_type definitions = defaultDefinitions(ps);

  string ptx = createPTX(devices, srcFilename, flags, definitions);
  gpu::Module module(createModule(ctx, srcFilename, ptx));
  cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  // Calculate bandpass weights and transfer to the device.
  // *************************************************************
  // Create the data arrays
  unsigned NR_STATIONS = ps.nrStations();  
  unsigned NR_CHANNELS = ps.nrChannelsPerSubband();
  unsigned NR_SAMPLES_PER_CHANNEL = ps.nrSamplesPerChannel();
  unsigned NR_TABS =  ps.nrTABs(0);
  unsigned NR_POLARIZATIONS = 2;
  unsigned COMPLEX = 2;

  size_t lengthWeightsData = NR_STATIONS * NR_CHANNELS * NR_TABS * COMPLEX ;
  size_t lengthBandPassCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX;
  size_t lengthComplexVoltagesData = NR_CHANNELS* NR_SAMPLES_PER_CHANNEL * NR_TABS * NR_POLARIZATIONS * COMPLEX;

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

  // reserve enough space for the output of the firFilterKernel, and the correlatorKernel.
  size_t devFilteredDataSize = std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>),
                      ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>));
  gpu::DeviceMemory devFilteredData(ctx, devFilteredDataSize);

  DelayAndBandPassKernel kernel(ps, module, 
                                devComplexVoltages,
                                devCorrectedData,
                                devBeamFormerWeights);

  unsigned subband = 0;
  kernel.enqueue(stream, subband);
  stream.synchronize();

  return 0;
}


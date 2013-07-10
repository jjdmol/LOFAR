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
//# $Id: tDelayAndBandPassKernel.cc 25199 2013-06-05 23:46:56Z amesfoort $

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Kernels/IntToFloatKernel.h>
#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main() {
  INIT_LOGGER("tIntToFloatKernel");
  
  
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

  Parset ps("tIntToFloatKernel.in_parset");
  size_t COMPLEX = 2;
  size_t nSampledData = ps.nrStations() * ps.nrSamplesPerSubband() * NR_POLARIZATIONS * COMPLEX;
  size_t sizeSampledData = nSampledData * sizeof(char);

  // Create some initialized host data
  gpu::HostMemory sampledData(ctx, sizeSampledData);
  char *samples = sampledData.get<char>();
  for (unsigned idx =0; idx < nSampledData; ++idx)
    samples[idx] = -128;  // set all to -128
  gpu::DeviceMemory devSampledData(ctx, nSampledData * sizeof(float));
  stream.writeBuffer(devSampledData, sampledData, true);
  
  // Device mem for output
  gpu::DeviceMemory devConvertedData(ctx, nSampledData * sizeof(float));
  gpu::HostMemory convertedData(ctx,  nSampledData * sizeof(float));
  //stream.writeBuffer(devConvertedData, sampledData, true);

  IntToFloatKernel kernel(ps, ctx, devConvertedData, devSampledData); 

  kernel.enqueue(stream);
  stream.synchronize();
  stream.readBuffer(convertedData, devConvertedData, true);
  stream.synchronize();
  float *samplesFloat = convertedData.get<float>();
  
  // Validate the output:
  // The inputs were all -128 with bits  per sample 8. 
  // Therefore they should all be converted to -127
  for (size_t idx =0; idx < nSampledData; ++idx)
    if(samplesFloat[idx] != -127)
    {
        cerr << "Found an uncorrect sample in the output array at idx: " << idx << endl
             << "Value found: " << samplesFloat[idx] << endl
             << "Test failed "  << endl;
        return -1;
    }
      
  return 0;
}


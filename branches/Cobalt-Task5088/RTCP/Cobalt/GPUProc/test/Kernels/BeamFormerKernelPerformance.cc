//# BeamFormerKernelPerformance.cc
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


#include <lofar_config.h>

//#include <complex>

#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>

#include <GPUProc/gpu_wrapper.h>
//#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
//#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>

//#include "BeamFormerSubbandProcStep.h"
//#include "BeamFormerCoherentStep.h"
#include <GPUProc/SubbandProcs/BeamFormerFactories.h>

#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "../TestUtil.h"
#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Kernels/BeamFormerKernel.h>
#include <CoInterface/BlockID.h>
#include "../TestUtil.h"

#include <boost/lexical_cast.hpp>
#include <GPUProc/PerformanceCounter.h>

//beamFormerBuffers = std::auto_ptr<BeamFormerKernel::Buffers>(
//  new BeamFormerKernel::Buffers(*devB, *devA, *devBeamFormerDelays));


using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;


void usage()
{
  cout << "Usage: BeamFormerKernelPerformance [options] " << endl;
  cout << "" << endl;
  cout << " Performance measurements: ( no output validation)" << endl;
  cout << "-t nrtabs     Number of tabs to create, default == 1" << endl;
  cout << "-c nrchannels Number of channels to create, default == 1" << endl;
  cout << "-i IdxGPU     GPU index to run kernel on, default == 0" << endl;
  cout << "" << endl; 
  //cout << "If no arguments are provide the kernel with be tested on output validity" << endl;
  cout << "" << endl;
}

int main(int argc, char *argv[])
{
  INIT_LOGGER("BeamFormerKernelPerformance");

  int opt;
  unsigned nrTabs = 1;
  unsigned nrChannels = 1;
  unsigned idxGPU = 0;
  // parse all command-line options
  while ((opt = getopt(argc, argv, "t:c:i:")) != -1) 
  {
    switch (opt) 
    {
    case 't':
      nrTabs = atoi(optarg);
      break;

    case 'c':
      nrChannels = atoi(optarg);
      break;

    case 'i':
      idxGPU = atoi(optarg);
      break;

    default: /* '?' */
      usage();
      exit(1);
    }
  }

  // we expect no further arguments
  if (optind != argc) {
    usage();
    exit(1);
  }

  // Boiledplate code: create the environment etc;
  gpu::Device device(gpu::Platform().devices()[idxGPU]);
  gpu::Context context(device);
  gpu::Stream stream(context);

  // Create a parset with the correct parameters to run a beamformer kernel
  Parset ps;
  ps.add("Observation.Beam[0].TiedArrayBeam[0].directionType","J2000");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].absoluteAngle1", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].absoluteAngle2", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].coherent", "true");
  ps.add("Observation.Beam[0].angle1", "5.0690771926813865");
  ps.add("Observation.Beam[0].angle2", "0.38194688387907605");
  ps.add("Observation.Beam[0].directionType", "J2000");
  ps.add("Observation.Beam[0].nrTabRings", "0");
  ps.add("Observation.Beam[0].nrTiedArrayBeams", lexical_cast<string>(nrTabs));

  string filenames = "[";
  filenames.append(lexical_cast<string>(nrTabs)).append("*BEAM000.h5]");
  string hosts = "[";
  hosts.append(lexical_cast<string>(nrTabs)).append("*localhost:.]");
  ps.add("Observation.DataProducts.Output_CoherentStokes.filenames", filenames);
  ps.add("Observation.DataProducts.Output_CoherentStokes.locations", hosts);
  ps.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");

  //ps.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", "1");
  ps.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", lexical_cast<string>(nrChannels));
  ps.add("Cobalt.BeamFormer.CoherentStokes.subbandsPerFile", "512");
  ps.add("Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", "16");
  ps.add("Cobalt.BeamFormer.CoherentStokes.which", "I");

  ps.add("Cobalt.BeamFormer.nrDelayCompensationChannels", "64");
  ps.add("Cobalt.BeamFormer.nrHighResolutionChannels", "64");
  ps.add("Observation.VirtualInstrument.stationList", "[RS106]");
  ps.add("Observation.antennaArray", "HBA");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.beamList", "[5 * 0]");
  ps.updateSettings();

  // Create a beamformer kernel factory  KernelFactory<BeamFormerKernel> factory(ps);
  KernelFactory<BeamFormerKernel> factory(ps);
  // Calculate beamformer delays and transfer to the device.
  // *************************************************************
  size_t lengthDelaysData =
    factory.bufferSize(BeamFormerKernel::BEAM_FORMER_DELAYS) /
    sizeof(float);
  size_t lengthBandPassCorrectedData =
    factory.bufferSize(BeamFormerKernel::INPUT_DATA) /
    sizeof(float);
  size_t lengthComplexVoltagesData =
    factory.bufferSize(BeamFormerKernel::OUTPUT_DATA) /
    sizeof(float);
  
  // Define the input and output arrays
  // ************************************************************* 
  float * complexVoltagesData = new float[lengthComplexVoltagesData];
  float * bandPassCorrectedData = new float[lengthBandPassCorrectedData];
  float * delaysData = new float[lengthDelaysData];

  // ***********************************************************
  // Baseline test: If all weight data is zero the output should be zero
  // The output array is initialized with 42s
  for (unsigned idx = 0; idx < lengthComplexVoltagesData / 2; ++idx)
  {
    complexVoltagesData[idx * 2] = 42.0f;
    complexVoltagesData[idx * 2 + 1] = 42.0f;
  }

  for (unsigned idx = 0; idx < lengthBandPassCorrectedData / 2; ++idx)
  {
    bandPassCorrectedData[idx * 2] = 1.0f;
    bandPassCorrectedData[idx * 2 + 1] = 1.0f;
  }

  for (unsigned idx = 0; idx < lengthDelaysData / 2; ++idx)
  {
    delaysData[idx * 2] = 0.0f;
    delaysData[idx * 2 + 1] = 0.0f;
  }

  size_t sizeDelaysData = lengthDelaysData * sizeof(float);
  DeviceMemory devDelaysMemory(context, sizeDelaysData);
  HostMemory rawDelaysData = getInitializedArray(context, sizeDelaysData, 1.0f);
  float *rawDelaysPtr = rawDelaysData.get<float>();
  for (unsigned idx = 0; idx < lengthDelaysData; ++idx)
    rawDelaysPtr[idx] = delaysData[idx];
  stream.writeBuffer(devDelaysMemory, rawDelaysData);

  size_t sizeBandPassCorrectedData =
    lengthBandPassCorrectedData * sizeof(float);
  DeviceMemory devBandPassCorrectedMemory(context, sizeBandPassCorrectedData);
  HostMemory rawBandPassCorrectedData =
    getInitializedArray(context, sizeBandPassCorrectedData, 2.0f);
  float *rawBandPassCorrectedPtr = rawBandPassCorrectedData.get<float>();
  for (unsigned idx = 0; idx < lengthBandPassCorrectedData; ++idx)
    rawBandPassCorrectedPtr[idx] = bandPassCorrectedData[idx];
  stream.writeBuffer(devBandPassCorrectedMemory, rawBandPassCorrectedData);

  size_t sizeComplexVoltagesData = lengthComplexVoltagesData * sizeof(float);
  DeviceMemory devComplexVoltagesMemory(context, sizeComplexVoltagesData);
  HostMemory rawComplexVoltagesData =
    getInitializedArray(context, sizeComplexVoltagesData, 3.0f);
  float *rawComplexVoltagesPtr = rawComplexVoltagesData.get<float>();
  for (unsigned idx = 0; idx < lengthComplexVoltagesData; ++idx)
    rawComplexVoltagesPtr[idx] = complexVoltagesData[idx];

  // Write output content.
  stream.writeBuffer(devComplexVoltagesMemory, rawComplexVoltagesData);

  BeamFormerKernel::Buffers buffers(devBandPassCorrectedMemory, devComplexVoltagesMemory, devDelaysMemory);

  auto_ptr<BeamFormerKernel> kernel(factory.create(stream, buffers));

  float subbandFreq = 60e6f;
  unsigned sap = 0;

  PerformanceCounter counter(context);
  BlockID blockId;
  kernel->enqueue(blockId, subbandFreq, sap);
  stream.synchronize();

  return 0;

}
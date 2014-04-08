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
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/SubbandProcs/CorrelatorSubbandProc.h>
#include <GPUProc/PerformanceCounter.h>
#include <CoInterface/BlockID.h>
#include <GPUProc/SubbandProcs/BeamFormerFactories.h>
#include <boost/lexical_cast.hpp>


using namespace std;
using namespace LOFAR::Cobalt;
using namespace boost;


void usage()
{
cout << "Usage: DelayAndBandPAssKernel [options] " << endl;
cout << "" << endl;
cout << " Performance measurements: ( no output validation)" << endl;
cout << "-t nrtabs     Number of tabs to create, default == 1" << endl;
cout << "-c nrchannels Number of channels to create, default == 1" << endl;
cout << "-i IdxGPU     GPU index to run kernel on, default == 0" << endl;
cout << "-s IdxGPU     Number of stations to create" << endl;
cout << "" << endl;
//cout << "If no arguments are provide the kernel with be tested on output validity" << endl;
cout << "" << endl;
}


int main(int argc, char *argv[])
{
INIT_LOGGER("tDelayAndBandPassKernel");


int opt;
unsigned nrTabs = 127;
unsigned nrChannels = 64;
unsigned idxGPU = 0;
unsigned nStation = 47;
// parse all command-line options
while ((opt = getopt(argc, argv, "t:c:i:s:")) != -1)
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

case 's':
nStation = atoi(optarg);
break;

default:

usage();
exit(1);
    }
  }

  // we expect no further arguments
  if (optind != argc) {
    usage();
    exit(1);
  }


  // Set up gpu environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " GPU devices" << endl;
  }
  catch (gpu::GPUException& e) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream stream(ctx);

  // Create a parset with the correct parameters to run a beamformer kernel
  Parset ps;
  ps.add("Observation.Beam[0].TiedArrayBeam[0].directionType", "J2000");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].absoluteAngle1", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].absoluteAngle2", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].coherent", "true");
  ps.add("Observation.Beam[0].angle1", "5.0690771926813865");
  ps.add("Observation.Beam[0].angle2", "0.38194688387907605");
  ps.add("Observation.Beam[0].directionType", "J2000");
  ps.add("Observation.Beam[0].nrTabRings", "0");
  ps.add("Observation.Beam[0].nrTiedArrayBeams", lexical_cast<string>(nrTabs));
  ps.add("Observation.Beam[0].subbandList", "[24..28]");
  string filenames = "[";
  filenames.append(lexical_cast<string>(nrTabs)).append("*BEAM000.h5]");
  string hosts = "[";
  hosts.append(lexical_cast<string>(nrTabs)).append("*localhost:.]");
  ps.add("Observation.DataProducts.Output_CoherentStokes.filenames", filenames);
  ps.add("Observation.DataProducts.Output_CoherentStokes.locations", hosts);
  ps.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");

  ps.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[SB0.MS, SB1.MS, SB2.MS, SB3.MS, SB4.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[5 * :.]");

  //ps.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", "1");
  ps.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", lexical_cast<string>(nrChannels));

  ps.add("Cobalt.BeamFormer.CoherentStokes.subbandsPerFile", "512");
  ps.add("Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", "16");
  ps.add("Cobalt.BeamFormer.CoherentStokes.which", "I");

  ps.add("Cobalt.BeamFormer.nrDelayCompensationChannels", "64");
  ps.add("Cobalt.BeamFormer.nrHighResolutionChannels", "64");
  ps.add("Cobalt.blockSize", "196608");

  string stations = "[";
  stations.append(lexical_cast<string>(nStation)).append("*RS106HBA]");
  ps.add("Observation.VirtualInstrument.stationList", stations);
  ps.add("Observation.antennaArray", "HBA");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.beamList", "[5 * 0]");
  ps.add("Observation.Dataslots.RS106HBA.DataslotList", "[0..4]");
  ps.add("Observation.Dataslots.RS106HBA.RSPBoardList", "[5 * 0]");
  ps.updateSettings();
  
  KernelFactory<DelayAndBandPassKernel> factory(ps);
  stream.synchronize();

  gpu::DeviceMemory
  inputData(ctx, factory.bufferSize(DelayAndBandPassKernel::INPUT_DATA)),
  filteredData(ctx, factory.bufferSize(DelayAndBandPassKernel::OUTPUT_DATA)),
  delaysAtBegin(ctx, factory.bufferSize(DelayAndBandPassKernel::DELAYS)),
  delaysAfterEnd(ctx, factory.bufferSize(DelayAndBandPassKernel::DELAYS)),
  phase0s(ctx, factory.bufferSize(DelayAndBandPassKernel::PHASE_ZEROS)),
  bandPassCorrectionWeights(ctx, factory.bufferSize(DelayAndBandPassKernel::BAND_PASS_CORRECTION_WEIGHTS));

  DelayAndBandPassKernel::Buffers buffers(inputData, filteredData, delaysAtBegin, delaysAfterEnd, phase0s, bandPassCorrectionWeights);

  auto_ptr<DelayAndBandPassKernel> kernel(factory.create(stream, buffers));
  
  
  size_t subbandIdx = 0;
  float centralFrequency = ps.settings.subbands[subbandIdx].centralFrequency;
  size_t SAP = ps.settings.subbands[subbandIdx].SAP;
  PerformanceCounter counter(ctx);
  BlockID blockId;
  kernel->enqueue(blockId, centralFrequency, SAP);
  stream.synchronize();

  return 0;
}



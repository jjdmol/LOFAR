//# tCorrelatorPerformance.cc: test Correlator CUDA kernel for performance regression
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

#include <UnitTest++.h>
#include <cstdlib>  // for rand()
#include <string>
#include <iostream>

#include <boost/format.hpp>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <CoInterface/BlockID.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using namespace LOFAR::TYPES;
using LOFAR::Exception;

gpu::Device *device;

void runTest(
  unsigned NR_STATIONS,
  unsigned BLOCKSIZE,
  unsigned NR_INTEGRATIONS,
  unsigned NR_CHANNELS,
  double expected_runtime_ms)
{
  Context ctx(*device);
  Stream stream(ctx);
  
  LOG_INFO_STR("Testing " << NR_STATIONS << " stations, " << BLOCKSIZE << " samples/block, " << NR_INTEGRATIONS << " subblocks, " << NR_CHANNELS << " channels.");

  Parset ps;
  ps.add("Observation.VirtualInstrument.stationList", str(format("[%u*CS001]") % NR_STATIONS));
  ps.add("Observation.antennaSet", "LBA_INNER");
  ps.add("Observation.Dataslots.CS001LBA.RSPBoardList", "[0]");
  ps.add("Observation.Dataslots.CS001LBA.DataslotList", "[0]");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.Beam[0].subbandList", "[0]");
  ps.add("Observation.DataProducts.Output_Correlated.enabled",   "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[SB0.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");
  ps.add("Cobalt.blockSize",                         str(format("%u") % BLOCKSIZE));
  ps.add("Cobalt.Correlator.nrChannelsPerSubband",   str(format("%u") % NR_CHANNELS));
  ps.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  ps.add("Cobalt.Correlator.nrIntegrationsPerBlock", str(format("%u") % NR_INTEGRATIONS));
  ps.updateSettings();

  CorrelatorKernel::Parameters params(ps);
  KernelFactory<CorrelatorKernel> factory(params);

  // Define dummy Block-ID
  BlockID blockId;

  DeviceMemory dInput(ctx, factory.bufferSize(CorrelatorKernel::INPUT_DATA));
  DeviceMemory dOutput(ctx, factory.bufferSize(CorrelatorKernel::OUTPUT_DATA));

  // Create kernel
  std::auto_ptr<CorrelatorKernel> kernel(factory.create(stream, dInput, dOutput));

  // Run kernel
  for (size_t i = 0; i < 10; ++i) {
    kernel->enqueue(blockId);
    stream.synchronize();
  }

  CHECK_CLOSE(expected_runtime_ms, kernel->getStats().mean(), 0.5);
}

TEST(48_Stations_250ms_16ch)
{
  unsigned NR_STATIONS     = 48;
  unsigned NR_INTEGRATIONS = 4;
  unsigned NR_CHANNELS     = 16;
  unsigned BLOCKSIZE       = 196608;

  runTest(
    NR_STATIONS,
    BLOCKSIZE,
    NR_INTEGRATIONS,
    NR_CHANNELS,
    10.8);
}

TEST(80_Stations_250ms_16ch)
{
  unsigned NR_STATIONS     = 80;
  unsigned NR_INTEGRATIONS = 4;
  unsigned NR_CHANNELS     = 16;
  unsigned BLOCKSIZE       = 196608;

  runTest(
    NR_STATIONS,
    BLOCKSIZE,
    NR_INTEGRATIONS,
    NR_CHANNELS,
    27.0);
}

TEST(48_Stations_1s_64ch)
{
  unsigned NR_STATIONS     = 48;
  unsigned NR_INTEGRATIONS = 1;
  unsigned NR_CHANNELS     = 64;
  unsigned BLOCKSIZE       = 196608;

  runTest(
    NR_STATIONS,
    BLOCKSIZE,
    NR_INTEGRATIONS,
    NR_CHANNELS,
    9.7);
}

TEST(80_Stations_1s_64ch)
{
  unsigned NR_STATIONS     = 80;
  unsigned NR_INTEGRATIONS = 1;
  unsigned NR_CHANNELS     = 64;
  unsigned BLOCKSIZE       = 196608;

  runTest(
    NR_STATIONS,
    BLOCKSIZE,
    NR_INTEGRATIONS,
    NR_CHANNELS,
    27.1);
}

int main()
{
  INIT_LOGGER("tCorrelatorPerformance");
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

  // Connect to the first device, and check if its our target
  // platform (for our performance measurements to make sense).
  Device _device(0);

  const std::string targetPlatform = "Tesla K10.G2.8GB";

  LOG_INFO_STR("Device: " << _device.getName());

  if (_device.getName() != targetPlatform) {
    cerr << "Not running on the target platform (" << targetPlatform << ")" << endl;
    return 3;
  }

  // Run the test
  device = &_device;

  return UnitTest::RunAllTests() > 0;
}


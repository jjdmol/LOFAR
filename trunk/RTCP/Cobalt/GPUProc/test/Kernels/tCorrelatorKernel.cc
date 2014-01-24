//# tCorrelatorKernel.cc: test CorrelatorKernel class
//#
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
#include <GPUProc/Kernels/CorrelatorKernel.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Config.h>
#include <CoInterface/BlockID.h>
#include <Common/lofar_iostream.h>
#include <UnitTest++.h>
#include <boost/format.hpp>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using boost::format;

struct TestFixture
{
  TestFixture() : ps("tCorrelatorKernel.in_parset"), factory(ps) {}
  ~TestFixture() {}
  Parset ps;
  KernelFactory<CorrelatorKernel> factory;
};

TEST_FIXTURE(TestFixture, InputData)
{
  CHECK_EQUAL(size_t(786432),
              factory.bufferSize(CorrelatorKernel::INPUT_DATA));
}

TEST_FIXTURE(TestFixture, OutputData)
{
  CHECK_EQUAL(size_t(512),
              factory.bufferSize(CorrelatorKernel::OUTPUT_DATA));
}

TEST_FIXTURE(TestFixture, MustThrow)
{
  CHECK_THROW(factory.bufferSize(CorrelatorKernel::BufferType(2)),
              GPUProcException);
}

unsigned baseline(unsigned major, unsigned minor)
{
  CHECK(major >= minor);

  return major * (major + 1) / 2 + minor;
}

TEST(DataValidity)
{
  /*
   * Test full correlator output for 62 stations.
   */

  size_t NR_STATIONS = 62;

  Parset ps;
  ps.add("Observation.VirtualInstrument.stationList",   str(format("[%d*CS001]") % NR_STATIONS));
  ps.add("Observation.antennaSet",                      "LBA_INNER");
  ps.add("Observation.Dataslots.CS001LBA.RSPBoardList", "[0]");
  ps.add("Observation.Dataslots.CS001LBA.DataslotList", "[0]");

  // 1 subband
  ps.add("Observation.nrBeams",             "1");
  ps.add("Observation.Beam[0].subbandList", "[128]");

  // Correlator settings
  ps.add("Observation.DataProducts.Output_Correlated.enabled", "T");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[SB000.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");
  ps.add("Cobalt.Correlator.nrChannelsPerSubband",      "64");
  ps.add("Cobalt.blockSize",                            "196608");
  ps.add("Cobalt.Correlator.nrBlocksPerIntegration",    "1");
  ps.updateSettings();

  // Configuration
  KernelFactory<CorrelatorKernel> factory(ps);

  // Stream for all I/O
  gpu::Platform platform;
  gpu::Context context(platform.devices()[0]);
  gpu::Stream stream(context);

  // Buffers
  gpu::DeviceMemory devInput(context, factory.bufferSize(CorrelatorKernel::INPUT_DATA));
  gpu::DeviceMemory devOutput(context, factory.bufferSize(CorrelatorKernel::OUTPUT_DATA));
  CorrelatorKernel::Buffers buffers(devInput, devOutput);

  // Kernel
  std::auto_ptr<CorrelatorKernel> kernel(factory.create(stream, buffers));

  // Fill input
  BlockID blockID;

  MultiDimArrayHostBuffer<fcomplex, 4> hostInput(boost::extents[NR_STATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], context);
  MultiDimArrayHostBuffer<fcomplex, 4> hostOutput(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], context);

  for (size_t st = 0; st < NR_STATIONS; st++) {
    for (size_t ch = 0; ch < ps.nrChannelsPerSubband(); ch++) {
      for (size_t t = 0; t < ps.nrSamplesPerChannel(); t++) {
        /*
         * NOTE: Using higher values will result in imprecision,
         * causing the output to be less predictable.
         */
        hostInput[st][ch][t][0] = fcomplex(st + 1, 0);
        hostInput[st][ch][t][1] = fcomplex(st + 1, 1);
      }
    }
  }

  // Run kernel
  stream.writeBuffer(devInput, hostInput);
  kernel->enqueue(blockID);
  stream.readBuffer(hostOutput, devOutput, true);

  // Verify output
  const size_t nrSamples = ps.nrSamplesPerChannel();
  for (size_t st1 = 0; st1 < NR_STATIONS; st1++) {
    for (size_t st2 = 0; st2 <= st1; st2++) {
      const unsigned bl = ::baseline(st1, st2);

      LOG_INFO_STR("Checking baseline " << bl << " = (" << st1 << ", " << st2 << ")");

      // Start checking at ch = 1, because channel 0 is skipped
      for (size_t ch = 1; ch < ps.nrChannelsPerSubband(); ch++) {
        LOG_INFO_STR("Checking channel " << ch);

        const fcomplex expected00(fcomplex(nrSamples, 0) * hostInput[st1][ch][0][0] * conj(hostInput[st2][ch][0][0]));
        const fcomplex expected01(fcomplex(nrSamples, 0) * hostInput[st1][ch][0][0] * conj(hostInput[st2][ch][0][1]));
        const fcomplex expected10(fcomplex(nrSamples, 0) * hostInput[st1][ch][0][1] * conj(hostInput[st2][ch][0][0]));
        const fcomplex expected11(fcomplex(nrSamples, 0) * hostInput[st1][ch][0][1] * conj(hostInput[st2][ch][0][1]));

        CHECK_CLOSE(real(expected00), real(hostOutput[bl][ch][0][0]), 0.0001);
        CHECK_CLOSE(imag(expected00), imag(hostOutput[bl][ch][0][0]), 0.0001);
        CHECK_CLOSE(real(expected01), real(hostOutput[bl][ch][0][1]), 0.0001);
        CHECK_CLOSE(imag(expected01), imag(hostOutput[bl][ch][0][1]), 0.0001);
        CHECK_CLOSE(real(expected10), real(hostOutput[bl][ch][1][0]), 0.0001);
        CHECK_CLOSE(imag(expected10), imag(hostOutput[bl][ch][1][0]), 0.0001);
        CHECK_CLOSE(real(expected11), real(hostOutput[bl][ch][1][1]), 0.0001);
        CHECK_CLOSE(imag(expected11), imag(hostOutput[bl][ch][1][1]), 0.0001);
      }
    }
  }
}

int main()
{
  INIT_LOGGER("tCorrelatorKernel");
  try {
    gpu::Platform pf;
  } catch (gpu::GPUException&) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
  return UnitTest::RunAllTests() > 0;
}

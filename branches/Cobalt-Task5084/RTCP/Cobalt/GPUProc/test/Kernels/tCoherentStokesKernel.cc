//# tCoherentStokesKernel.cc: test CoherentStokesKernel class
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

#include <GPUProc/Kernels/CoherentStokesKernel.h>

#include <Common/lofar_iostream.h>
#include <CoInterface/Parset.h>
#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;

// Fixture for testing correct translation of parset values
struct ParsetFixture
{
  const size_t 
    timeIntegrationFactor = sizeof(sine) / sizeof(float),
    nrChannels = 37,
    nrOutputSamples = 29,
    nrInputSamples = nrOutputSamples * timeIntegrationFactor, 
    blockSize = timeIntegrationFactor * nrChannels * nrInputSamples,
    nrStations = 43;

  Parset parset;

  ParsetFixture() {
    parset.add("Observation.DataProducts.Output_Beamformed.enabled", 
               "true");
    parset.add("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor", 
               lexical_cast<string>(timeIntegrationFactor));
    parset.add("OLAP.CNProc_IncoherentStokes.channelsPerSubband",
               lexical_cast<string>(nrChannels));
    parset.add("OLAP.CNProc_IncoherentStokes.which",
               "IQUV");
    parset.add("Observation.VirtualInstrument.stationList",
               str(format("[%d*RS000]") % nrStations));
    parset.add("Cobalt.blockSize", 
               lexical_cast<string>(blockSize)); 
    parset.updateSettings();
  }
};


struct TestFixture
{


  TestFixture() 
  : 
    ps("tCoherentStokesKernel.in_parset"), 
    factory(ps) 
  {


  }
  ~TestFixture() {}
  Parset ps;
  CoherentStokesKernel::Parameters params;
  KernelFactory<CoherentStokesKernel> factory;
};

TEST_FIXTURE(TestFixture, InputData)
{
  CHECK_EQUAL(size_t(16 * (3072/16) * 2 * 2 * 8),
              factory.bufferSize(CoherentStokesKernel::INPUT_DATA));
}

TEST_FIXTURE(TestFixture, OutputData)
{
  CHECK_EQUAL(size_t(2 * 1 * (3072/16) / 16 * 16 * 4),
              factory.bufferSize(CoherentStokesKernel::OUTPUT_DATA));
}

TEST_FIXTURE(TestFixture, MustThrow)
{
  CHECK_THROW(factory.bufferSize(CoherentStokesKernel::BufferType(2)),
              GPUProcException);
}

int main()
{
  INIT_LOGGER("tCoherentStokesKernel");
  try {
    gpu::Platform pf;
  } catch (gpu::GPUException&) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
  return UnitTest::RunAllTests() == 0 ? 0 : 1;
}


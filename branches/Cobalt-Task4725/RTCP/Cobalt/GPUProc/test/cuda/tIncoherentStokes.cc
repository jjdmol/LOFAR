//# tIncoherentStokes.cc: test incoherent Stokes kernel
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

#include <GPUProc/Kernels/IncoherentStokesKernel.h>
#include <CoInterface/Parset.h>
#include <Common/LofarLogger.h>

#include <UnitTest++.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <iostream>

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt;

struct Fixture
{
  static const size_t 
    timeIntegrationFactor = 3,
    nrChannels = 37,
    nrSamples = 87,
    blockSize = timeIntegrationFactor * nrChannels * nrSamples,
    nrStations = 43;

  Parset ps;

  Fixture() {
    ps.add("Observation.DataProducts.Output_Beamformed.enabled", "true");
    ps.add("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor", 
           lexical_cast<string>(timeIntegrationFactor));
    ps.add("OLAP.CNProc_IncoherentStokes.channelsPerSubband",
           lexical_cast<string>(nrChannels));
    ps.add("OLAP.CNProc_IncoherentStokes.which", "IQUV");
    ps.add("Observation.VirtualInstrument.stationList",
           str(format("[%d*RS000]") % nrStations));
    ps.add("Cobalt.blockSize", lexical_cast<string>(blockSize)); 
    ps.updateSettings();
  }
};

const size_t
  Fixture::timeIntegrationFactor,
  Fixture::nrChannels,
  Fixture::nrSamples,
  Fixture::blockSize,
  Fixture::nrStations;

TEST_FIXTURE(Fixture, BufferSizes)
{
  // Test correctness of reported buffer sizes
  const ObservationSettings::BeamFormer::StokesSettings &settings = 
    ps.settings.beamFormer.incoherentSettings;

  CHECK_EQUAL(timeIntegrationFactor, settings.timeIntegrationFactor);
  CHECK_EQUAL(nrChannels, settings.nrChannels);
  CHECK_EQUAL(4U, settings.nrStokes);
  CHECK_EQUAL(nrStations, ps.nrStations());
  CHECK_EQUAL(nrSamples, settings.nrSamples(blockSize));
}

TEST_FIXTURE(Fixture, KernelFactory)
{
  // Test if we can succesfully create a KernelFactory
  KernelFactory<IncoherentStokesKernel> kf(ps);
}

int main()
{
  INIT_LOGGER("tIncoherentStokes");
  try {
    gpu::Platform pf;
    return UnitTest::RunAllTests() > 0;
  } catch (gpu::GPUException& e) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
}

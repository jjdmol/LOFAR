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
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/Parset.h>
#include <Common/LofarLogger.h>

#include <UnitTest++.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <iostream>

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt;

typedef complex<float> fcomplex;

struct ParsetFixture
{
  static const size_t 
    timeIntegrationFactor = 3,
    nrChannels = 37,
    nrSamples = 87,
    blockSize = timeIntegrationFactor * nrChannels * nrSamples,
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

const size_t
  ParsetFixture::timeIntegrationFactor,
  ParsetFixture::nrChannels,
  ParsetFixture::nrSamples,
  ParsetFixture::blockSize,
  ParsetFixture::nrStations;

TEST_FIXTURE(ParsetFixture, BufferSizes)
{
  // Test correctness of reported buffer sizes
  const ObservationSettings::BeamFormer::StokesSettings &settings = 
    parset.settings.beamFormer.incoherentSettings;

  CHECK_EQUAL(timeIntegrationFactor, settings.timeIntegrationFactor);
  CHECK_EQUAL(nrChannels, settings.nrChannels);
  CHECK_EQUAL(4U, settings.nrStokes);
  CHECK_EQUAL(nrStations, parset.nrStations());
  CHECK_EQUAL(nrSamples, settings.nrSamples(blockSize));
}

TEST_FIXTURE(ParsetFixture, KernelFactory)
{
  // Test if we can succesfully create a KernelFactory
  KernelFactory<IncoherentStokesKernel> kf(parset);
}

struct KernelFixture : ParsetFixture
{
  KernelFactory<IncoherentStokesKernel> factory;
  gpu::Device device;
  gpu::Context context;
  gpu::Stream stream;
  gpu::DeviceMemory dInput, dOutput;
  gpu::HostMemory hInput, hOutput;

  KernelFixture() :
    factory(parset),
    device(gpu::Platform().devices()[0]),
    context(device),
    stream(context),
    dInput(context, factory.bufferSize(IncoherentStokesKernel::INPUT_DATA)),
    dOutput(context, factory.bufferSize(IncoherentStokesKernel::OUTPUT_DATA)),
    hInput(context, dInput.size()),
    hOutput(context, dOutput.size())
  {
    cout << "dInput.size() = " << dInput.size() << endl;
    cout << "dOutput.size() = " << dOutput.size() << endl;
  }
};

TEST_FIXTURE(KernelFixture, BasicTest)
{
  size_t nrStokes = parset.settings.beamFormer.incoherentSettings.nrStokes;
  MultiDimArrayHostBuffer<fcomplex, 3> 
    input(
      boost::extents[nrStations][nrChannels][nrSamples],
      context),
    output(
      boost::extents[nrStokes][nrSamples / timeIntegrationFactor][nrChannels],
      context);
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

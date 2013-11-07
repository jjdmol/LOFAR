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


#include <lofar_config.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/CoherentStokesKernel.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>
#include <Common/LofarLogger.h>

#include <UnitTest++.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt;

typedef complex<float> fcomplex;

// Sine and cosine tables for angles that are multiples of 30 degrees in the
// range [0 .. 360> degrees.
float sine[]   = { 0,  0.5,  0.86602540478,  1,  0.86602540478,  0.5,
                   0, -0.5, -0.86602540478, -1, -0.86602540478, -0.5 };
float cosine[] = { 1,  0.86602540478,  0.5,  0, -0.5, -0.86602540478,
                  -1, -0.86602540478, -0.5,  0,  0.5,  0.86602540478 };


// Fixture for testing correct translation of parset values
struct ParsetSUT
{
  size_t 
    timeIntegrationFactor,
    nrChannels,
    nrOutputSamples,
    nrStations, 
    nrInputSamples, 
    blockSize;

  Parset parset;

  ParsetSUT(size_t inrChannels = 13,
    size_t inrOutputSamples = 1024,
    size_t inrStations = 43,
    size_t inrTabs = 21,
    size_t itimeIntegrationFactor = 1) 
  :
    timeIntegrationFactor(itimeIntegrationFactor),
    nrChannels(inrChannels),
    nrOutputSamples(inrOutputSamples),
    nrStations(inrStations),
    nrInputSamples(nrOutputSamples * timeIntegrationFactor), 
    blockSize(timeIntegrationFactor * nrChannels * nrInputSamples)
  {

    parset.add("Observation.DataProducts.Output_Beamformed.enabled", 
      "true");
    parset.add("OLAP.CNProc_CoherentStokes.timeIntegrationFactor", 
      lexical_cast<string>(timeIntegrationFactor));
    parset.add("OLAP.CNProc_CoherentStokes.channelsPerSubband",
      lexical_cast<string>(nrChannels));
    parset.add("OLAP.CNProc_CoherentStokes.which",
      "IQUV");
    parset.add("Observation.VirtualInstrument.stationList",
      str(format("[%d*RS000]") % nrStations));
    parset.add("Cobalt.blockSize", 
      lexical_cast<string>(blockSize)); 
    parset.add("Observation.Beam[0].nrTiedArrayBeams",lexical_cast<string>(inrTabs));
    parset.add("Observation.DataProducts.Output_Beamformed.filenames","[L76966_SAP000_B000_S0_P000_bf.raw,L76966_SAP000_B001_S0_P000_bf.raw]");
    parset.add("Observation.DataProducts.Output_Beamformed.locations","[:.,:.]");
    parset.updateSettings();
  }
};


// Test correctness of reported buffer sizes
TEST(BufferSizes)
{
   ParsetSUT sut;
  const ObservationSettings::BeamFormer::StokesSettings &settings = 
    sut.parset.settings.beamFormer.coherentSettings;
  CHECK_EQUAL(sut.timeIntegrationFactor, settings.timeIntegrationFactor);
  CHECK_EQUAL(sut.nrChannels, settings.nrChannels);
  CHECK_EQUAL(4U, settings.nrStokes);
  CHECK_EQUAL(sut.nrStations, sut.parset.nrStations());
  CHECK_EQUAL(sut.nrInputSamples, settings.nrSamples(sut.blockSize));
}

// Test if we can succesfully create a KernelFactory
TEST(KernelFactory)
{
  ParsetSUT sut;
  KernelFactory<CoherentStokesKernel> kf(sut.parset);
}


// Fixture for testing the CoherentStokes kernel itself.

struct SUTWrapper:  ParsetSUT
{
  gpu::Device device;
  gpu::Context context;
  gpu::Stream stream;
  size_t nrStokes;
  size_t nrTabs;
  KernelFactory<CoherentStokesKernel> factory;
  MultiDimArrayHostBuffer<fcomplex, 4> hInput;
  MultiDimArrayHostBuffer<float, 4> hOutput;
  MultiDimArrayHostBuffer<float, 4> hRefOutput;
  CoherentStokesKernel::Buffers buffers;
  scoped_ptr<CoherentStokesKernel> kernel;

  SUTWrapper(size_t inrChannels = 13,
                size_t inrOutputSamples = 1024,
                size_t inrStations = 43,
                size_t inrTabs = 21,
                size_t itimeIntegrationFactor = 1) :
      ParsetSUT(inrChannels, inrOutputSamples, inrStations ,
                inrTabs,itimeIntegrationFactor),
    device(gpu::Platform().devices()[0]),
    context(device),
    stream(context),
    nrStokes(parset.settings.beamFormer.coherentSettings.nrStokes),
    nrTabs(parset.settings.beamFormer.maxNrTABsPerSAP()),
    factory(parset),
    hInput(
      boost::extents[nrTabs][NR_POLARIZATIONS][nrInputSamples][nrChannels],
      context),
    hOutput(
      boost::extents[nrTabs][nrStokes][nrOutputSamples][nrChannels],
      context),
    hRefOutput(
      boost::extents[nrTabs][nrStokes][nrOutputSamples][nrChannels],
      context),
    buffers(
      gpu::DeviceMemory(
        context, factory.bufferSize(CoherentStokesKernel::INPUT_DATA)),
      gpu::DeviceMemory(
        context, factory.bufferSize(CoherentStokesKernel::OUTPUT_DATA))),
    kernel(factory.create(stream, buffers))
  {
    initializeHostBuffers();
  }

  // Initialize all the elements of the input host buffer to zero, and all
  // elements of the output host buffer to NaN.
  void initializeHostBuffers()
  {
    cout << "\nInitializing host buffers..."
         // << "\n  timeIntegrationFactor = " << setw(7) << timeIntegrationFactor
         // << "\n  nrChannels            = " << setw(7) << nrChannels
         // << "\n  nrInputSamples        = " << setw(7) << nrInputSamples
         // << "\n  nrOutputSamples       = " << setw(7) << nrOutputSamples
         // << "\n  nrStations            = " << setw(7) << nrStations
         // << "\n  blockSize             = " << setw(7) << blockSize
         << "\n  buffers.input.size()  = " << setw(7) << buffers.input.size()
         << "\n  buffers.output.size() = " << setw(7) << buffers.output.size()
         << endl;
    CHECK_EQUAL(buffers.input.size(), hInput.size());
    CHECK_EQUAL(buffers.output.size(), hOutput.size());
    fill(hInput.data(), hInput.data() + hInput.num_elements(), 0.0f);
    fill(hOutput.data(), hOutput.data() + hOutput.num_elements(), 42);
    fill(hRefOutput.data(), hRefOutput.data() + hRefOutput.num_elements(), 0.0f);
  }

  void runKernel()
  {
    // Dummy BlockID
    BlockID blockId;
    // Copy input data from host- to device buffer synchronously
    stream.writeBuffer(buffers.input, hInput, true);
    // Launch the kernel
    kernel->enqueue(blockId);
    // Copy output data from device- to host buffer synchronously
    stream.readBuffer(hOutput, buffers.output, true);
  }

};

// An input of all zeros should result in an output of all zeros.
TEST(ZeroTest)
{
  // start the test vector at the largest size
  size_t tabs_sizes[] = {33,1,13};
  std::vector<size_t> tabs(tabs_sizes, tabs_sizes + sizeof(tabs_sizes) / sizeof(size_t) );
  size_t channel_sizes[] = {41,1,13};
  std::vector<size_t> channels(channel_sizes, channel_sizes + sizeof(channel_sizes) / sizeof(size_t) );
  size_t sample_sizes[] = {1024,16,512};
  std::vector<size_t> samples(sample_sizes, sample_sizes + sizeof(sample_sizes) / sizeof(size_t) );
  
  //loop over the three input vectors
  for (std::vector<size_t>::const_iterator itab = tabs.begin();itab != tabs.end(); ++itab)
  for (std::vector<size_t>::const_iterator ichan = channels.begin();ichan != channels.end(); ++ichan)
  for (std::vector<size_t>::const_iterator isamp = samples.begin();isamp != samples.end(); ++isamp)
  {
    cout << "*******testing tabs: " <<  *itab 
                   << " channels: " <<  *ichan
                   << " samples: "  <<  *isamp << endl;
    SUTWrapper sut(*ichan, *isamp, 43, *itab);
  // Host buffers are properly initialized for this test. Just run the kernel. 
  sut.runKernel();
  CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
                    sut.hOutput.data(),
                    sut.hOutput.num_elements());
  }
  
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


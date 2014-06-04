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

#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/CoherentStokesKernel.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/SubbandProcs/BeamFormerFactories.h>
#include <GPUProc/gpu_wrapper.h>
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


#include "KernelTestHelpers.h"

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt;
using namespace LOFAR::Cobalt::gpu;

typedef complex<float> fcomplex;

// Fixture for testing correct translation of parset values
struct ParsetSUT
{
  size_t 
    timeIntegrationFactor,
    nrChannels,
    nrOutputSamples,
    nrStations, 
    nrInputSamples, 
    blockSize,
    nrDelayCompensationChannels;

  Parset parset;

  ParsetSUT(size_t inrChannels =  16,
    size_t inrOutputSamples = 1024,
    size_t inrStations =  43,
    size_t inrTabs = 21,
    size_t itimeIntegrationFactor = 1,
    string stokes = "IQUV") 
  :
    timeIntegrationFactor(itimeIntegrationFactor),
    nrChannels(inrChannels),
    nrOutputSamples(inrOutputSamples),
    nrStations(inrStations),
    nrInputSamples(nrOutputSamples * timeIntegrationFactor), 
    blockSize(timeIntegrationFactor * nrChannels * nrInputSamples),
    nrDelayCompensationChannels(64)
  {
    size_t nr_files = inrTabs * 4; // 4 for number of stokes
    parset.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
    parset.add("Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", 
               lexical_cast<string>(timeIntegrationFactor));
    parset.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband",
      lexical_cast<string>(nrChannels));
    parset.add("Cobalt.BeamFormer.CoherentStokes.which", stokes);
    parset.add("Observation.VirtualInstrument.stationList",
      str(format("[%d*RS000]") % nrStations));
    parset.add("Observation.antennaSet", "LBA_INNER");
    parset.add("Observation.rspBoardList", "[0]");
    parset.add("Observation.rspSlotList", "[0]");
    parset.add("Cobalt.blockSize", 
      lexical_cast<string>(blockSize)); 
    parset.add("Observation.nrBeams", "1");
    parset.add("Observation.Beam[0].subbandList", "[0]");
    parset.add("Observation.Beam[0].nrTiedArrayBeams",lexical_cast<string>(inrTabs));
    parset.add("Observation.DataProducts.Output_CoherentStokes.filenames",
      str(format("[%d*dummy.raw]") % nr_files));
    parset.add("Observation.DataProducts.Output_CoherentStokes.locations", str(format("[%d*:.]") % nr_files));
    parset.add("Cobalt.BeamFormer.nrDelayCompensationChannels",
               lexical_cast<string>(nrDelayCompensationChannels));
    parset.updateSettings();
  }
};


// Test correctness of reported buffer sizes
TEST(BufferSizes)
{
  LOG_INFO("Test BufferSizes");
   ParsetSUT sut;
  const ObservationSettings::BeamFormer::StokesSettings &settings = 
    sut.parset.settings.beamFormer.coherentSettings;
  CHECK_EQUAL(sut.timeIntegrationFactor, settings.timeIntegrationFactor);
  CHECK_EQUAL(sut.nrChannels, settings.nrChannels);
  CHECK_EQUAL(4U, settings.nrStokes);
  CHECK_EQUAL(sut.nrStations, sut.parset.nrStations());
  CHECK_EQUAL(sut.nrInputSamples, settings.nrSamples);
}

// Test if we can succesfully create a KernelFactory
TEST(KernelFactory)
{
  LOG_INFO("Test KernelFactory");
  ParsetSUT sut;
  KernelFactory<CoherentStokesKernel> kf(sut.parset);
}

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

  SUTWrapper(size_t inrChannels = 16,
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
  LOG_INFO("Test ZeroTest");
  // start the test vector at the largest size
  size_t tabs_sizes[] = {1,13};
  std::vector<size_t> tabs(tabs_sizes, tabs_sizes + sizeof(tabs_sizes) / sizeof(size_t) );
  size_t channel_sizes[] = {1,16,32}; // only test valid sizes
  std::vector<size_t> channels(channel_sizes, channel_sizes + sizeof(channel_sizes) / sizeof(size_t) );
  size_t sample_sizes[] = { channel_sizes[0] * 1024,
                            channel_sizes[1] * 1024, 
                            channel_sizes[2] * 1024 };
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

// ***********************************************************
// tests if the stokes parameters are calculate correctly. For a single sample
// I = X *  con(X) + Y * con(Y)
// Q = X *  con(X) - Y * con(Y)
// U = 2 * RE(X * con(Y))
// V = 2 * IM(X * con(Y))
//
// This reduces to (validate on paper by Wouter and John):
// PX = RE(X) * RE(X) + IM(X) * IM(X)
// PY = RE(Y) * RE(Y) + IM(Y) * IM(Y)
// I = PX + PY
// Q = PX - PY
// 
// U = 2 * (RE(X) * RE(Y) + IM(X) * IM(Y))
// V = 2 * (IM(X) * RE(Y) - RE(X) * IM(Y))
TEST(CoherentNoComplex1SampleTest)
{
  LOG_INFO("Test CoherentNoComplex1SampleTest");
  SUTWrapper sut(1,  //channels
                 1,  // inrOutputSamples
                 1,  // inrStations
                 1,  // inrTabs
                 1); // itimeIntegrationFactor

  // Test a single sample with specific input
  sut.hInput.data()[0] = fcomplex(2.0f, 0.0f);
  sut.hInput.data()[1] = fcomplex(1.0f, 0.0f);
  sut.hInput.data()[2] = fcomplex(2.0f, 0.0f);
  sut.hInput.data()[3] = fcomplex(1.0f, 0.0f);

  // Host buffers are properly initialized for this test. Just run the kernel. 
  sut.runKernel();

  // Expected output
  sut.hRefOutput.data()[0] = 5.0f;
  sut.hRefOutput.data()[1] = 3.0f;
  sut.hRefOutput.data()[2] = 4.0f;
  sut.hRefOutput.data()[3] = 0.0f;

  CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
                    sut.hOutput.data(),
                    sut.hOutput.num_elements()); 
}


TEST(CoherentComplex1SampleTest)
{
  LOG_INFO("Test CoherentComplex1SampleTest");
  SUTWrapper sut(1,  //channels
                 1,  // inrOutputSamples
                 1,  // inrStations
                 1,  // inrTabs
                 1); // itimeIntegrationFactor

  // Test a single sample with specific input
  sut.hInput.data()[0] = fcomplex(0.0f, 2.0f);
  sut.hInput.data()[1] = fcomplex(0.0f, 1.0f);
  sut.hInput.data()[2] = fcomplex(0.0f, 2.0f);
  sut.hInput.data()[3] = fcomplex(0.0f, 1.0f);

  // Host buffers are properly initialized for this test. Just run the kernel. 
  sut.runKernel();

  // Expected output
  sut.hRefOutput.data()[0] = 5.0f;
  sut.hRefOutput.data()[1] = 3.0f;
  sut.hRefOutput.data()[2] = 4.0f;
  sut.hRefOutput.data()[3] = 0.0f;

  CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
                    sut.hOutput.data(),
                    sut.hOutput.num_elements()); 
}


TEST(Coherent4DifferentValuesSampleTest)
{
  LOG_INFO("Test Coherent4DifferentValuesSampleTest");
  SUTWrapper sut(1,  //channels
                 1,  // inrOutputSamples
                 1,  // inrStations
                 1,  // inrTabs
                 1); // itimeIntegrationFactor

  // Test a single sample with specific input
  sut.hInput.data()[0] = fcomplex(1.0f, 2.0f);
  sut.hInput.data()[1] = fcomplex(3.0f, 4.0f);
  // second polarization
  sut.hInput.data()[2] = fcomplex(1.0f, 2.0f);
  sut.hInput.data()[3] = fcomplex(3.0f, 4.0f);

  // Host buffers are properly initialized for this test. Just run the kernel. 
  sut.runKernel();

  // Expected output
  sut.hRefOutput.data()[0] = 30.0f;
  sut.hRefOutput.data()[1] = -20.0f;
  sut.hRefOutput.data()[2] = 22.0f;
  sut.hRefOutput.data()[3] = 4.0f;

  CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
                    sut.hOutput.data(),
                    sut.hOutput.num_elements()); 
}


TEST(BasicIntegrationTest)
{
  LOG_INFO("Test BasicIntegrationTest");
  // ***********************************************************
  // Test if the integration works by inputting non complex ones 
  // and integrating over the total number of samples
  // This should result in 2 * num samples in both I and V
  unsigned NR_SAMPLES_PER_CHANNEL = 16;

  SUTWrapper sut(1,  //channels
                 1,  // NR_SAMPLES_PER_CHANNEL
                 1,  // inrStations
                 1,  // inrTabs
                 16); // itimeIntegrationFactor

  // Set the input
  for (size_t idx = 0; idx < sut.hInput.size(); ++idx)
    sut.hInput.data()[idx] = fcomplex(1.0f, 0.0f);



  // Host buffers are properly initialized for this test. Just run the kernel. 
  sut.runKernel();

  // Expected output
  for (size_t idx = 0; idx < sut.hRefOutput.size() / (size_t) 2; ++idx)
  {
    sut.hRefOutput.data()[idx * 2] = 2.0f * NR_SAMPLES_PER_CHANNEL;
    sut.hRefOutput.data()[idx * 2 + 1 ] = 0.0f;
  }

  CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
                    sut.hOutput.data(),
                    sut.hOutput.num_elements()); 
}


TEST(Coherent2DifferentValuesAllDimTest)
{
  LOG_INFO("Test Coherent2DifferentValuesAllDimTest");
  // ***********************************************************
  // Full test performing all functionalities and runtime validate that the output
  // is correct.
  // 1. Insert both complex and non complex values. This should result specific values
  // for all Stokes parameters
  // 2. Do it time parallel
  // 3. Integrate 
  // 4. Use tabs and channels
  size_t NR_CHANNELS = 1;
  size_t NR_SAMPLES_PER_OUTPUT_CHANNEL = 200;
  size_t NR_TABS =  17;
  size_t INTEGRATION_SIZE = 3;

  SUTWrapper sut(NR_CHANNELS, 
                 NR_SAMPLES_PER_OUTPUT_CHANNEL,  
                 1,          
                 NR_TABS,  
                 INTEGRATION_SIZE); 
  // Set the input
  for (size_t idx = 0; idx < sut.hInput.size(); ++idx)
    sut.hInput.data()[idx] = fcomplex(1.0f, 2.0f);

  // Host buffers are properly initialized for this test. Just run the kernel. 
  sut.runKernel();

  // Expected output
  size_t value_repeat = NR_SAMPLES_PER_OUTPUT_CHANNEL;
  cout << "value_repeat: "  << value_repeat << endl;
  // For stokes parameters
  size_t size_tab = value_repeat * 4;
  for (size_t idx_tab = 0; idx_tab < NR_TABS; ++idx_tab)
  {
    // I
    for (size_t idx_value_repeat = 0 ; idx_value_repeat < value_repeat; ++idx_value_repeat)
    {
      sut.hRefOutput.data()[idx_tab * size_tab + idx_value_repeat]                    = 10.0f * INTEGRATION_SIZE;
      sut.hRefOutput.data()[idx_tab * size_tab + value_repeat + idx_value_repeat]     = 0.0f;
      sut.hRefOutput.data()[idx_tab * size_tab + value_repeat * 2 + idx_value_repeat] = 10.0f * INTEGRATION_SIZE;
      sut.hRefOutput.data()[idx_tab * size_tab + value_repeat * 3 +idx_value_repeat]  = 0.0f;
    }
  }

  CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
                    sut.hOutput.data(),
                    sut.hOutput.num_elements()); 
}


int main(int argc, char *argv[])
{
  const char * testName = "tCoherentStokesKernel";
  INIT_LOGGER(testName);

  Parset ps;
  KernelParameters params;
  parseCommandlineParameters(argc, argv, ps, params, testName);
  //  If no arguments were parsed
  try
  {
    gpu::Platform pf;
  }
  catch (gpu::GPUException&)
  {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }

  if (!params.parameterParsed)
  {
    cout << "Running unittests" << endl;
    return UnitTest::RunAllTests() == 0 ? 0 : 1;
  }

  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream stream(ctx);

  // Create the factory
  KernelFactory<CoherentStokesKernel> factory(
          BeamFormerFactories::coherentStokesParams(ps));

  DeviceMemory  coherentStokesInputMem(ctx, 
                  factory.bufferSize(CoherentStokesKernel::INPUT_DATA)),
                  coherentStokesOutputMem(ctx,
                   factory.bufferSize(CoherentStokesKernel::OUTPUT_DATA));


  CoherentStokesKernel::Buffers buffers(coherentStokesInputMem,
              coherentStokesOutputMem);

  // kernel
  auto_ptr<CoherentStokesKernel> kernel(factory.create(stream, buffers));

  BlockID blockId;
  // run
  kernel->enqueue(blockId);
  stream.synchronize();

  
}


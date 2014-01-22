//# tFFTShiftKernel.cc: test FFTShiftKernel class
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
#include <GPUProc/Kernels/FFTShiftKernel.h>
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

// Fixture for testing correct translation of parset values
struct ParsetSUT
{
  size_t  nrChannels, nrStations, nrSamplesSubband, nrInputSamples,
          nrOutputSamples, nrBlockSize;

  Parset parset;

  ParsetSUT(size_t inrChannels, size_t inrStations, size_t inrTabs,
            size_t inrSamplesChannel, string stokes)
  :
    nrChannels(inrChannels),
    nrStations(inrStations),
    nrSamplesSubband(inrSamplesChannel * inrChannels),
    nrInputSamples(nrSamplesSubband),
    nrOutputSamples(nrInputSamples),
    nrBlockSize(nrInputSamples)
  {
    // 4 for number of stokes
    size_t nr_files =  inrTabs * 4; 
    parset.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
    parset.add("OLAP.CNProc_CoherentStokes.which", stokes);
    parset.add("Observation.VirtualInstrument.stationList",
      str(format("[%d*RS000]") % nrStations));
    parset.add("Cobalt.blockSize",
      lexical_cast<string>(nrBlockSize));
    parset.add("Observation.Beam[0].nrTiedArrayBeams", 
               lexical_cast<string>(inrTabs));
    parset.add("Observation.DataProducts.Output_CoherentStokes.filenames",
      str(format("[%d*dummy.raw]") % nr_files));
    parset.add("Observation.DataProducts.Output_CoherentStokes.locations", 
               str(format("[%d*:.]") % nr_files));
    //parset.add(""); //ps.settings.beamFormer.nrDelayCompensationChannels
    parset.updateSettings();

  }
};

// Test correctness of reported buffer sizes
TEST(BufferSizes)
{
  cout << "running test: BufferSizes" << endl;
  ParsetSUT sut(1, 2, 2, 1024, "IQUV");
  const ObservationSettings::BeamFormer::StokesSettings &settings =
    sut.parset.settings.beamFormer.coherentSettings;
  CHECK_EQUAL(sut.nrChannels, settings.nrChannels);
  CHECK_EQUAL(4U, settings.nrStokes);
  CHECK_EQUAL(sut.nrStations, sut.parset.nrStations());
}



struct SUTWrapper : ParsetSUT
{
  gpu::Device device;
  gpu::Context context;
  gpu::Stream stream;
  size_t nrStokes;
  size_t nrTabs;
  KernelFactory<FFTShiftKernel> factory;
  MultiDimArrayHostBuffer<fcomplex, 4> hInput;
  MultiDimArrayHostBuffer<fcomplex, 4> hOutput;
  MultiDimArrayHostBuffer<fcomplex, 4> hRefOutput;
  gpu::DeviceMemory deviceMemory;
  FFTShiftKernel::Buffers buffers;
  scoped_ptr<FFTShiftKernel> kernel;

  SUTWrapper(size_t inrChannels , size_t inrStations,
    size_t inrTabs, size_t inrOutputinSamplesPerSuband) :
    ParsetSUT(inrChannels,  inrStations,
              inrTabs, inrOutputinSamplesPerSuband, "IQUV"),
    device(gpu::Platform().devices()[0]),
    context(device),
    stream(context),
    nrStokes(parset.settings.beamFormer.coherentSettings.nrStokes),
    nrTabs(parset.settings.beamFormer.maxNrTABsPerSAP()),
    factory(FFTShiftParams(parset)),
    hInput(
      boost::extents[inrStations][NR_POLARIZATIONS][nrChannels][inrOutputinSamplesPerSuband],
      context),
    hOutput(boost::extents[inrStations][NR_POLARIZATIONS][nrChannels][inrOutputinSamplesPerSuband], 
        context), 
    hRefOutput(
      boost::extents[inrStations][NR_POLARIZATIONS][nrChannels][inrOutputinSamplesPerSuband],
      context),
    deviceMemory(context, factory.bufferSize(FFTShiftKernel::INPUT_DATA)),
    buffers(deviceMemory, deviceMemory),
    kernel(factory.create(stream, buffers))
  {
    initializeHostBuffers();
  }

  // Needed to adapt the number of channels in the parameterset
  // These values are normaly added in the SubbandProc (compile time default).
  FFTShiftKernel::Parameters FFTShiftParams(Parset parset)
  {
    FFTShiftKernel::Parameters params(parset);

    // time integration has not taken place yet, so calculate the nrSamples
    // manually
    params.nrChannelsPerSubband = nrChannels;

    params.nrSamplesPerChannel =
      parset.nrSamplesPerSubband() / params.nrChannelsPerSubband;

    return params;
  }


  // Initialize all the elements of the input host buffer to zero, and all
  // elements of the output host buffer to NaN.
  void initializeHostBuffers()
  {
    cout << "Kernel buffersize set to: " << factory.bufferSize(
              FFTShiftKernel::INPUT_DATA) << endl;
    cout << "\nInitializing host buffers..." << endl
      << " buffers.input.size()  = " << setw(7) << buffers.input.size() << endl
      << " hInput.size()  = " << setw(7) << hInput.size() << endl
      << " buffers.output.size() = " << setw(7) << buffers.output.size() 
      << endl;
    CHECK_EQUAL(buffers.input.size(), hInput.size());
    CHECK_EQUAL(buffers.output.size(), hOutput.size());
    fill(hInput.data(), hInput.data() + hInput.num_elements(),
             fcomplex(0.0f, 0.0f));
    fill(hRefOutput.data(), hRefOutput.data() + hRefOutput.num_elements(),
             fcomplex(0.0f, 0.0f));
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

// Test if we can succesfully create a KernelFactory
TEST(KernelFactory)
{
  cout << "running test: KernelFactory" << endl;
  SUTWrapper sut(1, 2, 2, 4096);
  sut.runKernel();

}

// An input of all zeros should result in an output of all zeros.
TEST(ZeroTest)
{
  cout << "running test: ZeroTest" << endl;
  // number of stations
  size_t nrStations = 1;
  // start the test vector at the largest size
  size_t tabs_sizes[] = { 1, 13, 33};

  std::vector<size_t> tabs(
          tabs_sizes, tabs_sizes + sizeof(tabs_sizes) / sizeof(size_t));
  size_t channel_sizes[] = {  1 , 16, 64}; 

  std::vector<size_t> channels(channel_sizes,
        channel_sizes + sizeof(channel_sizes) / sizeof(size_t));
  size_t sample_sizes[] = { 64 * 1024, 64 *  2048, };
  std::vector<size_t> samples(sample_sizes,
    sample_sizes + sizeof(sample_sizes) / sizeof(size_t));

  //loop over the three input vectors
  for (std::vector<size_t>::const_iterator itab = tabs.begin();
              itab != tabs.end(); ++itab)
  for (std::vector<size_t>::const_iterator ichan = channels.begin(); 
              ichan != channels.end(); ++ichan)
  for (std::vector<size_t>::const_iterator isamp = samples.begin(); 
              isamp != samples.end(); ++isamp)
  {
    cout << "*******testing tabs: " << *itab
      << " channels: " << *ichan
      << " samples: " << *isamp << endl;
    SUTWrapper sut(*ichan, nrStations, *itab, *isamp);
    // Host buffers are properly initialized for this test. Just run the kernel. 
    
    sut.runKernel();
    CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
      sut.hOutput.data(),
      sut.hOutput.num_elements());
  }
}

// Test if the input is correctly flipped at each odd entry
TEST(FlipValues)
{
  cout << "running test: FlipValues"  << endl;
  // number of stations
  size_t nrStations = 1;
  // start the test vector at the largest size
  size_t tabs_sizes[] = { 1 , 13, 33};

  std::vector<size_t> tabs(
        tabs_sizes, tabs_sizes + sizeof(tabs_sizes) / sizeof(size_t));
  size_t channel_sizes[] = { 1, 16, 64}; 

  std::vector<size_t> channels(channel_sizes,
    channel_sizes + sizeof(channel_sizes) / sizeof(size_t));
  size_t sample_sizes[] = { 2048, 4096 };
  std::vector<size_t> samples(sample_sizes,
    sample_sizes + sizeof(sample_sizes) / sizeof(size_t));

  //loop over the three input vectors
  for (std::vector<size_t>::const_iterator itab = tabs.begin();
           itab != tabs.end(); ++itab)
  for (std::vector<size_t>::const_iterator ichan = channels.begin();
           ichan != channels.end(); ++ichan)
  for (std::vector<size_t>::const_iterator isamp = samples.begin(); 
            isamp != samples.end(); ++isamp)
  {
    cout << "*******testing tabs: " << *itab
      << " channels: " << *ichan
      << " samples: " << *isamp << endl;
    SUTWrapper sut(*ichan, nrStations, *itab, *isamp);

     // Test full input create the input
    for (unsigned idx = 0; idx < sut.hInput.num_elements(); ++idx)
      sut.hInput.data()[idx] = fcomplex(1.0f, 1.0f);

    // The output
    for (unsigned idx = 0; idx < (sut.hRefOutput.num_elements() / 2); ++idx)
    {
      sut.hRefOutput.data()[idx * 2] = fcomplex(1.0f, 1.0f);
      sut.hRefOutput.data()[idx * 2 + 1] = fcomplex(-1.0f, -1.0f);
    }

    sut.runKernel();

    CHECK_ARRAY_EQUAL(sut.hRefOutput.data(),
      sut.hOutput.data(),
      sut.hOutput.num_elements());
  }
}


int main()
{
  INIT_LOGGER("tFFTShiftKernel");

  try {
    gpu::Platform pf;
  }
  catch (gpu::GPUException&) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
  return UnitTest::RunAllTests() == 0 ? 0 : 1;  

}


//# tBandPass.cc: test delay and bandpass CUDA kernel
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

#include <cstdlib>
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <UnitTest++.h>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

using namespace std;
using namespace LOFAR::Cobalt;

using LOFAR::i16complex;
using LOFAR::i8complex;

typedef complex<float> fcomplex;

gpu::Stream *stream;

// default compile definitions
const unsigned NR_STATIONS = 2;
const unsigned NR_CHANNELS_1 = 1;
const unsigned NR_CHANNELS_2 = 16;
const unsigned NR_SAMPLES_PER_CHANNEL = 64;
const unsigned NR_BITS_PER_SAMPLE = 8;
const unsigned NR_POLARIZATIONS = 2;


// Initialize input AND output before calling runKernel().
// We copy both to the GPU, to make sure the final output is really from the
// kernel.  T is an LCS i*complex type, or complex<float> when #chnl > 1.
template <typename T>
void runKernel(gpu::Function kfunc,
               MultiDimArrayHostBuffer<fcomplex, 4> &outputData,
               MultiDimArrayHostBuffer<T,        5> &inputData,
               MultiDimArrayHostBuffer<float,    1> &bandPassFactors)
{
  gpu::Context ctx(stream->getContext());

  gpu::DeviceMemory devOutput         (ctx, outputData.size());
  gpu::DeviceMemory devInput          (ctx, inputData.size());
  gpu::DeviceMemory devBandPassFactors(ctx, bandPassFactors.size());

  kfunc.setArg(0, devOutput);
  kfunc.setArg(1, devInput);
  kfunc.setArg(2, devBandPassFactors);

  unsigned max_worksize = 512;
  unsigned xdim_local;
  if (NR_SAMPLES_PER_CHANNEL * NR_CHANNELS_2 > max_worksize)
  {
    for (unsigned exponent_idx =  8; exponent_idx > 2; -- exponent_idx)
    {
      // If the dimension is can be devided a whole number 2 ^ exp
      if ( (NR_SAMPLES_PER_CHANNEL * NR_CHANNELS_2) % unsigned(pow(2, exponent_idx)) == 0)
      {
        xdim_local = pow(2, exponent_idx);
        break;
      }
    }
  }
  else
    xdim_local = NR_SAMPLES_PER_CHANNEL * NR_CHANNELS_2 ;
    
  gpu::Grid globalWorkSize((NR_SAMPLES_PER_CHANNEL * NR_CHANNELS_2) / xdim_local,
                           NR_STATIONS,
                           1);

  gpu::Block localWorkSize(xdim_local,
                           1,
                           1);

  // Overwrite devOutput, so result verification is more reliable.
  stream->writeBuffer(devOutput,          outputData);
  stream->writeBuffer(devInput,           inputData);
  stream->writeBuffer(devBandPassFactors, bandPassFactors);
  stream->launchKernel(kfunc, globalWorkSize, localWorkSize);
  stream->synchronize(); // wait until transfer completes

  stream->readBuffer(outputData, devOutput);

  stream->synchronize(); // wait until transfer completes

}

gpu::Function initKernel(gpu::Context ctx, const CompileDefinitions& defs)
{
  // Compile to ptx. Copies the kernel to the current dir
  // (also the complex header, needed for compilation).
  string kernelPath("BandPass.cu");
  CompileFlags flags(defaultCompileFlags());
  vector<gpu::Device> devices(1, gpu::Device(0));
  string ptx(createPTX(kernelPath, defs, flags, devices));
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  gpu::Function kfunc(module, "correctBandPass");

  return kfunc;
}

CompileDefinitions getDefaultCompileDefinitions()
{
  CompileDefinitions defs;

  defs["NR_STATIONS"] =
    boost::lexical_cast<string>(NR_STATIONS);
  defs["NR_CHANNELS_1"] =
    boost::lexical_cast<string>(NR_CHANNELS_1);
  defs["NR_CHANNELS_2"] =
    boost::lexical_cast<string>(NR_CHANNELS_2);
  defs["NR_SAMPLES_PER_CHANNEL"] =
    boost::lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  defs["NR_POLARIZATIONS"] =
    boost::lexical_cast<string>(NR_POLARIZATIONS);
  defs["NR_BITS_PER_SAMPLE"] =
    boost::lexical_cast<string>(NR_BITS_PER_SAMPLE);

  return defs;
}


// T is an LCS i*complex type, or complex<float> when #chnl > 1.
// It is the value type of the data input array.
template <typename T>
vector<fcomplex> runTest(const CompileDefinitions& compileDefs,
                         float bandPassFactor)
{
  gpu::Context ctx(stream->getContext());

  boost::scoped_ptr<MultiDimArrayHostBuffer<fcomplex, 4> > outputData;
  boost::scoped_ptr<MultiDimArrayHostBuffer<T,        5> > inputData;

  outputData.reset(
      new MultiDimArrayHostBuffer<fcomplex, 4>(boost::extents
                                               [NR_STATIONS]
                                               [NR_CHANNELS_1 * NR_CHANNELS_2]
                                               [NR_SAMPLES_PER_CHANNEL]
                                               [NR_POLARIZATIONS]
                                               ,
                                               ctx));

  inputData.reset(
      new MultiDimArrayHostBuffer<fcomplex, 5>(boost::extents
                                               [NR_STATIONS]
                                               [NR_POLARIZATIONS]
                                               [NR_CHANNELS_1]
                                               [NR_SAMPLES_PER_CHANNEL]
                                               [NR_CHANNELS_2],
                                               ctx));

  MultiDimArrayHostBuffer<float, 1> bandPassFactors(boost::extents
                                                    [NR_CHANNELS_1 * NR_CHANNELS_2],
                                                   ctx);

  // set inputs
  for (size_t i = 0; i < inputData->num_elements(); i++) {
    inputData->origin()[i].real() = 1.0f;
    inputData->origin()[i].imag() = 1.0f;
  }

  for (size_t i = 0; i < bandPassFactors.num_elements(); i++) {
    bandPassFactors.origin()[i] = bandPassFactor;
  }

  // set output for proper verification later
  for (size_t i = 0; i < outputData->num_elements(); i++) {
    outputData->origin()[i].real() = 42.0f;
    outputData->origin()[i].imag() = 42.0f;
  }

  gpu::Function kfunc(initKernel(ctx, compileDefs));

  runKernel(kfunc, *outputData, *inputData, bandPassFactors);

  // Tests that use this function only check the first and last 2 output floats.
  const unsigned nrResultVals =  NR_STATIONS * NR_CHANNELS_1 * NR_CHANNELS_2*
                                 NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS;

  vector<fcomplex> resultVals(nrResultVals);
  for (unsigned idx = 0; idx < nrResultVals; ++ idx)
    resultVals[idx] = outputData->origin()[idx];
  

  if(true)
  {
    for(size_t idx1 = 0; idx1 < NR_STATIONS; ++ idx1)
      for(size_t idx2 = 0; idx2 < NR_CHANNELS_1 * NR_CHANNELS_2; ++ idx2)
      {
        for(size_t idx3 = 0; idx3 < NR_SAMPLES_PER_CHANNEL; ++ idx3)
        {
          cout << "{" << (*outputData)[idx1][idx2][idx3][0]
               << ", " << (*outputData)[idx1][idx2][idx3][1] 
               << "}" ;
        }
        cout << endl;
      }
  }


  return resultVals;
}

//TEST(BandPass)
void tester()
{
  // ***********************************************************
  // Test if the bandpass correction factor is applied correctly in isolation
  float bandPassFactor = 2.0f;

  CompileDefinitions defs(getDefaultCompileDefinitions());

  // The input samples are all ones. After correction, multiply with 2.
  // The first and the last complex values are retrieved. They should be scaled
  // with the bandPassFactor == 2
  vector<fcomplex> results(runTest<fcomplex>(
                             defs,
                             bandPassFactor)); // bandpass factor

  //CHECK_CLOSE(2.0, results[0].real(), 0.000001);
  //CHECK_CLOSE(2.0, results[0].imag(), 0.000001);
  //CHECK_CLOSE(2.0, results[1].real(), 0.000001);
  //CHECK_CLOSE(2.0, results[1].imag(), 0.000001);
}
//
//TEST(PhaseOffsets)
//{
//  //**********************************************************************
//  // Delaycompensation but only for the phase ofsets:
//  // All computations the drop except the phase ofset of 1,0 which is fed into a
//  // cosisin (or sincos) cosisin(pi) = -1
//  CompileDefinitions defs(getDefaultCompileDefinitions());
//  defs["DELAY_COMPENSATION"] = "1";
//  defs["SUBBAND_BANDWIDTH"] = "1.0";
//
//  vector<fcomplex> results(runTest<fcomplex>(
//                             defs,
//                             1.0f)); // bandpass factor
//
//  CHECK_CLOSE(-1.0, results[0].real(), 0.000001);
//  CHECK_CLOSE(-1.0, results[0].imag(), 0.000001);
//  CHECK_CLOSE(-1.0, results[1].real(), 0.000001);
//  CHECK_CLOSE(-1.0, results[1].imag(), 0.000001);
//}
//
//SUITE(DelayCompensation)
//{
//  TEST(ConstantDelay)
//  {
//    //*************************************************************************
//    // delays: begin 1, end 1; no phase offset; frequency 1; subband width 1
//    // frequency = subbandFrequency - .5 * SUBBAND_BANDWIDTH
//    //             + channel * (SUBBAND_BANDWIDTH / NR_CHANNELS)
//    //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
//    // cosisin(-3.14159+0 i) == -1
//    CompileDefinitions defs(getDefaultCompileDefinitions());
//    defs["DELAY_COMPENSATION"] = "1";
//    defs["SUBBAND_BANDWIDTH"] = "1.0";
//
//    vector<fcomplex> results(runTest<fcomplex>(
//                               defs,
//                               1.0f)); // bandpass factor
//
//    CHECK_CLOSE(-1.0, results[0].real(), 0.000001);
//    CHECK_CLOSE(-1.0, results[0].imag(), 0.000001);
//
//    // For verification: for the following vals, the kernel computes:
//    // major: offset within block of 16 samples
//    // frequency = 1.0 - 0.5*1.0 + (0 + 15) * (1.0 / 16) = 0.5 + 15/16 = 1.4375
//    // phiBegin = -2.0 * 3.141593 * delayAtBegin = -6.283185 * 1.0 = -6.283185
//    // deltaPhi = (phiEnd - phiBegin) / 64 = 0
//    // myPhiBegin = (-6.283185 + major * deltaPhi) * frequency + phaseOffset
//    //            = (-6.283185 + 0.0) * 1.4375 + 0.0 = -9.032079
//    // myPhiDelta = 16 (= time step) * deltaPhi * frequency = 0
//    // vX = ( cos(myPhiBegin.x), sin(myPhiBegin.x) ) = (-0.923880, -0.382683)
//    // vY = idem (as delays begin == delays end)
//    // dvX = ( cos(myPhiDelta.x), sin(myPhiDelta.x) ) = (1, 0)
//    // dvY = idem
//    // (vX, vY) *= weight (*1.0)
//    // sampleX = sampleY = (1.0, 1.0)
//    // After 64/16 rounds, (vX, vY) have been updated 64/16 times with
//    // (dvX, dvY).
//    // In this case, (dvX, dvY) stays (1, 0), so for the last sample, we get:
//    // sampleY = cmul(sampleY, vY) = -0.923880 - -0.382683 = -0.541196 (real)
//    //                             = -0.923880 + -0.382683 = -1.306563 (imag)
//
//    CHECK_CLOSE(-0.541196, results[1].real(), 0.000001);
//    CHECK_CLOSE(-1.306563, results[1].imag(), 0.000001);
//  }
//
//  TEST(SlopedDelay)
//  {
//    //*************************************************************************
//    // delays: begin 1, end 0; no phase offset; frequency 1; subband width 1;
//    // all (complex) input samples are set to (1.0, 1.0) in runTest().
//    //
//    // timeStep  = 16 (hard-coded)
//    // channel   = 0
//    // frequency = subbandFrequency - .5 * SUBBAND_BANDWIDTH
//    //             + channel * (SUBBAND_BANDWIDTH / NR_CHANNELS)
//    // phiBegin  = -2.0 * PI * delayAtBegin  = -6.283185 * 1.0 = -6.283185
//    // phiEnd    = -2.0 * PI * delayAfterEnd = -6.283185 * 0.0 =  0.0
//    // deltaPhi  = (phiEnd - phiBegin) / (NR_SAMPLES_PER_CHANNEL)
//    //           = (0.0 - -6.283135) / 64 = 0.0981748
//    //
//    // For result[0]:
//    // major = 0
//    // frequency  = 1.0 - 0.5 * 1.0 + (0 + 0) * (1.0 / 16) = 1 - 0.5 + 0 = 0.5
//    // myPhiBegin = (phiBegin + major * deltaPhi) * frequency + phaseOffset
//    //            = (-6.283185 + 0.0 * 0.0981748) * 0.5 + 0.0 = -3.141593
//    // myPhiDelta = timeStep * deltaPhi * frequency
//    //            = 16 * 0.0981748 * 0.5 = 0.785398
//    // vX =  vY   = (cos(myPhiBegin) + sin(myPhiBegin)j)
//    //            = (cos(-3.141593) + sin(-3.141593)j) = (-1 + 0j)
//    // dvX = dvY  = (cos(myPhiDelta) + sin(myPhiDelta)j)
//    //            = (cos(0.785398) + sin(0.785398)j) = (0.707107 + 0.707107j)
//    // sample     = sample * (cos(myPhiBegin) + sin(myPhiBegin)j)
//    //            = (1 + j) * (-1, 0j) = (-1, -j)
//    //
//    // For result[1]:
//    // major = 15
//    // frequency  = 1.0 - 0.5 * 1.0 + (0 + 15) * (1.0 / 16)
//    //            = 0.5 + 15/16 = 1.4375
//    // myPhiBegin = (phiBegin + major * deltaPhi) * frequency + phaseOffset
//    //            = (-6.283185 + 15 * 0.0981748) * 1.4375 + 0.0 = -6.915185
//    // myPhiDelta = timeStep * deltaPhi * frequency
//    //            = 16 * 0.0981748 * 1.4375 = 2.258020
//    // vX  = vY   = (cos(myPhiBegin) + sin(myPhiBegin)j) =
//    //            = (cos(-6.915185) + sin(-6.915185)j) = (0.806848 + -0.590760j)
//    // dvX = dvY  = (cos(myPhiDelta, sin(myPhiDelta))
//    //            = (cos(2.258020) + sin(2.258020)j) = (-0.634393 + 0.773010j)
//    //   After ((NR_SAMPLES_PER_CHANNEL - 1) / timeStep) rounds, we have
//    //   applied 63 / 16 = 3 times a phase rotation
//    // myPhiEnd   = myPhiBegin + 3 * myPhiDelta
//    // sample     = sample * (cos(myPhiEnd) + sin(myPhiEnd)j)
//    //            = (1, j) * (0.990058 + -0.140658j) = (1.130716 + 0.849400j)
//
//    CompileDefinitions defs(getDefaultCompileDefinitions());
//    defs["DELAY_COMPENSATION"] = "1";
//    defs["SUBBAND_BANDWIDTH"] = "1.0";
//
//    vector<fcomplex> results(runTest<fcomplex>(
//                               defs,
//                               1.0f)); // bandpass factor
//
//    CHECK_CLOSE(-1.0,     results[0].real(), 0.000001);
//    CHECK_CLOSE(-1.0,     results[0].imag(), 0.000001);
//    CHECK_CLOSE(1.130716, results[1].real(), 0.000001);
//    CHECK_CLOSE(0.849400, results[1].imag(), 0.000001);
//  }
//}
//
//TEST(AllAtOnce)
//{
//  //**************************************************************************
//  // delays: begin 1, end 0; phase offset 1 rad.; frequency: 1;
//  // subband width: 1; band-pass factor: 2
//  //
//  // timeStep  = 16 (hard-coded)
//  // channel   = 0
//  // frequency = subbandFrequency - .5 * SUBBAND_BANDWIDTH
//  //             + channel * (SUBBAND_BANDWIDTH / NR_CHANNELS)
//  // phiBegin  = -2.0 * PI * delayAtBegin  = -6.283185 * 1.0 = -6.283185
//  // phiEnd    = -2.0 * PI * delayAfterEnd = -6.283185 * 0.0 =  0.0
//  // deltaPhi  = (phiEnd - phiBegin) / (NR_SAMPLES_PER_CHANNEL)
//  //           = (0.0 - -6.283135) / 64 = 0.0981748
//  //
//  // For result[0]:
//  // major = 0
//  // frequency  = 1.0 - 0.5 * 1.0 + (0 + 0) * (1.0 / 16) = 1 - 0.5 + 0 = 0.5
//  // myPhiBegin = (phiBegin + major * deltaPhi) * frequency + phaseOffset
//  //            = (-6.283185 + 0.0 * 0.0981748) * 0.5 + 1.0 = -2.141593
//  // myPhiDelta = timeStep * deltaPhi * frequency
//  //            = 16 * 0.0981748 * 0.5 = 0.785398
//  // vX =  vY   = (cos(myPhiBegin) + sin(myPhiBegin))
//  //            = (cos(-2.141593) + sin(-2.141593)j) = (-0.540302 + -0.841471j)
//  // dvX = dvY  = (cos(myPhiDelta) + sin(myPhiDelta))
//  //            = (cos(0.785398) + sin(0.785398)j) = (0.707107 + 0.707107j)
//  // sample     = sample * weight * (cos(myPhiBegin) + sin(myPhiBegin)j)
//  //            = (1, j) * 2 * (-0.540302 + -0.841471j) =
//  //
//  // For result[1]:
//  // major = 15
//  // frequency  = 1.0 - 0.5 * 1.0 + (0 + 15) * (1.0 / 16)
//  //            = 0.5 + 15/16 = 1.4375
//  // myPhiBegin = (phiBegin + major * deltaPhi) * frequency + phaseOffset
//  //            = (-6.283185 + 15 * 0.0981748) * 1.4375 + 1.0 = -5.915185
//  // myPhiDelta = timeStep * deltaPhi * frequency
//  //            = 16 * 0.0981748 * 1.4375 = 2.258020
//  // vX  = vY   = (cos(myPhiBegin) + sin(myPhiBegin)j) =
//  //            = (cos(-5.915185), sin(-5.915185)j) = (0.933049 + 0.359750j)
//  // dvX = dvY  = (cos(myPhiDelta + sin(myPhiDelta)j)
//  //            = (cos(2.258020) + sin(2.258020)j) = (-0.634393 + 0.773010j)
//  //   After ((NR_SAMPLES_PER_CHANNEL - 1) / timeStep) rounds, we have
//  //   applied 63 / 16 = 3 times a phase rotation
//  // myPhiEnd   = myPhiBegin + 3 * myPhiDelta = 0.858874
//  // sample     = sample * weight * (cos(myPhiEnd) + sin(myPhiEnd)j)
//  //            = (1 + j) * 2 * (0.653291 + 0.757107j) = (-0.207633 + 2.820796j)
//
//  CompileDefinitions defs(getDefaultCompileDefinitions());
//  defs["DELAY_COMPENSATION"] = "1";
//  defs["SUBBAND_BANDWIDTH"] = "1.0";
//
//  vector<fcomplex> results(runTest<fcomplex>(
//                             defs,
//                             2.0f)); // bandpass factor (weights == 2)
//
//  CHECK_CLOSE( 0.602337, results[0].real(), 0.000001);
//  CHECK_CLOSE(-2.763547, results[0].imag(), 0.000001);
//  CHECK_CLOSE(-0.207633, results[1].real(), 0.000001);
//  CHECK_CLOSE( 2.820796, results[1].imag(), 0.000001);
//}



gpu::Stream initDevice()
{
  // Set up device (GPU) environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " GPU devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    exit(3); // test skipped
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream cuStream(ctx);

  return cuStream;
}

int main()
{
  INIT_LOGGER("tDelayAndBandPass");

  // init global(s): device, context/stream.
  gpu::Stream strm(initDevice());
  stream = &strm;

  tester();

  //int exitStatus = UnitTest::RunAllTests();
  //return exitStatus > 0 ? 1 : 0;
}


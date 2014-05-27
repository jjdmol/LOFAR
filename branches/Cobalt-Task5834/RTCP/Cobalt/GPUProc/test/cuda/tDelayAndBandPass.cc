//# tDelayAndBandPass.cc: test delay and bandpass CUDA kernel
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
const unsigned NR_CHANNELS = 16;
const unsigned NR_SAMPLES_PER_CHANNEL = 64;
const unsigned NR_SAMPLES_PER_SUBBAND = NR_SAMPLES_PER_CHANNEL * NR_CHANNELS;
const unsigned NR_BITS_PER_SAMPLE = 8;
const unsigned NR_POLARIZATIONS = 2;

const unsigned NR_SAPS = 8;
const double SUBBAND_BANDWIDTH = 0.0 * NR_CHANNELS;
const bool BANDPASS_CORRECTION = true;
const bool DELAY_COMPENSATION = false;
const bool DO_TRANSPOSE = true;


// Initialize input AND output before calling runKernel().
// We copy both to the GPU, to make sure the final output is really from the
// kernel.  T is an LCS i*complex type, or complex<float> when #chnl > 1.
template <typename T>
void runKernel(gpu::Function kfunc,
               MultiDimArrayHostBuffer<fcomplex, 4> &outputData,
               MultiDimArrayHostBuffer<T,        4> &inputData,
               MultiDimArrayHostBuffer<double,   3> &delaysAtBegin,
               MultiDimArrayHostBuffer<double,   3> &delaysAfterEnd,
               MultiDimArrayHostBuffer<double,   2> &phase0s,
               MultiDimArrayHostBuffer<float,    1> &bandPassFactors,
               double subbandFrequency,
               unsigned beam)
{
  gpu::Context ctx(stream->getContext());

  gpu::DeviceMemory devOutput         (ctx, outputData.size());
  gpu::DeviceMemory devInput          (ctx, inputData.size());
  gpu::DeviceMemory devDelaysAtBegin  (ctx, delaysAtBegin.size());
  gpu::DeviceMemory devDelaysAfterEnd (ctx, delaysAfterEnd.size());
  gpu::DeviceMemory devPhase0s   (ctx, phase0s.size());
  gpu::DeviceMemory devBandPassFactors(ctx, bandPassFactors.size());

  kfunc.setArg(0, devOutput);
  kfunc.setArg(1, devInput);
  kfunc.setArg(2, subbandFrequency);
  kfunc.setArg(3, beam);
  kfunc.setArg(4, devDelaysAtBegin);
  kfunc.setArg(5, devDelaysAfterEnd);
  kfunc.setArg(6, devPhase0s);
  kfunc.setArg(7, devBandPassFactors);

  gpu::Grid globalWorkSize(1,
                           NR_CHANNELS == 1 ? 1 : NR_CHANNELS / 16,
                           NR_STATIONS);
  gpu::Block localWorkSize(256, 1, 1);

  // Overwrite devOutput, so result verification is more reliable.
  stream->writeBuffer(devOutput,          outputData);
  stream->writeBuffer(devInput,           inputData);
  stream->writeBuffer(devDelaysAtBegin,   delaysAtBegin);
  stream->writeBuffer(devDelaysAfterEnd,  delaysAfterEnd);
  stream->writeBuffer(devPhase0s,    phase0s);
  stream->writeBuffer(devBandPassFactors, bandPassFactors);

  stream->launchKernel(kfunc, globalWorkSize, localWorkSize);
  stream->readBuffer(outputData, devOutput);
  stream->synchronize(); // wait until transfer completes
}

gpu::Function initKernel(gpu::Context ctx, const CompileDefinitions& defs)
{
  // Compile to ptx. Copies the kernel to the current dir
  // (also the complex header, needed for compilation).
  string kernelPath("DelayAndBandPass.cu");
  CompileFlags flags(defaultCompileFlags());
  vector<gpu::Device> devices(1, gpu::Device(0));
  string ptx(createPTX(kernelPath, defs, flags, devices));
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  gpu::Function kfunc(module, "applyDelaysAndCorrectBandPass");

  return kfunc;
}

CompileDefinitions getDefaultCompileDefinitions()
{
  CompileDefinitions defs;

  defs["NR_STATIONS"] =
    boost::lexical_cast<string>(NR_STATIONS);
  defs["NR_CHANNELS"] =
    boost::lexical_cast<string>(NR_CHANNELS);
  defs["NR_SAMPLES_PER_CHANNEL"] =
    boost::lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  defs["NR_SAMPLES_PER_SUBBAND"] =
    boost::lexical_cast<string>(NR_SAMPLES_PER_SUBBAND);
  defs["NR_BITS_PER_SAMPLE"] =
    boost::lexical_cast<string>(NR_BITS_PER_SAMPLE);
  defs["NR_POLARIZATIONS"] =
    boost::lexical_cast<string>(NR_POLARIZATIONS);
  defs["NR_SAPS"] =
    boost::lexical_cast<string>(NR_SAPS);
  defs["SUBBAND_BANDWIDTH"] =
    boost::lexical_cast<string>(SUBBAND_BANDWIDTH);

  if (BANDPASS_CORRECTION)
    defs["BANDPASS_CORRECTION"] = "1";
  if (DELAY_COMPENSATION)
    defs["DELAY_COMPENSATION"] = "1";
  if (DO_TRANSPOSE)
    defs["DO_TRANSPOSE"] = "1";

  return defs;
}

// T is an LCS i*complex type, or complex<float> when #chnl > 1.
// It is the value type of the data input array.
template <typename T>
vector<fcomplex> runTest(const CompileDefinitions& compileDefs,
                         double subbandFrequency,
                         unsigned beam,
                         double delayBegin,
                         double delayEnd,
                         double phase0,
                         float bandPassFactor)
{
  gpu::Context ctx(stream->getContext());

  boost::scoped_ptr<MultiDimArrayHostBuffer<fcomplex, 4> > outputData;
  boost::scoped_ptr<MultiDimArrayHostBuffer<T,        4> > inputData;

  if(compileDefs.find("DO_TRANSPOSE") != compileDefs.end())
    outputData.reset(
      new MultiDimArrayHostBuffer<fcomplex, 4>(boost::extents
                                               [NR_STATIONS]
                                               [NR_SAMPLES_PER_CHANNEL]
                                               [NR_CHANNELS]
                                               [NR_POLARIZATIONS],
                                               ctx));
  else // no transpose
    outputData.reset(
      new MultiDimArrayHostBuffer<fcomplex, 4>(boost::extents
                                               [NR_STATIONS]
                                               [NR_POLARIZATIONS]
                                               [NR_SAMPLES_PER_CHANNEL]
                                               [NR_CHANNELS],
                                               ctx));

  CompileDefinitions::const_iterator cit;
  ASSERT((cit = compileDefs.find("NR_CHANNELS")) != compileDefs.end());

  unsigned nchnl = boost::lexical_cast<unsigned>(cit->second);
  if (nchnl == 1) // integer input data (FIR+FFT skipped)
    inputData.reset(
      new MultiDimArrayHostBuffer<T, 4>(boost::extents
                                        [NR_STATIONS]
                                        [NR_SAMPLES_PER_CHANNEL]
                                        [NR_CHANNELS]
                                        [NR_POLARIZATIONS],
                                        ctx));
  else // specify fcomplex, which T must be too in this case
    inputData.reset(
      new MultiDimArrayHostBuffer<fcomplex, 4>(boost::extents
                                               [NR_STATIONS]
                                               [NR_POLARIZATIONS]
                                               [NR_SAMPLES_PER_CHANNEL]
                                               [NR_CHANNELS],
                                               ctx));

  MultiDimArrayHostBuffer<double, 3> delaysAtBegin(boost::extents
                                                   [NR_SAPS]
                                                   [NR_STATIONS]
                                                   [NR_POLARIZATIONS],
                                                   ctx);
  MultiDimArrayHostBuffer<double, 3> delaysAfterEnd(boost::extents
                                                    [NR_SAPS]
                                                    [NR_STATIONS]
                                                    [NR_POLARIZATIONS],
                                                    ctx);
  MultiDimArrayHostBuffer<double, 2> phase0s(boost::extents
                                                  [NR_STATIONS]
                                                  [NR_POLARIZATIONS],
                                                  ctx);
  MultiDimArrayHostBuffer<float, 1> bandPassFactors(boost::extents
                                                    [NR_CHANNELS],
                                                    ctx);

  // set inputs
  for (size_t i = 0; i < inputData->num_elements(); i++) {
    inputData->origin()[i].real() = 1.0f;
    inputData->origin()[i].imag() = 1.0f;
  }
  for (size_t i = 0; i < delaysAtBegin.num_elements(); i++) {
    delaysAtBegin.origin()[i] = delayBegin;
  }
  for (size_t i = 0; i < delaysAfterEnd.num_elements(); i++) {
    delaysAfterEnd.origin()[i] = delayEnd;
  }
  for (size_t i = 0; i < phase0s.num_elements(); i++) {
    phase0s.origin()[i] = phase0;
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

  runKernel(kfunc, *outputData, *inputData,
            delaysAtBegin, delaysAfterEnd, phase0s, bandPassFactors,
            subbandFrequency, beam);

  // Tests that use this function only check the first and last 2 output floats.
  const unsigned nrResultVals = 2;
  ASSERT(outputData->num_elements() >=
         nrResultVals * sizeof(fcomplex) / sizeof(float));
  vector<fcomplex> resultVals(nrResultVals);
  resultVals[0] = outputData->origin()[0];
  resultVals[1] = outputData->origin()[outputData->num_elements() - 1];

  return resultVals;
}

TEST(BandPass)
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
                             0.0, // sb freq
                             0U,  // beam
                             0.0, // delays begin
                             0.0, // delays end
                             0.0, // phase offsets
                             bandPassFactor)); // bandpass factor

  CHECK_CLOSE(2.0, results[0].real(), 0.000001);
  CHECK_CLOSE(2.0, results[0].imag(), 0.000001);
  CHECK_CLOSE(2.0, results[1].real(), 0.000001);
  CHECK_CLOSE(2.0, results[1].imag(), 0.000001);
}

TEST(Phase0s)
{
  //**********************************************************************
  // Delaycompensation but only for the phase ofsets:
  // All computations the drop except the phase ofset of 1,0 which is fed into a
  // cosisin (or sincos) cosisin(pi) = -1
  CompileDefinitions defs(getDefaultCompileDefinitions());
  defs["DELAY_COMPENSATION"] = "1";
  defs["SUBBAND_BANDWIDTH"] = "1.0";

  vector<fcomplex> results(runTest<fcomplex>(
                             defs,
                             1.0,    // sb freq
                             0U,     // beam
                             0.0,    // delays begin
                             0.0,    // delays end
                             -M_PI,  // phase offsets
                             1.0f)); // bandpass factor

  CHECK_CLOSE(-1.0, results[0].real(), 0.000001);
  CHECK_CLOSE(-1.0, results[0].imag(), 0.000001);
  CHECK_CLOSE(-1.0, results[1].real(), 0.000001);
  CHECK_CLOSE(-1.0, results[1].imag(), 0.000001);
}

SUITE(DelayCompensation)
{
  TEST(ConstantDelay)
  {
    //*************************************************************************
    // delays: begin 1, end 1; no phase offset; frequency 1; subband width 1
    // frequency = subbandFrequency - .5 * SUBBAND_BANDWIDTH
    //             + channel * (SUBBAND_BANDWIDTH / NR_CHANNELS)
    //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
    // cosisin(-3.14159+0 i) == -1
    CompileDefinitions defs(getDefaultCompileDefinitions());
    defs["DELAY_COMPENSATION"] = "1";
    defs["SUBBAND_BANDWIDTH"] = "1.0";

    vector<fcomplex> results(runTest<fcomplex>(
                               defs,
                               1.0,    // sb freq
                               0U,     // beam
                               1.0,    // delays begin
                               1.0,    // delays end
                               0.0,    // phase offsets
                               1.0f)); // bandpass factor

    // For verification: for the following vals, the kernel computes for
    // channel 0:
    //
    // frequency   = subbandFrequency - 0.5 * subbandBandwidth + channel  * channelBandwidth
    //             = 1.0              - 0.5 * 1.0              + 0        * (1.0 / 16)
    //             = 0.5
    // phiAtBegin  = -2.0 * M_PI         * frequency * delayAtBegin - phase0
    //             = -2.0 * 3.1415926536 * 0.5       * 1.0          - 0.0
    //             = -M_PI
    // phiAfterEnd = -2.0 * M_PI         * frequency * delayAfterEnd - phase0
    //             = -2.0 * 3.1415926536 * 0.5       * 1.0           - 0.0
    //             = -M_PI
    //
    // For all time steps t = [0..1):
    //   phi = phiAtBegin * (1.0 - t) + phiAfterEnd * t
    //        = -M_PI
    //    
    //   cos(phi) = -1.0
    //   sin(phi) = -0.0
    //
    // In:
    //   sampleX = sampleY = 1.0 + 1.0i
    // Out:
    //   sampleX *= cos(phi) + i*sin(phi)
    //            = -1.0 + -1.0i

    CHECK_CLOSE(-1.0, results[0].real(), 0.000001);
    CHECK_CLOSE(-1.0, results[0].imag(), 0.000001);

    // For verification: for the following vals, the kernel computes for
    // channel 15:
    //
    // frequency   = subbandFrequency - 0.5 * subbandBandwidth + channel  * channelBandwidth
    //             = 1.0              - 0.5 * 1.0              + (0 + 15) * (1.0 / 16)
    //             = 0.5 + 15/16 = 1.4375
    // phiAtBegin  = -2.0 * M_PI         * frequency * delayAtBegin - phase0
    //             = -2.0 * 3.1415926536 * 1.4375    * 1.0          - 0.0
    //             = -9.0320788791
    // phiAfterEnd = -2.0 * M_PI         * frequency * delayAfterEnd - phase0
    //             = -2.0 * 3.1415926536 * 1.4375    * 1.0           - 0.0
    //             = -9.0320788791
    //
    // For all time steps t = [0..1):
    //   phi = phiAtBegin * (1.0 - t) + phiAfterEnd * t
    //        = -9.0320788791
    //    
    //   cos(phi) = -0.9238795325
    //   sin(phi) = -0.3826834324
    //
    // In:
    //   sampleX = sampleY = 1.0 + 1.0i
    // Out:
    //   sampleY *= cos(phi) + i*sin(phi)
    //            = -0.5411961001 + -1.3065629649i

    CHECK_CLOSE(-0.5411961001, results[1].real(), 0.000001);
    CHECK_CLOSE(-1.3065629649, results[1].imag(), 0.000001);
  }

  TEST(SlopedDelay)
  {
    //*************************************************************************
    // delays: begin 1, end 0; no phase offset; frequency 1; subband width 1;
    // all (complex) input samples are set to (1.0, 1.0) in runTest().
    //
    //
    // For result[1]:
    // major = 15
    // frequency  = 1.0 - 0.5 * 1.0 + (0 + 15) * (1.0 / 16)
    //            = 0.5 + 15/16 = 1.4375
    // myPhiBegin = (phiBegin + major * deltaPhi) * frequency + phase0
    //            = (-6.283185 + 15 * 0.0981748) * 1.4375 + 0.0 = -6.915185
    // myPhiDelta = timeStep * deltaPhi * frequency
    //            = 16 * 0.0981748 * 1.4375 = 2.258020
    // vX  = vY   = (cos(myPhiBegin) + sin(myPhiBegin)j) =
    //            = (cos(-6.915185) + sin(-6.915185)j) = (0.806848 + -0.590760j)
    // dvX = dvY  = (cos(myPhiDelta, sin(myPhiDelta))
    //            = (cos(2.258020) + sin(2.258020)j) = (-0.634393 + 0.773010j)
    //   After ((NR_SAMPLES_PER_CHANNEL - 1) / timeStep) rounds, we have
    //   applied 63 / 16 = 3 times a phase rotation
    // myPhiEnd   = myPhiBegin + 3 * myPhiDelta
    // sample     = sample * (cos(myPhiEnd) + sin(myPhiEnd)j)
    //            = (1, j) * (0.990058 + -0.140658j) = (1.130716 + 0.849400j)

    CompileDefinitions defs(getDefaultCompileDefinitions());
    defs["DELAY_COMPENSATION"] = "1";
    defs["SUBBAND_BANDWIDTH"] = "1.0";

    vector<fcomplex> results(runTest<fcomplex>(
                               defs,
                               1.0,    // sb freq
                               0U,     // beam
                               1.0,    // delays begin
                               0.0,    // delays end
                               0.0,    // phase offsets
                               1.0f)); // bandpass factor

    // For verification: for the following vals, the kernel computes for
    // channel 0:
    //
    // frequency   = subbandFrequency - 0.5 * subbandBandwidth + channel  * channelBandwidth
    //             = 1.0              - 0.5 * 1.0              + 0        * (1.0 / 16)
    //             = 0.5
    // phiAtBegin  = -2.0 * M_PI         * frequency * delayAtBegin - phase0
    //             = -2.0 * 3.1415926536 * 0.5       * 1.0          - 0.0
    //             = -M_PI
    // phiAfterEnd = -2.0 * M_PI         * frequency * delayAfterEnd - phase0
    //             = -2.0 * 3.1415926536 * 0.5       * 0.0           - 0.0
    //             = 0.0
    //
    // For time step t = 0:
    //   phi = phiAtBegin * (1.0 - t) + phiAfterEnd * t
    //        = -M_PI
    //    
    //   cos(phi) = -1.0
    //   sin(phi) = -0.0
    //
    // In:
    //   sampleX = sampleY = 1.0 + 1.0i
    // Out:
    //   sampleX *= cos(phi) + i*sin(phi)
    //            = -1.0 + -1.0i

    CHECK_CLOSE(-1.0,     results[0].real(), 0.000001);
    CHECK_CLOSE(-1.0,     results[0].imag(), 0.000001);

    // For verification: for the following vals, the kernel computes for
    // channel 15:
    //
    // frequency   = subbandFrequency - 0.5 * subbandBandwidth + channel  * channelBandwidth
    //             = 1.0              - 0.5 * 1.0              + (0 + 15) * (1.0 / 16)
    //             = 0.5 + 15/16 = 1.4375
    // phiAtBegin  = -2.0 * M_PI         * frequency * delayAtBegin - phase0
    //             = -2.0 * 3.1415926536 * 1.4375    * 1.0          - 0.0
    //             = -9.0320788791
    // phiAfterEnd = -2.0 * M_PI         * frequency * delayAfterEnd - phase0
    //             = -2.0 * 3.1415926536 * 1.4375    * 0.0           - 0.0
    //             = 0.0
    //
    // For time step t = (NR_SAMPLES_PER_CHANNEL-1) / NR_SAMPLES_PER_CHANNEL
    //                 = 63/64
    //   phi = phiAtBegin * (1.0 - t) + phiAfterEnd * t
    //       = -9.0320788791 * 1/64   + 0.0
    //       = -0.1411262325
    //
    //   cos(phi) = -0.9900582103
    //   sin(phi) = -0.1406582393
    //
    // In:
    //   sampleX = sampleY = 1.0 + 1.0i
    // Out:
    //   sampleY *= cos(phi) + i*sin(phi)
    //            = 1.1307164496 + 0.8493999709i

    CHECK_CLOSE(1.1307164496, results[1].real(), 0.000001);
    CHECK_CLOSE(0.8493999709, results[1].imag(), 0.000001);
  }
}

TEST(AllAtOnce)
{
  //**************************************************************************
  // delays: begin 1, end 0; phase offset -1 rad.; frequency: 1;
  // subband width: 1; band-pass factor: 2

  CompileDefinitions defs(getDefaultCompileDefinitions());
  defs["DELAY_COMPENSATION"] = "1";
  defs["SUBBAND_BANDWIDTH"] = "1.0";

  vector<fcomplex> results(runTest<fcomplex>(
                             defs,
                             1.0,    // sb freq
                             0U,     // beam
                             1.0,    // delays begin
                             0.0,    // delays end
                             -1.0,   // phase offsets (-1 rad)
                             2.0f)); // bandpass factor (weights == 2)

  // For verification: for the following vals, the kernel computes for
  // channel 0:
  //
  // frequency   = subbandFrequency - 0.5 * subbandBandwidth + channel  * channelBandwidth
  //             = 1.0              - 0.5 * 1.0              + 0        * (1.0 / 16)
  //             = 0.5
  // phiAtBegin  = -2.0 * M_PI         * frequency * delayAtBegin - phase0
  //             = -2.0 * 3.1415926536 * 0.5       * 1.0          + 1.0
  //             = -2.1415926536
  // phiAfterEnd = -2.0 * M_PI         * frequency * delayAfterEnd - phase0
  //             = -2.0 * 3.1415926536 * 0.5       * 0.0           + 1.0
  //             = 1.0
  //
  // For time step t = 0:
  //   phi = phiAtBegin * (1.0 - t) + phiAfterEnd * t
  //        = -2.1415926536
  //    
  //   cos(phi) = -0.5403023059
  //   sin(phi) = -0.8414709845
  //
  //   weight = 2.0
  //
  // In:
  //   sampleX = sampleY = 1.0 + 1.0i
  // Out:
  //   sampleX *= weight * (cos(phi) + i*sin(phi))
  //           = 0.60233735788 + -2.7635465814i

  CHECK_CLOSE(0.60233735788, results[0].real(), 0.000001);
  CHECK_CLOSE(-2.7635465814, results[0].imag(), 0.000001);

  // For verification: for the following vals, the kernel computes for
  // channel 15:
  //
  // frequency   = subbandFrequency - 0.5 * subbandBandwidth + channel  * channelBandwidth
  //             = 1.0              - 0.5 * 1.0              + (0 + 15) * (1.0 / 16)
  //             = 0.5 + 15/16 = 1.4375
  // phiAtBegin  = -2.0 * M_PI         * frequency * delayAtBegin - phase0
  //             = -2.0 * 3.1415926536 * 1.4375    * 1.0          + 1.0
  //             = -8.0320788791
  // phiAfterEnd = -2.0 * M_PI         * frequency * delayAfterEnd - phase0
  //             = -2.0 * 3.1415926536 * 1.4375    * 0.0           + 1.0
  //             = 1.0
  //
  // For time step t = (NR_SAMPLES_PER_CHANNEL-1) / NR_SAMPLES_PER_CHANNEL
  //                 = 63/64
  //   phi = phiAtBegin    * (1.0 - t) + phiAfterEnd * t
  //       = -8.0320788791 * 1/64      + 1.0         * 63/64
  //       = 0.8588737675
  //
  //   cos(phi) = 0.6532905611
  //   sin(phi) = 0.7571072862
  //
  //   weight = 2.0
  //
  // In:
  //   sampleX = sampleY = 1.0 + 1.0i
  // Out:
  //   sampleX *= weight * (cos(phi) + i*sin(phi))
  //            = -0.2076334501 + 2.8207956946i

  CHECK_CLOSE(-0.2076334501, results[1].real(), 0.000001);
  CHECK_CLOSE(2.8207956946 , results[1].imag(), 0.000001);
}



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

  return UnitTest::RunAllTests() > 0;
}


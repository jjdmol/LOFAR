//# tCoherentStokesBeamFormerSubbandProcProcessSb: test of bf coh stokes sb proc
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

#include <complex>
#include <cmath>
#include <iomanip>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <CoInterface/fpequals.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/SubbandProcs/SubbandProc.h>
#include <GPUProc/SubbandProcs/KernelFactories.h>

#include "../Kernels/KernelTestHelpers.h"

using namespace std;
using namespace LOFAR::Cobalt;
using namespace LOFAR::TYPES;

template<typename T> T inputSignal(size_t t)
{
  size_t nrBits = sizeof(T) / 2 * 8;
  double amp = (1 << (nrBits - 1)) - 1;
#if 1  // Toggle to experiment with pulse type input
  // Sine wave
  // double freq = 1.0 / 4.0; // in samples
  double freq = (2 * 64.0 + 17.0) / 4096.0; // in samples
  double angle = (double)t * 2.0 * M_PI * freq;
  double s = ::sin(angle);
  double c = ::cos(angle);
  return T(::round(amp * c), ::round(amp * s));
#else
  // Pulse train
  if (t % (2 * 64 + 17) == 0) return T(amp);
  else return T(0);
#endif
}

int main(/*int argc, char *argv[]*/) {
  const char *testName = "tCoherentStokesBeamFormerSubbandProcProcessSb";
  INIT_LOGGER(testName);

  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    return 3;
  }

  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);

  Parset ps("tCoherentStokesBeamFormerSubbandProcProcessSb.parset");

//  Parset ps;
  KernelParameters params;
  // override the faults
/*
  params.nStation = 5;
  params.nrChannels = 4096;
  params.nTimeBlocks = 16;
  params.nrTabs = 2;
  params.stokesType = "I";
  params.nrDelayCompensationChannels = 64;
  params.nrChannelsPerSubband = 1;

  parseCommandlineParameters(argc, argv, ps, params, testName);
*/

  // Input array sizes
  const size_t nrBeams = ps.settings.SAPs.size();
  const size_t nrStations = ps.settings.antennaFields.size();
  const size_t nrPolarisations = ps.settings.nrPolarisations;
  const size_t maxNrTABsPerSAP = ps.settings.beamFormer.maxNrTABsPerSAP();
  const size_t nrSamplesPerSubband = ps.settings.blockSize;
  const size_t nrBitsPerSample = ps.settings.nrBitsPerSample;
  const size_t nrBytesPerComplexSample = ps.nrBytesPerComplexSample();

  const unsigned fft1Size = ps.settings.beamFormer.nrDelayCompensationChannels;
  const unsigned fft2Size = ps.settings.beamFormer.nrHighResolutionChannels / fft1Size;

  // We only support 8-bit or 16-bit input samples
  ASSERT(nrBitsPerSample == 8 || nrBitsPerSample == 16);

  // TODO: Amplitude is now calculated at two places. Dangerous!
  const size_t amplitude = (1 << (nrBitsPerSample - 1)) - 1;
  const size_t scaleFactor = nrBitsPerSample == 16 ? 1 : 16;

  LOG_INFO_STR(
    "Input info:" <<
    "\n  nrBeams = " << nrBeams <<
    "\n  nrStations = " << nrStations <<
    "\n  nrPolarisations = " << nrPolarisations <<
    "\n  maxNrTABsPerSAP = " << maxNrTABsPerSAP <<
    "\n  nrSamplesPerSubband = " << nrSamplesPerSubband <<
    "\n  nrBitsPerSample = " << nrBitsPerSample <<
    "\n  nrBytesPerComplexSample = " << nrBytesPerComplexSample <<
    "\n  fft1Size = " << fft1Size <<
    "\n  fft2Size = " << fft2Size);

  // Output array sizes
  const size_t nrStokes = ps.settings.beamFormer.coherentSettings.nrStokes;
  const size_t nrChannels = 
    ps.settings.beamFormer.coherentSettings.nrChannels;
  const size_t nrSamples = 
    ps.settings.beamFormer.coherentSettings.nrSamples;

  LOG_INFO_STR(
    "Output info:" <<
    "\n  nrStokes = " << nrStokes <<
    "\n  nrChannels = " << nrChannels <<
    "\n  nrSamples = " << nrSamples <<
    "\n  scaleFactor = " << scaleFactor);

  // Create very simple kernel programs, with predictable output. Skip as much
  // as possible. Nr of channels/sb from the parset is 1, so the PPF will not
  // even run. Parset also has turned of delay compensation and bandpass
  // correction (but that kernel will run to convert int to float and to
  // transform the data order).

  KernelFactories factories(ps);
  SubbandProc bwq(ps, ctx, factories);

  SubbandProcInputData in(
    nrBeams, nrStations, nrPolarisations, maxNrTABsPerSAP, 
    nrSamplesPerSubband, nrBytesPerComplexSample, ctx);

  // Initialize synthetic input to input signal
  for (size_t st = 0; st < nrStations; st++) {
    for (size_t i = 0; i < nrSamplesPerSubband; i++) {
      size_t pol = i % nrPolarisations;
    switch (nrBitsPerSample) {
    case 8:
      reinterpret_cast<i8complex&>(in.inputSamples[st][i][pol][0]) =
        inputSignal<i8complex>(i);
      break;
    case 16:
      reinterpret_cast<i16complex&>(in.inputSamples[st][i][pol][0]) =
        inputSignal<i16complex>(i);
      break;
    default:
      break;
    }
  }
  }

  // Initialize subbands partitioning administration (struct BlockID). We only
  // do the 1st block of whatever.

  // Block number: 0 .. inf
  in.blockID.block = 0;

 // Subband index in the observation: [0, ps.nrSubbands())
  in.blockID.globalSubbandIdx = 0;

  // Subband index for this pipeline/workqueue: [0, subbandIndices.size())
  in.blockID.localSubbandIdx = 0;

  // Subband index for this SubbandProc
  in.blockID.subbandProcSubbandIdx = 0;

  // Initialize delays. We skip delay compensation, but init anyway,
  // so we won't copy uninitialized data to the device.
  for (size_t i = 0; i < in.delaysAtBegin.num_elements(); i++)
    in.delaysAtBegin.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.delaysAfterEnd.num_elements(); i++)
    in.delaysAfterEnd.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.phase0s.num_elements(); i++)
    in.phase0s.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.tabDelays.num_elements(); i++)
    in.tabDelays.get<float>()[i] = 0.0f;

  SubbandProcOutputData out(ps, ctx);

  for (size_t i = 0; i < out.coherentData.num_elements(); i++)
    out.coherentData.get<float>()[i] = 42.0f;

  // Don't bother initializing out.blockID; processSubband() doesn't need it.

  cout << "processSubband()" << endl;
  bwq.processSubband(in, out);
  cout << "processSubband() done" << endl;

  cout << "Output: " << endl;

  // Output verification

  // We can calculate the expected output values, since we're supplying a
  // complex sine/cosine input signal. We only have Stokes-I, so the output
  // should be: (nrStations * amp * scaleFactor * fft1Size * fft2Size) ** 2
  // - amp is set to the maximum possible value for the bit-mode:
  //   i.e. 127 for 8-bit and 32767 for 16-bit mode
  // - scaleFactor is the scaleFactor applied by the IntToFloat kernel. 
  //   It is 16 for 8-bit mode and 1 for 16-bit mode.
  // Hence, each output sample should be (nrStations from parset): 
  // - for 16-bit input: (5 * 32767 * 1 * 64 * 64) ** 2 = 450332475365785600
  // - for 8-bit input: (5 * 127 * 16 * 64 * 64) ** 2 = 1731838187929600

  float outVal = 
    (nrStations * amplitude * scaleFactor * fft1Size * fft2Size) *
    (nrStations * amplitude * scaleFactor * fft1Size * fft2Size);
  cout << "outVal = " << setprecision(12) << outVal << endl;

  // Skip output validation when started with commandline parsed parameters!
  if (!params.parameterParsed)
  {
    cout << "Validating output" << endl;
    for (size_t tab = 0; tab < maxNrTABsPerSAP; tab++)
    for (size_t s = 0; s < nrStokes; s++)
    for (size_t t = 0; t < nrSamples; t++)
    for (size_t c = 0; c < nrChannels; c++)
    {
      ASSERTSTR(fpEquals(out.coherentData[tab][s][t][c], outVal, 1e-4f),
        "out.coherentData[" << tab << "][" << s << "][" << t << "][" << c << "] = " << setprecision(12) <<
        out.coherentData[tab][s][t][c] << "; outVal = " << outVal);
    }
  }
  return 0;
}


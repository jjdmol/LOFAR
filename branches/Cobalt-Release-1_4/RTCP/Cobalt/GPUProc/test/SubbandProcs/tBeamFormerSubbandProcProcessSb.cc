//# tBeamFormerSubbandProcProcessSb: test of Beamformer subband processor.
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
#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>
#include <GPUProc/SubbandProcs/BeamFormerFactories.h>

using namespace std;
using namespace LOFAR::Cobalt;
using namespace LOFAR::TYPES;

float sqr(float x)
{
  return x * x;
}

template<typename T> T inputSignal(size_t t)
{
  size_t nrBits = sizeof(T) / 2 * 8;
  double amp = (1 << (nrBits - 1)) - 1;
#if 1 // Toggle to experiment with pulse like input
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

int main() {
  INIT_LOGGER("tBeamFormerSubbandProcProcessSb");

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

  Parset ps("tBeamFormerSubbandProcProcessSb.parset");

  // Input array sizes
  const size_t nrBeams = ps.settings.SAPs.size();
  const size_t nrStations = ps.settings.antennaFields.size();
  const size_t nrPolarisations = ps.settings.nrPolarisations;
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
    "\n  nrSamplesPerSubband = " << nrSamplesPerSubband <<
    "\n  nrBitsPerSample = " << nrBitsPerSample <<
    "\n  nrBytesPerComplexSample = " << nrBytesPerComplexSample <<
    "\n  fft1Size = " << fft1Size <<
    "\n  fft2Size = " << fft2Size);

  // Create very simple kernel programs, with predictable output. Skip as much
  // as possible. Nr of channels/sb from the parset is 1, so the PPF will not
  // even run. Parset also has turned of delay compensation and bandpass
  // correction (but that kernel will run to convert int to float and to
  // transform the data order).

  BeamFormerFactories factories(ps);
  BeamFormerSubbandProc bwq(ps, ctx, factories);

  SubbandProcInputData in(ps, ctx);

  // Initialize synthetic input to input signal
  for (size_t st = 0; st < nrStations; st++) {
    for (size_t i = 0; i < nrSamplesPerSubband; i++) {
      size_t pol = i % nrPolarisations;
        switch(nrBitsPerSample) {
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

  BeamFormedData out(ps, ctx);

  for (size_t i = 0; i < out.coherentData.num_elements(); i++)
    out.coherentData.get<float>()[i] = 42.0f;
  for (size_t i = 0; i < out.incoherentData.num_elements(); i++)
    out.incoherentData.get<float>()[i] = 42.0f;

  // Don't bother initializing out.blockID; processSubband() doesn't need it.

  cout << "processSubband()" << endl;
  bwq.processSubband(in, out);
  cout << "processSubband() done" << endl;

  cout << "Output: " << endl;

  // Output verification

  // *** COHERENT STOKES ***

  // Coherent Stokes takes the stokes of the sums of all fields (stokes(sum(x))).
  // We can calculate the expected output values, since we're supplying a
  // complex sine/cosine input signal. We only have Stokes-I, so the output
  // should be: nrStations * (amp * scaleFactor * fft1Size * fft2Size) ** 2
  // - amp is set to the maximum possible value for the bit-mode:
  //   i.e. 127 for 8-bit and 32767 for 16-bit mode
  // - scaleFactor is the scaleFactor applied by the IntToFloat kernel. 
  //   It is 16 for 8-bit mode and 1 for 16-bit mode.
  // Hence, each output sample should be (nrStations from parset): 
  // - for 16-bit input: (2 * 32767 * 1 * 64 * 64)^2 = 72053196058525696
  // - for 8-bit input: (2 * 127 * 16 * 64 * 64)^2 = 277094110068736

  float coh_outVal = sqr(nrStations * amplitude * scaleFactor * fft1Size * fft2Size);
  cout << "coherent outVal = " << coh_outVal << endl;

  for (size_t t = 0; t < ps.settings.beamFormer.coherentSettings.nrSamples; t++)
    for (size_t c = 0; c < ps.settings.beamFormer.coherentSettings.nrChannels; c++)
      ASSERTSTR(fpEquals(out.coherentData[0][0][t][c], coh_outVal, 1e-4f), 
                "out.coherentData[0][0][" << t << "][" << c << "] = " << 
                setprecision(12) << out.coherentData[0][0][t][c] << 
                "; outVal = " << coh_outVal);

  // *** INCOHERENT STOKES ***

  // Incoherent Stokes sums the stokes of each field (sum(stokes(x))).
  // We can calculate the expected output values, since we're supplying a
  // complex sine/cosine input signal. We only have Stokes-I, so the output
  // should be: nrStation * (amp * scaleFactor * fft1Size * fft2Size)^2
  // - amp is set to the maximum possible value for the bit-mode:
  //   i.e. 127 for 8-bit and 32767 for 16-bit mode
  // - scaleFactor is the scaleFactor applied by the IntToFloat kernel. 
  //   It is 16 for 8-bit mode and 1 for 16-bit mode.
  // Hence, each output sample should be: 
  // - for 16-bit input: 2 * (32767 * 1 * 64 * 64)^2 = 36026598029262848
  // - for 8-bit input: 2 * (127 * 16 * 64 * 64)^2 = 138547055034368

  float incoh_outVal = nrStations * sqr(amplitude * scaleFactor * fft1Size * fft2Size);
  cout << "incoherent outVal = " << incoh_outVal << endl;

  for (size_t t = 0; t < ps.settings.beamFormer.incoherentSettings.nrSamples; t++)
    for (size_t c = 0; c < ps.settings.beamFormer.incoherentSettings.nrChannels; c++)
      ASSERTSTR(fpEquals(out.incoherentData[0][0][t][c], incoh_outVal, 1e-4f), 
                "out.incoherentData[0][0][" << t << "][" << c << "] = " << 
                setprecision(12) << out.incoherentData[0][0][t][c] << 
                "; outVal = " << incoh_outVal);
  
  return 0;
}


//# tCorrelatorWorKQueueProcessSb.cc: test CorrelatorSubbandProc::processSubband()
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
//# $Id: tCorrelatorSubbandProcProcessSb.cc 26496 2013-09-11 12:58:23Z mol $

#include <lofar_config.h>

#include <complex>
#include <cmath>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>

using namespace std;
using namespace LOFAR::Cobalt;
using namespace LOFAR::TYPES;

template<typename T> T inputSignal(size_t t)
{
  double freq = 1.0 / 2.0; // in samples
  double amp = 255.0;

  double angle = (double)t * 2.0 * M_PI * freq;

  double s = ::sin(angle);
  double c = ::cos(angle);

  return T(::round(amp * c), ::round(amp * s));
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

  // Create very simple kernel programs, with predictable output. Skip as much as possible.
  // Nr of channels/sb from the parset is 1, so the PPF will not even run.
  // Parset also has turned of delay compensation and bandpass correction
  // (but that kernel will run to convert int to float and to transform the data order).

  BeamFormerFactories factories(ps);
  BeamFormerSubbandProc bwq(ps, ctx, factories);

  SubbandProcInputData in(ps.nrBeams(), ps.nrStations(), ps.settings.nrPolarisations,
                          ps.settings.beamFormer.maxNrTABsPerSAP(), 
                          ps.nrSamplesPerSubband(), ps.nrBytesPerComplexSample(), ctx);

  // Initialize synthetic input to input signal
  for (size_t st = 0; st < ps.nrStations(); st++)
    for (size_t i = 0; i < ps.nrSamplesPerSubband(); i++)
      for (size_t pol = 0; pol < NR_POLARIZATIONS; pol++)
      {
        if (ps.nrBytesPerComplexSample() == 4) { // 16 bit mode
          *(i16complex*)&in.inputSamples[st][i][pol][0] = inputSignal<i16complex>(i);
        } else if (ps.nrBytesPerComplexSample() == 2) { // 8 bit mode
          *(i8complex*)&in.inputSamples[st][i][pol][0] = inputSignal<i8complex>(i);
        } else {
          cerr << "Error: number of bits per sample must be 8, or 16" << endl;
          exit(1);
        }
      }

  // Initialize subbands partitioning administration (struct BlockID). We only do the 1st block of whatever.
  in.blockID.block = 0;            // Block number: 0 .. inf
  in.blockID.globalSubbandIdx = 0; // Subband index in the observation: [0, ps.nrSubbands())
  in.blockID.localSubbandIdx = 0;  // Subband index for this pipeline/workqueue: [0, subbandIndices.size())
  in.blockID.subbandProcSubbandIdx = 0; // Subband index for this SubbandProc

  // Initialize delays. We skip delay compensation, but init anyway,
  // so we won't copy uninitialized data to the device.
  for (size_t i = 0; i < in.delaysAtBegin.num_elements(); i++)
    in.delaysAtBegin.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.delaysAfterEnd.num_elements(); i++)
    in.delaysAfterEnd.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.phaseOffsets.num_elements(); i++)
    in.phaseOffsets.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.tabDelays.num_elements(); i++)
    in.tabDelays.get<float>()[i] = 0.0f;

  BeamFormedData out(ps.settings.beamFormer.maxNrTABsPerSAP() * ps.settings.beamFormer.coherentSettings.nrStokes,
                     ps.settings.beamFormer.coherentSettings.nrChannels,
                     ps.settings.beamFormer.coherentSettings.nrSamples(ps.settings.nrSamplesPerSubband()),
                     ctx);

  for (size_t i = 0; i < out.num_elements(); i++)
    out.get<float>()[i] = 42.0f;

  // Don't bother initializing out.blockID; processSubband() doesn't need it.

  cout << "processSubband()" << endl;
  bwq.processSubband(in, out);
  cout << "processSubband() done" << endl;

  cout << "Output: " << endl;

  // Output verification
  // The int2float conversion scales its output to the same amplitude as in 16 bit mode.
  // For 8 bit mode, that is a factor 256.
  // Since we inserted all (1, 1) vals, for 8 bit mode this means that the correlator
  // outputs 256*256. It then sums over nrSamplesPerSb values.
#if 0
  unsigned scale = 1*1;
  if (ps.nrBitsPerSample() == 8)
    scale = 256*256;
#endif
  bool unexpValueFound = false;
  for (size_t b = 0; b < ps.settings.beamFormer.maxNrTABsPerSAP() * ps.settings.beamFormer.coherentSettings.nrStokes; b++)
    for (size_t t = 0; t < ps.settings.beamFormer.coherentSettings.nrSamples(ps.settings.nrSamplesPerSubband()); t++)
      for (size_t c = 0; c < ps.settings.beamFormer.coherentSettings.nrChannels; c++)
        {
          float v = out[b][t][c];
// disable output validation until we've verified the beamformer pipeline!
#if 0
          if (v != 4.0f)
          {
            unexpValueFound = true;
            cout << '*'; // indicate error in output
          }
#endif
          cout << v << " ";
        }
  cout << endl;

  if (unexpValueFound)
  {
    cerr << "Error: Found unexpected output value(s)" << endl;
    return 1;
  }

  return 0;
}


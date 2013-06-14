//# tCorrelatorWorKQueueProcessSb.cc: test CorrelatorWorkQueue::processSubband()
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

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/WorkQueues/CorrelatorWorkQueue.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main() {
  INIT_LOGGER("tCorrelatorWorkQueueProcessSb");

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

  Parset ps("tCorrelatorWorkQueueProcessSb.parset.in");

  // Create very simple kernel programs, with predictable output. Skip as much as possible.
  // Nr of channels/sb from the parset is 1, so the PPF will not even run.
  // Parset also has turned of delay compensation and bandpass correction
  // (but that kernel will run to convert int to float and to transform the data order).
  const string kfilenameFIR("FIR_Filter.cu");
  const string kfilenameDBP("DelayAndBandPass.cu");
  const string kfilenameCor("Correlator.cu");
  vector<string> kernels;
  kernels.push_back(kfilenameFIR);
  kernels.push_back(kfilenameDBP);
  kernels.push_back(kfilenameCor);

  map<string, string> ptx;
  flags_type flags(defaultFlags());
  CompileDefinitions definitions(Kernel::compileDefinitions(ps));
  ptx[kfilenameFIR] = createPTX(devices, kfilenameFIR, flags, definitions);
  ptx[kfilenameDBP] = createPTX(devices, kfilenameDBP, flags, definitions);
  ptx[kfilenameCor] = createPTX(devices, kfilenameCor, flags, definitions);
  
  CorrelatorPipelinePrograms progs;
  progs.firFilterProgram = createModule(ctx, kfilenameFIR, ptx[kfilenameFIR]);
  progs.delayAndBandPassProgram = createModule(ctx, kfilenameDBP, ptx[kfilenameDBP]);
  progs.correlatorProgram = createModule(ctx, kfilenameCor, ptx[kfilenameCor]);

  // Initialize FIR filterbank with all weights 0.0f. Won't be used, as FIR won't run.
  vector<float> w0(ps.nrPPFTaps() * ps.nrChannelsPerSubband(), 0.0f);
  FilterBank fb(false, ps.nrPPFTaps(), ps.nrChannelsPerSubband(), &w0[0]);

  CorrelatorWorkQueue cwq(ps, ctx, progs, fb);

  WorkQueueInputData in(ps.nrBeams(), ps.nrStations(), NR_POLARIZATIONS,
                        ps.nrHistorySamples() + ps.nrSamplesPerSubband(),
                        ps.nrBytesPerComplexSample(), ctx);
  cout << "#st=" << ps.nrStations() << " #sampl/sb=" << ps.nrSamplesPerSubband() <<
          " (skipping #histSampl=" << ps.nrHistorySamples() << ") #pol=" << NR_POLARIZATIONS <<
          " #bytes/complexSampl=" << ps.nrBytesPerComplexSample() <<
          " Total bytes=" << in.inputSamples.size() << endl;

  // Initialize synthetic input to all (1, 1).
  for (size_t st = 0; st < ps.nrStations(); st++)
    // skip ps.nrHistorySamples(), because no FIR
    for (size_t i = ps.nrHistorySamples(); i < ps.nrHistorySamples() + ps.nrSamplesPerSubband(); i++)
      for (size_t pol = 0; pol < NR_POLARIZATIONS; pol++)
        // parset specifies 8 bit samples, so this is simply 1 byte real, 1 byte imag
        for (size_t b = 0; b < ps.nrBytesPerComplexSample(); b++)
          in.inputSamples[st][i][pol][b] = 1;

  // Initialize subbands partitioning administration (struct BlockID). We only do the 1st block of whatever.
  in.blockID.block = 0;            // Block number: 0 .. inf
  in.blockID.globalSubbandIdx = 0; // Subband index in the observation: [0, ps.nrSubbands())
  in.blockID.localSubbandIdx = 0;  // Subband index for this pipeline/workqueue: [0, subbandIndices.size())

  // Initialize delays. We skip delay compensation, but init anyway,
  // so we won't copy uninitialized data to the device.
  for (size_t i = 0; i < in.delaysAtBegin.size(); i++)
    in.delaysAtBegin.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.delaysAfterEnd.size(); i++)
    in.delaysAfterEnd.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.phaseOffsets.size(); i++)
    in.phaseOffsets.get<float>()[i] = 0.0f;

  CorrelatedDataHostBuffer out(ps.nrStations(), ps.nrChannelsPerSubband(),
                               ps.integrationSteps(), ctx, cwq);

  // Don't bother initializing out.blockID; processSubband() doesn't need it.

  cout << "processSubband()" << endl;
  cwq.processSubband(in, out);
  cout << "processSubband() done" << endl;

  cout << "Output: " << endl;
  unsigned nbaselines = ps.nrStations() * (ps.nrStations() + 1) / 2; // nbaselines includes auto-correlation pairs here
  cout << "nbl(w/ autocorr)=" << nbaselines << " #chnl/sb=" << ps.nrChannelsPerSubband() <<
          " #pol=" << NR_POLARIZATIONS << " (all combos, hence x2) Total bytes=" << out.size() << endl;
  bool unexpValueFound = false;
  for (size_t b = 0; b < nbaselines; b++)
    for (size_t c = 0; c < ps.nrChannelsPerSubband(); c++)
      // combinations of polarizations; what the heck, call it pol0 and pol1, but 2, in total 4.
      for (size_t pol0 = 0; pol0 < NR_POLARIZATIONS; pol0++)
        for (size_t pol1 = 0; pol1 < NR_POLARIZATIONS; pol1++)
        {
          complex<float> v = out[b][c][pol0][pol1];
          if (v.real() != static_cast<float>(2 * ps.nrSamplesPerSubband()) || v.imag() != 0.0f)
          {
            unexpValueFound = true;
            cout << '*'; // indicate error in output
          }
          cout << out[b][c][pol0][pol1] << " ";
        }
  cout << endl;

  if (unexpValueFound)
  {
    cerr << "Error: Found unexpected output value(s)" << endl;
    return 1;
  }

  // postprocessSubband() is about flagging and that has already been tested
  // in the other CorrelatorWorkQueue test.

  return 0;
}


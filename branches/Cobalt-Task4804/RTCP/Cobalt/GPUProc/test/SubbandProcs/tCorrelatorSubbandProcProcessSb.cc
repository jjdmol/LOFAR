//# tCorrelatorSubbandProcProcessSb.cc 
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

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/SubbandProcs/CorrelatorSubbandProc.h>

#include "../fpequals.h"

using namespace std;
using namespace LOFAR;
using namespace LOFAR::Cobalt;

int main() {
  INIT_LOGGER("tCorrelatorSubbandProcProcessSb");

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

  Parset ps("tCorrelatorSubbandProcProcessSb.parset");

  // Input info
  const size_t nrBeams = ps.nrBeams();
  const size_t nrStations = ps.nrStations();
  const size_t nrPolarisations = ps.settings.nrPolarisations;
  const size_t maxNrTABsPerSAP = ps.settings.beamFormer.maxNrTABsPerSAP();
  const size_t nrSamplesPerChannel = ps.nrSamplesPerChannel();
  const size_t nrSamplesPerSubband = ps.nrSamplesPerSubband();
  const size_t nrBitsPerSample = ps.settings.nrBitsPerSample;
  const size_t nrBytesPerComplexSample = ps.nrBytesPerComplexSample();
  const fcomplex inputValue(1,1);

  // We only support 8-bit or 16-bit input samples
  ASSERT(nrBitsPerSample == 8 || nrBitsPerSample == 16);

  // Output info
  const size_t nrBaselines = nrStations * (nrStations + 1) / 2;
  const size_t nrBlocksPerIntegration = 
    ps.settings.correlator.nrBlocksPerIntegration;
  const size_t nrChannelsPerSubband = ps.nrChannelsPerSubband();
  const size_t integrationSteps = ps.integrationSteps();
  const size_t scaleFactor = nrBitsPerSample == 16 ? 1 : 16;

  // The output is the correlation-product of two inputs (with identical
  // `inputValue`) and the number of integration blocks.
  const fcomplex outputValue = 
    norm(inputValue) * scaleFactor * scaleFactor *
    nrBlocksPerIntegration;

  // Create very simple kernel programs, with predictable output. Skip as much
  // as possible. Nr of channels/sb from the parset is 1, so the PPF will not
  // even run.  Parset also has turned of delay compensation and bandpass
  // correction (but that kernel will run to convert int to float and to
  // transform the data order).

  CorrelatorFactories factories(ps);
  CorrelatorSubbandProc cwq(ps, ctx, factories);

  SubbandProcInputData in(
    nrBeams, nrStations, nrPolarisations, maxNrTABsPerSAP,
    nrSamplesPerSubband, nrBytesPerComplexSample, ctx);

  CorrelatedDataHostBuffer out(
    nrStations, nrChannelsPerSubband, integrationSteps, ctx);

  LOG_INFO_STR(
    "\nInput info:" <<
    "\n  nrBeams = " << nrBeams <<
    "\n  nrStations = " << nrStations <<
    "\n  nrPolarisations = " << nrPolarisations <<
    "\n  maxNrTABsPerSAP = " << maxNrTABsPerSAP <<
    "\n  nrSamplesPerChannel = " << nrSamplesPerChannel <<
    "\n  nrSamplesPerSubband = " << nrSamplesPerSubband <<
    "\n  nrBitsPerSample = " << nrBitsPerSample <<
    "\n  nrBytesPerComplexSample = " << nrBytesPerComplexSample << 
    "\n  inputValue = " << inputValue <<
    "\n  ----------------------------" <<
    "\n  Total bytes = " << in.inputSamples.size());

  LOG_INFO_STR(
    "\nOutput info:" <<
    "\n  nrBaselines = " << nrBaselines <<
    "\n  nrBlockPerIntegration = " << nrBlocksPerIntegration << 
    "\n  nrChannelsPerSubband = " << nrChannelsPerSubband <<
    "\n  integrationSteps = " << integrationSteps <<
    "\n  scaleFactor = " << scaleFactor << 
    "\n  outputValue = " << outputValue <<
    "\n  ----------------------------" <<
    "\n  Total bytes = " << out.size());

  // Initialize synthetic input to all (1, 1).
  for (size_t st = 0; st < nrStations; st++)
    for (size_t i = 0; i < nrSamplesPerSubband; i++)
      for (size_t pol = 0; pol < nrPolarisations; pol++)
      {
        switch(nrBitsPerSample) {
        case 8:
          reinterpret_cast<i8complex&>(in.inputSamples[st][i][pol][0]) = 
            i8complex(inputValue);
          break;
        case 16:
          reinterpret_cast<i16complex&>(in.inputSamples[st][i][pol][0]) = 
            i16complex(inputValue);
          break;
        }
      }

  // Initialize subbands partitioning administration (struct BlockID). We only
  // do the 1st block of whatever.
  in.blockID.block = 0;
  in.blockID.globalSubbandIdx = 0;
  in.blockID.localSubbandIdx = 0;
  in.blockID.subbandProcSubbandIdx = 0;

  // Initialize delays. We skip delay compensation, but init anyway,
  // so we won't copy uninitialized data to the device.
  for (size_t i = 0; i < in.delaysAtBegin.size(); i++)
    in.delaysAtBegin.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.delaysAfterEnd.size(); i++)
    in.delaysAfterEnd.get<float>()[i] = 0.0f;
  for (size_t i = 0; i < in.phase0s.size(); i++)
    in.phase0s.get<float>()[i] = 0.0f;

  bool integrationDone(false);
  size_t block(0);

  LOG_INFO("Processing ...");
  for (block = 0; block < nrBlocksPerIntegration && !integrationDone; block++) {
    LOG_DEBUG_STR("Processing block #" << block);
    cwq.processSubband(in, out);
    integrationDone = cwq.postprocessSubband(out);
  }
  ASSERT(block == nrBlocksPerIntegration);

  LOG_INFO("Verifying output ...");
  for (size_t b = 0; b < nrBaselines; b++)
    for (size_t c = 0; c < nrChannelsPerSubband; c++)
      for (size_t pol0 = 0; pol0 < nrPolarisations; pol0++)
        for (size_t pol1 = 0; pol1 < nrPolarisations; pol1++)
          ASSERTSTR(fpEquals(out[b][c][pol0][pol1], outputValue),
                    "out[" << b << "][" << c << "][" << pol0 << 
                    "][" << pol1 << "] = " << out[b][c][pol0][pol1] << 
                    "; outputValue = " << outputValue);

  LOG_INFO("Test OK");
  return 0;
}


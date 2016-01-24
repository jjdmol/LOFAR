//# gpu_load.cc 
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
#include <CoInterface/fpequals.h>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/SubbandProcs/KernelFactories.h>
#include <GPUProc/SubbandProcs/SubbandProc.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::Cobalt;

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " parset" << endl;
    return 1;
  }

  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    return 1;
  }

  INIT_LOGGER("gpu_load");

  Parset ps(argv[1]);

  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);

  // Input info
  const size_t nrBeams = ps.settings.SAPs.size();
  const size_t nrStations = ps.settings.antennaFields.size();
  const size_t nrPolarisations = ps.settings.nrPolarisations;
  const size_t maxNrTABsPerSAP = ps.settings.beamFormer.maxNrCoherentTABsPerSAP();
  const size_t nrSamplesPerChannel = ps.settings.correlator.nrSamplesPerIntegration();
  const size_t nrSamplesPerSubband = ps.settings.blockSize;
  const size_t nrBitsPerSample = ps.settings.nrBitsPerSample;
  const size_t nrBytesPerComplexSample = ps.nrBytesPerComplexSample();
  const fcomplex inputValue(1,1);

  // Output info
  const size_t nrBaselines = nrStations * (nrStations + 1) / 2;
  const size_t nrBlocksPerIntegration = 
    ps.settings.correlator.enabled ? ps.settings.correlator.nrBlocksPerIntegration : 1;
  const size_t nrChannelsPerSubband = ps.settings.correlator.nrChannels;
  const size_t integrationSteps = ps.settings.correlator.nrSamplesPerIntegration();

  // Create very simple kernel programs, with predictable output. Skip as much
  // as possible. Nr of channels/sb from the parset is 1, so the PPF will not
  // even run.  Parset also has turned of delay compensation and bandpass
  // correction (but that kernel will run to convert int to float and to
  // transform the data order).

  KernelFactories factories(ps, 1);
  SubbandProc cwq(ps, ctx, factories);

  SubbandProcInputData in(
    nrBeams, nrStations, nrPolarisations, maxNrTABsPerSAP,
    nrSamplesPerSubband, nrBytesPerComplexSample, ctx);

  SubbandProcOutputData out(ps, ctx);

  LOG_INFO_STR(
    "\nInput info:" <<
    "\n  nrBeams = " << nrBeams <<
    "\n  nrStations = " << nrStations <<
    "\n  nrPolarisations = " << nrPolarisations <<
    "\n  maxNrTABsPerSAP = " << maxNrTABsPerSAP <<
    "\n  nrSamplesPerChannel = " << nrSamplesPerChannel <<
    "\n  nrSamplesPerSubband = " << nrSamplesPerSubband <<
    "\n  nrBitsPerSample = " << nrBitsPerSample <<
    "\n  ----------------------------" <<
    "\n  Total bytes = " << in.inputSamples.size());

  if (ps.settings.correlator.enabled) {
    LOG_INFO_STR(
      "\nOutput info:" <<
      "\n  nrBaselines = " << nrBaselines <<
      "\n  nrBlockPerIntegration = " << nrBlocksPerIntegration << 
      "\n  nrChannelsPerSubband = " << nrChannelsPerSubband <<
      "\n  integrationSteps = " << integrationSteps <<
      "\n  ----------------------------" <<
      "\n  Total bytes = " << out.correlatedData.subblocks[0]->visibilities.size() * sizeof(fcomplex)); // TODO: wrong!
  }

  // Initialize subbands partitioning administration (struct BlockID). We only
  // do the 1st block of whatever.
  in.blockID.block = 0;
  in.blockID.globalSubbandIdx = 0;
  in.blockID.localSubbandIdx = 0;
  in.blockID.subbandProcSubbandIdx = 0;

  size_t block(0);

  LOG_INFO("Processing ...");
  for (block = 0; block < nrBlocksPerIntegration && !out.emit_correlatedData; block++) {
    LOG_DEBUG_STR("Processing block #" << block);
    cwq.processSubband(in, out);
    cwq.postprocessSubband(out);
  }

  return 0;
}


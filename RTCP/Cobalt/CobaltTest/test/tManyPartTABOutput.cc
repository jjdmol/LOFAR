//# tManyPartTABOutput.cc: test TAB sb range output into many files (parts)
//# Copyright (C) 2014  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <omp.h>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/Pipelines/BeamFormerPipeline.h>
#include <GPUProc/SubbandProcs/BeamFormerSubbandProc.h>
#include <GPUProc/Station/StationInput.h>
#include <GPUProc/Storage/StorageProcesses.h>

using namespace std;
using namespace LOFAR::Cobalt;
using boost::format;
using boost::str;

// Fill sb with all values equal to its station sb nr (see .parset).
SmartPtr<SubbandProcOutputData> getTestSbCohData(const Parset& ps, gpu::Context& ctx,
                                                 unsigned blockIdx, unsigned sbIdx)
{ 
  // BeamFormedData is a sub-class of SubbandProcOutputData.
  BeamFormedData *bfData = new BeamFormedData(ps, ctx);

  bfData->blockID.block = blockIdx;
  bfData->blockID.globalSubbandIdx = sbIdx;
  bfData->blockID.localSubbandIdx = sbIdx;
  bfData->blockID.subbandProcSubbandIdx = sbIdx;
  const unsigned sbFreqNr = ps.settings.subbands[sbIdx].stationIdx;
  for (size_t v = 0; v < bfData->coherentData.num_elements(); v++) {
    bfData->coherentData.origin()[v] = (float)sbFreqNr;
  }

  SmartPtr<SubbandProcOutputData> data = bfData;
  return data;
}

// Test strategy:
// Skip all the station input and GPU processing.
// Create a parset and BF pipeline, prepare data and write it to outputProc.
// This covers all multiple output parts specific lines of code.
// 
// Assume the startup script of this test has started outputProc. Supply parset.
// It can also test whether the output files contain the expected values.
int main()
{
  // CV XXYY   : 2 Beams, 4 "Stokes", 2+3 TABs: 5+11 sb in 2+3 parts per TAB: 4 * 2*2 + 3*3 = 52 files

  INIT_LOGGER("tManyPartTABOutput");

  Parset ps("tManyPartTABOutput.parset");

  // GPU devices for BF pipeline constr and context for BF host buffers.
  // The BF Sb Proc compiles kernels, but we don't use GPUs in this test.
  vector<gpu::Device> devices = gpu::Platform().devices();
  gpu::Context ctx(devices[0]);

  const unsigned nrSubbands = ps.nrSubbands();

  // Create BF Pipeline. We're the only rank: do all the subbands.
  // So for the rest of the test code globalSubbandIdx equals localSubbandIdx.
  vector<size_t> localSbIndices;
  for (unsigned i = 0; i < nrSubbands; i++) {
    localSbIndices.push_back(i);
  }
  omp_set_nested(true); // for around and within .multiSender.process()
  BeamFormerPipeline bfpl(ps, localSbIndices, devices);
  bfpl.allocateResources();

  // Set up control line to outputProc. This also supplies the parset.
  // This must happen after kernel compilation to avoid fork() during PortBroker getaddrinfo().
  string spLogPrefix = "StorageProcesses: ";
  StorageProcesses stPr(ps, spLogPrefix);

#pragma omp parallel sections num_threads(2)
  {
#  pragma omp section
    {
      // Set up data connections with outputProc.
      // Does not return until end of obs, so call from a thread.
      bfpl.multiSender.process();
    }

#  pragma omp section
    {
  // Insert 2 blocks of test data per sb.
  std::vector<struct BeamFormerPipeline::Output> writePool(nrSubbands); // [localSubbandIdx]
  for (unsigned i = 0; i < writePool.size(); i++) {
    SmartPtr<SubbandProcOutputData> data;
    unsigned blockIdx;

    writePool[i].bequeue = new BestEffortQueue< SmartPtr<SubbandProcOutputData> >(str(format("writePool [file %u]") % i), 3, ps.realTime());

    blockIdx = 0;
    data = getTestSbCohData(ps, ctx, blockIdx, i);
    ASSERT(writePool[i].bequeue->append(data));
    blockIdx = 1;
    data = getTestSbCohData(ps, ctx, blockIdx, i);
    ASSERT(writePool[i].bequeue->append(data));

    writePool[i].bequeue->noMore();
  }

  // Have it push a block of values per sb to outputProc.
  // writeOutput() takes a globalSubbandIdx.
  for (unsigned globalSubbandIdx = 0; globalSubbandIdx < nrSubbands;
       globalSubbandIdx++) {
    unsigned localSubbandIdx = globalSubbandIdx;
    bfpl.writeOutput(globalSubbandIdx, writePool[localSubbandIdx]);
  }

  bfpl.multiSender.finish();

    } // omp section
  } // omp parallel sections ...

  // Looks good, but the startup script will check the output files.
  return 0;
}


//# tSubbandProc.cc
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

#include <CoInterface/BudgetTimer.h>
#include <GPUProc/cuda/SubbandProcs/CorrelatorStep.h>
#include <GPUProc/cuda/gpu_wrapper.h>

#include <UnitTest++.h>
#include <iostream>
#include <CoInterface/Parset.h>
#include <CoInterface/SparseSet.h>
#include <CoInterface/MultiDimArray.h>
#include <Common/LofarLogger.h>
#include <complex>
#include <boost/format.hpp>

using boost::format;

using namespace LOFAR::Cobalt;
using namespace LOFAR::Cobalt::gpu;

TEST(convertFlagsToChannelFlags)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Cobalt.Correlator.nrChannelsPerSubband","4");
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  parset.add("Cobalt.blockSize", "1024");
  
  parset.add("Observation.VirtualInstrument.stationList", "[RS106, RS107]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");

  parset.updateSettings();
  
  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.settings.antennaFields.size()]);

  //// Insert some flag ranges
  //std::cout << "inputFlags.size(): " << inputFlags.size() << std::endl;
  //std::cout << "parset.settings.antennaFields.size(): " << parset.settings.antennaFields.size() << std::endl;
  inputFlags[0].include(62, 63);    // A. should result in channelflag (0,16) due to the filter width
  inputFlags[0].include(128, 129);  // B. Outside of the begin range result: (128 / 4) - 16 + 1 = 17 end 33
  inputFlags[0].include(255, 522);  // C. Flag all large ranges (48, 131)
  inputFlags[0].include(1000, 1050); // D. Outside of range should not be a problem but capped at max (235,257)

  inputFlags[1].include(100, 600); // E. Second station (10, 150)
  // The converted channel flags
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> flagsPerChannel(
          boost::extents[parset.settings.antennaFields.size()]);

  // ****** perform the translation
  CorrelatorStep::Flagger::convertFlagsToChannelFlags(parset, inputFlags, flagsPerChannel);
  // ******

  //validate the corner cases
  CHECK(0 == flagsPerChannel[0].getRanges()[0].begin && 
        16 == flagsPerChannel[0].getRanges()[0].end);  //A.
  CHECK(17 == flagsPerChannel[0].getRanges()[1].begin &&
        33 == flagsPerChannel[0].getRanges()[1].end);  //B.
  CHECK(48 == flagsPerChannel[0].getRanges()[2].begin &&
        131 == flagsPerChannel[0].getRanges()[2].end);  //C.
  CHECK(235 == flagsPerChannel[0].getRanges()[3].begin &&
        256 == flagsPerChannel[0].getRanges()[3].end);  //D.

  CHECK(10 == flagsPerChannel[1].getRanges()[0].begin &&
        150 == flagsPerChannel[1].getRanges()[0].end);  //E.
}

TEST(propagateFlags)
{
  /* Test the PERFORMANCE of CorrelatorStep::propagateFlags. */

  const unsigned nrChannels = 256;
  const unsigned nrSubblocks = 4;

  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Cobalt.Correlator.nrChannelsPerSubband",   str(format("%u") % nrChannels));
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", str(format("%u") % nrSubblocks));
  parset.add("Cobalt.blockSize", "196608");
  
  parset.add("Observation.VirtualInstrument.stationList", "[80*RS106]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");

  parset.updateSettings();

  // All flagged data
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> flags(boost::extents[parset.settings.antennaFields.size()]);
  for (size_t i = 0; i < parset.settings.antennaFields.size(); ++i) {
    flags[i].include(0, 196608);
  }

  // Create a default context
  Device device(0);
  Context ctx(device);

  // Output data holder
  SubbandProcOutputData::CorrelatedData correlatedData(nrSubblocks, parset.settings.antennaFields.size(), nrChannels, 65536, ctx);

  const unsigned nrSubbandsPerSubbandProc = 16;

  BudgetTimer processCPUTimer("processCPU", parset.settings.blockDuration() / nrSubbandsPerSubbandProc, true, true);

  processCPUTimer.start();
  CorrelatorStep::Flagger::propagateFlags(parset, flags, correlatedData);
  processCPUTimer.stop();
}


int main()
{
  INIT_LOGGER("tCorrelatorStep");

  try 
  {
    Platform pf;
    LOG_INFO_STR("Detected " << pf.size() << " CUDA devices");
  } 
  catch (CUDAException& e) 
  {
    LOG_ERROR_STR("Caught exception: " << e.what());
    return 3;
  }

  return UnitTest::RunAllTests() > 0;
}


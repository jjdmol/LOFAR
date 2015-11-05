//# tCorrelatorWorkQueue.cc
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

#include <GPUProc/WorkQueues/WorkQueue.h>

#include <UnitTest++.h>
#include <iostream>
#include <CoInterface/Parset.h>
#include <CoInterface/SparseSet.h>
#include <CoInterface/MultiDimArray.h>
#include <Common/LofarLogger.h>
#include <complex>

using namespace LOFAR::Cobalt;


TEST(log2)
{
  CHECK_EQUAL(6u, WorkQueue::Flagger::log2(64));
  CHECK_EQUAL(7u, WorkQueue::Flagger::log2(128));
  CHECK_EQUAL(0u, WorkQueue::Flagger::log2(1));

  //cant take 2log of zero: raise exception
  //CHECK_THROW(get2LogOfNrChannels(0), LOFAR::AssertError);
}

TEST(convertFlagsToChannelFlags)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","4"); //not to large a number of subbands, else the integration steps gets big
  parset.add("OLAP.IONProc.integrationSteps", "1");  
  parset.add("OLAP.CNProc.integrationSteps", "256");   //samples per channel 
  
  parset.add("OLAP.storageStationNames", "[RS106HBA, RS105HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();
  
  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.nrStations()]);

  //// Insert some flag ranges
  //std::cout << "inputFlags.size(): " << inputFlags.size() << std::endl;
  //std::cout << "parset.nrStations(): " << parset.nrStations() << std::endl;
  inputFlags[0].include(62, 63);    // A. should result in channelflag (0,16) due to the filter width
  inputFlags[0].include(128, 129);  // B. Outside of the begin range result: (128 / 4) - 16 + 1 = 17 end 33
  inputFlags[0].include(255, 522);  // C. Flag all large ranges (48, 131)
  inputFlags[0].include(1000, 1050); // D. Outside of range should not be a problem but capped at max (235,257)

  inputFlags[1].include(100, 600); // E. Second station (10, 150)
  // The converted channel flags
  MultiDimArray<LOFAR::SparseSet<unsigned>, 2> flagsPerChanel(
          boost::extents[parset.nrChannelsPerSubband()][parset.nrStations()]);

  // ****** perform the translation
  WorkQueue::Flagger::convertFlagsToChannelFlags(parset, inputFlags, flagsPerChanel);
  // ******

  //validate the corner cases
  CHECK(0 == flagsPerChanel[0][0].getRanges()[0].begin && 
        16 == flagsPerChanel[0][0].getRanges()[0].end);  //A.
  CHECK(17 == flagsPerChanel[0][0].getRanges()[1].begin &&
        33 == flagsPerChanel[0][0].getRanges()[1].end);  //B.
  CHECK(48 == flagsPerChanel[0][0].getRanges()[2].begin &&
        131 == flagsPerChanel[0][0].getRanges()[2].end);  //C.
  CHECK(235 == flagsPerChanel[0][0].getRanges()[3].begin &&
        257 == flagsPerChanel[0][0].getRanges()[3].end);  //D.

  CHECK(10 == flagsPerChanel[0][1].getRanges()[0].begin &&
        150 == flagsPerChanel[0][1].getRanges()[0].end);  //E.
}


int main()
{
  INIT_LOGGER("tWorkQueue");
  return UnitTest::RunAllTests() > 0;
}


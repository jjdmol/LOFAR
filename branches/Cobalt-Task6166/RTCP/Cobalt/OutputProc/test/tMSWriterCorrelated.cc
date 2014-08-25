//# tMSWriterCorrelated.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/FinalMetaData.h>
#include <CoInterface/StreamableData.h>
#include <CoInterface/Parset.h>
#include <OutputProc/MSWriterCorrelated.h>

using namespace std;
using namespace LOFAR;
using namespace Cobalt;

int main()
{
  INIT_LOGGER("tMSWriterCorrelated");

  Parset parset("tMSWriterCorrelated.parset");

  {
    // Create MeasurementSet
    MSWriterCorrelated writer("", "tMSWriterCorrelated.in_1/SB000.MS", parset, 0);

    // Write some data
    CorrelatedData data(parset.nrMergedStations(), parset.nrChannelsPerSubband(), parset.integrationSteps(), heapAllocator, 512);

    writer.write(&data);

    // Add broken tile information
    FinalMetaData fmd;
    struct FinalMetaData::BrokenRCU rcu;

    rcu.station = "CS013";
    rcu.time = "2012-01-01 00:00:00";
    rcu.type = "RCU";
    rcu.seqnr = 2;

    fmd.brokenRCUsAtBegin.push_back(rcu);

    rcu.station = "CS013";
    rcu.time = "2012-01-01 00:00:00";
    rcu.type = "LBA";
    rcu.seqnr = 4;

    fmd.brokenRCUsAtBegin.push_back(rcu);

    writer.augment(fmd);
  }

  return 0;
}


//# failedtiles2ms.cc: Program to write failed tile info into the MS
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id: failedtiles2ms.cc 19920 2012-01-24 14:57:23Z diepen $

#include <lofar_config.h>
#include <Beaminfo/FailedTileInfo.h>
#include <Common/InputParSet.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/SystemUtil.h>    // needed for basename

using namespace LOFAR;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main (int argc, char* argv[])
{
  try {
    // Init logger.
    INIT_LOGGER (LOFAR::basename(argv[0]));
    // Define the input parameters.
    InputParSet inputs;
    inputs.setVersion ("24-Jan-2012 SD/GvD");
    inputs.create ("ms", "",
		   "Name of MeasurementSet to update",
		   "string");
    inputs.create ("before", "",
		   "File with tiles (RCUs) broken at start of observation",
		   "string");
    inputs.create ("during", "",
		   "File with tiles (RCUs) broken during the observation",
		   "string");
    inputs.readArguments (argc, argv);
    string msName     = inputs.getString ("ms");
    string beforeName = inputs.getString ("before");
    string duringName = inputs.getString ("during");
    // Read the failed tile info and write into MS.
    FailedTileInfo::failedTiles2MS (msName, beforeName, duringName);
  } catch (Exception& ex) {
    cerr << "Unexpected exception: " << ex << endl;
    return 1;
  }
  return 0;
}

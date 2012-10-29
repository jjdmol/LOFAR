//# addfailedtileinfo.cc: Program to add failed tile info to an MS
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
//# $Id$
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <MSLofar/FailedTileInfo.h>
#include <MsLofar/Package__Version.h>
#include <Common/InputParSet.h>
#include <Common/SystemUtil.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main (int argc, char* argv[])
{
  TEST_SHOW_VERSION (argc, argv, MSLofar);
  INIT_LOGGER(basename(string(argv[0])));
  try {
    InputParSet inputs;
    // Define the input parameters.
    inputs.setVersion("2012Oct29-GvD");
    inputs.create ("ms", "",
		   "Name of MeasurementSet",
		   "string");
    inputs.create ("BrokenTileFile", "brokenTiles.txt",
		   "File containing tiles broken before the observation",
		   "string");
    inputs.create ("FailedTileFile", "failedTiles.txt",
		   "File containing tiles failing during the observation",
		   "string");
    inputs.readArguments (argc, argv);
    string msName      = inputs.getString("ms");
    string brokenName  = inputs.getString("BrokenTileFile");
    string failedName  = inputs.getString("FailedTileFile");
    ASSERT (! msName.empty());
    ASSERT (! brokenName.empty());
    ASSERT (! failedName.empty());
    FailedTileInfo::failedTiles2MS (msName, brokenName, failedName); 
  } catch (Exception& x) {
    cerr << "Unexpected LOFAR exception: " << x << endl;
    return 1;
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

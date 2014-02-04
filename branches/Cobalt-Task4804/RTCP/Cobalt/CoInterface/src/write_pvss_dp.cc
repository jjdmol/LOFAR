//# write_pvss_dp.cc: write PVSS datapoint(s) util
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

#include <string>
#include <vector>
#include <iostream>

#include <Common/LofarLogger.h>
#include <MACIO/RTmetadata.h>

using namespace std;

static void usage(const char *argv0) {
  cerr << "Usage: " << argv0 << " [PVSS gateway host|IP] key=value ..." << endl;

}

int main(int argc, char *argv[]) {
  INIT_LOGGER(argv[0]);

  if (argc < 3)
  {
    usage(argv[0]);
    exit(1);
  }

  unsigned obsId = 12345;
  string registerName("OnlineControl"); // see table in LOFAR/MAC/APL/APLCommon/src/ControllerDefines.cc
  string defaultHostname("ccu001.control.lofar");
  LOFAR::MACIO::RTmetadata rtmd(obsId, registerName, defaultHostname);
  bool rv;

  vector<string> keys;
  vector<string> vals;
  vector<double> times(4, 393939393.004567);

  keys.push_back("LOFAR_ObsSW_TempObs0001_CobaltOutputProc.written[0]");
  vals.push_back("42");
  keys.push_back("LOFAR_ObsSW_TempObs0001_CobaltOutputProc.written[1]");
  vals.push_back("43");
  keys.push_back("LOFAR_ObsSW_TempObs0001_CobaltOutputProc.dropped[0]");
  vals.push_back("21");
  keys.push_back("LOFAR_ObsSW_TempObs0001_CobaltOutputProc.dropped[1]");
  vals.push_back("22");
  rv = rtmd.log(keys, vals, times);
  if (!rv)
  {
    LOG_WARN("Failed to write data points");
  }


  string key0("nonexisting_test-by-alexander");
  string val0("3.3");
  rv = rtmd.log(key0, val0);
  if (!rv)
  {
    LOG_WARN("Failed to write data point");
  }

  return 0;
}


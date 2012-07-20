//# combinevds.cc: Program to combine the description of MS parts
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <MS/VdsMaker.h>
#include <MS/Package__Version.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/Exception.h>
#include <stdexcept>
#include <iostream>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::CEP;

// Define handler that tries to print a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main (int argc, const char* argv[])
{
  try {
    TEST_SHOW_VERSION (argc, argv, MS);
    INIT_LOGGER(basename(string(argv[0])));
    if (argc < 3) {
      cout << "Run as:  combinevds outName in1 in2 ..." << endl;
      return 0;
    }
    // Form the vector of VDSnames.
    vector<string> vdsNames;
    vdsNames.reserve (argc-2);
    for (int i=2; i<argc; ++i) {
      // Multiple names can be given separated by commas.
      vector<string> names = StringUtil::split (string(argv[i]), ',');
      vdsNames.insert (vdsNames.end(), names.begin(), names.end());
    }
    // Combine them.
    VdsMaker::combine (argv[1], vdsNames);

  } catch (LOFAR::Exception& err) {
    std::cerr << "LOFAR Exception detected: " << err << std::endl;
    return 1;
  }
  return 0;
}

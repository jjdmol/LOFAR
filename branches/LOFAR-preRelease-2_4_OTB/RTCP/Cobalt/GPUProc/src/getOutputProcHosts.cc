//# getOutputProcHosts.cc: Real-Time Central Processor application, GPU cluster version
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
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include <CoInterface/Parset.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::Cobalt;

static void usage(const char *argv0)
{
  cerr << "getOutputProcHosts: Helper program used in combination with rtcp" << endl;
  cerr << "Outputs a space seperate list of outputProc hosts on the stdout" << endl;
  cerr << "These hosts are retrieved using the parset (parset) class" << endl;

  cerr << endl;
  cerr << "Usage: " << argv0 << " parset" << endl;
  cerr << endl;

}

int main(int argc, char **argv)
{
  /*
  * Parse command-line options
  */

  int opt;
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {

    default: /* '?' */
      usage(argv[0]);
      exit(1);
    }
  }

  // we expect a parset filename as an additional parameter
  if (optind >= argc) {
    usage(argv[0]);
    exit(1);
  }

  INIT_LOGGER("getOutputProcHosts");

  // Open the parset
  Parset ps(argv[optind]);

  // Get the list of stations and output to stdout space separated
  for (std::vector<string>::const_iterator host = ps.settings.outputProcHosts.begin();
       host != ps.settings.outputProcHosts.end();
       ++host)
  {
    cout << *host << " ";
  }

  return 0;
 }
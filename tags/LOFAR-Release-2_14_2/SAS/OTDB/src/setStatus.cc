//#  setStatus.cc: Force the status of an observation or pipeline
//#
//#  Copyright (C) 2004-2012
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: MACScheduler.cc 27964 2014-01-17 11:04:09Z mol $
#include <lofar_config.h>
#include <iostream>
#include <unistd.h>

#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeStateConv.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace std;

void usage(const char *argv0)
{
  cerr << "Usage: " << argv0 << " -o obsID" << endl;
  cerr << "     Print the current status of `obsID`." << endl;
  cerr << endl;
  cerr << "Usage: " << argv0 << " -o obsID -s status" << endl;
  cerr << "     Put the status of `obsID` to `status`." << endl;
  cerr << endl;
  cerr << "Valid statusses: approved, queued, active, completing, finished, aborted" << endl;
}

int main(int argc, char **argv)
{
  INIT_LOGGER("setStatus");

  treeIDType obsID = 0;
  std::string status; // queued, active, completing, finished, aborted

  /*
   * Parse command-line options
   */

  int opt;
  while ((opt = getopt(argc, argv, "o:s:h")) != -1) {
    switch (opt) {
    case 'o':
      obsID = atoi(optarg);
      break;

    case 's':
      status = optarg;
      break;

    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;

    default: /* '?' */
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (obsID <= 0) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  // Try to connect to the SAS database.
  ConfigLocator cl;
  ParameterSet ps(cl.locate("setStatus.conf"));
  string DBname 	= ps.getString("OTDBdatabase");
  string hostname	= ps.getString("OTDBhostname");

  LOG_INFO_STR ("Trying to connect to the OTDB on " << DBname << "@" << hostname);
  OTDBconnection *conn = new OTDBconnection("paulus", "boskabouter", DBname, hostname);
  TreeStateConv tsc(conn);

  if (!status.empty()) {
    LOG_INFO_STR ("Setting status of observation " << obsID << " to " << status);
    OTDB::TreeMaintenance tm(conn);
    tm.setTreeState(obsID, tsc.get(status));
  }

  LOG_INFO_STR ("Getting status of observation " << obsID);
  vector<TreeState> states = conn->getStateList(obsID);

  if (states.empty()) {
    cout << "UNKNOWN" << endl;
  } else {
    // report last known state
    cout << tsc.get(states.rbegin()->newState) << endl;
  }

  LOG_INFO_STR ("Disconnecting from OTDB");
  delete conn;
}

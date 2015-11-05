//# showsourcedb.cc: Show contents of a SourceDB catalog
//#
//# Copyright (C) 2013
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
//# $Id: mergesourcedb.cc 24953 2013-05-17 11:34:36Z diepen $

// This program shows the contents of a SourceDB catalogs.
// It can show only patches or sources or both.
//
// The program can be run as:
//    showsourcedb  in=inname mode=all|patch|source
// in            name of the SourceDB.
// mode          all    = show patches and sources
//               patch  = only show patches
//               source = only show sources

#include <lofar_config.h>
#include <ParmDB/SourceDB.h>
#include <ParmDB/Package__Version.h>
#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <Common/SystemUtil.h>
#include <Common/Exception.h>

#include <casa/Quanta/MVAngle.h>
#include <casa/Inputs/Input.h>
#include <vector>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace BBS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);


void show (const string& name, const string& mode)
{
  // Open the input SourceDB.
  SourceDB in ((ParmDBMeta(string(), name)));
  // Read all patches from the SourceDB and write them.
  vector<PatchInfo> patch (in.getPatchInfo());
  for (size_t i=0; i<patch.size(); ++i) {
    if (mode != "showsource") {
      cout << patch[i] << endl;
    }
    if (mode != "showpatch") {
      vector<SourceData> sources(in.getPatchSourceData (patch[i].getName()));
      for (vector<SourceData>::const_iterator iter=sources.begin();
           iter!=sources.end(); ++iter) {
        iter->print (cout);
      }
    }
  }
}


int main (int argc, char* argv[])
{
  TEST_SHOW_VERSION (argc, argv, ParmDB);
  INIT_LOGGER(basename(string(argv[0])));
  try {
    // Define the input parameters.
    Input inputs(1);
    inputs.version ("GvD 2013-Jun-12");
    inputs.create("in", "",
                  "Input SourceDB", "string");
    inputs.create ("mode", "all",
                   "patch=show all patches, "
                   "source=show all sources, "
                   "all=show patches and sources",
                   "string");
    // Read and check the input parameters.
    inputs.readArguments(argc, argv);
    string in = inputs.getString("in");
    ASSERTSTR (!in.empty(), "no input sourcedb name given");
    string mode = toLower(inputs.getString("mode"));
    ASSERTSTR (mode=="patch" || mode=="source" || mode=="all",
               "incorrect mode given");
    show (in, mode);
  } catch (Exception& x) {
    cerr << "Caught LOFAR exception: " << x << endl;
    return 1;
  } catch (AipsError& x) {
    cerr << "Caught AIPS error: " << x.what() << endl;
    return 1;
  }
  return 0;
}

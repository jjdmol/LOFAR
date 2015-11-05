//# mergesourcedb.cc: Merge two SourceDB catalogs
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

// This program merges two SourceDB catalogs resulting in a new SourceDB.
// It checks if in both catalogs patches are defined at the same position
// (within a given radius). If so, the user can choose to take them from the
// first or second SourceDB.
//
// The program can be run as:
//    mergesourcedb  in1=inname in2=inname out=outname outtype=xx append=T|F
//                   radius=xx, mode=all|match
// in1            gives the first input SourceDB. All its patches are used.
// in2            gives the second input SourceDB. A patch matching the position
//                of a patch in in1, will be used instead of the patch in in1.
//                The mode parameter defines if its other patches are used too.
// out            gives the name of the SourceDB to be created or appended
// outtype        gives the output SourceDB type (casa or blob).
// append         T = create new output (is the default)
//                F = append to existing SourceDB
// radius         the position matching uncertainty radius (default 10arcsec).
//                It can be given in casacore format like 10arcsec, 0d0m10, etc.
// mode           all   = use all in2 patches
//                match = only use matching in2 patches (is the default)

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


void writePatch (SourceDB& out, SourceDB& in, const PatchInfo& patch)
{
  // Write the patch.
  out.addPatch (patch.getName(), patch.getCategory(),
                patch.apparentBrightness(),
                patch.getRa(), patch.getDec(), false);
  // Get the fixed source info of all sources in the patch..
  vector<SourceData> sources(in.getPatchSourceData (patch.getName()));
  for (vector<SourceData>::const_iterator iter=sources.begin();
       iter!=sources.end(); ++iter) {
    out.addSource (*iter, false);
  }
}

void merge (const string& name1, const string& name2,
            const string& outName, const string& outType, bool append,
            const string& mode, double radius)
{
  double cosRadius = cos(radius);
  // Open the input SourceDBs.
  SourceDB in1 ((ParmDBMeta(string(), name1)));
  SourceDB in2 ((ParmDBMeta(string(), name2)));
  // Open/create the output.
  SourceDB out(ParmDBMeta(outType, outName), !append);
  // Read all patches from both SourceDBs.
  vector<PatchInfo> patch1 (in1.getPatchInfo());
  vector<PatchInfo> patch2 (in2.getPatchInfo());
  vector<int> match1(patch1.size(), -1);
  vector<int> match2(patch2.size(), -1);
  // Loop through all in1 patches and see if a match in in2 is found.
  for (size_t i1=0; i1<patch1.size(); ++i1) {
    double sinDec1 = sin(patch1[i1].getDec());
    double cosDec1 = cos(patch1[i1].getDec());
    double ra1 = patch1[i1].getRa();
    for (size_t i2=0; i2<patch2.size(); ++i2) {
      double ra2  = patch2[i2].getRa();
      double dec2 = patch2[i2].getDec();
      if (cosRadius <=
          sinDec1 * sin(dec2) + cosDec1 * cos(dec2) * cos(ra1 - ra2)) {
        // A match. Check if not matched before.
        if (match2[i2] >= 0) {
          cout << "Warning: in2 patch " << patch2[i2].getName()
               << " matches in1 patch " << patch1[i1].getName()
               << ", but matched in1 patch " << patch1[match2[i2]].getName()
               << " before" << endl;
        } else {
          // Set the indices of the matching patch.
          match1[i1] = i2;
          match2[i2] = i1;
        }
      }
    }
  }
  // Now write all in1 patches, but use in2 patch if matching.
  int nin1 = 0;
  int nin2 = 0;
  int nrep = 0;
  for (size_t i1=0; i1<patch1.size(); ++i1) {
    if (match1[i1] < 0) {
      writePatch (out, in1, patch1[i1]);
      nin1++;
    } else {
      writePatch (out, in2, patch2[match1[i1]]);
      cout << "in1 patch " << patch1[i1].getName()
           << " replaced by in2 patch " << patch2[match1[i1]].getName() << endl;
      nrep++;
      nin2++;
    }
  }
  // If needed, write all non-matching in2 patches.
  if (mode == "all") {
    for (size_t i2=0; i2<patch2.size(); ++i2) {
      if (match2[i2] < 0) {
        writePatch (out, in2, patch2[i2]);
        nin2++;
      }
    }
  }
  cout << endl;
  cout << "Copied " << nin1 << " patches from in1=" << name1 << endl;
  cout << "Copied " << nin2 << " patches from in2=" << name2 << endl;
  cout << "       " << nrep << " of them replaced an in1 patch" << endl;
}


void copy (const string& name1,
           const string& outName, const string& outType, bool append)
{
  // Open the input SourceDB.
  SourceDB in1 ((ParmDBMeta(string(), name1)));
  // Open/create the output.
  SourceDB out(ParmDBMeta(outType, outName), !append);
  // Read all patches from the SourceDB and write them.
  vector<PatchInfo> patch1 (in1.getPatchInfo());
  for (size_t i1=0; i1<patch1.size(); ++i1) {
    writePatch (out, in1, patch1[i1]);
  }
  cout << endl;
  cout << "Copied " << patch1.size() << " patches from " << name1 << endl;
}


int main (int argc, char* argv[])
{
  TEST_SHOW_VERSION (argc, argv, ParmDB);
  INIT_LOGGER(basename(string(argv[0])));
  try {
    // Define the input parameters.
    Input inputs(1);
    inputs.version ("GvD 2013-Jun-10");
    inputs.create("in1", "",
                  "First input SourceDB", "string");
    inputs.create("in2", "",
                  "Second input SourceDB", "string");
    inputs.create("out", "",
                  "Output SourceDB name", "string");
    inputs.create ("outtype", "casa",
                   "Output type (casa or blob)", "string");
    inputs.create ("append", "false",
                   "Append to possibly existing SourceDB?", "bool");
    inputs.create ("radius", "10arcsec",
                   "Uncertainty radius for finding duplicate patch positions",
                   "string");
    inputs.create ("mode", "match",
                   "all=use all in2 patches, "
                   "match=only use matching in2 patches"
                   "string");
    // Read and check the input parameters.
    inputs.readArguments(argc, argv);
    string in1 = inputs.getString("in1");
    ASSERTSTR (!in1.empty(), "no in1 input sourcedb name given");
    string in2 = inputs.getString("in2");
    string out = inputs.getString("out");
    ASSERTSTR (!out.empty(), "no output sourcedb name given");
    string outType = toLower(inputs.getString("outtype"));
    bool   append  = inputs.getBool("append");
    string radStr  = inputs.getString("radius");
    string mode    = toLower(inputs.getString("mode"));
    Quantity q;
    ASSERTSTR (MVAngle::read (q, radStr),
               "Error in interpreting radius " << radStr);
    double radius = q.getValue ("rad");
    ASSERTSTR (mode=="match" || mode=="all", "incorrect mode given");
    // Do the copy or merge.
    if (in2.empty()) {
      copy (in1, out, outType, append);
    } else {
      merge (in1, in2, out, outType, append, mode, radius);
    }
  } catch (Exception& x) {
    cerr << "Caught LOFAR exception: " << x << endl;
    return 1;
  } catch (AipsError& x) {
    cerr << "Caught AIPS error: " << x.what() << endl;
    return 1;
  }
  return 0;
}

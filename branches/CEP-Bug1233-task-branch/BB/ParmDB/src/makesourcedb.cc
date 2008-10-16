//# makesourcedb.cc: Create a SourceDB from a CSV file
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$


#include <lofar_config.h>
#include <ParmDB/SourceDB.h>
#include <string>                // for getline
#include <iostream>
#include <fstream>
#include <sstream>
#include <casa/Quanta/MVAngle.h>
#include <unistd.h>
#include <libgen.h>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace BBS;

SourceInfo::Type string2type (const string& str)
{
  string s = toLower(str);
  if (s == "point"  ||  s.empty()) {
    return SourceInfo::POINT;
  } else if (s == "gaussian") {
    return SourceInfo::GAUSSIAN;
  } else if (s == "disk") {
    return SourceInfo::DISK;
  } else if (s == "shapelet") {
    return SourceInfo::SHAPELET;
  } else if (s == "sun") {
    return SourceInfo::SUN;
  } else if (s == "moon") {
    return SourceInfo::MOON;
  } else if (s == "jupiter") {
    return SourceInfo::JUPITER;
  } else if (s == "mars") {
    return SourceInfo::MARS;
  } else if (s == "venus") {
    return SourceInfo::VENUS;
  }
  ASSERTSTR (false, str << " is an invalid source type");
}

double string2real (const vector<string>& fields, uint fieldnr, double def)
{
  if (fieldnr >= fields.size()  ||  fields[fieldnr].empty()) {
    return def;
  }
  istringstream istr(fields[fieldnr]);
  double val;
  istr >> val;
  return val;
}

double string2pos (const vector<string>& fields, uint fieldnr, double def)
{
  if (fieldnr >= fields.size()  ||  fields[fieldnr].empty()) {
    return def;
  }
  Quantity q;
  ASSERT (MVAngle::read (q, fields[fieldnr]));
  return q.getValue ("rad");
}

void add (ParmMap& defVal, const string& name, double value)
{
  defVal.define (name, ParmValueSet(ParmValue(value)));
}

void process (const string& line, SourceDB& pdb, char sep)
{
  //  cout << line << endl;
  vector<string> fields = StringUtil::split (line, sep);
  ASSERT (fields.size() >= 2);
  string srcName = fields[0];
  SourceInfo::Type type = string2type (fields[1]);
  double ra = string2pos (fields, 2, -1e9);
  double dec = string2pos (fields, 3, -1e9);
  ParmMap defValues;
  double fluxI = string2real (fields, 4, 1);
  add (defValues, "I", fluxI);
  add (defValues, "Q", string2real (fields, 5, 0));
  add (defValues, "U", string2real (fields, 6, 0));
  add (defValues, "V", string2real (fields, 7, 0));
  add (defValues, "SpInx", string2real (fields, 8, 1));
  if (type == SourceInfo::GAUSSIAN) {
    add (defValues, "Major", string2real (fields, 9,  1));
    add (defValues, "Minor", string2real (fields, 10, 1));
    add (defValues, "Phi",   string2real (fields, 11, 1));
  }
  // Add the source (as Cat2).
  // Do not check for duplicates yet.
  pdb.addSource (srcName, 2, fluxI, type, defValues, ra, dec, false);
}

void make (const char* in, const string& out, char sep)
{
  ParmDBMeta ptm("casa", out);
  SourceDB pdb(ptm, true);
  ifstream infile(in);
  ASSERTSTR (infile, "File " << in << " could not be opened");
  string line;
  // Read first line.
  getline (infile, line);
  while (infile) {
    // Remove comments.
    bool skip = true;
    for (uint i=0; i<line.size(); ++i) {
      if (line[i] == '#') {
        break;
      }
      if (line[i] != ' '  &&  line[i] != '\t') {
        skip = false;
        break;
      }
    }
    if (!skip) {
      process (line, pdb, sep);
    }
    // Read next line
    getline (infile, line);
  }
  pdb.checkDuplicates();
}

int main (int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  // Get the arguments.
  ASSERTSTR (argc >= 3, "Run as:   makesourcedb inname outname [separator]");
  try {
    // Default separator is a comma.
    char sep = ',';
    if (argc > 3  &&  argv[3][0] != 0) sep = argv[3][0];
    make (argv[1], argv[2], sep);
  } catch (std::exception& x) {
    std::cerr << "Caught exception: " << x.what() << std::endl;
    return 1;
  }
  
  return 0;
}

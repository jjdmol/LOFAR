//# pkgdep.cc: Find dependencies of packages
//#
//# Copyright (C) 2004
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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>

using namespace std;

// Define the struct to hold the usage.
struct Used
{
  // Constructor.
  Used() : itsNused(0) {};
  // Number of times the package is used.
  int          itsNused;
  // Names of the packages being used.
  set<string> itsUses;
};

// Define the map to hold the usage.
typedef map<string,Used> UsedMap;

// Write the dependency tree.
void writeDep (const string& pkg, UsedMap& dep, const string& indent,
	       int depth, int maxdepth)
{
  cout << indent << pkg << endl;
  // write used packages if any and if maxdepth not reached.
  if (maxdepth < 0  ||  depth < maxdepth) {
    string newIndent = indent + ' ';
    set<string> uses = dep[pkg].itsUses;
    for (set<string>::const_iterator iter = uses.begin();
	 iter != uses.end();
	 iter++) {
      writeDep (*iter, dep, newIndent, depth+1, maxdepth);
    }
  }
}

// Determine all dependencies.
void findFlatDep (const string& pkg, UsedMap& dep, set<string>& flatUses)
{
  set<string> uses = dep[pkg].itsUses;
  for (set<string>::const_iterator iter = uses.begin();
       iter != uses.end();
       iter++) {
    flatUses.insert (*iter);
    findFlatDep (*iter, dep, flatUses);
  }
}

int main(int argc, const char* argv[])
{
  if (argc < 2) {
    cerr << "Use as:   pkgdep inputfile [flat|maxdepth=-1]" << endl;
    return 1;
  }
  bool flat=false;
  int maxdepth=-1;
  if (argc > 2) {
    if (string(argv[2]) == "flat") {
      flat = true;
      maxdepth = 1;
    } else {
      istringstream istr(argv[2]);
      istr >> maxdepth;
    }
  }

  // Hold the dependencies and the used in.
  UsedMap dep;

  // Read all packages and record which packages it uses and how often used.
  ifstream ifs(argv[1]);
  string pkg1, pkg2;
  while (ifs) {
    ifs >> pkg1;
    if (ifs) {
      ifs >> pkg2;
      dep[pkg1].itsUses.insert (pkg2);
      dep[pkg2].itsNused++;
    }
  }

  UsedMap* depPtr = &dep;
  // If flat dependencies have to be determined, do so for all packages.
  UsedMap flatdep;
  if (flat) {
    for (UsedMap::const_iterator iter = dep.begin();
	 iter != dep.end();
	 iter++) {
      findFlatDep (iter->first, dep, flatdep[iter->first].itsUses);
    }
    depPtr = &flatdep;
  }
    
  // Write the dependencies starting at all root packages
  // (i.e. packages not used by others).
  for (UsedMap::const_iterator iter = depPtr->begin();
       iter != depPtr->end();
       iter++) {
    //    if (iter->second.itsNused == 0) {
      writeDep (iter->first, *depPtr, "", 0, maxdepth);
      //    }
  }
}

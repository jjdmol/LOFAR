//# tParmFacade.cc: test program for class ParmFacade
//#
//# Copyright (C) 2006
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
#include <ParmDB/ParmFacade.h>
#include <Common/StreamUtil.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayIO.h>
#include <iostream>
#include <sstream>

using namespace LOFAR;
using namespace BBS;
using namespace casa;
using namespace std;

// tParmFacade.run creates the pdb with parmdb.
// So it must match the output expected here.

void showValues (ParmFacade& acc, const string& pattern, int nf, int nt)
{
  // Now get the values of all parameters.
  vector<string> names = acc.getNames(pattern);
  cout << "Names: " << names << endl;
  vector<double> rng = acc.getRange("*");
  cout << "Range: " << rng << endl;
  map<string,vector<double> > values = acc.getValuesMap (pattern,
                                                         rng[0], rng[2], nf,
                                                         rng[1], rng[3], nt,
                                                         true);
  for (map<string,vector<double> >::const_iterator iter=values.begin();
       iter != values.end();
       iter++) {
    cout <<iter->first << ' ' << iter->second << endl;
  }
  cout << endl;
  // Get values without giving a grid.
  Record rec = acc.getValuesGrid (pattern, rng[0], rng[2], rng[1], rng[3]);
  for (uint i=0; i<rec.nfields(); ++i) {
    if (rec.name(i) != "_grid") {
      cout << rec.name(i) << ' ' << rec.asArrayDouble(i) << endl;
    }
  }
  Record ginfo = rec.subRecord ("_grid");
  for (uint i=0; i<ginfo.nfields(); ++i) {
    cout << ginfo.name(i) << ' ' << ginfo.asArrayDouble(i) << endl;
  }
}

int main (int argc, const char* argv[])
{
  try {
    if (argc < 2) {
      cerr << "Run as: tParmFacade parmtable [pattern nf nt]"
	   << endl;
      return 1;
    }
    string pattern ="*";
    int nf = 4;
    int nt = 2;
    if (argc > 2) {
      pattern = argv[2];
    }
    if (argc > 3) {
      istringstream istr(argv[3]);
      istr >> nf;
      if (nf <= 0) nf = 1;
    }
    if (argc > 4) {
      istringstream istr(argv[4]);
      istr >> nt;
      if (nt <= 0) nt = 1;
    }
    // Open the facade.
    ParmFacade acc(argv[1]);
    // Get values.
    showValues (acc, pattern, nf, nt);
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

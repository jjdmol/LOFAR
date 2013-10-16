//# tParmFacade.cc: test program for class ParmFacade
//#
//# Copyright (C) 2006
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

void showDefRec (const Record& rec)
{
  for (uint i=0; i<rec.nfields(); ++i) {
    cout << rec.name(i) << ' ' << rec.asArrayDouble(i) << endl;
  }
}

void showRec (const Record& rec)
{
  cout << ">start<" << endl;
  for (uint i=0; i<rec.nfields(); ++i) {
    Record rc = rec.subRecord (i);
    cout << rec.name(i) << ' ' << rc.asArrayDouble("values") << endl;
    cout << " freqs: " << rc.asArrayDouble("freqs") << endl;
    cout << " times: " << rc.asArrayDouble("times") << endl;
    cout << " freqwidths: " << rc.asArrayDouble("freqwidths") << endl;
    cout << " timewidths: " << rc.asArrayDouble("timewidths") << endl;
    cout << endl;
  }
  cout << ">end<" << endl;
}

void showValues (ParmFacade& acc, const string& pattern, int nf, int nt,
                 bool includeDefaults)
{
  // Now get the values of all parameters.
  vector<string> names = acc.getNames(pattern, includeDefaults);
  cout << ">start<" << endl;
  cout << "Names: " << names << endl;
  vector<double> rng = acc.getRange("*");
  cout << "Range: " << rng << endl;
  cout << ">end<" << endl;
  double fs = (rng[1] - rng[0]) / nf;
  double ts = (rng[3] - rng[2]) / nt;
  map<string,vector<double> > values = acc.getValuesMap (pattern,
                                                         rng[0], rng[1], fs,
                                                         rng[2], rng[3], ts,
                                                         true, includeDefaults);
  cout << ">start<" << endl;
  for (map<string,vector<double> >::const_iterator iter=values.begin();
       iter != values.end();
       iter++) {
    cout <<iter->first << ' ' << iter->second << endl;
  }
  cout << endl;
  cout << ">end<" << endl;
  // Get values without giving a grid.
  // Exactly matching domain.
  showRec (acc.getValuesGrid (pattern, rng[0], rng[1], rng[2], rng[3]));
  // Much bigger domain.
  showRec (acc.getValuesGrid (pattern, 0,20,0,20));
  // No matches.
  showRec (acc.getValuesGrid (pattern, 20,40,20,40));
  // A smaller domain (so using only 2nd part for tParmFacadeDistr).
  showRec (acc.getValuesGrid (pattern, 10,16,6,20));
  // Get the coeff and errors.
  cout << ">start<" << endl;
  cout << acc.getCoeff (pattern) << endl;
  // Set and get the default freq/time step.
  vector<double> steps(2);
  steps[0] = 1.;
  steps[1] = 1.5;
  acc.setDefaultSteps (steps);
  cout << "defaultsteps=" << acc.getDefaultSteps() << endl;
  cout << ">end<" << endl;
}

void createPDB (const string& name)
{
    Matrix<Double> vald(2,1);
    Record recd, recds, recs;
  {
    // Create a new local ParmDB.
    ParmFacade acc(name, true);
    vald(0,0) = 3.;
    vald(1,0) = 1.;
    recds.define ("value", vald);
    recd.defineRecord ("parmdef", recds);
    acc.addDefValues (recd);
    Record rec1;
    recs.define ("freqs",      Vector<Double>(1,1.));
    recs.define ("freqwidths", Vector<Double>(1,3.));
    recs.define ("times",      Vector<Double>(1,5.));
    recs.define ("timewidths", Vector<Double>(1,5.));
    recs.define ("values",     Matrix<Double>(1,1,2.));
    rec1.defineRecord ("parm1", recs);
    acc.addValues (rec1);
  }
  {
    // Open the new local ParmDB.
    ParmFacade acc(name);
    recs.removeField ("values");
    vald(0,0) = 2;
    vald(1,0) = 0.1;
    recs.define ("values", vald);
    recs.define ("type", "polc");
    Record rec2;
    rec2.defineRecord ("parm2", recs);
    acc.addValues (rec2);
  }
}

void showhelp()
{
  cerr << "Run as: tParmFacade [-c] parmtable [pattern nf nt]" << endl;
  cerr << "    -c  create a new local ParmDB" << endl;
}

int main (int argc, const char* argv[])
{
  try {
    int  starg  = 1;
    bool create = false;
    if (argc > 1  &&  String(argv[1]) == "-c") {
      starg  = 2;
      create = true;
    }
    if (argc < starg+1) {
      showhelp();
      return 1;
    }
    string pattern ="*";
    int nf = 4;
    int nt = 2;
    if (argc > starg+1) {
      pattern = argv[starg+1];
    }
    if (argc > starg+2) {
      istringstream istr(argv[starg+2]);
      istr >> nf;
      if (nf <= 0) nf = 1;
    }
    if (argc > starg+3) {
      istringstream istr(argv[starg+3]);
      istr >> nt;
      if (nt <= 0) nt = 1;
    }
    // If needed, create the ParmTable using ParmFacade.
    if (create) {
      createPDB (argv[starg]);
    }
    // Open the facade.
    ParmFacade acc(argv[starg]);
    // Get values.
    cout << endl << "includedefaults=false..." << endl;
    showValues (acc, pattern, nf, nt, false);
    cout << endl << "includedefaults=true..." << endl;
    showValues (acc, pattern, nf, nt, true);
    // Get default values.
    cout << ">start<" << endl;
    cout << "get default values ..." << endl;
    cout << acc.getDefNames() << endl;
    cout << acc.getDefNames("parm*") << endl;
    cout << acc.getDefNames("parx*") << endl;
    showDefRec (acc.getDefValues());
    cout << ">end<" << endl;
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

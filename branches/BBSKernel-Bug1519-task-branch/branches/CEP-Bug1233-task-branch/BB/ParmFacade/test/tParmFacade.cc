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


#include <ParmFacade/ParmFacade.h>
#include <Common/StreamUtil.h>
#include <iostream>
#include <sstream>

using namespace LOFAR;
using namespace ParmDB;
using namespace std;

// Create tParmFacade.in_mep with parmdb using:
//   create tablename='tParmFacade.in_mep'
//   add parm1 domain=[1,5,4,10],values=2
//   add parm2 domain=[1,5,4,10],values=[2,0.1],nx=2
//   adddef parm3 type='parmexpr',expr='parm1*parm2'
//   adddef parm4 type='parmexpr',expr='parm2*parm3'


void showValues (ParmFacade& acc, const string& pattern, int nf, int nt)
{
  // Show all expressions.
  const map<string,set<string> >& exprs = acc.getExprs();
  cout << "Parm expressions: " << endl;
  for (map<string,set<string> >::const_iterator iter=exprs.begin();
       iter != exprs.end();
       iter++) {
    cout << "  " << iter->first << " using";
    for (set<string>::const_iterator ch=iter->second.begin();
	 ch != iter->second.end();
	 ch++) {
      cout << ' ' << *ch;
    }
    cout << endl;
  }
  // Now get the values of all parameters.
  vector<string> names = acc.getNames(pattern);
  cout << "Names: " << names << endl;
  vector<double> rng = acc.getRange("*");
  cout << "Range: " << rng << endl;
  map<string,vector<double> > values = acc.getValues (pattern,
						      rng[0], rng[1], nf,
						      rng[2], rng[3], nt);
  for (map<string,vector<double> >::const_iterator iter=values.begin();
       iter != values.end();
       iter++) {
    cout <<iter->first << ' ' << iter->second << endl;
  }
}

void showHistory (ParmFacade& acc, const string& pattern)
{
  map<string,vector<double> > values = acc.getHistory (pattern,
						       0, 1e25,
						       0, 1e25);
  for (map<string,vector<double> >::const_iterator iter=values.begin();
       iter != values.end();
       iter++) {
    cout <<iter->first << ' ' << iter->second << endl;
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
    }
    if (argc > 4) {
      istringstream istr(argv[4]);
      istr >> nt;
    }
    // Open the facade.
    ParmFacade acc(argv[1]);
    if (nf > 0) {
      // Get values.
      showValues (acc, pattern, nf, nt);
    } else {
      showHistory (acc, pattern);
    }
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

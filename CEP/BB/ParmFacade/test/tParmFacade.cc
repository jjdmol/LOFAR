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
#include <Common/VectorUtil.h>
#include <iostream>

using namespace LOFAR;
using namespace ParmDB;
using namespace std;


int main (int argc, const char* argv[])
{
  try {
    if (argc < 2) {
      cerr << "Run as: tParmFacade parmtable"
	   << endl;
      return 1;
    }

    ParmFacade acc(argv[1]);
    vector<string> names = acc.getNames();
    cout << "Names: " << names << endl;
    vector<double> range = acc.getRange();
    cout << "Range: " << range << endl;
    map<string,vector<double> > values = acc.getValues ("*",
							range[0], range[1], 4,
							range[2], range[3], 2);
    for (map<string,vector<double> >::const_iterator iter=values.begin();
	 iter != values.end();
	 iter++) {
      cout <<iter->first << ' ' << iter->second << endl;
    }

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

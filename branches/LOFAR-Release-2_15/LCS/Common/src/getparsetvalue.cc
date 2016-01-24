//# getparsetvalue.cc: Get a value as a string from a parameter set
//#
//# Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/ParameterSet.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <stdexcept>
#include <iostream>

using namespace std;

void showHelp()
{
  cerr << endl
       << "getparsetvalue gives the string value of a parset parameter."
       << endl
       << "If the value in the parset is a vector of values, the i-th value"
       << endl
       << "can be obtained by giving an index as the 3rd argument."
       << endl
       << "An exception is raised if the index exceeds the vector length."
       << endl
       << endl
       << "Using the -d option a default value can be supplied which will be"           << endl
       << "used if the parameter does not exist."
       << endl
       << "If the parameter does not exist and an index is given, the default"          << endl
       << "value is used as a single value and the index is ignored."
       << endl
       << "Note that a parameter exists if its value is empty!"
       << endl
       << endl
       << "Run as:"
       << endl
       <<"     getparsetvalue [-d defaultvalue] parsetfile parmname [index]"
       << endl
       << "         a negative index counts from the end (a la python)"
       << endl
       << endl;
}

int main (int argc, const char* argv[])
{
  int st = 1;
  try {
    bool hasDefVal = false;
    string defVal;
    while (true) {
      if (argc < 3) {
        showHelp();
        return 1;
      }
      if (string(argv[st]) == "-d") {
        hasDefVal = true;
        defVal = argv[st+1];
        st += 2;
        argc -= 2;
      } else {
        break;
      }
    }
    if (argc < 2 || argc > 4) {
      showHelp();
      return 1;
    }
    int inx = 0;
    bool useAll = true;
    if (argc > 3) {
      inx = LOFAR::strToInt (argv[st+2]);
      useAll = false;
    }
    LOFAR::ParameterSet parset(argv[st]);
    if (useAll) {
      string value;
      if (hasDefVal) {
        value = parset.getString (argv[st+1], defVal);
      } else {
        value = parset.getString (argv[st+1]);
      }
      cout << value << endl;
    } else {
      if (hasDefVal) {
        if (! parset.isDefined (argv[st+1])) {
          cout << defVal << endl;
          return 0;
        }
      }
      vector<string> values = parset.getStringVector (argv[st+1]);
      // Negative index counts from the end.
      int i = inx;
      if (i < 0) {
	i += values.size();
      }
      ASSERTSTR (i >= 0  &&  i < int(values.size()),
		 "Index " << inx
		 << " exceeds the number of values " << values.size()
		 << " of parameter " << argv[st+1]);
      cout << values[i] << endl;
    }
  } catch (exception &x) {
    cerr << x.what() << "; file=" << argv[st] << ", key=" << argv[st+1]
         << endl;
    return 1;
  }
  return 0;
}

//# msselect.cc: Create a persistent selection of an MS
//#
//# Copyright (C) 2005
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

#include <ms/MeasurementSets/MSSelection.h>
#include <casa/Inputs/Input.h>
#include <iostream>

using namespace casa;
using namespace std;

int main (int argc, char* argv[])
{
  try {
    // enable input in no-prompt mode
    Input inputs(1);
    // define the input structure
    inputs.version("20100520GvD");
    inputs.create ("in", "",
		   "Name of input MeasurementSet",
		   "string");
    inputs.create ("out", "",
		   "Name of output table",
		   "string");
    inputs.create ("deep", "0",
		   "Is the output a deep copy of the MeasurementSet selection?" 
		   "int");
    inputs.create ("baseline", "",
                   "selection string for antennae and baselines",
                   "string");
    // Fill the input structure from the command line.
    inputs.readArguments (argc, argv);

    // Get and check the input specification.
    String msin (inputs.getString("in"));
    if (msin.empty()) {
      throw AipsError(" an input MeasurementSet must be given");
    }
    // Get the output name.
    String out(inputs.getString("out"));
    if (out.empty()) {
      throw AipsError(" an output table name must be given");
    }
    // Get the deep option.
    int deep = inputs.getInt("deep");
    // Get the baseline selection string.
    string baseline(inputs.getString("baseline"));

    MeasurementSet ms(msin);
    MSSelection select;
    // Set given selection strings.
    if (!baseline.empty()) {
      select.setAntennaExpr (baseline);
    }
    // Create a table expression over a MS representing the selection
    TableExprNode node = select.toTableExprNode (&ms);
    // Make the selection and write the resulting RefTable.
    // If no selection was made, create it explicitly from all rows.
    Table mssel = ms(node);
    if (mssel.nrow() == ms.nrow()) {
      mssel = ms(ms.rowNumbers());
    }
    if (deep == 0) {
      mssel.rename (out, Table::New);
      cout << "Created RefTable " << out;
    } else {
      mssel.deepCopy (out, Table::New);
      cout << "Created MeasurementSet " << out;
    }
    cout << " containing " << mssel.nrow() << " rows (out of "
         << ms.nrow() << ')' << endl;
  } catch (std::exception& x) {
    cerr << "Error: " << x.what() << endl;
      return 1;
  }
  return 0;
}

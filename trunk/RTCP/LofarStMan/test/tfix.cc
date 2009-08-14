//# tfix.cc: Fix oldMSs by adding WEIGHT_SPECTRUM column
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <LofarStMan/LofarStMan.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrColDesc.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

using namespace LOFAR;
using namespace casa;

// This program tests the class LofarStMan and related classes.
// The results are written to stdout. The script executing this program,
// compares the results with the reference output file.


void fixTable (const String& name)
{
  Table t(name, Table::Update);
  if (t.tableDesc().isColumn("WEIGHT_SPECTRUM")) {
    cout << "MS already contains column WEIGHT_SPECTRUM" << endl;
  } else {
    TableDesc td;
    ArrayColumnDesc<Float> cd("WEIGHT_SPECTRUM");
    t.addColumn (cd, "LofarStMan", True);
    cout << "Added column WEIGHT_SPECTRUM to the MS" << endl;
  }
}

int main (int argc, char* argv[])
{
  try {
    // Register LofarStMan to be able to read it back.
    LofarStMan::registerClass();
    if (argc <= 1) {
      cout << "Run as:  tfix msname" << endl;
      return 0;
    }
    fixTable (argv[1]);
  } catch (AipsError x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}

//# tMSData.cc: Program to check if Prediffer reads correct data
//#
//# Copyright (C) 2005
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
#include <BBS3/Prediffer.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayLogical.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

void doIt (const string& msName, Prediffer& prediff, const string& column,
	   int stchan, int nrchan)
{
  cout << "Checking data in " << msName << " for channels " << stchan << '-'
       << stchan+nrchan-1 << endl;
  // Open the table and sort in time,baseline order.
  Table tab(msName);
  Block<String> sortkeys(3);
  sortkeys[0] = "TIME";
  sortkeys[1] = "ANTENNA1";
  sortkeys[2] = "ANTENNA2";
  Table tabs(tab.sort (sortkeys));
  // Create an iterator to iterate in time order.
  Block<String> iterkeys(1);
  iterkeys[0] = "TIME";
  TableIterator tabIter(tabs, iterkeys, TableIterator::DontCare);
  // Loop through all times and check if data in mapped file is
  // the same as in the original table.
  Array<Complex> data;
  Array<Bool> flags;
  while (! tabIter.pastEnd()) {
    ROArrayColumn<Complex> dataCol(tabIter.table(), column);
    ROArrayColumn<Bool> flagCol(tabIter.table(), "FLAG");
    ROScalarColumn<Double> timeCol(tabIter.table(), "TIME");
    ROScalarColumn<Double> intCol(tabIter.table(), "INTERVAL");
    double time = timeCol(0);
    double interval = intCol(0);
    // Set a domain of some channels and this time.
    prediff.setDomain (137750000-250000+stchan*500000, nrchan*500000,
		       time-interval/2, interval);
    // Get the data and remove the time axis.
    prediff.getData (data, flags);
    Array<Complex> data1 = data.nonDegenerate (IPosition(3,0,1,2));
    Array<Bool> flags1 = flags.nonDegenerate (IPosition(3,0,1,2));
    int ncorr = data1.shape()[0];
    Slicer slicer(IPosition(2,0,stchan), IPosition(2,ncorr,nrchan));
    Array<Complex> data2 = dataCol.getColumn(slicer);
    ASSERT (allEQ (data1, data2));
    ASSERT (allEQ (flags1, flagCol.getColumn(slicer)));
    tabIter++;
  }
}

int main(int argc, char** argv)
{
  try {
    if (argc < 3) {
      cout << "Run as:  tMSData ms user msname meqparmtable skyparmtable [datacolumn]" << endl;
      cout << "   datacolumn defaults to MODEL_DATA" << endl;
      return 0;
    }

    // Read the info for the ParmTables
    ACC::APS::ParameterSet ps;
    string meqModelName(argv[4]);
    string skyModelName(argv[5]);
    ps["meqModel"] = meqModelName;
    ps["skyModel"] = skyModelName;
    ps["DBType"] = "aips";
    ParmTableData meqPdt("meqModel", ps);
    ParmTableData skyPdt("skyModel", ps);


    string column("MODEL_DATA");
    if (argc > 6) {
      column = argv[6];
    }
    vector<int> antVec(100);
    for (uint i=0; i<antVec.size(); ++i) {
      antVec[i] = i;
    }
    vector<vector<int> > srcgrp;
    Prediffer pre (argv[3], "meqModel", meqPdt, "skyModel", skyPdt, 
		   antVec, "LOFAR", srcgrp, false);
    doIt (argv[1], pre, column, 0, 50);
    doIt (argv[1], pre, column, 10,10);
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}

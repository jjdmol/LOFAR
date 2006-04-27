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
#include <BBS/Prediffer.h>
#include <MS/MSDesc.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayLogical.h>
#include <fstream>
#include <sstream>

using namespace LOFAR;
using namespace casa;
using namespace std;

void doIt (const string& msName, Prediffer& prediff, const string& column,
	   int nrant, int stchan, int nrchan, bool useTree)
{
  cout << "Checking data in " << msName << " for channels " << stchan << '-'
       << stchan+nrchan-1 << endl;
  // Open the table and sort in time,baseline order.
  // Only use antenna < nrant.
  Table tab(msName);
  Table tabsel = tab(tab.col("ANTENNA1")<nrant && tab.col("ANTENNA2")<nrant);
  Block<String> sortkeys(3);
  sortkeys[0] = "TIME";
  sortkeys[1] = "ANTENNA1";
  sortkeys[2] = "ANTENNA2";
  Table tabs(tabsel.sort (sortkeys));
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
    prediff.setWorkDomain (stchan, stchan+nrchan-1,
			   time-interval/2, interval);
    // Get the data and remove the time axis.
    prediff.getData ("DATA", useTree, data, flags);
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
    if (argc < 7) {
      cout << "Run as:  tMSData ms user msname meqparmtable skyparmtable nrant [datacolumn]" << endl;
      cout << "   datacolumn defaults to MODEL_DATA" << endl;
      return 0;
    }

    // Read the info for the ParmTables
    ParmDB::ParmDBMeta meqPdm("aips", argv[4]);
    ParmDB::ParmDBMeta skyPdm("aips", argv[5]);

    // Open the description file.
    MSDesc msd;
    {
      string fileName (argv[3]);
      string name(fileName+"/vis.des");
      std::ifstream istr(name.c_str());
      ASSERTSTR (istr, "File " << fileName << "/vis.des could not be opened");
      BlobIBufStream bbs(istr);
      BlobIStream bis(bbs);
      bis >> msd;
    }

    uint nrant;
    std::istringstream istr(argv[6]);
    istr >> nrant;
    string column("MODEL_DATA");
    if (argc > 7) {
      column = argv[7];
    }
    if (nrant > msd.antNames.size()) {
      nrant = msd.antNames.size();
    }

    // Fill the antenna numbers.
    vector<int> antVec(nrant);
    for (uint i=0; i<antVec.size(); ++i) {
      antVec[i] = i;
    }
    vector<vector<int> > srcgrp;
    Prediffer pre (argv[3], meqPdm, skyPdm, 
		   antVec, "", srcgrp, false);
    doIt (argv[1], pre, column, nrant, 0, 50, false);
    doIt (argv[1], pre, column, nrant, 10,10, false);
    doIt (argv[1], pre, column, nrant, 0, 50, true);
    doIt (argv[1], pre, column, nrant, 10,11, true);
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}

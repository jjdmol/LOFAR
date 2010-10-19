//# tLofarStMan.cc: Test program for the LofarStMan class
//# Copyright (C) 2009
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

#include <LofarStMan/LofarStMan.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/BlockIO.h>
#include <casa/OS/RegularFile.h>
#include <casa/Utilities/Assert.h>
#include <casa/IO/RegularFileIO.h>
#include <casa/IO/RawIO.h>
#include <casa/IO/CanonicalIO.h>
#include <casa/OS/HostInfo.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

using namespace LOFAR;
using namespace casa;

// This program tests the class LofarStMan and related classes.
// The results are written to stdout. The script executing this program,
// compares the results with the reference output file.


void createTable()
{
  // Build the table description.
  // Add all mandatory columns of the MS main table.
  TableDesc td("", "1", TableDesc::Scratch);
  td.comment() = "A test of class Table";
  td.addColumn (ScalarColumnDesc<Double>("TIME"));
  td.addColumn (ScalarColumnDesc<Int>("ANTENNA1"));
  td.addColumn (ScalarColumnDesc<Int>("ANTENNA2"));
  td.addColumn (ScalarColumnDesc<Int>("FEED1"));
  td.addColumn (ScalarColumnDesc<Int>("FEED2"));
  td.addColumn (ScalarColumnDesc<Int>("DATA_DESC_ID"));
  td.addColumn (ScalarColumnDesc<Int>("PROCESSOR_ID"));
  td.addColumn (ScalarColumnDesc<Int>("FIELD_ID"));
  td.addColumn (ScalarColumnDesc<Int>("ARRAY_ID"));
  td.addColumn (ScalarColumnDesc<Int>("OBSERVATION_ID"));
  td.addColumn (ScalarColumnDesc<Int>("STATE_ID"));
  td.addColumn (ScalarColumnDesc<Int>("SCAN_NUMBER"));
  td.addColumn (ScalarColumnDesc<Double>("INTERVAL"));
  td.addColumn (ScalarColumnDesc<Double>("EXPOSURE"));
  td.addColumn (ScalarColumnDesc<Double>("TIME_CENTROID"));
  td.addColumn (ScalarColumnDesc<Bool>("FLAG_ROW"));
  td.addColumn (ArrayColumnDesc<Double>("UVW",IPosition(1,3),
                                        ColumnDesc::Direct));
  td.addColumn (ArrayColumnDesc<Complex>("DATA"));
  td.addColumn (ArrayColumnDesc<Float>("SIGMA"));
  td.addColumn (ArrayColumnDesc<Float>("WEIGHT"));
  td.addColumn (ArrayColumnDesc<Float>("WEIGHT_SPECTRUM"));
  td.addColumn (ArrayColumnDesc<Bool>("FLAG"));
  td.addColumn (ArrayColumnDesc<Bool>("FLAG_CATEGORY"));
  // Now create a new table from the description.
  SetupNewTable newtab("tLofarStMan_tmp.data", td, Table::New);
  // Create the storage manager and bind all columns to it.
  LofarStMan sm1;
  newtab.bindAll (sm1);
  // Finally create the table. The destructor writes it.
  Table tab(newtab);
}

uInt nalign (uInt size, uInt alignment)
{
  return (size + alignment-1) / alignment * alignment - size;
}

void createData (uInt nseq, uInt nant, uInt nchan, uInt npol,
                 Double startTime, Double interval, const Complex& startValue,
                 uInt alignment, Bool bigEndian, uInt myStManVersion=1)
{
  // Create the baseline vectors (no autocorrelations).
  uInt nrbl = nant*(nant-1)/2;
  Block<Int> ant1(nrbl);
  Block<Int> ant2(nrbl);
  uInt inx=0;
  for (uInt i=0; i<nant; ++i) {
    for (uInt j=i+1; j<nant; ++j) {
      ant1[inx] = i;
      ant2[inx] = j;
      ++inx;
    }
  }
  // Create the meta file. The destructor closes it.
  {
    // If not big-endian, use local format (which might be big-endian).
    if (!bigEndian) {
      bigEndian = HostInfo::bigEndian();
    }
    double maxNSample = 32768;

    AlwaysAssertExit(myStManVersion <= 2);

    AipsIO aio("tLofarStMan_tmp.data/table.f0meta", ByteIO::New);
    aio.putstart ("LofarStMan", myStManVersion);     // version 1 or 2
    aio << ant1 << ant2 << startTime << interval << nchan
        << npol << maxNSample << alignment << bigEndian;
  }
  // Now create the data file.
  RegularFileIO file(RegularFile("tLofarStMan_tmp.data/table.f0data"),
                     ByteIO::New);
  // Write in canonical (big endian) or local format.
  TypeIO* cfile;
  if (bigEndian) {
    cfile = new CanonicalIO(&file);
  } else {
    cfile = new RawIO(&file);
  }
  // Create and initialize data and nsample.
  Array<Complex> data(IPosition(2,npol,nchan));
  indgen (data, startValue, Complex(0.01, 0.01));

  Array<uShort> nsample(IPosition(1,nchan));
  indgen (nsample);
  Array<uInt> nsampleV2(IPosition(1,1));
  indgen(nsampleV2);

  // Allocate space for possible block alignment.
  if (alignment < 1) {
    alignment = 1;
  }
  Block<Char> align1(nalign(4, alignment), 0);
  Block<Char> align2(nalign(nrbl*8*data.size(), alignment), 0);

  uInt nsamplesSize=0;
  if (myStManVersion == 1) nsamplesSize = nrbl*2*nsample.size();
  else nsamplesSize = nrbl*2*nsampleV2.size();

  Block<Char> align3(nalign(nsamplesSize, alignment), 0);

  // Write the data as nseq blocks.
  for (uInt i=0; i<nseq; ++i) {
    cfile->write (1, &i);
    if (align1.size() > 0) {
      cfile->write (align1.size(), align1.storage());
    }
    for (uInt j=0; j<nrbl; ++j) {
      cfile->write (data.size(), data.data());
      data += Complex(0.01, 0.02);
    }
    if (align2.size() > 0) {
      cfile->write (align2.size(), align2.storage());
    }
    
    if (myStManVersion == 1) {
      for (uInt j=0; j<nrbl; ++j) {
	cfile->write (nsample.size(), nsample.data());
	nsample += uShort(1);
      }
    } else {
      for (uInt j=0; j<nrbl; ++j) {
	cfile->write (nsampleV2.size(), nsampleV2.data());
	nsampleV2 += uInt(1);
      }
    }

    if (align3.size() > 0) {
      cfile->write (align3.size(), align3.storage());
    }
  }
  delete cfile;
}

void readTable (uInt nseq, uInt nant, uInt nchan, uInt npol,
                Double startTime, Double interval, const Complex& startValue, 
		uInt myStManVersion=1)
{
  uInt nbasel = nant*(nant-1)/2;
  // Open the table and check if #rows is as expected.
  Table tab("tLofarStMan_tmp.data");
  uInt nrow = tab.nrow();
  AlwaysAssertExit (nrow = nseq*nbasel);
  AlwaysAssertExit (!tab.canAddRow());
  AlwaysAssertExit (!tab.canRemoveRow());
  AlwaysAssertExit (tab.canRemoveColumn(Vector<String>(1, "DATA")));
  // Create objects for all mandatory MS columns.
  ROArrayColumn<Complex> dataCol(tab, "DATA");
  ROArrayColumn<Float> weightCol(tab, "WEIGHT");
  ROArrayColumn<Float> wspecCol(tab, "WEIGHT_SPECTRUM");
  ROArrayColumn<Float> sigmaCol(tab, "SIGMA");
  ROArrayColumn<Double> uvwCol(tab, "UVW");
  ROArrayColumn<Bool> flagCol(tab, "FLAG");
  ROArrayColumn<Bool> flagcatCol(tab, "FLAG_CATEGORY");
  ROScalarColumn<Double> timeCol(tab, "TIME");
  ROScalarColumn<Double> centCol(tab, "TIME_CENTROID");
  ROScalarColumn<Double> intvCol(tab, "INTERVAL");
  ROScalarColumn<Double> expoCol(tab, "EXPOSURE");
  ROScalarColumn<Int> ant1Col(tab, "ANTENNA1");
  ROScalarColumn<Int> ant2Col(tab, "ANTENNA2");
  ROScalarColumn<Int> feed1Col(tab, "FEED1");
  ROScalarColumn<Int> feed2Col(tab, "FEED2");
  ROScalarColumn<Int> ddidCol(tab, "DATA_DESC_ID");
  ROScalarColumn<Int> pridCol(tab, "PROCESSOR_ID");
  ROScalarColumn<Int> fldidCol(tab, "FIELD_ID");
  ROScalarColumn<Int> arridCol(tab, "ARRAY_ID");
  ROScalarColumn<Int> obsidCol(tab, "OBSERVATION_ID");
  ROScalarColumn<Int> stidCol(tab, "STATE_ID");
  ROScalarColumn<Int> scnrCol(tab, "SCAN_NUMBER");
  ROScalarColumn<Bool> flagrowCol(tab, "FLAG_ROW");
  // Create and initialize expected data and weight.
  Array<Complex> dataExp(IPosition(2,npol,nchan));
  indgen (dataExp, startValue, Complex(0.01, 0.01));
  Array<Float> weightExp(IPosition(2,1,nchan));
  
  if (myStManVersion == 1) {
    indgen (weightExp, Float(0),Float(1./32768));
  }else {
    indgen (weightExp, Float(0), Float(0));
  }
  // Loop through all rows in the table and check the data.
  uInt row=0;
  for (uInt i=0; i<nseq; ++i) {
    for (uInt j=0; j<nant; ++j) {
      for (uInt k=j+1; k<nant; ++k) {
        // Contents must be present except for FLAG_CATEGORY.
//         AlwaysAssertExit (dataCol.isDefined (row));
        AlwaysAssertExit (weightCol.isDefined (row));
        AlwaysAssertExit (wspecCol.isDefined (row));
        AlwaysAssertExit (sigmaCol.isDefined (row));
        AlwaysAssertExit (flagCol.isDefined (row));
        AlwaysAssertExit (!flagcatCol.isDefined (row));
        // Check data, weight, sigma, weight_spectrum, flag
        AlwaysAssertExit (allNear (dataCol(row), dataExp, 1e-7));
        AlwaysAssertExit (weightCol.shape(row) == IPosition(1,npol));
        AlwaysAssertExit (allEQ (weightCol(row), Float(1)));
        AlwaysAssertExit (sigmaCol.shape(row) == IPosition(1,npol));
        AlwaysAssertExit (allEQ (sigmaCol(row), Float(1)));
        Array<Float> weights = wspecCol(row);
        AlwaysAssertExit (weights.shape() == IPosition(2,npol,nchan));
        for (uInt p=0; p<npol; ++p) {

	  std::cout << "weights: " << std::endl;
	  std::cout << weights(IPosition(2,p,0), IPosition(2,p,nchan-1)) << std::endl;
	  
	  std::cout << "wegithExp: " << std::endl;
	  std::cout << weightExp << std:: endl;

          AlwaysAssertExit (allNear (weights(IPosition(2,p,0),
                                             IPosition(2,p,nchan-1)),
                                     weightExp, 1e-7));
        }
        Array<Bool> flagExp (weights == Float(0));
        AlwaysAssertExit (allEQ (flagCol(row), flagExp));
        // Check ANTENNA1 and ANTENNA2
        AlwaysAssertExit (ant1Col(row) == int32(j));
        AlwaysAssertExit (ant2Col(row) == int32(k));
        dataExp += Complex(0.01, 0.02);

        if (myStManVersion == 1) weightExp += Float(1./32768);
	else {
	  weightExp += Float(512);
	  weightExp(IPosition(2,0,0)) = Float(0.);
	}
        ++row;
      }
    }
  }
  // Check values in TIME column.
  Vector<Double> times = timeCol.getColumn();
  AlwaysAssertExit (times.size() == nrow);
  row=0;
  startTime += interval/2;
  for (uInt i=0; i<nseq; ++i) {
    for (uInt j=0; j<nbasel; ++j) {
      AlwaysAssertExit (near(times[row], startTime));
      ++row;
    }
    startTime += interval;
  }
  // Check the other columns (UVW is 0).
  AlwaysAssertExit (allNear(centCol.getColumn(), times, 1e-13));
  AlwaysAssertExit (allNear(intvCol.getColumn(), interval, 1e-13));
  AlwaysAssertExit (allNear(expoCol.getColumn(), interval, 1e-13));
  AlwaysAssertExit (allEQ(uvwCol.getColumn(), 0.));
  AlwaysAssertExit (allEQ(feed1Col.getColumn(), 0));
  AlwaysAssertExit (allEQ(feed2Col.getColumn(), 0));
  AlwaysAssertExit (allEQ(ddidCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(pridCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(fldidCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(arridCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(obsidCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(stidCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(scnrCol.getColumn(), 0));
  AlwaysAssertExit (allEQ(flagrowCol.getColumn(), False));
}

void updateTable (uInt nchan, uInt npol, const Complex& startValue)
{
  // Open the table for write.
  Table tab("tLofarStMan_tmp.data", Table::Update);
  uInt nrow = tab.nrow();
  // Create object for DATA column.
  ArrayColumn<Complex> dataCol(tab, "DATA");
  // Check we can write the column, but not change the shape.
  AlwaysAssertExit (tab.isColumnWritable ("DATA"));
  AlwaysAssertExit (!dataCol.canChangeShape());
  // Create and initialize data.
  Array<Complex> data(IPosition(2,npol,nchan));
  indgen (data, startValue, Complex(0.01, 0.01));
  // Loop through all rows in the table and write the data.
  for (uInt row=0; row<nrow; ++row) {
    dataCol.put (row, data);
    data += Complex(0.01, 0.02);
  }
}

void copyTable()
{
  Table tab("tLofarStMan_tmp.data");
  // Deep copy the table.
  tab.deepCopy ("tLofarStMan_tmp.datcp", Table::New, true);
}


int main (int argc, char* argv[])
{
  try {
    // Register LofarStMan to be able to read it back.
    LofarStMan::registerClass();
    // Get nseq, nant, nchan, npol from argv.
    uInt nseq=10;
    uInt nant=16;
    uInt nchan=256;
    uInt npol=4;
    if (argc > 1) {
      istringstream istr(argv[1]);
      istr >> nseq;
    }
    if (nseq == 0) {
      Table tab(argv[2]);
      cout << "nrow=" << tab.nrow() << endl;
      ROArrayColumn<double> uvwcol(tab, "UVW");
      cout << "uvws="<< uvwcol(0) << endl;
      cout << "uvws="<< uvwcol(1) << endl;
      cout << "uvwe="<< uvwcol(tab.nrow()-1) << endl;
    } else {
      if (argc > 2) {
        istringstream istr(argv[2]);
        istr >> nant;
      }
      if (argc > 3) {
        istringstream istr(argv[3]);
        istr >> nchan;
      }
      if (argc > 4) {
        istringstream istr(argv[4]);
        istr >> npol;
      }
      // Create the table.
      createTable();
      // Write data in big-endian and check it. Align on 512.
      createData (nseq, nant, nchan, npol, 1e9, 10., Complex(0.1, 0.1),
                  512, True);
      readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(0.1, 0.1));
      // Update the table and check again.
      updateTable (nchan, npol, Complex(-3.52, -20.3));
      readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(-3.52, -20.3));
      // Write data in local format and check it. No alignment.
      createData (nseq, nant, nchan, npol, 1e9, 10., Complex(3.1, -5.2),
                  0, False);
      readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(3.1, -5.2));
      // Update the table and check again.
      updateTable (nchan, npol, Complex(3.52, 20.3));
      readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(3.52, 20.3));
      copyTable();

      // Create the table.
//        createTable();
//        // Write data in big-endian and check it. Align on 512.
//        createData (nseq, nant, nchan, npol, 1e9, 10., Complex(0.1, 0.1),
//                    512, True, 2);
//        readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(0.1, 0.1), 2);
//       // Update the table and check again.
//       updateTable (nchan, npol, Complex(-3.52, -20.3));
//       readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(-3.52, -20.3));
//       // Write data in local format and check it. No alignment.
//       createData (nseq, nant, nchan, npol, 1e9, 10., Complex(3.1, -5.2),
//                   0, False);
//       readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(3.1, -5.2));
//       // Update the table and check again.
//       updateTable (nchan, npol, Complex(3.52, 20.3));
//       readTable  (nseq, nant, nchan, npol, 1e9, 10., Complex(3.52, 20.3));
//       copyTable();


    }
  } catch (AipsError x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}

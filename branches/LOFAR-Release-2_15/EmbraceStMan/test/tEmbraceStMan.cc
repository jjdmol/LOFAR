//# tEmbraceStMan.cc: Test program for the EmbraceStMan class
//# Copyright (C) 2012
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
#include <EmbraceStMan/EmbraceStMan.h>
#include <Common/ParameterSet.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/BlockIO.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Path.h>
#include <casa/Utilities/Assert.h>
#include <casa/IO/RegularFileIO.h>
#include <casa/IO/RawIO.h>
#include <casa/IO/CanonicalIO.h>
#include <casa/OS/HostInfo.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

using namespace EMBRACE;
using namespace LOFAR;
using namespace casa;

// This program tests the class EmbraceStMan and related classes.
// The results are written to stdout. The script executing this program,
// compares the results with the reference output file.
//
// It uses the given phase center, station positions, times, and baselines to
// get known UVW coordinates. In this way the UVW calculation can be checked.


void createField (Table& mainTable)
{
  // Build the table description.
  TableDesc td;
  td.addColumn (ArrayColumnDesc<Double>("PHASE_DIR"));
  TableMeasRefDesc measRef(MDirection::J2000);
  TableMeasValueDesc measVal(td, "PHASE_DIR");
  TableMeasDesc<MDirection> measCol(measVal, measRef);
  measCol.write (td);
  // Now create a new table from the description.
  SetupNewTable newtab("tEmbraceStMan_tmp.data/FIELD", td, Table::New);
  Table tab(newtab, 1);
  ArrayMeasColumn<MDirection> fldcol (tab, "PHASE_DIR");
  Vector<MDirection> phaseDir(1);
  phaseDir[0] = MDirection(MVDirection(Quantity(-1.92653768, "rad"),
                                       Quantity(1.09220917, "rad")),
                           MDirection::J2000);
  fldcol.put (0, phaseDir);
  mainTable.rwKeywordSet().defineTable ("FIELD", tab);
}

void createAntenna (Table& mainTable, uInt nant)
{
  // Build the table description.
  TableDesc td;
  td.addColumn (ArrayColumnDesc<Double>("POSITION"));
  TableMeasRefDesc measRef(MPosition::ITRF);
  TableMeasValueDesc measVal(td, "POSITION");
  TableMeasDesc<MPosition> measCol(measVal, measRef);
  measCol.write (td);
  // Now create a new table from the description.
  SetupNewTable newtab("tEmbraceStMan_tmp.data/ANTENNA", td, Table::New);
  Table tab(newtab, nant);
  ScalarMeasColumn<MPosition> poscol (tab, "POSITION");
  // Use the positions of WSRT RT0-3.
  Vector<double> vals(3);
  vals[0] = 3828763; vals[1] = 442449; vals[2] = 5064923;
  poscol.put (0, MPosition(Quantum<Vector<double> >(vals,"m"),
                           MPosition::ITRF));
  vals[0] = 3828746; vals[1] = 442592; vals[2] = 5064924;
  poscol.put (1, MPosition(Quantum<Vector<double> >(vals,"m"),
                           MPosition::ITRF));
  vals[0] = 3828729; vals[1] = 442735; vals[2] = 5064925;
  poscol.put (2, MPosition(Quantum<Vector<double> >(vals,"m"),
                           MPosition::ITRF));
  vals[0] = 3828713; vals[1] = 442878; vals[2] = 5064926;
  poscol.put (3, MPosition(Quantum<Vector<double> >(vals,"m"),
                           MPosition::ITRF));
  // Write the remaining columns with the same position.
  // They are not used in the UVW check.
  for (uInt i=4; i<nant; ++i) {
    poscol.put (i, MPosition(Quantum<Vector<double> >(vals,"m"),
                             MPosition::ITRF));
  }
  mainTable.rwKeywordSet().defineTable ("ANTENNA", tab);
}

void createTable (uInt nant)
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
  SetupNewTable newtab("tEmbraceStMan_tmp.data", td, Table::New);
  // Create the storage manager and bind all columns to it.
  EmbraceStMan sm1;
  newtab.bindAll (sm1);
  // Finally create the table. The destructor writes it.
  Table tab(newtab);
  // Create the subtables needed to calculate UVW coordinates.
  createField (tab);
  createAntenna (tab, nant);
}

uInt nalign (uInt size, uInt alignment)
{
  return (size + alignment-1) / alignment * alignment - size;
}

void createData (uInt nseq, uInt nant, uInt nchan, uInt npol,
                 Double startTime, Double interval, const Complex& startValue,
                 Bool bigEndian, uInt myStManVersion)
{
  // Create the baseline vectors (with autocorrelations).
  uInt nrbl = nant*(nant+1)/2;
  Block<Int> ant1(nrbl);
  Block<Int> ant2(nrbl);
  uInt inx=0;

  for (uInt i=0; i<nant; ++i) {
    for (uInt j=i; j<nant; ++j) {
      // Use baselines 0,0, 0,1, 1,1 ... n,n
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
    AlwaysAssertExit(myStManVersion == 1);

    AipsIO aio("tEmbraceStMan_tmp.data/table.f0meta", ByteIO::New);
    aio.putstart ("EmbraceStMan", myStManVersion);
    aio << ant1 << ant2
        << startTime << interval << nchan << npol << bigEndian
        << Path("tEmbraceStMan_tmp.datafile").absoluteName();
    aio.putend();
  }
  // Now create the data file.
  RegularFileIO file(RegularFile("tEmbraceStMan_tmp.datafile"),
                     ByteIO::New);
  // Write in canonical (big endian) or local format.
  TypeIO* cfile;
  if (bigEndian) {
    cfile = new CanonicalIO(&file);
  } else {
    cfile = new RawIO(&file);
  }

  // Create and initialize data and nsample.
  Array<Float> data(IPosition(2,npol,nchan));
  indgen (data, startValue.real(), Float(0.01));

  // Write the data as nseq blocks.
  for (uInt i=0; i<nseq; ++i) {
    for (uInt j=0; j<nrbl; ++j) {
      cfile->write (data.size(), data.data());
      data += Float(0.01);
    }
  }
  delete cfile;
}

void checkUVW (uInt row, uInt nant, const Vector<Double>& uvw)
{
  // Expected outcome of UVW for antenna 0-3 and seqnr 0-1
  static double uvwVals[] = {
    0, 0, 0,
    0.423756, -127.372, 67.1947,
    0.847513, -254.744, 134.389,
    0.277918, -382.015, 201.531,
    -0.423756, 127.372, -67.1947,
    0, 0, 0,
    0.423756, -127.372, 67.1947,
    -0.145838, -254.642, 134.336,
    -0.847513, 254.744, -134.389,
    -0.423756, 127.372, -67.1947,
    0, 0, 0,
    -0.569594, -127.27, 67.1417,
    -0.277918, 382.015, -201.531,
    0.145838, 254.642, -134.336,
    0.569594, 127.27, -67.1417,
    0, 0, 0,
    0, 0, 0,
    0.738788, -127.371, 67.1942,
    1.47758, -254.742, 134.388,
    1.22276, -382.013, 201.53,
    -0.738788, 127.371, -67.1942,
    0, 0, 0,
    0.738788, -127.371, 67.1942,
    0.483976, -254.642, 134.336,
    -1.47758, 254.742, -134.388,
    -0.738788, 127.371, -67.1942,
    0, 0, 0,
    -0.254812, -127.271, 67.1421,
    -1.22276, 382.013, -201.53,
    -0.483976, 254.642, -134.336,
    0.254812, 127.271, -67.1421,
    0, 0, 0
  };
  uInt nrbl = nant*(nant+1)/2;
  uInt seqnr = row / nrbl;
  uInt bl = row % nrbl;
  uInt ant1 = int(2*nant+1.001 - sqrt((2*nant+1)*(2*nant+1)-8.*bl));
  uInt ant2 = bl - (2*nant+1-ant1)*ant1/2;
  // Only check first two time stamps and first four antennae.
  if (seqnr < 2  &&  ant1 < 4  &&  ant2 < 4) {
    AlwaysAssertExit (near(uvw[0],
                           uvwVals[3*(seqnr*16 + 4*ant1 + ant2)],
                           1e-5))
  }
}

void readTable (uInt nseq, uInt nant, uInt nchan, uInt npol,
                Double startTime, Double interval, const Complex& startValue,
                const String& tabName, Bool hasWeightSpectrum=True)
{
  uInt nbasel = nant*(nant+1)/2;
  // Open the table and check if #rows is as expected.
  Table tab(tabName);
  uInt nrow = tab.nrow();
  AlwaysAssertExit (nrow = nseq*nbasel);
  AlwaysAssertExit (!tab.canAddRow());
  AlwaysAssertExit (!tab.canRemoveRow());
  AlwaysAssertExit (tab.canRemoveColumn(Vector<String>(1, "DATA")));
  // Create objects for all mandatory MS columns.
  ROArrayColumn<Complex> dataCol(tab, "DATA");
  ROArrayColumn<Float> weightCol(tab, "WEIGHT");
  ROArrayColumn<Float> wspecCol;
  if (hasWeightSpectrum) {
    wspecCol.attach (tab, "WEIGHT_SPECTRUM");
  }
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
  indgen (dataExp, startValue, Complex(0.01, 0.0));

  // Loop through all rows in the table and check the data.
  uInt row=0;
  for (uInt i=0; i<nseq; ++i) {
    for (uInt j=0; j<nant; ++j) {
      for (uInt k=j; k<nant; ++k) {
        // Contents must be present except for FLAG_CATEGORY.
	AlwaysAssertExit (dataCol.isDefined (row));
	AlwaysAssertExit (weightCol.isDefined (row));
        if (hasWeightSpectrum) {
          AlwaysAssertExit (wspecCol.isDefined (row));
        }
        AlwaysAssertExit (sigmaCol.isDefined (row));
        AlwaysAssertExit (flagCol.isDefined (row));
        AlwaysAssertExit (!flagcatCol.isDefined (row));
        // Check data, weight, sigma, weight_spectrum, flag
        AlwaysAssertExit (allNear (dataCol(row), dataExp, 1e-7));
        AlwaysAssertExit (weightCol.shape(row) == IPosition(1,npol));
        AlwaysAssertExit (allEQ (weightCol(row), Float(1)));
        AlwaysAssertExit (sigmaCol.shape(row) == IPosition(1,npol));
        AlwaysAssertExit (allEQ (sigmaCol(row), Float(1)));
        if (hasWeightSpectrum) {
          Array<Float> weights = wspecCol(row);
          AlwaysAssertExit (weights.shape() == IPosition(2,npol,nchan));
          AlwaysAssertExit (allEQ (weights, Float(1)));
        }
        AlwaysAssertExit (allEQ (flagCol(row), False));
        // Check ANTENNA1 and ANTENNA2
        AlwaysAssertExit (ant1Col(row) == Int(j));
        AlwaysAssertExit (ant2Col(row) == Int(k));

        dataExp += Complex(0.01, 0.0);
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
  // Check the other columns.
  AlwaysAssertExit (allNear(centCol.getColumn(), times, 1e-13));
  AlwaysAssertExit (allNear(intvCol.getColumn(), interval, 1e-13));
  AlwaysAssertExit (allNear(expoCol.getColumn(), interval, 1e-13));
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
  // Check the UVW coordinates.
  for (uInt i=0; i<nrow; ++i) {
    checkUVW (i, nant, uvwCol(i));
  }
  RefRows rownrs(0,2,1);
  Slicer slicer(IPosition(2,0,0), IPosition(2,1,1));
  Array<Complex> dg = dataCol.getColumnCells (rownrs);
}

void updateTable (uInt nchan, uInt npol, const Complex& startValue)
{
  // Open the table for write.
  Table tab("tEmbraceStMan_tmp.data", Table::Update);
  uInt nrow = tab.nrow();
  // Create object for DATA column.
  ArrayColumn<Complex> dataCol(tab, "DATA");
  // Check we can write the column, but not change the shape.
  AlwaysAssertExit (tab.isColumnWritable ("DATA"));
  AlwaysAssertExit (!dataCol.canChangeShape());
  // Create and initialize data.
  Array<Complex> data(IPosition(2,npol,nchan));
  indgen (data, startValue, Complex(0.01, 0.0));
  // Loop through all rows in the table and write the data.
  for (uInt row=0; row<nrow; ++row) {
    dataCol.put (row, data);
    data += Complex(0.01, 0.0);
  }
}

void copyTable()
{
  Table tab("tEmbraceStMan_tmp.data");
  // Deep copy the table.
  tab.deepCopy ("tEmbraceStMan_tmp.datcp", Table::New, true);
}

void makeEmbraceTable (uInt nchan, uInt npol)
{
  ParameterSet ps;
  ps.add ("MSName", "tEmbraceStMan_tmp.ms_et");
  ps.add ("FileName", "tEmbraceStMan_tmp.datafile");
  ps.add ("BigEndian", "false");
  ps.add ("NTimes", "1");
  ps.add ("NBands", "1");
  ps.add ("NFrequencies", String::toString(nchan));
  ps.add ("NPolarizations", String::toString(npol));
  ps.add ("StartFreq", "1e9");
  ps.add ("StepFreq", "1e6");
  ps.add ("StepTime", "30");
  ps.add ("StartTime", "2000/08/03/13:22:05.000"); //same starttime as in main
  ps.add ("RightAscension", "-1.92653768rad");
  ps.add ("Declination", "1.09220917rad");
  ps.add ("WriteAutoCorr", "true");
  ps.add ("AntennaTableName", "tEmbraceStMan_tmp.data/ANTENNA");
  ps.writeFile ("tEmbraceStMan_tmp.cfg");
  system ("../src/makemsembrace tEmbraceStMan_tmp.cfg");
}


int main (int argc, char* argv[])
{
  try {
    // Register EmbraceStMan to be able to read it back.
    EmbraceStMan::registerClass();
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
      createTable (nant);
      // Write data in big-endian and check it.
      // Use this start time and interval to get known UVWs.
      // They are the same as used in DPPP/tUVWFlagger.
      double interval= 30.;
      double startTime = 4472025740.0 - interval*0.5;
      createData (nseq, nant, nchan, npol, startTime, interval,
                  Complex(0.1, 0.0), True, 1);
      readTable (nseq, nant, nchan, npol, startTime, interval,
                 Complex(0.1, 0.0), "tEmbraceStMan_tmp.data");
      // Update the table and check again.
      updateTable (nchan, npol, Complex(-3.52, 0.0));
      readTable (nseq, nant, nchan, npol, startTime, interval,
                 Complex(-3.52, 0.0), "tEmbraceStMan_tmp.data");
      // Write data in local format and check it. No alignment.
      createData (nseq, nant, nchan, npol, startTime, interval,
                  Complex(3.1, 0.0), False, 1);
      readTable (nseq, nant, nchan, npol, startTime, interval,
                 Complex(3.1, 0.0), "tEmbraceStMan_tmp.data");
      // Update the table and check again.
      updateTable (nchan, npol, Complex(3.52, 0.0));
      readTable (nseq, nant, nchan, npol, startTime, interval,
                 Complex(3.52, 0.0), "tEmbraceStMan_tmp.data");
      copyTable();
      // Now create the MS using embracems.
      // It does not have the column WEIGHT_SPECTRUM.
      makeEmbraceTable (nchan, npol);
      readTable (nseq, nant, nchan, npol, startTime, interval,
                 Complex(3.52, 0.0), "tEmbraceStMan_tmp.ms_et", False);
    }
  } catch (AipsError& x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}

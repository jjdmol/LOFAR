//# MSDesc.cc: Program write the description of an MS
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

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>
#include <Common/BlobOStream.h>
#include <Common/BlobOBufStream.h>
#include <Common/BlobArray.h>
#include <Common/LofarLogger.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

void getFreq (MS& ms, int ddid, int& nrchan,
	      double& startFreq, double& endFreq, double& stepFreq)
{
  // Get the frequency domain of the given data descriptor id
  // which gives the spwid.
  MSDataDescription mssub1(ms.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  int spw = mssub1c.spectralWindowId()(ddid);
  MSSpectralWindow mssub(ms.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  Vector<double> chanFreq = mssubc.chanFreq()(spw);
  Vector<double> chanWidth = mssubc.chanWidth()(spw);
  // So far, only equal frequency spacings are possible.
  if (! allEQ (chanWidth, chanWidth(0))) {
    throw AipsError("Channels must have equal spacings");
  }
  nrchan    = chanWidth.nelements();
  stepFreq  = chanWidth(0);
  startFreq = chanFreq(0) - stepFreq/2;
  endFreq   = startFreq + nrchan*stepFreq;
}

void getPhaseRef (MS& ms, double& ra, double& dec)
{
  MSField mssub(ms.field());
  ROMSFieldColumns mssubc(mssub);
  // Use the phase reference of the first field.
  MDirection phaseRef = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
  // Get RA and DEC in J2000.
  MDirection dir = MDirection::Convert (phaseRef, MDirection::J2000)();
  Quantum<Vector<double> > angles = dir.getAngle();
  ra  = angles.getBaseValue()(0);
  dec = angles.getBaseValue()(1);
}

void doIt (const string& in,
	   const string& dataFile, const string& flagFile,
	   const string& uvwFile, const string& column)
{
  // Open the table and make sure it is in the correct order.
  MS ms(in);
  {
    Block<String> cols(3);
    cols[0] = "TIME";
    cols[1] = "ANTENNA1";
    cols[2] = "ANTENNA2";
    Table tab = ms.sort(cols);
    Vector<uInt> nrs(tab.nrow());
    indgen (nrs);
    ASSERTSTR (allEQ(nrs, tab.rowNumbers(ms)), "MS not in correct order");
  }
  // Get all unique baselines.
  Vector<Int> a1;
  Vector<Int> a2;
  {
    Block<String> colsb(2);
    colsb[0] = "ANTENNA1";
    colsb[1] = "ANTENNA2";
    Table tabb = ms.sort(colsb, Sort::Ascending,
			 Sort::QuickSort+Sort::NoDuplicates);
    ROScalarColumn<Int> ant1(tabb,"ANTENNA1");
    ROScalarColumn<Int> ant2(tabb,"ANTENNA2");
    a1 = ant1.getColumn();
    a2 = ant2.getColumn();
  }
  // Get all unique times.
  ROScalarColumn<double> time(ms, "TIME");
  Vector<double> tim1 = time.getColumn();
  Vector<uInt> index;
  uInt nt = GenSortIndirect<double>::sort (index, tim1, Sort::Ascending,
					   Sort::InsSort+Sort::NoDuplicates);
  Vector<double> tim2(nt);
  Vector<double> interval2(nt);
  {
    ROScalarColumn<double> intvCol(ms, "INTERVAL");
    Vector<double> interval1 = intvCol.getColumn();
    for (uInt i=0; i<nt; i++) {
      tim2[i] = tim1[index[i]];
      interval2[i] = interval1[index[i]];
    }
  }
  // Check if they span the entire table.
  if (nt * a1.nelements() != ms.nrow()) {
    throw AipsError("#rows in MS " + in + " mismatches #times * #baselines");
  }

  // Get npol,nfreq from data.
  int npol,nfreq;
  {
    ROArrayColumn<Complex> cold(ms,column);
    IPosition shp = cold.shape (0);
    npol = shp[0];
    nfreq = shp[1];
  }
  // Now write out all descriptive data.
  {
    int nchan;
    double startFreq, endFreq, stepFreq, ra, dec;
    getFreq (ms, 0, nchan, startFreq, endFreq, stepFreq);
    getPhaseRef (ms, ra, dec);
    ASSERT (nchan == nfreq);
    string name(in+".des");
    std::ofstream ostr(name.c_str());
    BlobOBufStream bbs(ostr);
    BlobOStream bos(bbs);
    bos.putStart("ms.des", 1);
    bos << ra << dec << npol << nfreq << startFreq << endFreq << stepFreq;
    bos << a1 << a2;
    bos << tim2;
    bos << interval2;
    MSAntenna mssub(ms.antenna());
    ROMSAntennaColumns mssubc(mssub);
    bos << mssubc.position().getColumn();
    bos.putEnd();
    bos.putStart("ms.desf", 1);
    bos << dataFile << flagFile << uvwFile;
    bos.putEnd();
  }
  cout << "      " << npol << " polarizations" << endl;
  cout << "      " << nfreq << " frequency channels" << endl;
  cout << "      " << a1.nelements() << " baselines" << endl;
  cout << "      " << tim2.nelements() << " times" << endl;
  cout << " in file " << in << ".des" << endl;
  cout << " using data file " << in << dataFile << endl;
  cout << "       flag file " << in << flagFile << endl;
  cout << "        uvw file " << in << uvwFile << endl;
}

int main(int argc, char** argv)
{
  try {
    if (argc < 5) {
      cout << "Run as:  MSDesc in datafilename flag UVW [datacolumn]" << endl;
      cout << "   datacolumn defaults to DATA" << endl;
      return 0;
    }
    string column("DATA");
    if (argc > 5) {
      column = argv[5];
    }
    doIt (argv[1], argv[2], argv[3], argv[4], column);
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}

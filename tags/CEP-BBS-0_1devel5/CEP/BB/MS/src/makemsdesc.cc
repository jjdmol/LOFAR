//# makemsdesc.cc: Program write the description of an MS
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
#include <MS/MSDesc.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobArray.h>
#include <Common/LofarLogger.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/Stokes.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/OS/Path.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>

//# If needed, include for instantiation of tovector.
#ifdef AIPS_NO_TEMPLATE_SRC
#include <casa/Arrays/Vector2.cc>
#endif

#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

void getFreqInfo (MS& ms, vector<int>& nrchan,
		  vector<double>& startFreq, vector<double>& endFreq)
{
  MSDataDescription mssub1(ms.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  MSSpectralWindow mssub(ms.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  int nrspw = mssub1.nrow();
  nrchan.resize (nrspw);
  startFreq.resize (nrspw);
  endFreq.resize (nrspw);
  for (int spw=0; spw<nrspw; ++spw) {
    Vector<double> chanFreq = mssubc.chanFreq()(spw);
    Vector<double> chanWidth = mssubc.chanWidth()(spw);
    // So far, only equal frequency spacings are possible.
    ASSERTSTR (allEQ (chanWidth, chanWidth(0)),
	       "Channels must have equal spacings");
    // Fill in.
    int nrch    = chanWidth.nelements();
    nrchan[spw] = nrch;
    double step = abs(chanWidth(0));
    if (chanFreq(0) > chanFreq(nrch-1)) {
      startFreq[spw] = chanFreq(0) + step/2;
      endFreq[spw]   = startFreq[spw] - nrch*step;
    } else {
      startFreq[spw] = chanFreq(0) - step/2;
      endFreq[spw]   = startFreq[spw] + nrch*step;
    }
  }
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

void getObsInfo (MS& ms, vector<string>& antNames, Array<double>& antPos,
		 vector<double>& arrayPos,
		 double& startTime, double& endTime)
{
  startTime = 0;
  endTime = 0;
  MSAntenna mssub(ms.antenna());
  ROMSAntennaColumns mssubc(mssub);
  int nrant = mssub.nrow();
  MVPosition sum;
  antNames.resize (nrant);
  antPos.resize (IPosition(2,3,nrant));
  for (int ant=0; ant<nrant; ++ant) {
    antNames[ant] = mssubc.name()(ant);
    MPosition antp = mssubc.positionMeas()(ant);
    antp = MPosition::Convert (antp, MPosition::ITRF) ();
    const MVPosition& mvpos = antp.getValue();
    sum += mvpos;
    for (int i=0; i<3; ++i) {
      antPos(IPosition(2,i,ant)) = mvpos(i);
    }
  }
  // Use the array position in ITRF coordinates.
  // If not found, use the average of the antenna positions.
  MSObservation msobs(ms.observation());
  ROMSObservationColumns msobsc(msobs);
  MPosition pos;
  MVPosition aPos;
  if (msobs.nrow() > 0  &&
      MeasTable::Observatory(pos, msobsc.telescopeName()(0))) {
    pos = MPosition::Convert (pos, MPosition::ITRF) ();
    aPos = pos.getValue();
  } else {
    aPos = sum * (1./nrant);
  }
  arrayPos.resize (3);
  for (int i=0; i<3; ++i) {
    arrayPos[i] = aPos(i);
  }
  if (msobs.nrow() > 0) {
    Vector<double> times = msobsc.timeRange()(0);
    startTime = times(0);
    endTime   = times(1);
  }
}

void getCorrInfo (MS& ms, vector<string>& corrTypes)
{
  MSPolarization mssub(ms.polarization());
  ASSERT (mssub.nrow() > 0);
  ROMSPolarizationColumns mssubc(mssub);
  Vector<Int> ctvec = mssubc.corrType()(0);
  int nrp = ctvec.nelements();
  corrTypes.resize (nrp);
  for (int i=0; i<nrp; ++i) {
    corrTypes[i] = Stokes::name (Stokes::type(ctvec(i)));
  }
}


void doIt (const string& in, bool global)
{
  // Open the table and make sure it is in the correct order.
  MS ms(in);
  if (!global) {
    Block<String> cols(5);
    cols[0] = "ARRAY_ID";
    cols[1] = "DATA_DESC_ID";
    cols[2] = "TIME";
    cols[3] = "ANTENNA1";
    cols[4] = "ANTENNA2";
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
  // Get all unique array-ids and data-desc-ids.
  ROScalarColumn<int> aidcol(ms, "ARRAY_ID");
  Vector<int> aid = aidcol.getColumn();
  uInt naid = GenSort<int>::sort (aid, Sort::Ascending,
				  Sort::InsSort+Sort::NoDuplicates);
  aid.resize (naid, True);
  ROScalarColumn<int> ddidcol(ms, "DATA_DESC_ID");
  Vector<int> ddid = ddidcol.getColumn();
  uInt nddid = GenSort<int>::sort (ddid, Sort::Ascending,
				   Sort::InsSort+Sort::NoDuplicates);
  ddid.resize (nddid, True);
  // Get all unique times.
  ROScalarColumn<double> time(ms, "TIME");
  Vector<double> tim1 = time.getColumn();
  Vector<uInt> index;
  uInt nt = GenSortIndirect<double>::sort (index, tim1, Sort::Ascending,
					   Sort::InsSort+Sort::NoDuplicates);

  // Check if they span the entire table.
  if (naid * nddid * nt * a1.nelements() != ms.nrow()) {
    throw AipsError("#rows in MS " + in +
		    " mismatches #array-ids * #dd-ids * #times * #baselines");
  }

  // Create and fill the MSDesc object.
  MSDesc msd;
  // Fill in MS path and name.
  Path mspr(in);
  Path msp(mspr.absoluteName());
  msd.msPath = msp.dirName();
  msd.msName = msp.baseName();
  msd.npart  = 1;
  // Fill in antennae used in baselines.
  a1.tovector (msd.ant1);
  a2.tovector (msd.ant2);
  // Fill in time info.
  msd.times.resize (nt);
  msd.exposures.resize (nt);
  {
    ROScalarColumn<double> exposCol(ms, "EXPOSURE");
    Vector<double> interval1 = exposCol.getColumn();
    for (uInt i=0; i<nt; i++) {
      msd.times[i] = tim1[index[i]];
      msd.exposures[i] = interval1[index[i]];
    }
  }
  // Fill in phase center.
  getPhaseRef (ms, msd.ra, msd.dec);
  // Fill in correlation info.
  getCorrInfo (ms, msd.corrTypes);
  // Fill in freq info.
  getFreqInfo (ms, msd.nchan, msd.startFreq, msd.endFreq);
  ASSERTSTR (msd.nchan.size() == nddid,
	     "Mismatch in #bands in main table and SPECTRAL_WINDOW subtable");
  // Fill in observation and array info.
  getObsInfo (ms, msd.antNames, msd.antPos, msd.arrayPos,
	      msd.startTime, msd.endTime);
  if (msd.startTime >= msd.endTime) {
    // Invalid times; derive from times/interval.
    // Difference between interval and exposure is startup time which
    // is taken into account.
    if (nt > 0) {
      ROScalarColumn<double> intvCol(ms, "INTERVAL");
      msd.startTime = msd.times[0] - msd.exposures[0]/2 +
	              (intvCol(0) - msd.exposures[0]);
      msd.endTime   = msd.times[nt-1] + intvCol(nt-1)/2;
    }
  }
  // Now write out all descriptive data in a blob.
  if (!global) {
    // Once for each MS part (which is only 1).
    string name(in+"/vis.des");
    ofstream ostr(name.c_str());
    BlobOBufStream bbs(ostr);
    BlobOStream bos(bbs);
    bos << msd;
  } else {
    // Once for the MS as a whole.
    string name(in+".des");
    ofstream ostr(name.c_str());
    BlobOBufStream bbs(ostr);
    BlobOStream bos(bbs);
    bos << msd;
  }
  // Now write in ASCII form.
  if (global) {
    string name(in+".dess");
    ofstream ostr(name.c_str());
    msd.writeDesc (ostr);
  }
  // Finally show summary.
  cout << msd;
}

int main(int argc, char** argv)
{
  try {
    if (argc < 3) {
      cout << "Run as:  makemsdesc ms global" << endl;
      cout << "   global can be y or n" << endl;
      return 0;
    }
    string global(argv[2]);
    doIt (argv[1], global=="y");
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}

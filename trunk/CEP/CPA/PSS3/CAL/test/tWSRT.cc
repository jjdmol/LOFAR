//# tWSRT.cc: Test program for WSRT model classes
//# Copyright (C) 2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/MeasurementSets/MSAntenna.h>
#include <aips/MeasurementSets/MSAntennaColumns.h>
#include <aips/MeasurementSets/MSField.h>
#include <aips/MeasurementSets/MSFieldColumns.h>
#include <aips/MeasurementSets/MSDataDescription.h>
#include <aips/MeasurementSets/MSDataDescColumns.h>
#include <aips/MeasurementSets/MSSpectralWindow.h>
#include <aips/MeasurementSets/MSSpWindowColumns.h>
#include <aips/Measures/MDirection.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/Quanta/MVPosition.h>
#include <aips/Quanta/MVBaseline.h>
#include <aips/Tables/Table.h>
#include <aips/Tables/TableIter.h>
#include <aips/Tables/ColumnDesc.h>
#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/Slicer.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/Arrays/ArrayLogical.h>
#include <aips/Functionals/Polynomial.h>
#include <aips/Mathematics/AutoDiff.h>
#include <aips/Mathematics/Constants.h>
#include <aips/Exceptions/Error.h>
#include <aips/OS/Timer.h>
#include <trial/Fitting/LinearFit.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqJonesNode.h>
#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqPointDFT.h>
#include <MNS/MeqWsrtPoint.h>
#include <MNS/MeqUVWPolc.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqHist.h>
#include <MNS/ParmTable.h>
#include <GSM/SkyModel.h>
#include <Common/Debug.h>
#include <stdexcept>
#include <iomanip>
#include <stdio.h>

// Get the phase reference of the first field.
MDirection getPhaseRef (const MeasurementSet& ms)
{
  MSField mssub(ms.field());
  ROMSFieldColumns mssubc(mssub);
  MDirection dir = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
  return MDirection::Convert (dir, MDirection::J2000)();
}

// Get the frequency domain of the first spectral window.
// So far, only equal frequency spacings are possible.
void getFreq (const MeasurementSet& ms, double& start, double& end, int& nf)
{
  MSDataDescription mssub1(ms.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  int spw = mssub1c.spectralWindowId()(0);
  MSSpectralWindow mssub(ms.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  Vector<double> chanFreq = mssubc.chanFreq()(spw);
  Vector<double> chanWidth = mssubc.chanWidth()(spw);
  AssertMsg (allEQ (chanWidth, chanWidth(0)),
	     "Channels must have equal spacings");
  nf = chanWidth.nelements();
  start = chanFreq(0) - chanWidth(0)/2;
  end = start + nf*chanWidth(0);
}

// Get all stations.
// Fill their positions as parameters.
vector<MeqStation> fillStations (const MeasurementSet& ms)
{
  vector<MeqStation> stations;
  MSAntenna mssub(ms.antenna());
  ROMSAntennaColumns mssubc(mssub);
  char str[8];
  for (uInt i=0; i<mssub.nrow(); i++) {
    Vector<Double> antpos = mssubc.position()(i);
    sprintf (str, "%d", i);
    MeqParmSingle* px = new MeqParmSingle ("AntPosX_"+String(str), antpos(0));
    MeqParmSingle* py = new MeqParmSingle ("AntPosY_"+String(str), antpos(1));
    MeqParmSingle* pz = new MeqParmSingle ("AntPosZ_"+String(str), antpos(2));
    stations.push_back (MeqStation(px, py, pz, ""));
  }
  return stations;
}

// Get all baselines from the MS.
// Create an MVBaseline object for each of them.
vector<MVBaseline> fillBaselines (Matrix<int>& index,
				  const Vector<int>& ant1,
				  const Vector<int>& ant2,
				  const vector<MeqStation>& stations)
{
  index.resize (stations.size(), stations.size());
  index = -1;
  vector<MVBaseline> baselines;
  baselines.reserve (ant1.nelements());
  MeqDomain domain;
  MeqRequest req(domain, 1, 1);
  for (unsigned int i=0; i<ant1.nelements(); i++) {
    uInt a1 = ant1(i);
    uInt a2 = ant2(i);
    Assert (a1 < stations.size()  &&  a2 < stations.size());
    index(a1,a2) = baselines.size();
    MVPosition pos1
        (stations[a1].getPosX()->getResult(req).getValue().getDouble(),
	 stations[a1].getPosY()->getResult(req).getValue().getDouble(),
	 stations[a1].getPosZ()->getResult(req).getValue().getDouble());
    MVPosition pos2
        (stations[a2].getPosX()->getResult(req).getValue().getDouble(),
	 stations[a2].getPosY()->getResult(req).getValue().getDouble(),
	 stations[a2].getPosZ()->getResult(req).getValue().getDouble());

    baselines.push_back (MVBaseline (pos1, pos2));
  }
  return baselines;
}

// Make the expression tree for a single baseline.
MeqJonesExpr* makeExpr (const MDirection& phaseRef, MeqUVWPolc* uvw,
			ParmTable&, GSM::SkyModel& gsm,
			MeqHist& celltHist,
			MeqHist& cellfHist)
{
  // Represent the stations by constant parms.
  MeqExpr* stat1_11 = new MeqParmSingle ("Station.RT_0.11",
					 double(1));
  MeqExpr* stat1_12 = new MeqParmSingle ("Station.RT_0.12",
					 double(0));
  MeqExpr* stat1_21 = new MeqParmSingle ("Station.RT_0.21",
					 double(0));
  MeqExpr* stat1_22 = new MeqParmSingle ("Station.RT_0.22",
					 double(1));
  MeqJonesNode* stat1 = new MeqJonesNode (stat1_11, stat1_12,
					  stat1_21, stat1_22);
  MeqExpr* stat2_11 = new MeqParmSingle ("Station.RT_1.11",
					 double(1));
  MeqExpr* stat2_12 = new MeqParmSingle ("Station.RT_1.12",
					 double(0));
  MeqExpr* stat2_21 = new MeqParmSingle ("Station.RT_1.21",
					 double(0));
  MeqExpr* stat2_22 = new MeqParmSingle ("Station.RT_1.22",
					 double(1));
  MeqJonesNode* stat2 = new MeqJonesNode (stat2_11, stat2_12,
					  stat2_21, stat2_22);
  // Get the point sources from the GSM.
  vector<MeqPointSource> src;
  gsm.getPointSources (src);
  // Create the DFT kernel.
  MeqPointDFT* dft = new MeqPointDFT (src, phaseRef, uvw);
  MeqWsrtPoint* pnt = new MeqWsrtPoint (src, dft,
					&celltHist, &cellfHist);
  return new MeqWsrtInt (pnt, stat1, stat2);
}

int main(int argc, char* argv[])
{
  try {
    if (argc < 4) {
      cout << "Run as:  tWSRT MSname parmtable skymodel" << endl;
      return 0;
    }
    ParmTable ptable(argv[2]);
    Table gsmtab(argv[3]);
    GSM::SkyModel gsm(gsmtab);
    // Open the MS.
    MeasurementSet ms(argv[1], Table::Update);
    // Add the MODEL_DATA column if not existing yet.
    if (! ms.tableDesc().isColumn("MODEL_DATA")) {
      ArrayColumnDesc<Complex> mdcol("MODEL_DATA");
      ms.addColumn (mdcol);
      cout << "Added column MODEL_DATA to MS" << endl;
    }
    // Get phase reference (for field 0), frequencies (for spw 0)
    // and all station positions from it.
    MDirection phaseRef = getPhaseRef(ms);
    double startFreq, endFreq;
    int nrChan;
    getFreq (ms, startFreq, endFreq, nrChan);
    cout << "Freq: " << startFreq << ' ' << endFreq << " (" <<
      endFreq-startFreq << " Hz) " << nrChan << endl;
    vector<MeqStation> stations = fillStations(ms);
    // We only handle field 0 and spectral window 0 (for the time being).
    // Get all baselines by doing a unique sort.
    Table selMS = ms(ms.col("FIELD_ID")==0 && ms.col("DATA_DESC_ID")==0);
    Block<String> keys(2);
    keys[0] = "ANTENNA1";
    keys[1] = "ANTENNA2";
    selMS = selMS.sort (keys);
    Table sselMS = selMS.sort (keys, Sort::Ascending,
			       Sort::NoDuplicates+Sort::InsSort);
    cout << ms.nrow() << ' ' << selMS.nrow() << ' ' << sselMS.nrow() << endl;
    // Now generate the baseline objects for them.
    // If we ever want to solve for station positions, we cannot use the
    // fixed MVBaseline objects, but instead they should be recalculated
    // after each solve iteration.
    ROScalarColumn<int> ant1(sselMS, "ANTENNA1");
    ROScalarColumn<int> ant2(sselMS, "ANTENNA2");
    Matrix<int> index;
    vector<MVBaseline> baselines = fillBaselines (index,
						  ant1.getColumn(),
						  ant2.getColumn(),
						  stations);
    // Iterate through the MS per baseline.
    TableIterator iter (selMS, keys, TableIterator::Ascending,
			TableIterator::NoSort);
    // Set up the expression tree for a single baseline.
    MeqUVWPolc uvwpolc;
    MeqHist celltHist;
    MeqHist cellfHist;
    MeqJonesExpr* expr = makeExpr (phaseRef, &uvwpolc, ptable, gsm,
				   celltHist, cellfHist);
    vector<double> diffu, diffv, diffw;
    int cnt = 0;
    cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
	 << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
    cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
	 << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;
    while (!iter.pastEnd()) {
      Table tab = iter.table();
      ///      MeqPointDFT::doshow = tab.rowNumbers()(0) == 44;
      ArrayColumn<Complex> mdcol(tab, "MODEL_DATA");
      ROArrayColumn<Complex> dcol(tab, "DATA");
      ROArrayColumn<double> uvwcol(tab, "UVW");
      ROScalarColumn<double> timcol(tab, "TIME");
      ROScalarColumn<double> intcol(tab, "INTERVAL");
      int npol = dcol(0).shape()(0);
      Vector<double> dt = timcol.getColumn();
      Matrix<double> uvws = uvwcol.getColumn();
      uvwpolc.calcCoeff (dt, uvws, false);
      double step = intcol(0);
      int nrTime = dt.nelements();
      double startTime = dt(0)-step/2;
      double endTime =  dt(nrTime-1)+step/2;
      Assert (near(endTime-startTime, nrTime*step));
      MeqDomain domain (startTime, endTime, startFreq, endFreq);
      MeqRequest request(domain, nrTime, nrChan);
      cout << setw(5) << cnt << ":  ";
      Timer timer;
      expr->calcResult (request);
      timer.show();
      ///      cout << expr->getResult11().getValue() << endl;
      ///      cout << expr->getResult12().getValue() << endl;
      ///      cout << expr->getResult21().getValue() << endl;
      ///      cout << expr->getResult22().getValue() << endl;
      // Write the predicted data into the MODEL_DATA column.
      // They have to be converted from double to float complex.
      Matrix<complex<float> > data(npol,nrChan);
      Matrix<DComplex> xx=expr->getResult11().getValue().getDComplexMatrix();
      Matrix<DComplex> xy=expr->getResult12().getValue().getDComplexMatrix();
      Matrix<DComplex> yx=expr->getResult21().getValue().getDComplexMatrix();
      Matrix<DComplex> yy=expr->getResult22().getValue().getDComplexMatrix();
      for (int j=0; j<nrTime; j++) {
	for (int i=0; i<nrChan; i++) {
	  const DComplex& xxji = xx(j,i);
	  data(0,i) = complex<float> (xxji.real(), xxji.imag());
	  ///	  if (abs(data(0,i)) < 0.4) {
	    ///	    cout << cnt << ' ' << j << ' ' << i << ' ' << abs(data(0,i)) << endl;
	    ///	  }
	  if (npol > 2) {
	    const DComplex& xyji = xy(j,i);
	    data(1,i) = complex<float> (xyji.real(), xyji.imag());
	    const DComplex& yxji = yx(j,i);
	    data(2,i) = complex<float> (yxji.real(), yxji.imag());
	  }
	  const DComplex& yyji = yy(j,i);
	  if (npol != 1) {
	    data(npol-1,i) = complex<float> (yyji.real(), yyji.imag());
	  }
	}
	///	cout <<"result: " << tab.rowNumbers()[j] << ' ' << data << endl;
	mdcol.put (j, data);
      }
      iter++;
      cnt++;
      //      cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
      //	   << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
      //      cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
      //	   << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;
    }
  } catch (AipsError& x) {
    cout << "Unexpected AIPS++ exception: " << x.getMesg() << endl;
    return 1;
  } catch (std::exception& x) {
    cout << "Unexpected AIPS++ exception: " << x.what() << endl;
    return 1;
  }
  cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
       << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
  cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
       << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;
  cout << "OK" << endl;
  return 0;
}

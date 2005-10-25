//# tPoly.cc: Test program for uvw fitting
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


#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/MVBaseline.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ExprNode.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <scimath/Fitting/LinearFit.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqUVWPolc.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <stdexcept>
#include <stdio.h>

using namespace casa;


MDirection getPhaseRef(const MeasurementSet& ms)
{
  MSField mssub(ms.field());
  ROMSFieldColumns mssubc(mssub);
  MDirection dir = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
  return MDirection::Convert (dir, MDirection::J2000)();
}

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
    stations.push_back (MeqStation(px, py, pz, mssubc.name()(i)));
  }
  return stations;
}

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

int main(int argc, char* argv[])
{
  try {
    if (argc < 2) {
      cout << "Run as:  tuvw MSname" << endl;
      return 0;
    }
    // Open the MS.
    // Get phase reference and all station positions from it.
    MeasurementSet ms(argv[1]);
    MDirection phaseRef = getPhaseRef(ms);
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
    cout << index << endl;
    // Iterate through the MS per baseline.
    TableIterator iter (selMS, keys, TableIterator::Ascending,
			TableIterator::NoSort);
    MeqUVWPolc uvwpolc;
    vector<double> diffu, diffv, diffw;
    while (!iter.pastEnd()) {
      Table tab = iter.table();
      ROArrayColumn<double> uvwcol(tab, "UVW");
      ROScalarColumn<double> timcol(tab, "TIME");
      Vector<double> dt = timcol.getColumn() - timcol(0);
      Matrix<double> uvws = uvwcol.getColumn();
      uvwpolc.calcCoeff (dt, uvws, false);
      double step = dt(1) - dt(0);
      MeqDomain domain (dt(0)-step/2, dt(dt.nelements()-1)+step/2, -1., 1.);
      MeqRequest request(domain, dt.nelements(), 1);
      uvwpolc.calcUVW (request);
      Matrix<double> u,v,w;
      u = uvws(IPosition(2,0,0), IPosition(2,0,dt.nelements()-1));
      Matrix<double> u1 = u.reform(IPosition(2,dt.nelements(),1));
      v = uvws(IPosition(2,1,0), IPosition(2,1,dt.nelements()-1));
      Matrix<double> v1 = v.reform(IPosition(2,dt.nelements(),1));
      w = uvws(IPosition(2,2,0), IPosition(2,2,dt.nelements()-1));
      Matrix<double> w1 = w.reform(IPosition(2,dt.nelements(),1));
      diffu.push_back
	(max(abs((u1 - uvwpolc.getU().getValue().getDoubleMatrix()) / u1)));
      diffv.push_back
	(max(abs((v1 - uvwpolc.getV().getValue().getDoubleMatrix()) / v1)));
      diffw.push_back
	(max(abs((w1 - uvwpolc.getW().getValue().getDoubleMatrix()) / w1)));
      iter++;
    }
    Vector<double> mu(IPosition(1,diffu.size()), &(diffu[0]), SHARE);
    cout << "Max diff U = " << max(mu);
    cout << "   Mean diff U = " << mean(mu) << endl;
    Vector<double> mv(IPosition(1,diffv.size()), &(diffv[0]), SHARE);
    cout << "Max diff V = " << max(mv);
    cout << "   Mean diff V = " << mean(mv) << endl;
    Vector<double> mw(IPosition(1,diffw.size()), &(diffw[0]), SHARE);
    cout << "Max diff W = " << max(mw);
    cout << "   Mean diff W = " << mean(mw) << endl;
  } catch (AipsError& x) {
    cout << "Unexpected AIPS++ exception: " << x.getMesg() << endl;
    return 1;
  } catch (std::exception& x) {
    cout << "Unexpected AIPS++ exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}

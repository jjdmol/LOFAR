//# filluvw.cc: Fill uvw coordinates in an MS
//#
//# Copyright (C) 2003
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


#include <MNS/MeqStation.h>
#include <MNS/MeqStatUVW.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqPhaseRef.h>
#include <MNS/MeqParmSingle.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/OS/Timer.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ExprNode.h>

#include <stdexcept>
#include <stdio.h>


  MeasurementSet itsMS;
  MeqPhaseRef           itsPhaseRef;    //# Phase reference position in J2000
  vector<MeqStation*>   itsStations;
  vector<MeqStatUVW*>   itsStatUVW;

//----------------------------------------------------------------------
//
// ~getPhaseRef
//
// Get the phase reference of the first field.
//
//----------------------------------------------------------------------
void getPhaseRef()
{
  MSField mssub(itsMS.field());
  ROMSFieldColumns mssubc(mssub);
  // Use the phase reference of the first field.
  MDirection phaseRef = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
  // Use the time in the first MS row.
  MSColumns itsMSCol(itsMS);
  double startTime = itsMSCol.time()(0);
  itsPhaseRef = MeqPhaseRef (phaseRef, startTime);
}

//----------------------------------------------------------------------
//
// ~fillStations
//
// Fill the station positions and names.
//
//----------------------------------------------------------------------
void fillStations (const Vector<Int>& ant1,
		   const Vector<Int>& ant2)
{
  MSAntenna          mssub(itsMS.antenna());
  ROMSAntennaColumns mssubc(mssub);
  int nrant = mssub.nrow();
  itsStations = vector<MeqStation*>(nrant, (MeqStation*)0);
  itsStatUVW  = vector<MeqStatUVW*> (nrant, (MeqStatUVW*)0);
  // Get all stations actually used.
  for (uInt i=0; i<ant1.nelements(); i++) {
    for (int j=0; j<2; j++) {
      int ant = ant1(i);
      if (j==1) ant = ant2(i);
      Assert (ant < nrant);
      if (itsStations[ant] == 0) {
	// Store each position as a constant parameter.
	// Use the antenna name as the parameter name.
	Vector<Double> antpos = mssubc.position()(ant);
	String name = mssubc.name()(ant);
	MeqParmSingle* px = new MeqParmSingle ("AntPosX." + name,
					       antpos(0));
	MeqParmSingle* py = new MeqParmSingle ("AntPosY." + name,
					       antpos(1));
	MeqParmSingle* pz = new MeqParmSingle ("AntPosZ." + name,
					       antpos(2));
	itsStations[ant] = new MeqStation(px, py, pz, name);
	// Expression to calculate UVW per station
	itsStatUVW[i] = new MeqStatUVW (itsStations[i], &itsPhaseRef);
      }
    }
  }
}


//----------------------------------------------------------------------
//
// ~main
//
// Constructor. Initialize a MeqCalibrater object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
int main (int argc, const char* argv[])
{
  if (argc < 2) {
    cerr << "Run as:  filluvw msname" << endl;
    return 0;
  }
  itsMS = MeasurementSet(argv[1], Table::Update);
  MSColumns itsMSCol(itsMS);

  // Get phase reference (for field 0).
  getPhaseRef();

  // We only handle field 0.
  // Sort the MS in order of baseline.
  TableExprNode expr = (itsMS.col("FIELD_ID")==0);
  Table itsSelMS = itsMS(expr);
  Block<String> keys(2);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  // Sort uniquely to get all baselines.
  // It looks as if QuickSort|NoDuplicates is incorrect
  // (it looks as if it skips some entries).
  Table sortMS = itsSelMS.sort (keys, Sort::Ascending);
  Table blMS = sortMS.sort (keys, Sort::Ascending,
			    Sort::InsSort | Sort::NoDuplicates);

  // Generate the baseline objects for the baselines.
  // If we ever want to solve for station positions, we cannot use the
  // fixed MVBaseline objects, but instead they should be recalculated
  // after each solve iteration.
  // It also forms an index giving for each (ordered) antenna pair the
  // index in the vector of baselines. -1 means that no baseline exists
  // for that antenna pair.
  ROScalarColumn<int> ant1col(blMS, "ANTENNA1");
  ROScalarColumn<int> ant2col(blMS, "ANTENNA2");
  Vector<int> ant1data = ant1col.getColumn();
  Vector<int> ant2data = ant2col.getColumn();
  // First find all used stations (from the ANTENNA subtable of the MS).
  fillStations (ant1data, ant2data);

  MeqRequest req(MeqDomain(0,1,0,1), 1, 1, 0);
  double lastTime = -1;
  Vector<double> uvw(3);
  for (int rownr=0; rownr<int(itsMS.nrow()); rownr++) {
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    double time = itsMSCol.time()(rownr);
    if (time != lastTime) {
      double range = itsMSCol.interval()(rownr);
      MeqDomain dom(time-range/2, time+range/2, 0, 1);
      req = MeqRequest (dom, 1, 1, 0);
      lastTime = time;
    }
    MeqResult ul = itsStatUVW[ant1]->getU(req);
    MeqResult vl = itsStatUVW[ant1]->getV(req);
    MeqResult wl = itsStatUVW[ant1]->getW(req);
    MeqResult ur = itsStatUVW[ant2]->getU(req);
    MeqResult vr = itsStatUVW[ant2]->getV(req);
    MeqResult wr = itsStatUVW[ant2]->getW(req);
    uvw(0) = ur.getValue().getDouble() - ul.getValue().getDouble();
    uvw(1) = vr.getValue().getDouble() - vl.getValue().getDouble();
    uvw(2) = wr.getValue().getDouble() - wl.getValue().getDouble();
    cout << uvw(0) << ' ' << uvw(1) << ' ' << uvw(2) << endl;
    itsMSCol.uvw().put (rownr, uvw);
  }
}

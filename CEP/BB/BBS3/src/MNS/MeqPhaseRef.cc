//# MeqPhaseRef.cc: Phase reference position and derived values
//#
//# Copyright (C) 2002
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

#include <BBS3/MNS/MeqPhaseRef.h>
#include <Common/Debug.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Constants.h>


namespace LOFAR {

MeqPhaseRef::MeqPhaseRef (const MDirection& phaseRef, double startTime)
: itsStartTime (startTime)
{
  // Get RA and DEC in J2000.
  itsDir = MDirection::Convert (phaseRef, MDirection::J2000)();
  Quantum<Vector<double> > angles = itsDir.getAngle();
  itsRa  = angles.getBaseValue()(0);
  itsDec = angles.getBaseValue()(1);
  itsSinDec = sin(itsDec);
  itsCosDec = cos(itsDec);

  // Get hourangle for Greenwich meridian.
  // Put the position in a frame.
  MVPosition mgwPos;
  MPosition gwPos (mgwPos, MPosition::WGS84);
  // Use the Dwingeloo position instead of Greenwich.
  Assert (MeasTable::Observatory(gwPos, "DWL"));
  MeasFrame frame(gwPos);
  // Convert start time to an epoch and put in frame.
  Quantum<Double> qtime(0, "s");
  qtime.setValue(startTime);
  frame.set (MEpoch(qtime, MEpoch::UTC));
  // Convert phaseRef to HADEC using the frame.
  MDirection hadec = MDirection::Convert
	       (phaseRef, MDirection::Ref (MDirection::HADEC, frame)) ();
  // Get HA out of it.
  angles = hadec.getAngle();
  itsStartHA = angles.getBaseValue()(0);

  // Do the same for 1 hour later to get the HA rate.
  qtime.setValue(startTime+3600);
  frame.set (MEpoch(qtime, MEpoch::UTC));
  // Convert phaseRef to HADEC using the frame.
  hadec = MDirection::Convert
	       (phaseRef, MDirection::Ref (MDirection::HADEC, frame)) ();
  // Get HA.
  angles = hadec.getAngle();
  double ha = angles.getBaseValue()(0);
  if (ha < itsStartHA) {
    ha = ha + C::_2pi;
  }
  // Calculate HA increment per UT second.
  itsScaleHA = (ha - itsStartHA) / 3600;
/*
  cout << "meqphasref: " << itsStartHA << ' ' << ha << ' ' << itsScaleHA
       << ' ' << itsStartTime-4.4977e9 << ' ' << gwPos << endl;
*/
  // Set earth position.
  itsEarthPos = gwPos;

  MVEpoch ep1(startTime);
  MEpoch ep2(ep1, MEpoch::UTC);
  MEpoch ep3 = MEpoch::Convert (ep2, MEpoch::GAST)();

//  cout << "ep3 " << ep3 << endl;
}

}

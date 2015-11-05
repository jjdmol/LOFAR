//# tMSLofarTable.cc: Test program for the LOFAR MS tables
//# Copyright (C) 2011
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>

#include <MSLofar/MSAntennaField.h>
#include <MSLofar/MSAntennaFieldColumns.h>
#include <MSLofar/MSStation.h>
#include <MSLofar/MSStationColumns.h>
#include <MSLofar/MSElementFailure.h>
#include <MSLofar/MSElementFailureColumns.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/SetupNewTab.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayLogical.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

void checkAntennaField()
{
  MSAntennaField af("tMSLofarTable_tmp.af", Table::Old);
  ROMSAntennaFieldColumns afcol(af);
  ASSERT (afcol.nrow() == 2);
  ASSERT (afcol.antennaId()(0) == 1);
  ASSERT (afcol.name()(0) == "HBA0");
  ASSERT (allEQ(afcol.position()(0), Vector<Double>(3, 2.5)));
  ASSERT (allEQ(afcol.coordinateAxes()(0), Matrix<Double>(3,3, 3.5)));
  ASSERT (allEQ(afcol.elementOffset()(0), Matrix<Double>(3,24, 5.)));
  ASSERT (allEQ(afcol.elementFlag()(0), Matrix<Bool>(2,24, False)));
  ASSERT (afcol.tileRotation()(0) == -2.);
  ASSERT (! afcol.tileElementOffset().isDefined(0));
  ASSERT (allEQ(afcol.tileElementOffset()(1), Matrix<Double>(3,16, -4.)));
  {
    Vector<Quantum<Double> > pos = afcol.positionQuant()(0);
    ASSERT (pos.size() == 3);
    ASSERT (pos[0].getUnit()=="m" && pos[1].getUnit()=="m" &&
            pos[2].getUnit()=="m");
    ASSERT (pos[0].getValue()==2.5 && pos[1].getValue()==2.5 &&
            pos[2].getValue()==2.5);
  }
  {
    Vector<Quantum<Double> > pos = afcol.positionQuant()(0, "cm");
    ASSERT (pos.size() == 3);
    ASSERT (pos[0].getUnit()=="cm" && pos[1].getUnit()=="cm" &&
            pos[2].getUnit()=="cm");
    ASSERT (pos[0].getValue()==250 && pos[1].getValue()==250 &&
            pos[2].getValue()==250);
  }
  MPosition pos = afcol.positionMeas()(0);
}

void testAntennaField()
{
  {
    SetupNewTable newtab("tMSLofarTable_tmp.af",
                         MSAntennaField::requiredTableDesc(), Table::New);
    MSAntennaField af(newtab, 0, False);
    af.addRow();
    MSAntennaFieldColumns afcol(af);
    ASSERT (afcol.nrow() == 1);
    afcol.antennaId().put (0, 1);
    afcol.name().put (0, "HBA0");
    afcol.position().put (0, Vector<Double>(3, 2.5));
    afcol.coordinateAxes().put (0, Matrix<Double>(3,3, 3.5));
    afcol.elementOffset().put (0, Matrix<Double>(3,24, 5.));
    afcol.elementFlag().put (0, Matrix<Bool>(2,24, False));
    afcol.tileRotation().put (0, -2.);
    af.addRow();
    afcol.tileElementOffset().put (1, Matrix<Double>(3,16, -4.));
  }
  checkAntennaField();
}

int main()
{
  try {
    testAntennaField();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

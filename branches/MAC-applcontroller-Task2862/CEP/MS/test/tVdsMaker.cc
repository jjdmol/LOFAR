//# tVdsMaker.cc: Test for class VdsMaker
//#
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
//#  $Id$

//# Includes
#include <MS/VdsMaker.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <casa/Quanta/MVTime.h>
#include <iostream>

using namespace LOFAR;
using namespace casa;
using namespace std;

int main()
{
  try {
    {
      // Create an empty MS.
      string msname("tVdsMaker_tmp.ms1");
      TableDesc simpleDesc = MS::requiredTableDesc();
      SetupNewTable newTab(msname, simpleDesc, Table::New);
      MeasurementSet ms(newTab);
      ms.createDefaultSubtables(Table::New);
      ms.flush (True);
      MSField msfield(ms.field());
      MSFieldColumns fldCols(msfield);
      fldCols.setDirectionRef (MDirection::SUN);
      msfield.addRow();
      Matrix<double> dirs(2,2, 0.);
      fldCols.referenceDir().put (0, dirs);
      VdsMaker::create (msname, msname+".vds", string());
    }
    {
      // Create a filled MS.
      string msname("tVdsMaker_tmp.ms2");
      TableDesc simpleDesc = MS::requiredTableDesc();
      SetupNewTable newTab(msname, simpleDesc, Table::New);
      MeasurementSet ms(newTab);
      ms.createDefaultSubtables(Table::New);
      ms.addRow (1);
      MSColumns mscol(ms);
      mscol.antenna1().put (0, 0);
      mscol.antenna2().put (0, 0);
      MVTime time(2011, 11, 7, 0);
      mscol.time().put (0, time.second());
      mscol.interval().put (0, 1.);
      mscol.exposure().put (0, 1.);
      ms.flush (True);
      VdsMaker::create (msname, msname+".vds", string());
    }
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}

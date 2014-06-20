//# tParm.cc: Program to test a missing polc
//#
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
#include <ParmDB/Parm.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;

// Test evaluating a polc for a single and multiple ParmValues.
void testResultCoeff(double stime)
{
  // Open the ParmDB created in tParmPolc.run.
  ParmDBMeta dbm1("casa", "tParmPolc_tmp.pdb");
  ParmDB db1(dbm1);
  ParmSet pset;
  ParmId parmid = pset.addParm (db1, "Clock:DE601HBA");
  // Get all freqs and a bit more than the times in the parmdb.
  ParmCache pcache(pset, Box(Point(1,stime),Point(5,30)));
  Parm parm(pcache, parmid);
  // Evaluate the parm for a grid of that size.
  {
    Axis::ShPtr freqAxis(new RegularAxis(0,1,5));
    Axis::ShPtr timeAxis(new RegularAxis(stime,1,30));
    Grid grid(freqAxis, timeAxis);
    Array<double> result;
    parm.getResult (result, grid);
    cout << result;
  }
}

int main()
{
  try {
    INIT_LOGGER("tParm");
    testResultCoeff(0);    // finds values in ParmDB
    testResultCoeff(22);   // no values in ParmDB
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

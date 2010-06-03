//# tParmDBLog.cc: Program to test the ParmDBLog class
//#
//# Copyright (C) 2010
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
#include <ParmDB/ParmDBLog.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;

void fill (bool create)
{
  // Create a parm data base.
  ParmDBLog db1("tParmDBLog_tmp.tab", create, create);
  vector<double> sol(3);
  sol[0] = 1.;
  sol[1] = 2.;
  sol[2] = 3.;
  Array<double> corrMat(IPosition(2, 11, 11));
  corrMat = 1.;
  db1.add (1e5, 2e5, 0., 10., 1, 5, 25, 3, 1.03, 0.5, sol, "msg1");
  db1.add (1e5, 2e5, 0., 10., 1, 5, 25, 3, 1.03, 0.5, sol, "msg2", corrMat);
}

void check (uint nrow)
{
  Table db1("tParmDBLog_tmp.tab");
  ASSERT (db1.nrow() == nrow);
}

int main()
{
  try {
    INIT_LOGGER("tParmDBLog");
    // Create.
    fill (true);
    check (2);
    // Append.
    fill (false);
    check (4);
    // Create.
    fill (true);
    check (2);
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

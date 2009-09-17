//# tParmSet.cc: Program to test the basics of the ParmSet class
//#
//# Copyright (C) 2008
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
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;

void testSet()
{
  // Create two parm data bases.
  ParmDBMeta dbm1("casa", "tParmSet_tmp.pdb1");
  ParmDBMeta dbm2("casa", "tParmSet_tmp.pdb2");
  ParmDB db1(dbm1, true);
  ParmDB db2(dbm2, true);
  // Check the constructor.
  ParmSet pset;
  ASSERT (pset.size() == 0);
  ASSERT (pset.getDBs().size() == 0);
  // Test the addition of parms. Check the resulting parmid.
  ASSERT (pset.addParm (db1, "parm1") == 0);
  ASSERT (pset.size() == 1);
  ASSERT (pset.getDBs().size() == 1);
  ASSERT (pset.getDBs()[0] == &db1);
  ASSERT (pset.addParm (db1, "parm2") == 1);
  ASSERT (pset.addParm (db1, "parm3") == 2);
  ASSERT (pset.size() == 3);
  ASSERT (pset.getDBs().size() == 1);
  ASSERT (pset.getDBs()[0] == &db1);
  ASSERT (pset.addParm (db2, "parm4") == 3);
  ASSERT (pset.size() == 4);
  ASSERT (pset.getDBs().size() == 2);
  ASSERT (pset.getDBs()[0] == &db1);
  ASSERT (pset.getDBs()[1] == &db2);
}

int main()
{
  try {
    INIT_LOGGER("tParmSet");
    testSet();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

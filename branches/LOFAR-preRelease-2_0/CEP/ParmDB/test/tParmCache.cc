//# tParmCache.cc: Program to test class ParmCache
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
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/Table.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;


void fillDef (ParmDB& pdb1, ParmDB& pdb2)
{
  // Write default values.
  // Initialize an [2,3] array.
  Array<double> coeff(IPosition(2,2,3));
  coeff = 0.;
  coeff(IPosition(2,0,0)) = 1.;
  // Put with a scalar value 1.
  pdb1.putDefValue ("gain",   ParmValueSet(ParmValue(1.)));
  pdb1.putDefValue ("gain:x", ParmValueSet(ParmValue(1.5)));
  pdb1.putDefValue ("phase",  ParmValueSet(ParmValue(0.1)));
  ParmValue defaultValue;
  defaultValue.setCoeff (coeff);
  pdb2.putDefValue ("ra", ParmValueSet(defaultValue, ParmValue::Polc));
}

void testCreate()
{
  ParmDB pdb1(ParmDBMeta("casa", "tParmCache_tmp.tab1"), true);
  ParmDB pdb2(ParmDBMeta("casa", "tParmCache_tmp.tab2"), true);
  fillDef (pdb1, pdb2);
  Table tab1("tParmCache_tmp.tab1");
  Table tab2("tParmCache_tmp.tab2");
  ASSERT (tab1.nrow() == 0);
  ASSERT (tab2.nrow() == 0);
  ParmSet parmset;
  ParmId gainx1Id = parmset.addParm (pdb1, "gain:x:S1");
  ParmId gainy1Id = parmset.addParm (pdb1, "gain:y:S1");
  ParmId phasx1Id = parmset.addParm (pdb1, "phase:x:S1");
  ParmId phasy1Id = parmset.addParm (pdb1, "phase:y:S1");
  ParmId rasrcId  = parmset.addParm (pdb2, "ra:src");
  ParmId decsrcId = parmset.addParm (pdb2, "dec:src");
  Box workDomain(make_pair(3,4), make_pair(10,12));
  // Create the cache and fill for the work domain.
  ParmCache parmCache(parmset, workDomain);
  // Check all values.
  // Note that no default exists for dec:src, so it gets default 0.
  ParmValueSet& pset1 = parmCache.getValueSet(gainx1Id);
  ASSERT (pset1.size() == 0);
  ASSERT (pset1.getFirstParmValue().nx() == 1);
  ASSERT (pset1.getFirstParmValue().ny() == 1);
  ASSERT (pset1.getFirstParmValue().getValues().data()[0] == 1.5);
  ParmValueSet& pset2 = parmCache.getValueSet(gainy1Id);
  ASSERT (pset2.size() == 0);
  ASSERT (pset2.getFirstParmValue().nx() == 1);
  ASSERT (pset2.getFirstParmValue().ny() == 1);
  ASSERT (pset2.getFirstParmValue().getValues().data()[0] == 1.);
  ParmValueSet& pset3 = parmCache.getValueSet(phasx1Id);
  ASSERT (pset3.size() == 0);
  ASSERT (pset3.getFirstParmValue().nx() == 1);
  ASSERT (pset3.getFirstParmValue().ny() == 1);
  ASSERT (pset3.getFirstParmValue().getValues().data()[0] == 0.1);
  ParmValueSet& pset4 = parmCache.getValueSet(phasy1Id);
  ASSERT (pset4.size() == 0);
  ASSERT (pset4.getFirstParmValue().nx() == 1);
  ASSERT (pset4.getFirstParmValue().ny() == 1);
  ASSERT (pset4.getFirstParmValue().getValues().data()[0] == 0.1);
  ParmValueSet& pset5 = parmCache.getValueSet(rasrcId);
  ASSERT (pset5.size() == 0);
  ASSERT (pset5.getFirstParmValue().nx() == 2);
  ASSERT (pset5.getFirstParmValue().ny() == 3);
  ASSERT (pset5.getFirstParmValue().getValues().data()[0] == 1.);
  ASSERT (pset5.getFirstParmValue().getValues().data()[1] == 0.);
  ParmValueSet& pset6 = parmCache.getValueSet(decsrcId);
  ASSERT (pset6.size() == 0);
  ASSERT (pset6.getFirstParmValue().nx() == 1);
  ASSERT (pset6.getFirstParmValue().ny() == 1);
  ASSERT (pset6.getFirstParmValue().getValues().data()[0] == 0.);
  {
    // Set a solve grid for some parameters (from (4,6) till (9,10))
    // Also set to dirty, so they'll be written.
    Axis::ShPtr ax02 (new RegularAxis(4,1,3));
    Axis::ShPtr ax12 (new RegularAxis(6,2,2));
    Grid grid2(ax02, ax12);
    pset1.setSolveGrid (grid2);
    pset1.setDirty();
    ASSERT (pset1.size() == 1);
    pset2.setSolveGrid (grid2);
    pset2.setDirty();
    ASSERT (pset2.size() == 1);
    pset5.setSolveGrid (grid2);
    pset5.setDirty();
    ASSERT (pset5.size() == 6);
    ASSERT (pset5.getParmValue(0).getValues().size() == 6);
    ASSERT (pset5.getParmValue(0).getGrid().size() == 1);
    pset6.setSolveGrid (grid2);
    pset6.setDirty();
    ASSERT (pset6.size() == 1);
    parmCache.flush();
    // Make sure dirty flag is cleared.
    ASSERT (! pset1.isDirty());
    ASSERT (! pset2.isDirty());
    ASSERT (! pset5.isDirty());
    ASSERT (! pset6.isDirty());
    // Several rows are written now.
    ASSERT (tab1.nrow() == 2);
    ASSERT (tab2.nrow() == 7);
  }
  {
    // Set a solve grid for full work domain.
    // Also set to dirty, so it'll be written.
    Axis::ShPtr ax02 (new RegularAxis(3,1,7));
    Axis::ShPtr ax12 (new RegularAxis(4,2,4));
    Grid grid2(ax02, ax12);
    pset3.setSolveGrid (grid2);
    pset3.setDirty();
    ASSERT (pset3.size() == 1);
    pset4.setSolveGrid (grid2);
    pset4.setDirty();
    ASSERT (pset4.size() == 1);
    pset5.setSolveGrid (grid2);
    pset5.setDirty();
    ASSERT (pset5.size() == 28);
    ASSERT (pset5.getParmValue(0).getValues().size() == 6);
    ASSERT (pset5.getParmValue(0).getGrid().size() == 1);
    pset6.setSolveGrid (grid2);
    pset6.setDirty();
    ASSERT (pset6.size() == 1);
    parmCache.flush();
    // Several new rows are written.
    ASSERT (tab1.nrow() == 4);
    ASSERT (tab2.nrow() == 29);
  }
}

void testRead()
{
  ParmDB pdb1(ParmDBMeta("casa", "tParmCache_tmp.tab1"));
  ParmDB pdb2(ParmDBMeta("casa", "tParmCache_tmp.tab2"));
  Table tab1("tParmCache_tmp.tab1");
  Table tab2("tParmCache_tmp.tab2");
  ASSERT (tab1.nrow() == 4);
  ASSERT (tab2.nrow() == 29);
  ParmSet parmset;
  ParmId gainx1Id = parmset.addParm (pdb1, "gain:x:S1");
  ParmId gainy1Id = parmset.addParm (pdb1, "gain:y:S1");
  ParmId phasx1Id = parmset.addParm (pdb1, "phase:x:S1");
  ParmId phasy1Id = parmset.addParm (pdb1, "phase:y:S1");
  ParmId rasrcId  = parmset.addParm (pdb2, "ra:src");
  ParmId decsrcId = parmset.addParm (pdb2, "dec:src");
  // Make a work domain that partly overlaps the previous one.
  Box workDomain(make_pair(8,10), make_pair(16,18));
  // Create the cache and fill for the work domain.
  ParmCache parmCache(parmset, workDomain);
  // Check all values.
  // gain:x was 'solved' outside this domain, so has no values.
  // The same for gain:y.
  // phas:x and phas:y partly match this domain, so a value is found.
  // ra:src also partly matches the domain. It has coefficients, so a value
  // per solve domains. It was 'solved' from (3,4) till (10,12) on a (1,2) grid,
  // so 2 intervals match this work domain.
  // dec:src is a scalar and partly matches.
  ParmValueSet& pset1 = parmCache.getValueSet(gainx1Id);
  ASSERT (pset1.size() == 0);
  ASSERT (pset1.getGrid().size() == 1);
  const ParmValue& pval1 = pset1.getFirstParmValue();
  ASSERT (pval1.nx() == 1);
  ASSERT (pval1.ny() == 1);
  ASSERT (pval1.getValues().data()[0] == 1.5);
  ParmValueSet& pset2 = parmCache.getValueSet(gainy1Id);
  ASSERT (pset2.size() == 0);
  ASSERT (pset2.getGrid().size() == 1);
  const ParmValue& pval2 = pset2.getFirstParmValue();
  ASSERT (pval2.nx() == 1);
  ASSERT (pval2.ny() == 1);
  ASSERT (pval2.getValues().data()[0] == 1.);
  ParmValueSet& pset3 = parmCache.getValueSet(phasx1Id);
  ASSERT (pset3.size() == 1);
  ASSERT (pset3.getGrid().size() == 1);
  ASSERT (pset3.getGrid().getCell(0) == Box(Point(3,4),Point(10,12)));
  const ParmValue& pval3 = pset3.getFirstParmValue();
  ASSERT (pval3.nx() == 7);
  ASSERT (pval3.ny() == 4);
  ASSERT (pval3.getValues().data()[0] == 0.1);
  ASSERT (pval3.getGrid().size() == 28);
  ASSERT (pval3.getGrid().getBoundingBox() == Box(Point(3,4),Point(10,12)));
  ParmValueSet& pset4 = parmCache.getValueSet(phasy1Id);
  ASSERT (pset4.size() == 1);
  ASSERT (pset4.getGrid().size() == 1);
  ASSERT (pset4.getGrid().getCell(0) == Box(Point(3,4),Point(10,12)));
  const ParmValue& pval4 = pset4.getFirstParmValue();
  ASSERT (pval4.nx() == 7);
  ASSERT (pval4.ny() == 4);
  ASSERT (pval4.getValues().data()[0] == 0.1);
  ASSERT (pval4.getGrid().size() == 28);
  ASSERT (pval4.getGrid().getBoundingBox() == Box(Point(3,4),Point(10,12)));
  ParmValueSet& pset5 = parmCache.getValueSet(rasrcId);
  ASSERT (pset5.size() == 2);
  ASSERT (pset5.getGrid().size() == 2);
  ASSERT (pset5.getGrid().getBoundingBox() == Box(Point(8,10),Point(10,12)));
  const ParmValue& pval5 = pset5.getParmValue(0);
  ASSERT (pval5.nx() == 2);
  ASSERT (pval5.ny() == 3);
  ASSERT (pval5.getValues().data()[0] == 1.);
  ASSERT (pval5.getValues().data()[1] == 0.);
  ASSERT (pval5.getGrid().size() == 1);
  ParmValueSet& pset6 = parmCache.getValueSet(decsrcId);
  ASSERT (pset6.size() == 1);
  ASSERT (pset6.getGrid().size() == 1);
  ASSERT (pset6.getGrid().getBoundingBox() == Box(Point(3,4),Point(10,12)));
  const ParmValue& pval6 = pset6.getFirstParmValue();
  ASSERT (pval6.nx() == 7);
  ASSERT (pval6.ny() == 4);
  ASSERT (pval6.getValues().data()[0] == 0.);
  ASSERT (pval6.getGrid().size() == 28);
  ASSERT (pval6.getGrid().getBoundingBox() == Box(Point(3,4),Point(10,12)));
}

int main()
{
  try {
    INIT_LOGGER("tParmCache");
    testCreate();
    testRead();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

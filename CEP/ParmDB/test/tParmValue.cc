//# tParmValue.cc: Program to test the ParmValue classes
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
#include <ParmDB/ParmValue.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;

void testValue()
{
  ParmValue pvalue;
  ASSERT (pvalue.nx() == 1);
  ASSERT (pvalue.ny() == 1);
  ASSERT (!pvalue.hasErrors());
  ASSERT (pvalue.getRowId() < 0);
  ASSERT (pvalue.getValues().data()[0] == 0);
  pvalue.setRowId(10);
  ASSERT (pvalue.getRowId() == 10);
  Array<double> values(IPosition(2,5,6));
  indgen(values);
  pvalue.setCoeff (values);
  ASSERT (pvalue.nx() == 5);
  ASSERT (pvalue.ny() == 6);
  for (int i=0; i<30; ++i) {
    ASSERT (pvalue.getValues().data()[i] == i);
  }
  // Assure a copy of the data array was made.
  ASSERT (pvalue.getValues().data() != values.data());
  ASSERT (allEQ(pvalue.getValues(), values));
  // Check setting of errors.
  values += 1.;
  pvalue.setErrors (values);
  ASSERT (pvalue.hasErrors());
  ASSERT (pvalue.getErrors().data() != values.data());
  ASSERT (allEQ(pvalue.getErrors(), values));

  // Test assignment.
  ParmValue pvalue2;
  pvalue2 = pvalue;
  ASSERT (pvalue2.nx() == 5);
  ASSERT (pvalue2.ny() == 6);
  // Assure a copy of the data array was made.
  ASSERT (pvalue2.getValues().data() != pvalue.getValues().data());
  ASSERT (allEQ(pvalue2.getValues(), pvalue.getValues()));
  ASSERT (pvalue2.getErrors().data() != pvalue.getErrors().data());
  ASSERT (allEQ(pvalue2.getErrors(), values));
}

void testEmptySet()
{
  ParmValueSet pset;
  ASSERT (pset.size() == 0);
  ASSERT (pset.getType() == ParmValue::Scalar);
  ASSERT (!pset.isDirty());
  ASSERT (pset.getPerturbation() == 1e-6);
  ASSERT (pset.getPertRel());
  ASSERT (pset.getSolvableMask().nelements() == 0);
}

void testDefaultSet()
{
  ParmValue defaultValue(2);
  ParmValueSet pset(defaultValue);
  ASSERT (pset.size() == 0);
  ASSERT (pset.getType() == ParmValue::Scalar);
  ASSERT (pset.getGrid().size() == 1);
  ASSERT (!pset.isDirty());
  ASSERT (pset.getPerturbation() == 1e-6);
  ASSERT (pset.getPertRel());
  ASSERT (pset.getSolvableMask().nelements() == 0);
  const ParmValue& pvalue = pset.getFirstParmValue();
  ASSERT (pvalue.nx() == 1);
  ASSERT (pvalue.ny() == 1);
  ASSERT (pvalue.getValues().data()[0] == 2);
}

void testRegularSet()
{
  vector<ParmValue::ShPtr> values;
  vector<Box> domains;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      values.push_back (ParmValue::ShPtr(new ParmValue(j+4*i)));
      domains.push_back (Box(Point(j*2,i*3), Point(j*2+2,i*3+3)));
    }
  }
  ParmValueSet pset(Grid(domains), values, ParmValue(),
                    ParmValue::Polc, 2e-6, false);
  ASSERT (pset.size() == 12);
  ASSERT (pset.getType() == ParmValue::Polc);
  ASSERT (!pset.isDirty());
  ASSERT (pset.getPerturbation() == 2e-6);
  ASSERT (!pset.getPertRel());
  ASSERT (pset.getSolvableMask().nelements() == 0);
  for (int i=0; i<12; ++i) {
    const ParmValue& pvalue = pset.getParmValue(i);
    ASSERT (pvalue.nx() == 1);
    ASSERT (pvalue.ny() == 1);
    ASSERT (pvalue.getValues().data()[0] == i);
  }
  const Grid& grid = pset.getGrid();
  ASSERT (grid.size() == 12);
  int k=0;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      Box box = grid.getCell(k++);
      ASSERT (box == Box(Point(j*2,i*3), Point(j*2+2,i*3+3)));
    }
  }
  ASSERT (grid.getAxis(0)->classType() == "RegularAxis");
  ASSERT (grid.getAxis(1)->classType() == "RegularAxis");
}

void checkSolveScalarSet (const ParmValueSet& pset, uint nrx, uint nry)
{
  ASSERT (pset.size() == 1);
  ASSERT (pset.getType() == ParmValue::Scalar);
  ASSERT (pset.getGrid().size() == 1);
  ASSERT (pset.getParmValue(0).getGrid().size() == nrx*nry);
  ASSERT (!pset.isDirty());
  ASSERT (pset.getPerturbation() == 1e-6);
  ASSERT (pset.getPertRel());
  ASSERT (pset.getSolvableMask().nelements() == 0);
  const ParmValue& pvalue = pset.getParmValue(0);
  ASSERT (pvalue.getGrid().size() == nrx*nry);
  ASSERT (pvalue.nx() == nrx);
  ASSERT (pvalue.ny() == nry);
  ASSERT (pvalue.getValues().size() == nrx*nry);
  ASSERT (allEQ (pvalue.getValues(), 2.));
}

void testSolveScalarSet()
{
  // Define a default value.
  ParmValue defaultValue(2);
  ParmValueSet pset(defaultValue);
  // Define a solve grid.
  Axis::ShPtr ax0 (new RegularAxis(10,2,5));
  Axis::ShPtr ax1 (new RegularAxis(20,10,8));
  Grid grid(ax0, ax1);
  // Create a value set for the solve grid.
  pset.setSolveGrid(grid);
  checkSolveScalarSet (pset, 5, 8);
  // Set the solve grid again; this should not change anything.
  pset.setSolveGrid(grid);
  checkSolveScalarSet (pset, 5, 8);
  // Check setDirty.
  pset.setDirty();
  ASSERT (pset.isDirty());
  pset.setDirty(false);
  ASSERT (!pset.isDirty());
  // Make a bigger solve grid and set it again.
  // This should extend the ParmValue.
  Axis::ShPtr ax02 (new RegularAxis(6,2,10));
  Axis::ShPtr ax12 (new RegularAxis(10,10,12));
  Grid grid2(ax02, ax12);
  pset.setSolveGrid (grid2);
  checkSolveScalarSet (pset, 10, 12);
}

void checkSolveCoeffSet (const ParmValueSet& pset, uint nrx, uint nry)
{
  ASSERT (pset.size() == nrx*nry);
  ASSERT (pset.getType() == ParmValue::Polc);
  ASSERT (pset.getGrid().size() == nrx*nry);
  ASSERT (!pset.isDirty());
  ASSERT (pset.getPerturbation() == 1e-6);
  ASSERT (pset.getPertRel());
  ASSERT (pset.getSolvableMask().nelements() == 0);
  const ParmValue& pvalue = pset.getParmValue(0);
  ASSERT (pvalue.nx() == 2);
  ASSERT (pvalue.ny() == 3);
  ASSERT (pvalue.getValues().size() == 2*3);
  Array<double> coeff(IPosition(2,2,3));
  coeff = 0.;
  coeff(IPosition(2,0,0)) = 1.;
  ASSERT (allEQ (pvalue.getValues(), coeff));
}

void testSolveCoeffSet()
{
  // Define a default value.
  ParmValue defaultValue;
  Array<double> coeff(IPosition(2,2,3));
  coeff = 0.;
  coeff(IPosition(2,0,0)) = 1.;
  defaultValue.setCoeff (coeff);
  ParmValueSet pset(defaultValue, ParmValue::Polc);
  // Define a solve grid.
  Axis::ShPtr ax0 (new RegularAxis(10,2,5));
  Axis::ShPtr ax1 (new RegularAxis(20,10,8));
  Grid grid(ax0, ax1);
  // Create a value set for the solve grid.
  pset.setSolveGrid(grid);
  checkSolveCoeffSet (pset, 5, 8);
  // Set the solve grid again; this should not change anything.
  pset.setSolveGrid(grid);
  checkSolveCoeffSet (pset, 5, 8);
  // Check setDirty.
  pset.setDirty();
  ASSERT (pset.isDirty());
  pset.setDirty(false);
  ASSERT (!pset.isDirty());
  // Make a bigger solve grid and set it again.
  // This should extend the ParmValue.
  Axis::ShPtr ax02 (new RegularAxis(6,2,10));
  Axis::ShPtr ax12 (new RegularAxis(10,10,12));
  Grid grid2(ax02, ax12);
  pset.setSolveGrid (grid2);
  checkSolveCoeffSet (pset, 10, 12);
}

int main()
{
  try {
    INIT_LOGGER("tParmValue");
    cout << "testing ParmValue ..." << endl;
    testValue();
    cout << "testing empty set ..." << endl;
    testEmptySet();
    cout << "testing default set ..." << endl;
    testDefaultSet();
    cout << "testing regular set ..." << endl;
    testRegularSet();
    cout << "testing setSolveScalarGrid ..." << endl;
    testSolveScalarSet();
    cout << "testing setSolveCoeffSet ..." << endl;
    testSolveCoeffSet();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

//# tParmDBCasa.cc: Program to test the ParmDBCasa class
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <ParmDB/ParmDBCasa.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;

void testCreate()
{
  {
    ParmDBCasa pdb("tParmDBCasa_tmp.tab", true);
    ASSERT (pdb.getDefaultSteps()[0] == 1000.);
    ASSERT (pdb.getDefaultSteps()[1] == 1.);
  }
  // Test if the created tables exist and are empty.
  Table t1("tParmDBCasa_tmp.tab");
  ASSERT (t1.nrow() == 0);
  ASSERT (t1.keywordSet().asTable("NAMES").nrow() == 0);
  ASSERT (t1.keywordSet().asTable("DEFAULTVALUES").nrow() == 0);
  Table t2("tParmDBCasa_tmp.tab/DEFAULTVALUES");
  ASSERT (t2.nrow() == 0);
  Table t3("tParmDBCasa_tmp.tab/NAMES");
  ASSERT (t3.nrow() == 0);
  {
    ParmDBCasa pdb("tParmDBCasa_tmp.tab");
    ASSERT (pdb.getDefaultSteps()[0] == 1000.);
    ASSERT (pdb.getDefaultSteps()[1] == 1.);
    vector<double> defSteps(2);
    defSteps[0] = 2000;
    defSteps[1] = 5;
    pdb.setDefaultSteps (defSteps);
    ASSERT (pdb.getDefaultSteps()[0] == 2000.);
    ASSERT (pdb.getDefaultSteps()[1] == 5.);
  }
  {
    ParmDBCasa pdb("tParmDBCasa_tmp.tab");
    ASSERT (pdb.getDefaultSteps()[0] == 2000.);
    ASSERT (pdb.getDefaultSteps()[1] == 5.);
  }
}

void checkDef (const ParmValueSet& pset, double value,
               const Array<double>& values = Array<double>())
{
  // Check the value set of a default value.
  ASSERT (pset.size() == 0);
  ASSERT (!pset.isDirty());
  ASSERT (pset.getPerturbation() == 1e-6);
  ASSERT (pset.getSolvableMask().nelements() == 0);
  const ParmValue& pvalue = pset.getFirstParmValue();
  // Compare with array (if given) or scalar value.
  if (values.nelements() == 0) {
    // Scalar
    ASSERT (pvalue.nx() == 1);
    ASSERT (pvalue.ny() == 1);
    ASSERT (pvalue.getValues().data()[0] == value);
  } else {
    // Array
    ASSERT (pvalue.nx() == static_cast<uint>(values.shape()[0]));
    ASSERT (pvalue.ny() == static_cast<uint>(values.shape()[1]));
    ASSERT (allEQ(pvalue.getValues(), values));
  }
}

void testDef()
{
  // Test writing a default value.
  ParmDBCasa pdb("tParmDBCasa_tmp.tab");
  Table t1("tParmDBCasa_tmp.tab/DEFAULTVALUES");
  ASSERT (t1.nrow() == 0);
  // Initialize an [2,3] array.
  Array<double> coeff(IPosition(2,2,3));
  indgen(coeff);
  {
    // Put with a scalar value 2.
    ParmValue defaultValue(2);
    ParmValueSet pset(defaultValue);

    pdb.putDefValue ("test1", pset);
    ASSERT (t1.nrow() == 1);
  }
  {
    // Put with an array value.
    ParmValue defaultValue;
    defaultValue.setCoeff (coeff);
    ParmValueSet pset(defaultValue, ParmValue::Polc);
    pdb.putDefValue ("test2a", pset);
    ASSERT (t1.nrow() == 2);
  }
  {
    // Put with scalar value 4.
    ParmValue defaultValue(4);
    ParmValueSet pset(defaultValue);
    pdb.putDefValue ("test1.ra", pset);
    ASSERT (t1.nrow() == 3);
  }
  // Read the values back and check if thy match.
  // Use default scalar value 0 for non-existing ones.
  checkDef (pdb.getDefValue ("test1", ParmValue(0)), 2);
  checkDef (pdb.getDefValue ("test1.ra", ParmValue(0)), 4);
  checkDef (pdb.getDefValue ("test1.dec", ParmValue(0)), 0);
  checkDef (pdb.getDefValue ("test2", ParmValue(0)), 0);
  checkDef (pdb.getDefValue ("test2a", ParmValue(0)), 0, coeff);
  // Delete a value and check if thereafter its value uses default value 10.
  ASSERT (t1.nrow() == 3);
  pdb.deleteDefValues ("test1");
  ASSERT (t1.nrow() == 2);
  checkDef (pdb.getDefValue ("test1", ParmValue(10)), 10);
}

void testPutValues()
{
  // Test putting some parameters values with domains.
  ParmDBCasa pdb("tParmDBCasa_tmp.tab");
  Table t1("tParmDBCasa_tmp.tab");
  ASSERT (t1.nrow() == 0);
  // Initialize coefficient array.
  Array<double> coeff(IPosition(2,2,3));
  indgen(coeff, 1.);
  // Define two parm value sets (one for arrays, one for scalars).
  vector<ParmValue::ShPtr> values1;    // gets array values
  vector<ParmValue::ShPtr> values2;    // gets scalar values
  // Also create the domains (regular).
  vector<Box> domains;
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      ParmValue pv;
      pv.setCoeff (coeff+(j+4.*i));
      values1.push_back (ParmValue::ShPtr(new ParmValue(pv)));
      values2.push_back (ParmValue::ShPtr(new ParmValue(j+4.*i)));
      domains.push_back (Box(Point(j*2,i*3), Point(j*2+2,i*3+3)));
    }
  }
  ParmValueSet pset1(Grid(domains), values1, ParmValue(),
                     ParmValue::Polc, 2e-6, false);
  ParmValueSet pset2(Grid(domains), values2, ParmValue(),
                     ParmValue::Scalar, 2e-6, true);
  // Write arrays, thus one row per domain will be used.
  int nameId = -1;             // test.ra1 is a new name
  pdb.putValues ("test1.ra1", nameId, pset1);
  // The unique nameIds given start at 1.
  ASSERT (nameId == 0);
  ASSERT (t1.nrow() == 12);
  // Check if the rowids are filled in.
  for (uint i=0; i<pset1.size(); ++i) {
    ASSERT (pset1.getParmValue(i).getRowId() == int(i));
  }
  // Write scalars, thus all domains are put in a single row.
  nameId = -1;                 // test1.ra2 is new name
  pdb.putValues ("test1.ra2", nameId, pset2);
  ASSERT (nameId == 1);
  ASSERT (t1.nrow() == 24);
  for (uint i=0; i<pset2.size(); ++i) {
    ASSERT (pset2.getParmValue(i).getRowId() == int(12+i));
  }
}

void testGetValues()
{
  // Test reading the values back.
  ParmDBCasa pdb("tParmDBCasa_tmp.tab");
  // Check getting nameIds for known and unknown parameters.
  ASSERT (pdb.getNameId("test1") == -1);
  ASSERT (pdb.getNameId("test1.ra") == -1);
  ASSERT (pdb.getNameId("test1.ra1") == 0);
  ASSERT (pdb.getNameId("test1.ra2") == 1);
  // Checking getting all names.
  vector<string> names= pdb.getNames("");
  ASSERT (names.size()==2 && names[0]=="test1.ra1" && names[1]=="test1.ra2");
  // Check getting the range (bounding box of all domains).
  Box range = pdb.getRange("");
  ASSERT (range.lowerX()==0 && range.lowerY()==0);
  ASSERT (range.upperX()==8 && range.upperY()==9);
  // Read the values of all domains for the two parameters.
  // The parmIds give the indices of the elements in the ValueSet array
  // to be used.
  vector<uint> nameIds(2);
  nameIds[0]=0; nameIds[1]=1;
  vector<ParmId> parmIds(2);
  parmIds[0]=0; parmIds[1]=2;
  vector<ParmValueSet> values(3);
  pdb.getValues (values, nameIds, parmIds, range);
  // Check the array values and domains.
  Array<double> coeff(IPosition(2,2,3));
  indgen(coeff, 1.);
  ParmValueSet pset1 = values[0];
  ASSERT (pset1.size() == 12);
  ASSERT (pset1.getType() == ParmValue::Polc);
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      int inx = j+i*4;
      ASSERT (pset1.getParmValue(inx).getRowId() == inx);
      const ParmValue& pvalue = pset1.getParmValue(inx);
      ASSERT (pvalue.nx() == 2);
      ASSERT (pvalue.ny() == 3);
      ASSERT (allEQ(pvalue.getValues(), coeff+double(inx)));
    }
  }
  // Check the scalar values and domains.
  ParmValueSet pset2 = values[2];
  ASSERT (pset2.size() == 12);
  ASSERT (pset1.getType() == ParmValue::Polc);
  for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
      int inx = j+i*4;
      ASSERT (pset2.getParmValue(inx).getRowId() == 12+inx);
      const ParmValue& pvalue = pset2.getParmValue(inx);
      ASSERT (pvalue.nx() == 1);
      ASSERT (pvalue.ny() == 1);
      ASSERT (pvalue.getValues().data()[0] == inx);
    }
  }
}

int main()
{
  try {
    INIT_LOGGER("tParmDBCasa");
    testCreate();
    testDef();
    testPutValues();
    testGetValues();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

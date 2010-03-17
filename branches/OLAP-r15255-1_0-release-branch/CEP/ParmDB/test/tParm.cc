//# tParm.cc: Program to test the basics of the Parm class
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
#include <ParmDB/Parm.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;
using namespace std;

// Test the basic functionality.
void testBasic()
{
  // Create a parm data base.
  ParmDBMeta dbm1("casa", "tParmSet_tmp.pdb1");
  ParmDB db1(dbm1, true);
  // Create a parm in the ParmSet.
  ParmSet pset;
  ParmId parmid = pset.addParm (db1, "parm1");
  // Create a Parm object for it.
  ParmCache pcache(pset, Box(Point(1,5),Point(2,6)));
  Parm parm(pcache, parmid);
  Parm parm2(pcache, "parm1");
}

// Test the helper functions doing most of the work.
void testMakeCoeff()
{
  // Make an array with coefficients.
  Matrix<double> coeff(2,3);
  indgen (coeff, 1.);     // initialize with 1,2,3,...
  // Make the coefficients with an empty mask without applying the mask.
  vector<double> c1 = Parm::copyValues (coeff, Array<Bool>(), false);
  ASSERT (c1.size() == 6);
  for(uint i=0; i<c1.size(); ++i) {
    ASSERT (c1[i] = i+1);
  }
  // Make the coefficients with an empty mask without applying the mask.
  vector<double> c2 = Parm::copyValues (coeff, Array<Bool>(), true);
  ASSERT (c2.size() == 6);
  for(uint i=0; i<c2.size(); ++i) {
    ASSERT (c2[i] = i+1);
  }
  Matrix<bool> mask(2,3);
  mask = true;
  mask(1,1) = false;
  // Make the coefficients with a filled mask without applying the mask.
  vector<double> c3 = Parm::copyValues (coeff, mask, false);
  ASSERT (c3.size() == 6);
  for(uint i=0; i<c3.size(); ++i) {
    ASSERT (c3[i] = i+1);
  }
  // Make the coefficients with a filled mask with applying the mask.
  vector<double> c4 = Parm::copyValues (coeff, mask, true);
  ASSERT (c4.size() == 5);
  ASSERT (c4[0] == 1);
  ASSERT (c4[1] == 2);
  ASSERT (c4[2] == 3);
  ASSERT (c4[3] == 5);
  ASSERT (c4[4] == 6);
}

// Test evaluating a polc for a single and multiple ParmValues.
void testResultCoeff()
{
  AxisMappingCache axisCache;
  {
    // Test a set with a single ParmValue.
    Matrix<double> coeff(2,3);
    indgen (coeff, 1.);     // initialize with 1,2,3,...
    vector<ParmValue::ShPtr> pvec(1, ParmValue::ShPtr(new ParmValue));
    pvec[0]->setCoeff (coeff);
    Grid domainGrid(Axis::ShPtr(new RegularAxis(1,   6, 1)),
                    Axis::ShPtr(new RegularAxis(0.5, 4, 1)));
    ParmValueSet pvset(domainGrid, pvec, ParmValue(),
                       ParmValue::Polc, 1e-6, true);
    // Form a predict grid.
    Axis::ShPtr axis1(new RegularAxis(1,2,3));
    Axis::ShPtr axis2(new RegularAxis(0.5,1,4));
    Grid grid(axis1, axis2);
    Matrix<double> result;
    Parm::getResultCoeff (&result, grid, pvset, vector<double>(), axisCache);
    ASSERT (result.shape() == IPosition(2,3,4));
    for (int j=0; j<4; ++j) {
      double y = (j+1 - 0.5) / 4.;
      for (int i=0; i<3; ++i) {
        double x = (2*(i+1) - 1.) / 6.;
        double v = ((coeff(0,0) + coeff(1,0)*x) +
                    (coeff(0,1) + coeff(1,1)*x)*y +
                    (coeff(0,2) + coeff(1,2)*x)*y*y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result(i,j), v));
      }
    }
  }
  {
    // Test a set with multiple ParmValues.
    Matrix<double> coeff(2,3);
    indgen (coeff, 1.);     // initialize with 1,2,3,...
    vector<ParmValue::ShPtr> pvec;
    for (int i=0; i<4; ++i) {
      pvec.push_back (ParmValue::ShPtr(new ParmValue));
    }
    pvec[0]->setCoeff (coeff);
    pvec[1]->setCoeff (coeff+1.);
    pvec[2]->setCoeff (coeff+2.);
    pvec[3]->setCoeff (coeff+3.);
    // Use 2 domains in both x and y.
    Grid domainGrid(Axis::ShPtr(new RegularAxis(1,   3, 2)),
                    Axis::ShPtr(new RegularAxis(0.5, 2, 2)));
    ParmValueSet pvset(domainGrid, pvec, ParmValue(),
                       ParmValue::Polc, 1e-6, true);
    // Form a predict grid.
    // Note that the center of the middle x-cell is right on the border of
    // the parmvalue domains. It should take the right one below.
    Axis::ShPtr axis1(new RegularAxis(1,2,3));
    Axis::ShPtr axis2(new RegularAxis(0.5,1,4));
    Grid grid(axis1, axis2);
    Matrix<double> result;
    Parm::getResultCoeff (&result, grid, pvset, vector<double>(), axisCache);
    ASSERT (result.shape() == IPosition(2,3,4));
    for (int j=0; j<4; ++j) {
      double   y = (j+1 - 0.5) / 2.;
      if (j>1) y = (j+1 - 2.5) / 2.;
      double coeffadd = (j<2 ? 0:2);
      for (int i=0; i<3; ++i) {
        if (i==1) coeffadd += 1;
        double   x = (2*(i+1) - 1.) / 3.;
        if (i>0) x = (2*(i+1) - 4.) / 3.;
        double v = ((coeff(0,0) + coeffadd + (coeff(1,0) + coeffadd)*x) +
                    (coeff(0,1) + coeffadd + (coeff(1,1) + coeffadd)*x)*y +
                    (coeff(0,2) + coeffadd + (coeff(1,2) + coeffadd)*x)*y*y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result(i,j), v));
      }
    }
  }
  {
    // Test as above, but with perturbed values.
    Matrix<double> coeff(2,3);
    indgen (coeff, 0.1);    // initialize with 0.1,1.1,...
    vector<double> pertc(coeff.begin(), coeff.end());
    indgen (coeff, 1.);     // initialize with 1,2,3,...
    vector<ParmValue::ShPtr> pvec;
    for (int i=0; i<4; ++i) {
      pvec.push_back (ParmValue::ShPtr(new ParmValue));
    }
    pvec[0]->setCoeff (coeff);
    pvec[1]->setCoeff (coeff+1.);
    pvec[2]->setCoeff (coeff+2.);
    pvec[3]->setCoeff (coeff+3.);
    // Use 2 domains in both x and y.
    Grid domainGrid(Axis::ShPtr(new RegularAxis(1,   3, 2)),
                    Axis::ShPtr(new RegularAxis(0.5, 2, 2)));
    ParmValueSet pvset(domainGrid, pvec, ParmValue(),
                       ParmValue::Polc, 1e-6, true);
    // Form a predict grid.
    // Note that the center of the middle x-cell is right on the border of
    // the parmvalue domains. It should take the right one below.
    Axis::ShPtr axis1(new RegularAxis(1,2,3));
    Axis::ShPtr axis2(new RegularAxis(0.5,1,4));
    Grid grid(axis1, axis2);
    Array<double> results[7];
    Parm::getResultCoeff (results, grid, pvset, pertc, axisCache);
    const Array<double>& result = results[0];
    ASSERT (result.shape() == IPosition(2,3,4));
    for (int j=0; j<4; ++j) {
      double   y = (j+1 - 0.5) / 2.;
      if (j>1) y = (j+1 - 2.5) / 2.;
      double coeffadd = (j<2 ? 0:2);
      for (int i=0; i<3; ++i) {
        if (i==1) coeffadd += 1;
        double   x = (2*(i+1) - 1.) / 3.;
        if (i>0) x = (2*(i+1) - 4.) / 3.;
        double v = ((coeff(0,0) + coeffadd + (coeff(1,0) + coeffadd)*x) +
                    (coeff(0,1) + coeffadd + (coeff(1,1) + coeffadd)*x)*y +
                    (coeff(0,2) + coeffadd + (coeff(1,2) + coeffadd)*x)*y*y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result.data()[i+j*3], v));
      }
    }
    // Check the perturbed values.
    for (int ip=0; ip<6; ++ip) {
      const Array<double>& result = results[ip+1];
      coeff(ip%2, ip/2) += pertc[ip];
      ASSERT (result.shape() == IPosition(2,3,4));
      for (int j=0; j<4; ++j) {
        double   y = (j+1 - 0.5) / 2.;
        if (j>1) y = (j+1 - 2.5) / 2.;
        double coeffadd = (j<2 ? 0:2);
        for (int i=0; i<3; ++i) {
          if (i==1) coeffadd += 1;
          double   x = (2*(i+1) - 1.) / 3.;
          if (i>0) x = (2*(i+1) - 4.) / 3.;
          double v = ((coeff(0,0) + coeffadd + (coeff(1,0) + coeffadd)*x) +
                      (coeff(0,1) + coeffadd + (coeff(1,1) + coeffadd)*x)*y +
                      (coeff(0,2) + coeffadd + (coeff(1,2) + coeffadd)*x)*y*y);
          //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
          ASSERT (casa::near (result.data()[i+j*3], v));
        }
      }
      coeff(ip%2, ip/2) -= pertc[ip];
    }
  }
}

// Test a single ParmValue containing a scalar array.
void testResultOneScalar()
{
  AxisMappingCache axisCache;
  // Test a ParmValue with an array of scalars.
  Matrix<double> scalars(2,3);
  indgen (scalars, 1.);     // initialize with 1,2,3,...
  // Form the grid belonging to the values.
  Grid domainGrid(Axis::ShPtr(new RegularAxis(1,   3, 2)),
                  Axis::ShPtr(new RegularAxis(0.5, 4, 3)));
  ParmValue pval;
  pval.setScalars (domainGrid, scalars);
  {
    // Form a predict grid which is a subset of the domain grid.
    Axis::ShPtr axis1(new RegularAxis(2,1,4));
    Axis::ShPtr axis2(new RegularAxis(1,1,4));
    Grid grid(axis1, axis2);
    Matrix<double> result;
    Parm::getResultScalar (result, grid, pval, axisCache);
    ASSERT (result.shape() == IPosition(2,4,4));
    for (int j=0; j<4; ++j) {
      int y = (j<3 ? 0:1);
      for (int i=0; i<4; ++i) {
        int x = i/2;
        double v = scalars(x,y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result(i,j), v));
      }
    }
  }
  {
    // Form a predict grid for the entire domain grid.
    Axis::ShPtr axis1(new RegularAxis(1,1,6));
    Axis::ShPtr axis2(new RegularAxis(0.5,1,12));
    Grid grid(axis1, axis2);
    Matrix<double> result;
    Parm::getResultScalar (result, grid, pval, axisCache);
    ASSERT (result.shape() == IPosition(2,6,12));
    for (int j=0; j<12; ++j) {
      int y = j/4;
      for (int i=0; i<6; ++i) {
        int x = i/3;
        double v = scalars(x,y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result(i,j), v));
      }
    }
  }
}

// Test multiple ParmValues containing scalar arrays.
void testResultMultiScalar (bool setErrors)
{
  AxisMappingCache axisCache;
  // Create a few ParmValues with an array of scalars.
  vector<ParmValue::ShPtr> parmValues;
  for (uint j=0; j<4; ++j) {
    for (uint i=0; i<4; ++i) {
      Matrix<double> scalars(2,3);
      indgen (scalars, 10.*(i+4*j));
      // Form the grid belonging to the values.
      Grid domainGrid(Axis::ShPtr(new RegularAxis( 6*i+1, 3, 2)),
                      Axis::ShPtr(new RegularAxis(12*j+5, 4, 3)));
      ParmValue pval;
      pval.setScalars (domainGrid, scalars);
      if (setErrors) {
        pval.setErrors (scalars);
      }
      parmValues.push_back (ParmValue::ShPtr(new ParmValue(pval)));
    }
  }
  // Form the grid of the ParmValues.
  Grid parmGrid(Axis::ShPtr(new RegularAxis(1,  6, 4)),
                Axis::ShPtr(new RegularAxis(5, 12, 4)));
  ParmValueSet pvset(parmGrid, parmValues);
  {
    // Form a predict grid which is a subset of the parm grid.
    // Note that the x-width of a scalar value is 3, while predict width is 1.
    // In y it is 4 and 2. So every 3 predicted x-values and 2 y-values
    // have the same value.
    Axis::ShPtr axis1(new RegularAxis(2, 1,12));
    Axis::ShPtr axis2(new RegularAxis(11,2,18));
    Grid grid(axis1, axis2);
    // Get the result for this predict grid.
    Matrix<double> result;
    Matrix<double> errors;
    Parm::getResultScalar (result, &errors, grid, pvset, axisCache);
    ASSERT (result.shape() == IPosition(2,12,18));
    ASSERT (errors.shape() == result.shape());
    if (setErrors) {
      ASSERT (allEQ (errors, result));
    } else {
      ASSERT (allEQ (errors, -1.));
    }
    // Now check the result.
    for (int iy=0; iy<18; ++iy) {
      int pvy = (3+iy)/6;             // 6 y-values per ParmValue
      int y = (((3+iy)%6) / 2) * 2;   // y-value in ParmValue
      for (int ix=0; ix<12; ++ix) {
        int pvx = (1+ix)/6;           // 6 x-values per ParmValue
        int x = ((1+ix)%6) / 3;       // x-value in ParmValue
        double v = x + 10*pvx + y + 40*pvy;
        //cout << result(ix,iy) << ' ' << v << ' '<<pvx<<' '<<x<<' '<<pvy<<' '<<y<<endl;
        ASSERT (casa::near (result(ix,iy), v));
      }
    }
  }
  {
    // Form a predict grid for the entire domain grid.
    Axis::ShPtr axis1(new RegularAxis(1,1,24));
    Axis::ShPtr axis2(new RegularAxis(5,1,48));
    Grid grid(axis1, axis2);
    // Get the result for this predict grid.
    Matrix<double> result;
    Parm::getResultScalar (result, 0, grid, pvset, axisCache);
    ASSERT (result.shape() == IPosition(2,24,48));
    // Now check the result.
    for (int iy=0; iy<48; ++iy) {
      int pvy = (iy)/12;             // 6 y-values per ParmValue
      int y = (((iy)%12) / 4) * 2;   // y-value in ParmValue
      for (int ix=0; ix<24; ++ix) {
        int pvx = (ix)/6;            // 6 x-values per ParmValue
        int x = ((ix)%6) / 3;        // x-value in ParmValue
        double v = x + 10*pvx + y + 40*pvy;
        //cout << result(ix,iy) << ' ' << v << ' '<<pvx<<' '<<x<<' '<<pvy<<' '<<y<<endl;
        ASSERT (casa::near (result(ix,iy), v));
      }
    }
  }
}

// Test the Parm::getCoeff and setCoeff functions.
// It writes into the ParmDB, which is checked by testSetGetCoeff2.
void testSetGetCoeff1()
{
  // Create parmdb and write default values in it.
  ParmDB pdb(ParmDBMeta("casa", "tParm_tmp.tab1"), true);
  // Create default values in it.
  Array<double> coeff(IPosition(2,2,3));
  indgen(coeff);
  ParmValue defaultValue;
  defaultValue.setCoeff (coeff);
  pdb.putDefValue ("ra", ParmValueSet(defaultValue, ParmValue::Polc));
  defaultValue.setCoeff (coeff+10.);
  Array<bool> solvMask(coeff.shape());
  solvMask = true;
  solvMask(IPosition(2,1,2)) = false;
  ParmValueSet pvset(defaultValue, ParmValue::Polc);
  pvset.setSolvableMask(solvMask);
  pdb.putDefValue ("dec", pvset);
  // Create parmset.
  ParmSet parmset;
  parmset.addParm (pdb, "ra");
  parmset.addParm (pdb, "dec");
  Box workDomain(make_pair(3,4), make_pair(10,12));
  // Create the cache and fill for the work domain.
  ParmCache parmCache(parmset, workDomain);
  // Create the parms.
  Parm parmra(parmCache, "ra");
  Parm parmdc(parmCache, "dec");
  // Set a solve grid for some parameters (from (4,6) till (9,10))
  Axis::ShPtr ax02 (new RegularAxis(4,1,3));
  Axis::ShPtr ax12 (new RegularAxis(6,2,2));
  Grid grid2(ax02, ax12);
  parmra.setSolveGrid (grid2);
  parmdc.setSolveGrid (grid2);
  {
    // Check the coeff of the first solve grid cell.
    vector<double> coeffra = parmra.getCoeff (Location(0,0));
    ASSERT (coeffra.size() == coeff.size());
    for (uint i=0; i<coeffra.size(); ++i) {
      ASSERT (coeffra[i] == i);
    }
    vector<double> coeffdc = parmdc.getCoeff (Location(0,0));
    ASSERT (coeffdc.size() == coeff.size()-1);
    for (uint i=0; i<coeffdc.size(); ++i) {
      ASSERT (coeffdc[i] == i+10);
    }
    coeffdc = parmdc.getCoeff (Location(0,0), false);
    ASSERT (coeffdc.size() == coeff.size());
    for (uint i=0; i<coeffdc.size(); ++i) {
      ASSERT (coeffdc[i] == i+10);
    }
  }
  // Set coefficients 
  vector<double> newCoeff(5), newError(5);
  for (uint i=0; i<newCoeff.size(); ++i) {
    newCoeff[i] = (i+1)*0.1;
    newError[i] = (i+2)*0.001;
  }
  parmdc.setCoeff (Location(0,0), &(newCoeff[0]), newCoeff.size(),
                   &(newError[0]));
  vector<double> coeffdc = parmdc.getCoeff (Location(0,0));
  ASSERT (coeffdc.size() == newCoeff.size());
  for (uint i=0; i<coeffdc.size(); ++i) {
    ASSERT (coeffdc[i] == (i+1)*0.1);
  }
  vector<double> errordc = parmdc.getErrors (Location(0,0));
  ASSERT (errordc.size() == newError.size());
  for (uint i=0; i<errordc.size(); ++i) {
    ASSERT (errordc[i] == (i+2)*0.001);
  }
  parmCache.flush();
}

// Check if values were correctly written by testSetGetCoeff1.
void testSetGetCoeff2()
{
  // Open the parmdb (written by testSetGetCoeff1).
  ParmDB pdb(ParmDBMeta("casa", "tParm_tmp.tab1"));
  // Create parmset.
  ParmSet parmset;
  parmset.addParm (pdb, "dec");
  Box workDomain(make_pair(3,4), make_pair(10,12));
  // Create the cache and fill for the work domain.
  ParmCache parmCache(parmset, workDomain);
  // Create the parms.
  Parm parmdc(parmCache, "dec");
  // Set a solve grid (from (4,6) till (9,10))
  Axis::ShPtr ax02 (new RegularAxis(4,1,3));
  Axis::ShPtr ax12 (new RegularAxis(6,2,2));
  Grid grid2(ax02, ax12);
  parmdc.setSolveGrid (grid2);
  {
    // Check the coeff of the non-first solve grid cell.
    vector<double> coeffdc = parmdc.getCoeff (Location(1,1));
    ASSERT (coeffdc.size() == 5);
    for (uint i=0; i<coeffdc.size(); ++i) {
      ASSERT (coeffdc[i] == i+10);
    }
    vector<double> errordc = parmdc.getErrors (Location(1,1));
    ASSERT (errordc.size() == 0);
    // Check the coeff of the first solve grid cell.
    coeffdc = parmdc.getCoeff (Location(0,0));
    ASSERT (coeffdc.size() == 5);
    for (uint i=0; i<coeffdc.size(); ++i) {
      ASSERT (coeffdc[i] == (i+1)*0.1);
    }
    errordc = parmdc.getErrors (Location(0,0));
    ASSERT (errordc.size() == 5);
    for (uint i=0; i<errordc.size(); ++i) {
      ASSERT (errordc[i] == (i+2)*0.001);
    }
  }
}

// Test getting perturbed values for scalar values.
void testScalarPert()
{
  // Create parmdb and write default values in it.
  ParmDB pdb(ParmDBMeta("casa", "tParm_tmp.tab2"), true);
  // Create parmset.
  ParmSet parmset;
  parmset.addParm (pdb, "gain");
  Box workDomain(make_pair(3,4), make_pair(10,12));
  // Create the cache and fill for the work domain.
  ParmCache parmCache(parmset, workDomain);
  // Create the parms.
  Parm parmg(parmCache, "gain");
  // Getting or setting coeff should not work now.
  bool ok = false;
  try {
    parmg.getCoeff(Location(0,0));
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    ok = true;
  }
  ASSERT (ok);
  ok = false;
  try {
    parmg.setCoeff(Location(0,0), 0, 0);
  } catch (std::exception& x) {
    cout << "Expected exception: " << x.what() << endl;
    ok = true;
  }
  ASSERT (ok);
  // Now we want to set values for the parm.
  Matrix<double> scalars(2,3);
  indgen (scalars, 10.);
  // This requires it to be made solvable, so make solve grid.
  Grid solveGrid(Axis::ShPtr(new RegularAxis(1,   3, 2)),
                 Axis::ShPtr(new RegularAxis(0.5, 4, 3)));
  parmg.setSolveGrid (solveGrid);
  // Set the values for each cell.
  for (int i=0; i<6; ++i) {
    parmg.setCoeff(solveGrid.getCellLocation(i), scalars.data()+i, 1);
  }
  // Check them.
  for (int i=0; i<6; ++i) {
    vector<double> v = parmg.getCoeff(solveGrid.getCellLocation(i));
    ASSERT (v.size() == 1);
    ASSERT (v[0] == scalars.data()[i]);
  }
  {
    // Form a predict grid which is a subset of the domain grid.
    Axis::ShPtr axis1(new RegularAxis(2,1,4));
    Axis::ShPtr axis2(new RegularAxis(1,1,4));
    Grid grid(axis1, axis2);
    Matrix<double> result;
    parmg.getResult (result, grid);
    ASSERT (result.shape() == IPosition(2,4,4));
    for (int j=0; j<4; ++j) {
      int y = (j<3 ? 0:1);
      for (int i=0; i<4; ++i) {
        int x = i/2;
        double v = scalars(x,y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result(i,j), v));
      }
    }
  }
  {
    // Form a predict grid for the entire domain grid.
    Axis::ShPtr axis1(new RegularAxis(1,1,6));
    Axis::ShPtr axis2(new RegularAxis(0.5,1,12));
    Grid grid(axis1, axis2);
    vector<Array<double> > resultVec;
    parmg.getResult (resultVec, grid, true);
    ASSERT (resultVec.size() == 2);
    Matrix<double> result(resultVec[0]);
    Matrix<double> pert(resultVec[1]);
    ASSERT (result.shape() == IPosition(2,6,12));
    for (int j=0; j<12; ++j) {
      int y = j/4;
      for (int i=0; i<6; ++i) {
        int x = i/3;
        double v = scalars(x,y);
        //cout << result(i,j) << ' ' << v << ' '<<x<<' '<<y<<endl;
        ASSERT (casa::near (result(i,j), v));
        ASSERT (casa::near (pert(i,j), v + scalars(0,0)*1e-6));
      }
    }
  }
}

int main()
{
  try {
    INIT_LOGGER("tParm");
    testBasic();
    testMakeCoeff();
    testResultCoeff();
    testResultOneScalar();
    testResultMultiScalar(false);
    testResultMultiScalar(true);
    testSetGetCoeff1();
    testSetGetCoeff2();
    testScalarPert();
  } catch (exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}

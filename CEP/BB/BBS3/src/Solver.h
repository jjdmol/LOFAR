//# Solver.h: Calculate parameter values using a least squares fitter
//#
//# Copyright (C) 2004
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

#ifndef LOFAR_BB_BBS3_SOLVER_H
#define LOFAR_BB_BBS3_SOLVER_H

// \file Solver.h
// Calculates parameter values using a least squares fitter

#include <scimath/Fitting/LSQaips.h>
#include <BBS3/ParmData.h>
#include <BBS3/Quality.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

// Solver calculates new parameter values from the equations given by the
// Prediffer class.
class Solver
{

public:
  // Create Solver object.
  Solver();

  // Destructor
  ~Solver();

  // Initialize the solvable parm data.
  void initSolvableParmData (int nrPrediffers);

  // Set the solvable parm data for a given prediffer.
  void setSolvableParmData (const vector<ParmData>&, int prediffer);

  // Get the solvable parameter name info.
  const vector<ParmData>& getSolvableParmData() const
    { return itsSolvableParms; }

  // Get the solvable parameter values.
  const vector<double>& getSolvableValues() const
    { return itsSolvableValues; }

  // Set the equations for a given prediffer.
  // The data array has to be 4-dimensional with C-style shape
  // [nresult,nrspid+1,nval].
  // The first value on the spid axis is the difference between measured and
  // predicted data. The other values are the derivatives for each spid.
  // An equation is added to the solver for each value,result.
  // Note that nrspid has to match the ParmData object given to
  // setSolvableParmData for this prediffer.
  // Also note that setEquations cannot be called before setSolvableParmData
  // has been called for all prediffers.
  // After the last setEquations, the solve function can be called.
  void setEquations (const double* data, int nresult, int nrspid,
		     int nval, int prediffer);

  // Solve which returns solved parameter values in a vector and fit value 
  // in Quality object.
  void solve (bool useSVD,
	      Quality& resultQuality);

  // Get the solutions per prediffer.
  vector<vector<double> > getSolutions() const;

private:
  // Copy constructor and assignment are not allowed.
  // <group>
  Solver(const Solver& other);
  Solver& operator=(const Solver& other);
  // </group>


  casa::LSQaips  itsSolver;
  MeqMatrix      itsSolution;            //# Solution as complex numbers
  vector<double> itsFitME;
  Quality itsSol;                        //# Solution quality
  map<string,int>      itsSolvableMap;   //# Map solv.parameter name to index
  vector<ParmData>     itsSolvableParms; //# All solvable parameters
  vector<vector<int> > itsIndices;       //# Index of coefficient in prediffer
                                         //# to coefficient in solver
  vector<double>       itsSolvableValues;
  bool                 itsDoSet;         //# true = do itsSolver.set
};

// @}

} // namespace LOFAR

#endif

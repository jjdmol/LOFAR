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

#ifndef BB_BBS3_SOLVER_H
#define BB_BBS3_SOLVER_H

#include <scimath/Fitting/LSQaips.h>
#include <BBS3/MNS/MeqDomain.h>
#include <BBS3/MNS/ParmTable.h>
#include <BBS3/Quality.h>
#include <Common/LofarTypes.h>


namespace LOFAR
{

// Solver calculates new parameter values from the equations given by the
// Prediffer class.
class Solver
{

public:
  // Create Solver object.
  Solver (const string& msName,
	  const string& meqModel,
	  const string& skyModel,
	  const string& dbType,
	  const string& dbName,
	  const string& dbHost,
	  const string& dbPwd);

  // Destructor
  ~Solver();

  // Set the time interval for which to solve.
  void setTimeInterval (double intervalInSeconds);

  // Reset the iterator.
  void resetIterator();

  // Advance the iterator.
  // \returns false if at end of iteration.
  bool nextInterval (bool callReadPolcs = true);

  // Solve which returns solved parameter values in a vector and fit value 
  // in Quality object.
  void solve (Bool useSVD,
	      vector<string>& resultParmNames, 
	      vector<double>& resultParmValues,
	      Quality& resultQuality);

  // Save solved parameters to the MEP database.
  void saveParms();

  // Save all solvable parameters to the MEP database.
  void saveAllSolvableParms();

private:
  // Copy constructor and assignment are not allowed.
  // <group>
  Solver(const Solver& other);
  Solver& operator=(const Solver& other);
  // </group>


  string                itsMEPName;     // Common parmtable name
  ParmTable             itsMEP;         //# Common parmtable
  string                itsGSMMEPName;  // GSM parameters parmtable name
  ParmTable             itsGSMMEP;      //# parmtable for GSM parameters
  MeqDomain             itsSolveDomain;
  LSQaips      itsSolver;
  int          itsNrScid;               //# Nr of solvable parameter coeff.
  MeqMatrix    itsSolution;             //# Solution as complex numbers
  vector<double> itsFitME;
  vector<complex<double> > itsDeriv;    //# derivatives of predict
  Quality itsSol;                       //# Solution quality
  Vector<String> itsSolvableParms;     // Solvable parameters
};

} // namespace LOFAR

#endif

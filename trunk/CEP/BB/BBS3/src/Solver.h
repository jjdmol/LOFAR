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

class ParmData;

// Solver calculates new parameter values from the equations given by the
// Prediffer class.
class Solver
{

public:
  // Create Solver object.
  Solver (const string& meqModel,
	  const string& skyModel,
	  const string& dbType,
	  const string& dbName,
	  const string& dbHost,
	  const string& dbPwd);

  // Destructor
  ~Solver();

  // Start another time interval.
  // Hereafter getSolvableParmData can be called.
  // It returns false if the start value is outside the observation domain.
  // Length is trimmed if beyond end of observation.
  bool nextInterval (double start, double length, bool callReadPolcs = true);

  // Set the solvable parm data for a given prediffer.
  void setSolvableParmData (const ParmData&, int prediffer);

  // Set the equations for a given prediffer.
  // The data array has to be 4-dimensional with C-style shape
  // [nresult,nrspid+1,nrtime,nrfreq].
  // The first value on the spid axis is the difference between measured and
  // predicted data. The other values are the derivatives for each spid.
  // An equation is added to the solver for each freq,time,result.
  // Note that nrspid has to match the ParmData object given to
  // setSolvableParmData for this prediffer.
  // Also note that setEquations cannot be called before setSolvableParmData
  // has been called for all prediffers.
  // After the last setEquations, the solve function can be called.
  void setEquations (const dcomplex* data, int nresult, int nrspid,
		     int nrtime, int nrfreq, int prediffer);

  // Solve which returns solved parameter values in a vector and fit value 
  // in Quality object.
  void solve (casa::Bool useSVD,
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
  casa::LSQaips      itsSolver;
  int          itsNrScid;               //# Nr of solvable parameter coeff.
  MeqMatrix    itsSolution;             //# Solution as complex numbers
  vector<double> itsFitME;
  vector<complex<double> > itsDeriv;    //# derivatives of predict
  Quality itsSol;                       //# Solution quality
  casa::Vector<casa::String> itsSolvableParms;     // Solvable parameters

  std::vector<double> itsTimes;     // All times in MS
  std::vector<double> itsIntervals; // All intervals in MS
  unsigned int   itsTimeIndex;      // The index of the current time
  unsigned int   itsNrTimes;        // The number of times in the time interval
  double itsStartFreq;
  double itsEndFreq;
};

} // namespace LOFAR

#endif

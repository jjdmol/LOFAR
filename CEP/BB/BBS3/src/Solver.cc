//# Solver.cc: Calculate parameter values using a least squares fitter
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

#include <lofar_config.h>

#include <BBS3/Solver.h>
#include <Common/VectorUtil.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <iostream>
#include <fstream>
#include <stdexcept>


namespace LOFAR
{

//----------------------------------------------------------------------
//
// ~Solver
//
// Constructor. Initialize a Solver object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
Solver::Solver ()
: itsSolver (1),
  itsDoSet  (true),
  itsNUsed  (0),
  itsNFlag  (0)
{
  LOG_INFO_STR( "Solver constructor" );
}

//----------------------------------------------------------------------
//
// ~~Solver
//
// Destructor for a Solver object.
//
//----------------------------------------------------------------------
Solver::~Solver()
{
  LOG_TRACE_FLOW( "Solver destructor" );
}


// ~solve
//
// Solve for the solvable parameters on the current time domain.
// Returns solution and fitness.
//
//----------------------------------------------------------------------

void Solver::solve (bool useSVD,
		    Quality& resultQuality)
{
  LOG_INFO_STR( "solve using file ");

  NSTimer timer;
  timer.start();
  ASSERT ((unsigned) itsSolvableValues.size() > 0);

  // Initialize the solver (needed after a setSolvable).
  // Note it is usually already done in setEquations.
  if (itsDoSet) {
    itsSolver.set ((unsigned) itsSolvableValues.size());
    itsNUsed = 0;
    itsNFlag = 0;
    itsDoSet = false;
  }
  // Solve the equation. 
  uint rank;
  double fit;
  cout << "Nr of used data points:    " << itsNUsed << endl;
  cout << "Nr of flagged data points: " << itsNFlag << endl;
  LOG_INFO_STR( "Nr of used data points:    " << itsNUsed);
  LOG_INFO_STR( "Nr of flagged data points: " << itsNFlag);
  LOG_INFO_STR( "Solution before: " << itsSolvableValues);
  rank = 0;
  bool solFlag = itsSolver.solveLoop (fit, rank, &(itsSolvableValues[0]),
				      useSVD);
  LOG_INFO_STR( "Solution after:  " << itsSolvableValues);

  resultQuality.init();
  resultQuality.itsSolFlag = solFlag;
  resultQuality.itsRank = rank;
  resultQuality.itsFit = fit;
  resultQuality.itsMu = itsSolver.getWeightedSD();
  resultQuality.itsStddev = itsSolver.getSD();
  resultQuality.itsChi = itsSolver.getChi();
  cout << resultQuality << endl;

  // Store the new values in the ParmData vector.
  const double* val = &(itsSolvableValues[0]);
  for (uint i=0; i<itsSolvableParms.size(); ++i) {
    ParmData& parm = itsSolvableParms[i];
    double* value = parm.getRWValues().doubleStorage();
    for (int j=0; j<parm.getNrSpid(); ++j) {
      value[j] = *val++;
    }
  }

  timer.stop();
  cout << "BBSTest: solver     " << timer << endl;
  itsNUsed = 0;
  itsNFlag = 0;
  return;
}


void Solver::setEquations (const double* data, const char* flags,
			   int nresult, int nrspid, int nval, int prediffer)
{
  ASSERT (uint(prediffer) < itsIndices.size());
  // Initialize the solver (needed after a setSolvable).
  if (itsDoSet) {
    itsSolver.set ((unsigned) itsSolvableValues.size());
    itsNUsed = 0;
    itsNFlag = 0;
    itsDoSet = false;
  }
  vector<int>& predInx = itsIndices[prediffer];
  ASSERT (uint(nrspid) == predInx.size());
  // Use a consecutive vector to assemble all derivatives.
  vector<double> derivVec(nrspid);
  double* derivs = &(derivVec[0]);
  // Each result is a 2d array of [nrval,nrspid+1] (nrval varies most rapidly).
  // The first value is the difference; the others the derivatives. 
  int nrval = nval;
  for (int i=0; i<nresult; ++i) {
    for (int j=0; j<nrval; ++j) {
      // Each value result,freq gives an equation (unless flagged).
      if (*flags++ == 0) {
	double diff = data[0];
	const double* derivdata = data + nrval;
	for (int k=0; k<nrspid; ++k) {
	  derivs[k] = derivdata[k*nrval];
	}
	itsSolver.makeNorm (predInx.size(), &(predInx[0]), &(derivVec[0]),
			    1., diff);
	itsNUsed++;
      } else {
	itsNFlag++;
      }
      // Go to next time,freq value.
      data++;
    }
    // Go to next result (note that data has already been incremented nrval
    // times, so here we use nrspid instead of nrspid+1.
    data += nrspid*nrval;
  }
}

void Solver::initSolvableParmData (int nrPrediffers)
{
  itsSolvableMap.clear();
  itsSolvableParms.resize (0);
  itsIndices.resize (nrPrediffers);
  itsSolvableValues.resize (0);
  itsDoSet = true;
}

// The solvable parameters coming from the prediffers have to be collected
// and put into a single solvercollection.
// It means that the prediffer spids have to be mapped to a solver spids.
void Solver::setSolvableParmData (const std::vector<ParmData>& data,
				  int prediffer)
{
  if (data.size() == 0) {
    return;
  }
  itsDoSet = true;
  // Usually the last entry has the highest spid; so use that in reserve to
  // avoid (hopefully) resizes.
  vector<int> predInx;
  predInx.reserve (data[data.size()-1].getLastSpid() + 1);
  // Loop through all parm entries and see if already used.
  for (uint i=0; i<data.size(); ++i) {
    int parminx = itsSolvableMap.size();
    int spidinx = itsSolvableValues.size();
    ParmData parm(data[i]);
    map<string,int>::const_iterator value =
                                     itsSolvableMap.find (parm.getName());
    if (value == itsSolvableMap.end()) {
      // Not in use yet; so add it to the map.
      itsSolvableMap[parm.getName()] = parminx;
      // Its first solver spid is the length of the vector so far.
      parm.setFirstSpid (spidinx);
      // Save the parm.
      itsSolvableParms.push_back (parm);
      // Store the current values in the vector.
      const double* value = parm.getValues().doubleStorage();
      for (int i=0; i<parm.getNrSpid(); ++i) {
	itsSolvableValues.push_back (value[i]);
      }
    } else {
      // The parameter has already been used. Get its index.
      parminx = value->second;
      ASSERT (itsSolvableParms[parminx] == parm);
      spidinx = itsSolvableParms[parminx].getFirstSpid();
    }
    // Store the solver spid to get a mapping of prediffer spid to solver spid.
    int spidnr = parm.getFirstSpid();
    for (int i=0; i<parm.getNrSpid(); ++i) {
      predInx.push_back (spidnr++);
    }
  }
  ASSERT (uint(prediffer) < itsIndices.size());
  itsIndices[prediffer] = predInx;
}

vector<vector<double> > Solver::getSolutions() const
{
  vector<vector<double> > solutions(itsIndices.size());
  for (uint i=0; i<itsIndices.size(); ++i) {
    const vector<int>& predInx = itsIndices[i];
    vector<double>& sols = solutions[i];
    sols.resize (predInx.size());
    for (uint j=0; j<predInx.size(); ++j) {
      sols[j] = itsSolvableValues[predInx[j]];
    }
  }
  return solutions;
}

} // namespace LOFAR

//# Instantiate the makeNorm template.
#include <scimath/Fitting/LSQFit2.cc>
template void casa::LSQFit::makeNorm<double, double*, int*>(unsigned, int* const&, double* const&, double const&, double const&, bool, bool);

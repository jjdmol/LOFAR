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

#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Timer.h>

#include <Common/BlobIStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobArray.h>

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
: itsSolver (1)
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

void Solver::solve(bool useSVD, vector<string>& resultParmNames, 
		   vector<double>& resultParmValues,
		   Quality& resultQuality)
{
  //  LOG_INFO_STR( "solve using file " << itsDataMap->getFileName());

//   Timer timer;
//   NSTimer solveTimer, totTimer;
//   totTimer.start();
//   if (itsNrScid == 0) {
//     throw AipsError ("No parameters are set to solvable");
//   }
//   int nrpoint = 0;

//   double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
//   double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
//   int nrchan = 1+itsLastChan-itsFirstChan;

//   bool showd = false;
  
//   // Complex values are separated in real and imaginary.
//   // Loop over all times in current time interval.
//   for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
//   {
//     unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNPol;
//     double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
//     double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];
 
//     MeqDomain domain(time-interv/2, time+interv/2, startFreq, endFreq);
//     MeqRequest request(domain, 1, nrchan, itsNrScid);

//     for (unsigned int bl=0; bl<itsNrBl; bl++)
//     {
//       uInt ant1 = itsAnt1Data(bl);
//       uInt ant2 = itsAnt2Data(bl);
//       if (itsBLSelection(ant1,ant2) == true)
//       {
//        // Get condition equations (measured-predicted and all derivatives).
//         vector<MeqResult> res = getCondeq (request, bl, ant1, ant2, showd);
//         // Add them all to the solver.
//       } // Matches: if (itsBLSelection== )
//     } // End loop baselines
//   } // End loop timesteps


//   // The actual solve
//   //  if (Debug(1)) timer.show("fill ");

//   ASSERT (nrpoint >= itsNrScid);
//   // Solve the equation. 
//   uInt rank;
//   double fit;

//   LOG_INFO_STR( "Solution before: " << itsSolution);
//   //  cout << "Solution before: " << itsSolution << endl;
//   // It looks as if LSQ has a bug so that solveLoop and getCovariance
//   // interact badly (maybe both doing an invert).
//   // So make a copy to separate them.
//   Matrix<double> covar;
//   Vector<double> errors;
// ///   LSQaips tmpSolver = itsSolver;
// ///   tmpSolver.getCovariance (covar);
// ///   tmpSolver.getErrors (errors);
//   int nrs = itsSolution.nelements();
//   Vector<double> sol(nrs);
//   double* solData = itsSolution.doubleStorage();
//   for (int i=0; i<itsSolution.nelements(); i++) {
//     sol[i] = solData[i];
//   }
//   solveTimer.start();
//   bool solFlag = itsSolver.solveLoop (fit, rank, sol, useSVD);
//   solveTimer.stop();
//   for (int i=0; i<itsSolution.nelements(); i++) {
//     solData[i] = sol[i];
//   }
//   //  if (Debug(1)) timer.show("solve");
//   LOG_INFO_STR( "Solution after:  " << itsSolution );
//   //cout << "Solution after:  " << itsSolution << endl;


  
//   resultParmValues.clear();
//   for (int nr=0; nr < itsSolution.nx(); nr++)
//   {
//     resultParmValues.push_back(itsSolution.getDouble(nr, 0));
//   }

//   resultParmNames.clear();
 
//   // Update all parameters.
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();
//   int i=0;
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     if (itsIsParmSolvable[i]) {
//       resultParmNames.push_back((*iter)->getName());

//       (*iter)->update (itsSolution);
//     }
//     i++;
//   }

//   resultQuality.init();
//   resultQuality.itsSolFlag = solFlag;
//   resultQuality.itsRank = rank;
//   resultQuality.itsFit = fit;
//   resultQuality.itsMu = itsSolver.getWeightedSD();;
//   resultQuality.itsStddev = itsSolver.getSD();;
//   resultQuality.itsChi = itsSolver.getChi();
//   cout << resultQuality << endl;

//   totTimer.stop();
//   cout << "BBSTest: solver     " << solveTimer << endl;
//   cout << "BBSTest: total-iter " << totTimer << endl;
//   timer.show("BBSTest: solve ");

  return;
}


void Solver::setEquations (const dcomplex* data, int nresult, int nrspid,
		   int nrtime, int nrfreq, int prediffer)
{}

void Solver::setSolvableParmData (const ParmData&, int prediffer)
{}

} // namespace LOFAR

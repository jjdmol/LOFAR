//# Solver.cc: Base class for an expression node
//#
//# Copyright (C) 2003
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

#include <MEQ/Solver.h>
#include <MEQ/Condeq.h>
#include <MEQ/Request.h>
#include <MEQ/Vells.h>
#include <MEQ/Function.h>
#include <MEQ/MeqVocabulary.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Vector.h>
    

namespace Meq {

Solver::Solver()
: itsSolver  (1, LSQBase::REAL),
  itsNumStep (1),
  itsEpsilon (0)
{}

Solver::~Solver()
{}

TypeId Solver::objectType() const
{
  return TpMeqSolver;
}

// no need for now
// void Solver::init (DataRecord::Ref::Xfer &initrec, Forest* frst)
// {
//   Node::init(initrec,frst);
// }
// 

void Solver::checkChildren()
{
  // Copy all Condeq nodes to the vector.
  // All other nodes are ignored.
  itsCondeqs.reserve (numChildren());
  for (int i=0; i<numChildren(); i++) {
    Condeq* condeq = dynamic_cast<Condeq*>(&(getChild(i)));
    if (condeq) {
      itsCondeqs.push_back (condeq);
    }
  }
  Assert (itsCondeqs.size() > 0);
}

// do nothing here -- we'll do it manually in getResult()
int Solver::pollChildren (std::vector<Result::Ref> &,
                          Result::Ref &,const Request &)
{
  return 0; 
}

// Get the result for the given request.
int Solver::getResult (Result::Ref &resref, 
                       const std::vector<Result::Ref> &,
                       const Request &request,bool newreq)
{
  // The result has 1 plane.
  Result& result = resref <<= new Result(request,1);
  VellSet& vellset = result.setNewVellSet(0);
  // Allocate variables needed for the solution.
  uInt rank;
  double fit;
  double stddev;
  double mu;
  Matrix<double> covar;
  Vector<double> errors;
  int nrch = itsCondeqs.size();
  std::vector<Result::Ref> child_results(nrch);
  // Copy the request and attach the solvable parms to it.
  Request newReq = request;
  if (state()[FSolvableParm].exists()) {
    if (! newReq[FNodeState].exists()) {
      newReq[FNodeState] <<= new DataRecord();
    }
    newReq[FNodeState][FSolvableParm] <<= 
      state()[FSolvableParm].as_p<DataRecord>();
  }
  // Iterate as many times as needed.
  for (int step=0; step<itsNumStep; step++) {
    // collect child results, using Node's standard method
    int retcode = Node::pollChildren(child_results,resref,request);
    // a fail or a wait is returned immediately
    if( retcode&(RES_FAIL|RES_WAIT) )
      return retcode;
    // else process 
    vector<VellSet*> chvellsets;
    chvellsets.reserve(nrch * child_results[0]->numVellSets());
    // Find the set of all spids from all results.
    for (int i=0; i<nrch; i++) {
      for (int iplane=0; iplane<child_results[i]->numVellSets(); iplane++) {
        if (! child_results[i]().vellSet(iplane).isFail()) {
          chvellsets.push_back (&(child_results[i]().vellSet(iplane)));
        }
      }
    }
    vector<int> spids = Function::findSpids (chvellsets);
    // Initialize solver.
    uint nspid = spids.size();
    itsSolver.set (nspid, 1u, 0u);
    // Now feed the solver with equations from the results.
    // Define the vector with derivatives (for real and imaginary part).
    vector<double> derivReal(nspid);
    vector<double> derivImag(nspid);
    // Loop through all results and fill the deriv vectors.
    for (uint i=0; i<chvellsets.size(); i++) {
      VellSet& chresult = *chvellsets[i];
      bool isReal = chresult.getValue().isReal();
      // Get nr of elements in the values.
      int nrval = chresult.getValue().nelements();
      // Get pointer to all perturbed values.
      int index=0;
      if (isReal) {
        const double* values = chresult.getValue().realStorage();
        vector<const double*> perts(nspid, 0);
        for (uint j=0; j<nspid; j++) {
          if (chresult.isDefined (spids[j], index)) {
            Assert (chresult.getPerturbedValue(index-1).nelements() == nrval);
            perts[j] = chresult.getPerturbedValue(index-1).realStorage();
          }
        }
        // Generate an equation for each value element.
        // An equation contains the value and all derivatives.
        for (int j=0; j<nrval; j++) {
          for (uint spid=0; spid<perts.size(); spid++) {
            if (perts[spid]) {
              derivReal[spid] = perts[spid][j];
            } else {
              derivReal[spid] = 0;
            }
          }
          itsSolver.makeNorm (&derivReal[0], 1., values+j);
        }
      } else {
        const dcomplex* values = chresult.getValue().complexStorage();
        vector<const dcomplex*> perts(nspid, 0);
        for (uint j=0; j<nspid; j++) {
          if (chresult.isDefined (spids[j], index)) {
            Assert (chresult.getPerturbedValue(index-1).nelements() == nrval);
            perts[j] = chresult.getPerturbedValue(index-1).complexStorage();
          }
        }
        // Generate an equation for each value element.
        // An equation contains the value and all derivatives.
        double val;
        for (int j=0; j<nrval; j++) {
          for (uint spid=0; spid<perts.size(); spid++) {
            if (perts[spid]) {
              derivReal[spid] = perts[spid][j].real();
              derivImag[spid] = perts[spid][j].imag();
            } else {
              derivReal[spid] = 0;
              derivImag[spid] = 0;
            }
          }
          val = values[j].real();
          itsSolver.makeNorm (&derivReal[0], 1., &val);
          val = values[j].imag();
          itsSolver.makeNorm (&derivImag[0], 1., &val);
        }
      }
    }
    // Solve the equation.
    double* sol = vellset.setReal(nspid, 1).data();
    Vector<double> solution (IPosition(1,nspid), sol, SHARE);
    solution = 0;
    // It looks as if LSQ has a bug so that solveLoop and getCovariance
    // interact badly (maybe both doing an invert).
    // So make a copy to separate them.
    FitLSQ tmpSolver = itsSolver;
    tmpSolver.getCovariance (covar);
    tmpSolver.getErrors (errors);
    bool solFlag = itsSolver.solveLoop (fit, rank, solution,
                                        stddev, mu, itsUseSVD);
    cout << "Solution after:  " << solution << endl;
    // Remove the rider for the next iterations.
    if (state()[FSolvableParm].exists()) {
      newReq[FNodeState][FSolvableParm].remove();
    }
  }
  // result depends on domain, and has -- most likely -- been updated
  return RES_DEP_DOMAIN|RES_UPDATED;
}

void Solver::setStateImpl (DataRecord& newst,bool initializing)
{
  Node::setStateImpl(newst,initializing);
  getStateField(itsNumStep,newst,FNumSteps);
  getStateField(itsEpsilon,newst,FEpsilon);
  getStateField(itsUseSVD,newst,FUseSVD);
}


} // namespace Meq

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
                       const Request &request, bool newreq)
{
  // The result has 1 plane.
  Result& result = resref <<= new Result(request, 1);
  VellSet& vellset = result.setNewVellSet(0);
  // Allocate variables needed for the solution.
  uint nspid;
  vector<int> spids;
  Vector<double> solution;
  Vector<double> allSolutions;
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
      wstate()[FSolvableParm].as_wp<DataRecord>();
  }
  // Iterate as many times as needed.
  int step;
  for (step=0; step<itsNumStep; step++) {
    // collect child results, using Node's standard method
    int retcode = Node::pollChildren (child_results, resref, newReq);
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
    spids = Function::findSpids (chvellsets);
    // Initialize solver.
    nspid = spids.size();
    AssertStr (nspid > 0, "No solvable parameters found in solver " << name());
    itsSolver.set (nspid, 1u, 0u);
    // Now feed the solver with equations from the results.
    // Define the vector with derivatives (for real and imaginary part).
    vector<double> derivReal(nspid);
    vector<double> derivImag(nspid);
    // Loop through all results and fill the deriv vectors.
    uint nreq = 0;
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
          int inx = chresult.isDefined (spids[j], index);
	  if (inx >= 0) {
            Assert (chresult.getPerturbedValue(inx).nelements() == nrval);
            perts[j] = chresult.getPerturbedValue(inx).realStorage();
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
	  nreq++;
        }
      } else {
        const dcomplex* values = chresult.getValue().complexStorage();
        vector<const dcomplex*> perts(nspid, 0);
        for (uint j=0; j<nspid; j++) {
          int inx = chresult.isDefined (spids[j], index);
	  if (inx >= 0) {
            Assert (chresult.getPerturbedValue(inx).nelements() == nrval);
            perts[j] = chresult.getPerturbedValue(inx).complexStorage();
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
	  nreq++;
          val = values[j].imag();
          itsSolver.makeNorm (&derivImag[0], 1., &val);
	  nreq++;
        }
      }
      // Increment the last part of the request id if possible.
      HIID rid = newReq.id();
      if (rid.size() > 0) {
	rid[rid.size()-1] = rid[rid.size()-1].id() + 1;
	newReq.setId (rid);
      }
    }
    // Solve the equation.
    AssertStr (nreq >= nspid, "Only " << nreq << " equations for " << nspid
	       << " solvable parameters in solver " << name());
    // Keep all solutions in a vector.
    // The last part is the current solution.
    allSolutions.resize ((step+1)*nspid, True);
    Vector<double> vec(allSolutions(Slice(step*nspid, nspid)));
    solution.reference (vec);
    solution = 0;
    // It looks as if LSQ has a bug so that solveLoop and getCovariance
    // interact badly (maybe both doing an invert).
    // So make a copy to separate them.
    FitLSQ tmpSolver = itsSolver;
    tmpSolver.getCovariance (covar);
    tmpSolver.getErrors (errors);
    bool solFlag = itsSolver.solveLoop (fit, rank, solution,
                                        stddev, mu, itsUseSVD);
    cdebug(4) << "Solution after:  " << solution << endl;
    // Put the solution in the FNodeState,FByNodeIndex data record.
    // That will contain a DataRecord for each parm with the parmid
    // as the index.
    DataRecord& dr1 =
      newReq[FNodeState][FSolvableParm].replace() <<= new DataRecord;
    DataRecord& dr2 = dr1[FByNodeIndex] <<= new DataRecord;
    fillSolution (dr2, spids, solution);
  }
  // Distribute the last solution.
  // Do that in an empty request.
  Request lastReq;
  DataRecord& ldr1 = lastReq[FNodeState] <<= new DataRecord;
  DataRecord& ldr2 = ldr1[FSolvableParm] <<= new DataRecord;
  DataRecord& dr2 = ldr2[FByNodeIndex] <<= new DataRecord;
  fillSolution (dr2, spids, solution);
  Node::pollChildren (child_results, resref, lastReq);
  // result depends on domain, and has -- most likely -- been updated
  double* sol = vellset.setReal(nspid, step).data();
  memcpy (sol, allSolutions.data(), nspid*step*sizeof(double));
  return RES_DEP_DOMAIN|RES_UPDATED;
}

void Solver::fillSolution (DataRecord& rec, const vector<int> spids,
			   const Vector<double>& solution)
{
  // Split the solution into vectors for each parm.
  // Reserve enough space in the vector.
  vector<double> parmSol;
  uint nspid = spids.size();
  parmSol.reserve (nspid);
  int lastParmid = spids[0] / 256;
  for (uint i=0; i<nspid; i++) {
    if (spids[i]/256 != lastParmid) {
      DataRecord& drp = rec[lastParmid] <<= new DataRecord;
      drp[FValue] = parmSol;
      lastParmid = spids[i] / 256;
      parmSol.resize(0);
    }
    parmSol.push_back (solution[i]);
  }
  DataRecord& drp = rec[lastParmid] <<= new DataRecord;
  drp[FValue] = parmSol;
}

void Solver::setStateImpl (DataRecord& newst,bool initializing)
{
  Node::setStateImpl(newst,initializing);
  getStateField(itsNumStep,newst,FNumSteps);
  getStateField(itsEpsilon,newst,FEpsilon);
  getStateField(itsUseSVD,newst,FUseSVD);
}


} // namespace Meq

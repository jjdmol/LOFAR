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
#include <MEQ/ParmTable.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Vector.h>
    

namespace Meq {

//##ModelId=400E53550260
Solver::Solver()
: itsSolver  (1, LSQBase::REAL),
  itsNumStep (1),
  itsEpsilon (0),
  itsParmGroup(AidParm)
{}

//##ModelId=400E53550261
Solver::~Solver()
{}

//##ModelId=400E53550263
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

//##ModelId=400E53550265
void Solver::checkChildren()
{
  // count the number of Condeq nodes
  itsNumCondeqs = 0;
  itsIsCondeq.resize(numChildren());
  for (int i=0; i<numChildren(); i++) 
    if( itsIsCondeq[i] = ( getChild(i).objectType() == TpMeqCondeq ) )
      itsNumCondeqs++;
  FailWhen(!itsNumCondeqs,"Solver node must have at least one Condeq child");
}

// do nothing here -- we'll do it manually in getResult()
//##ModelId=400E5355026B
int Solver::pollChildren (std::vector<Result::Ref> &,
                          Result::Ref &,const Request &)
{
  return 0; 
}

// Get the result for the given request.
//##ModelId=400E53550270
int Solver::getResult (Result::Ref &resref, 
                       const std::vector<Result::Ref> &,
                       const Request &request, bool newreq)
{
  // Use 1 derivative by default, or 2 if specified in request
  int calcDeriv = std::max(request.calcDeriv(),1);
  // The result has 1 plane.
  Result& result = resref <<= new Result(request, 1);
  VellSet& vellset = result.setNewVellSet(0);
  DataRecord& metricsRec = result[FMetrics] <<= new DataRecord;
  // Check if we have to restart the solver.
  if (request.clearSolver()) {
    itsSpids.clear();
  }
  // Allocate variables needed for the solution.
  uint nspid;
  vector<int> spids;
  Vector<double> solution;
  Vector<double> allSolutions;
  uint rank;
  double fit;
  double stddev;
  double mu;
  Matrix<double> covar;
  Vector<double> errors;
  std::vector<Result::Ref> child_results(numChildren());
  // normalize the request ID to make sure iteration counters, etc,
  // are part of it
  HIID rqid = makeNormalRequestId(request.id());
  // Copy the request and attach the solvable parm specification if needed.
  Request::Ref reqref;
  Request & newReq = reqref <<= new Request(request.cells(),calcDeriv,rqid);
  newReq[FRider] <<= new DataRecord;
  if( itsSpids.empty()  &&  state()[FSolvable].exists() )
  {
    newReq[FRider][itsParmGroup] <<= wstate()[FSolvable].as_wp<DataRecord>();
    newReq.validateRider();
  } else {
    newReq[FRider][itsParmGroup] <<= new DataRecord;
  }
  // Iterate as many times as needed.
  int step;
  for (step=0; step<itsNumStep; step++) 
  {
    // collect child results, using Node's standard method
    int retcode = Node::pollChildren (child_results, resref, newReq);
    // a fail or a wait is returned immediately
    if( retcode&(RES_FAIL|RES_WAIT) )
      return retcode;
    // else process 
    vector<VellSet*> chvellsets;
    chvellsets.reserve(numChildren() * child_results[0]->numVellSets());
    // Find the set of all spids from all condeq results.
    for (uint i=0; i<child_results.size(); i++)
      if( itsIsCondeq[i] )
      {
        for (int iplane=0; iplane<child_results[i]->numVellSets(); iplane++) 
        {
          if (! child_results[i]().vellSet(iplane).isFail()) 
          {
            chvellsets.push_back (&(child_results[i]().vellSet(iplane)));
          }
        }
      }
    spids = Function::findSpids (chvellsets);
    nspid = spids.size();
    // It first time, initialize the solver.
    // Otherwise check if spids are still the same.
    if (itsSpids.empty()) {
      AssertStr (nspid > 0,
		 "No solvable parameters found in solver " << name());
      itsSolver.set (nspid, 1u, 0u);
      itsSpids = spids;
    } else {
      AssertStr (itsSpids == spids,
		 "Different spids while solver is not restarted");
    }
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
    {
      FitLSQ tmpSolver = itsSolver;
      tmpSolver.getCovariance (covar);
      tmpSolver.getErrors (errors);
    }
    // Make a copy of the solver for the actual solve.
    // This is needed because the solver does in-place transformations.
    FitLSQ solver = itsSolver;
    bool solFlag = solver.solveLoop (fit, rank, solution,
				     stddev, mu, itsUseSVD);
    cdebug(4) << "Solution after:  " << solution << endl;
    // Put the statistics in a record the result.
    DataRecord& solrec = metricsRec[step] <<= new DataRecord;
    solrec[FRank] = int(rank);
    solrec[FFit] = fit;
    //  solrec[FErrors] = errors;
    //  solrec[FCoVar ] = covar; 
    solrec[FFlag] = solFlag; 
    solrec[FMu] = mu;
    solrec[FStdDev] = stddev;
    //  solrec[FChi   ] = itsSolver.getChi());
    
    // Put the solution in the rider:
    //    [FRider][<parm_group>][CommandByNodeIndex][<parmid>]
    // will contain a DataRecord for each parm 
    DataRecord& dr1 = newReq[FRider][itsParmGroup].replace() <<= new DataRecord;
    fillSolution(dr1[FCommandByNodeIndex] <<= new DataRecord, 
                 spids,solution,false);
    newReq.validateRider();
//    // Lock all parm tables used.
//    ParmTable::lockTables();
    // update request ID
    newReq.setId(nextIterationId(rqid));
    // Unlock all parm tables used.
    ParmTable::unlockTables();
  }
  // Put the spids in the result.
  vellset.setSpids(spids);
  // Distribute the last solution.
  // Do that in an empty request.
  Request & lastReq = reqref <<= new Request;
  lastReq.setId(rqid);
  lastReq[FRider] <<= new DataRecord;
  DataRecord &dr1 = lastReq[FRider][itsParmGroup] <<= new DataRecord;
  fillSolution(dr1[FCommandByNodeIndex] <<= new DataRecord, 
               spids,solution,true);
  // Lock all parm tables used.
  ParmTable::lockTables();
  // Update the parms.
  lastReq.validateRider();
  Node::pollChildren (child_results, resref, lastReq);
  // Unlock all parm tables used.
  ParmTable::unlockTables();
  // result depends on domain, and has -- most likely -- been updated
  double* sol = vellset.setReal(nspid, step).data();
  memcpy (sol, allSolutions.data(), nspid*step*sizeof(double));
  return RES_DEP_DOMAIN|RES_UPDATED;
}

//##ModelId=400E53550276
void Solver::fillSolution (DataRecord& rec, const vector<int>& spids,
                           const Vector<double>& solution, bool save_polc)
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
      drp[FUpdateValues] = parmSol;
      lastParmid = spids[i] / 256;
      parmSol.resize(0);
      if( save_polc )
        drp[FSavePolcs] = true;
    }
    parmSol.push_back (solution[i]);
  }
  DataRecord& drp = rec[lastParmid] <<= new DataRecord;
  drp[FUpdateValues] = parmSol;
  if( save_polc )
    drp[FSavePolcs] = true;
}

//##ModelId=400E53550267
void Solver::setStateImpl (DataRecord& newst,bool initializing)
{
  Node::setStateImpl(newst,initializing);
  getStateField(itsNumStep,newst,FNumSteps);
  getStateField(itsEpsilon,newst,FEpsilon);
  getStateField(itsUseSVD,newst,FUseSVD);
  getStateField(itsParmGroup,newst,FParmGroup);
}


} // namespace Meq

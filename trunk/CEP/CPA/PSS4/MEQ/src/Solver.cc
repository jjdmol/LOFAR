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

InitDebugContext(Solver,"MeqSolver");

//##ModelId=400E53550260
Solver::Solver()
: itsSolver          (1, LSQBase::REAL),
  itsNrEquations     (0),
  itsDefNumIter      (1),
  itsDefEpsilon      (1e-8),
  itsDefUseSVD       (true),
  itsDefClearMatrix  (true),
  itsDefInvertMatrix (true),
  itsDefSavePolcs    (true),
  itsParmGroup       (AidParm),
  itsSolveDependMask (RQIDM_VALUE)
{
  resetCur();
  // Set this flag, so setCurState will be called in first getResult.
  itsResetCur = true;
}

//##ModelId=400E53550261
Solver::~Solver()
{}

//##ModelId=400E53550263
TypeId Solver::objectType() const
{
  return TpMeqSolver;
}

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

// Process rider for the given request
// (this will be called prior to getResult() on the same request)
void Solver::processCommands (const DataRecord &rec,const Request &request)
{
  Node::processCommands(rec,request); // required
  // Get new current values (use default if not given).
  itsCurNumIter   = rec[FNumIter].as<int>(itsDefNumIter);
  if (itsCurNumIter < 1) itsCurNumIter = 1;
  itsCurEpsilon   = rec[FEpsilon].as<double>(itsDefEpsilon);
  itsCurUseSVD    = rec[FUseSVD].as<bool>(itsDefUseSVD);
  itsCurSavePolcs = rec[FSavePolcs].as<bool>(itsDefSavePolcs);
  bool clearGiven  = rec[FClearMatrix].get(itsCurClearMatrix);
  bool invertGiven = rec[FInvertMatrix].get(itsCurInvertMatrix);
  // Take care that these current values are used in getResult (if called).
  itsResetCur = false;
  cdebug(1)<<"Solver rider: "
           <<itsCurNumIter<<','
           <<itsCurEpsilon<<','
           <<itsCurUseSVD<<','
           <<clearGiven<<':'<<itsCurClearMatrix<<','
           <<invertGiven<<':'<<itsCurInvertMatrix<<','
           <<itsCurSavePolcs<<endl;
  // Update wstate.
  setCurState();
  // getResult won't be called if the request has no cells.
  // So process here if clear or invert has to be done.
  if (! request.hasCells()) {
    // Remove all spids if the matrix has to be cleared.
    if (clearGiven && itsCurClearMatrix) {
      itsSpids.clear();
    } else if (invertGiven && itsCurInvertMatrix) {
      Vector<double> solution(itsSpids.size());
      DataRecord::Ref solRef;
      DataRecord& solRec = solRef <<= new DataRecord;
      std::vector<Result::Ref> child_results;
      Result::Ref resref;
      solve (solution, request, solRec, resref, child_results,
             false, true);
    }
    // Take care that current gets reset to default if no
    // processCommands is called for the next getResult.
    itsResetCur = true;
  }
}

// Get the result for the given request.
//##ModelId=400E53550270
int Solver::getResult (Result::Ref &resref, 
                       const std::vector<Result::Ref> &,
                       const Request &request, bool newreq)
{
  // Reset current values if needed.
  // That is possible if a getResult is done without a processCommands
  // (thus without a rider in the request).
  if (itsResetCur) {
    resetCur();
    setCurState();
  }
  // Use 1 derivative by default, or 2 if specified in request
  int calcDeriv = std::max(request.calcDeriv(),1);
  // The result has 1 plane.
  Result& result = resref <<= new Result(request, 1);
  VellSet& vellset = result.setNewVellSet(0);
  DataRecord& metricsRec = result[FMetrics] <<= new DataRecord;
  // Allocate variables needed for the solution.
  uint nspid;
  vector<int> spids;
  Vector<double> solution;
  Vector<double> allSolutions;
  std::vector<Result::Ref> child_results(numChildren());
  // get the request ID -- we're going to be incrementing the 
  // itsSolveDependMask part of it
  HIID rqid = request.id();
  // Copy the request and attach the solvable parm specification if needed.
  Request::Ref reqref;
  Request & newReq = reqref <<= new Request(request.cells(),calcDeriv,rqid);
  newReq[FRider] <<= new DataRecord;
  if( state()[FSolvable].exists() ) {
    newReq[FRider][itsParmGroup] <<= wstate()[FSolvable].as_wp<DataRecord>();
    newReq.validateRider();
  } else {
    newReq[FRider][itsParmGroup] <<= new DataRecord;
  }
  // Take care that the matrix is cleared if needed.
  if (itsCurClearMatrix) {
      itsSpids.clear();
  }
  // Iterate as many times as needed.
  int step;
  for (step=0; step<itsCurNumIter; step++) 
  {
    // collect child results, using Node's standard method
    int retcode = Node::pollChildren (child_results, resref, newReq);
    // a fail or a wait is returned immediately
    if( retcode&(RES_FAIL|RES_WAIT) )
      return retcode;
    // else process 
    vector<const VellSet*> chvellsets;
    chvellsets.reserve(numChildren() * child_results[0]->numVellSets());
    // Find the set of all spids from all condeq results.
    for (uint i=0; i<child_results.size(); i++)
      if( itsIsCondeq[i] )
      {
        for (int iplane=0; iplane<child_results[i]->numVellSets(); iplane++) 
        {
          if (! child_results[i]->vellSet(iplane).isFail()) 
          {
            chvellsets.push_back (&(child_results[i]->vellSet(iplane)));
          }
        }
      }
    spids = Function::findSpids (chvellsets);
    nspid = spids.size();
    // It first time, initialize the solver.
    // Otherwise check if spids are still the same and initialize
    // solver for the 2nd step and so.
    if (itsSpids.empty()) {
      AssertStr (nspid > 0,
                 "No solvable parameters found in solver " << name());
      itsSolver.set (nspid, 1u, 0u);
      itsNrEquations = 0;
      itsSpids = spids;
    } else {
      AssertStr (itsSpids == spids,
                 "Different spids while solver is not restarted");
      if (step > 0) {
        itsSolver.set (nspid, 1u, 0u);
        itsNrEquations = 0;
      }
    }
    // Now feed the solver with equations from the results.
    // Define the vector with derivatives (for real and imaginary part).
    vector<double> derivReal(nspid);
    vector<double> derivImag(nspid);
    // Loop through all results and fill the deriv vectors.
    for (uint i=0; i<chvellsets.size(); i++) {
      const VellSet& chresult = *chvellsets[i];
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
          itsNrEquations++;
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
          itsNrEquations++;
          val = values[j].imag();
          itsSolver.makeNorm (&derivImag[0], 1., &val);
          itsNrEquations++;
        }
      }
    }
    // Size the solutions vector.
    // Fill it with zeroes and stop if no invert will be done.
    if (step == itsCurNumIter-1  &&  !itsCurInvertMatrix) {
      if (step == 0) {
        allSolutions.resize (nspid);
        allSolutions = 0.;
      }
      break;
    }
    // Keep all solutions in a singlevector.
    allSolutions.resize ((step+1)*nspid, True);
    // The last part is the current solution.
    Vector<double> vec(allSolutions(Slice(step*nspid, nspid)));
    solution.reference (vec);
    // Solve the equation.
    DataRecord& solRec = metricsRec[step] <<= new DataRecord;
    solve (solution, newReq, solRec, resref, child_results,
           false, step==itsCurNumIter-1);
    // increment the solce-dependent parts of the request ID
    incrSubId(rqid,itsSolveDependMask);
    newReq.setId(rqid);
    // Unlock all parm tables used.
    ParmTable::unlockTables();
  }
  // Put the spids in the result.
  vellset.setSpids(spids);
  // Distribute the last solution (if there is one).
  // result depends on domain, and has -- most likely -- been updated
  double* sol = vellset.setReal(nspid, step).data();
  memcpy (sol, allSolutions.data(), nspid*step*sizeof(double));
  return 0;
}

void Solver::solve (Vector<double>& solution, const Request& request,
                    DataRecord& solRec, Result::Ref& resref,
                    std::vector<Result::Ref>& child_results,
                    bool savePolcs, bool lastIter)
{
  // Do some checks and initialize.
  int nspid = itsSpids.size();
  Assert (int(solution.nelements()) == nspid);
  AssertStr (itsNrEquations >= nspid, "Only " << itsNrEquations
             << " equations for "
             << nspid << " solvable parameters in solver " << name());
  solution = 0;
  Request::Ref reqref;
  Request* req = const_cast<Request*>(&request);
  if (lastIter) {
    // Do the last solve via an empty request.
    Request & lastReq = reqref <<= new Request;
    req = &lastReq;
    lastReq.setId(incrSubId(request.id(),itsSolveDependMask));
    lastReq[FRider] <<= new DataRecord;
  }
  // It looks as if in LSQ solveLoop and getCovariance
  // interact badly (maybe both doing an invert).
  // So make a copy to separate them.
  uint rank;
  double fit;
  double stddev;
  double mu;
  Matrix<double> covar;
  Vector<double> errors;
  {
    FitLSQ tmpSolver = itsSolver;
    tmpSolver.getCovariance (covar);
    tmpSolver.getErrors (errors);
  }
  // Make a copy of the solver for the actual solve.
  // This is needed because the solver does in-place transformations.
  FitLSQ solver = itsSolver;
  bool solFlag = solver.solveLoop (fit, rank, solution,
                                   stddev, mu, itsCurUseSVD);
  cdebug(4) << "Solution after:  " << solution << endl;
  // Put the statistics in a record the result.
  solRec[FRank] = int(rank);
  solRec[FFit] = fit;
  //  solRec[FErrors] = errors;
  //  solRec[FCoVar ] = covar; 
  solRec[FFlag] = solFlag; 
  solRec[FMu] = mu;
  solRec[FStdDev] = stddev;
  //  solRec[FChi   ] = itsSolver.getChi());
  
  // Put the solution in the rider:
  //    [FRider][<parm_group>][CommandByNodeIndex][<parmid>]
  // will contain a DataRecord for each parm 
  DataRecord& dr1 = (*req)[FRider][itsParmGroup].replace() <<= new DataRecord;
  fillSolution (dr1[FCommandByNodeIndex] <<= new DataRecord, 
                itsSpids, solution, savePolcs);
  // Lock all parm tables used.
  ParmTable::lockTables();
  // Update the parms.
  req->validateRider();
  if (lastIter) {
    Node::pollChildren (child_results, resref, *req);
  }
  // Unlock all parm tables used.
  ParmTable::unlockTables();
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
  newst[FParmGroup].get(itsParmGroup,initializing);
  DataRecord *pdef = newst[FDefault].as_wpo<DataRecord>();
  
  newst[FSolveDependMask].get(itsSolveDependMask,initializing);
  
  // if no default record at init time, create a new one
  if( !pdef && initializing )
    newst[FDefault] <<= pdef = new DataRecord; 
  if( pdef )
  {
    DataRecord &def = *pdef;
    if( def[FNumIter].get(itsDefNumIter,initializing) &&
        itsDefNumIter < 1 )
      def[FNumIter] = itsDefNumIter = 1;
    def[FClearMatrix].get(itsDefClearMatrix,initializing);
    def[FInvertMatrix].get(itsDefInvertMatrix,initializing);
    def[FSavePolcs].get(itsDefSavePolcs,initializing);
    def[FEpsilon].get(itsDefEpsilon,initializing);
    def[FUseSVD].get(itsDefUseSVD,initializing);
  }
}

void Solver::resetCur()
{
  itsCurNumIter      = itsDefNumIter;
  itsCurInvertMatrix = itsDefInvertMatrix;
  itsCurClearMatrix  = itsDefClearMatrix;
  itsCurSavePolcs    = itsDefSavePolcs;
  itsCurEpsilon      = itsDefEpsilon;
  itsCurUseSVD       = itsDefUseSVD;
  itsResetCur = false;
}

void Solver::setCurState()
{
  wstate()[FNumIter]      = itsCurNumIter;
  wstate()[FClearMatrix]  = itsCurClearMatrix;
  wstate()[FInvertMatrix] = itsCurInvertMatrix;
  wstate()[FSavePolcs]    = itsCurSavePolcs;
  wstate()[FEpsilon]      = itsCurEpsilon;
  wstate()[FUseSVD]       = itsCurUseSVD;
}


} // namespace Meq

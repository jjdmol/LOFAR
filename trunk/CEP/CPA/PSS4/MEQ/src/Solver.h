//# Solver.h: Class to solve equations
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

#ifndef MEQ_SOLVER_H
#define MEQ_SOLVER_H
    
#include <MEQ/Node.h>
#include <MEQ/VellSet.h>
#include <aips/Fitting/FitLSQ.h>


#pragma types #Meq::Solver


// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqSolver
//  Represents a solver,
//  A MeqSolver can have an arbitrary number of children.
//  Only the results from the children that are a MeqCondeq are used.
//  The other children can be other solvers that wait for this solver
//  to finish.
//field: num_step 1  
//  number of iterations to do in a solve
//field: epsilon 0
//  convergence criterium; not used at the moment
//field: usesvd false
//  Use singular value decomposition in solver?
//field: parm_group hiid('parm')
//  HIID of the parameter group to use. 
//field: solvable
//  Command record which is sent up in the rider of the first request
//  (as req.rider.<parm_group>). This is meant to set parms to solvable. 
//  The simplest way to create this is by using meq.solvable_list("names"), 
//  which returns such a record, given a  vector of solvable parm names. 
//  It is also possible to create more elaborate command records from scratch,
//  if more sophisticated manipulation of state is required.
//defrec end


namespace Meq {

class Request;


//##ModelId=400E5304008C
class Solver : public Node
{
public:
    //##ModelId=400E53550260
  Solver();

    //##ModelId=400E53550261
  virtual ~Solver();

  // Returns the class TypeId
    //##ModelId=400E53550263
  virtual TypeId objectType() const;

  // Check the children after they have been resolved in class Node.
  // The order of the children is the order as given when the Node object
  // was created.
    //##ModelId=400E53550265
  virtual void checkChildren();

// no need for now
//  virtual void init (DataRecord::Ref::Xfer& initrec, Forest* frst);

protected:
  // virtual void checkInitState (DataRecord &rec);
    //##ModelId=400E53550267
  virtual void setStateImpl (DataRecord& rec,bool initializing);
  
  // override this, since we poll children ourselves
    //##ModelId=400E5355026B
  virtual int pollChildren (std::vector<Result::Ref> &child_results,
                            Result::Ref &resref,
                            const Request &req);
  
  // Get the result for the given request.
    //##ModelId=400E53550270
  virtual int getResult (Result::Ref &resref, 
                         const std::vector<Result::Ref> &childres,
                         const Request &req,bool newreq);

private:
  // Fill the solution (per parmid) in the DataRecord.
    //##ModelId=400E53550276
  void fillSolution (DataRecord& rec, const vector<int>& spids,
		     const Vector<double>& solution,bool save_polc);

    //##ModelId=400E53550257
  int             itsNumCondeqs;
  std::vector<bool> itsIsCondeq;
  
    //##ModelId=400E5355025A
  FitLSQ          itsSolver;
    //##ModelId=400E5355025C
  int             itsNumStep;
    //##ModelId=400E5355025D
  double          itsEpsilon;
    //##ModelId=400E5355025F
  bool            itsUseSVD;
  vector<int>     itsSpids;
  
  // solvable parm group for this solver ("Parm" by default)
  HIID            itsParmGroup;
};


} // namespace Meq

#endif

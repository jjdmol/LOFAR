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


namespace Meq {

class Request;


class Solver : public Node
{
public:
  Solver();

  virtual ~Solver();

  // Returns the class TypeId
  virtual TypeId objectType() const;

  // Check the children after they have been resolved in class Node.
  // The order of the children is the order as given when the Node object
  // was created.
  virtual void checkChildren();

// no need for now
//  virtual void init (DataRecord::Ref::Xfer& initrec, Forest* frst);

protected:
  // virtual void checkInitState (DataRecord &rec);
  virtual void setStateImpl (DataRecord& rec,bool initializing);
  
  // override this, since we poll children ourselves
  virtual int pollChildren (std::vector<Result::Ref> &child_results,
                            Result::Ref &resref,
                            const Request &req);
  
  // Get the result for the given request.
  virtual int getResult (Result::Ref &resref, 
                         const std::vector<Result::Ref> &childres,
                         const Request &req,bool newreq);

private:
  // Fill the solution (per parmid) in the DataRecord.
  void fillSolution (DataRecord& rec, const vector<int> spids,
		     const Vector<double>& solution);

  vector<Condeq*> itsCondeqs;
  DataRecord::Ref itsSolvableParms;
  FitLSQ          itsSolver;
  int             itsNumStep;
  double          itsEpsilon;
  bool            itsUseSVD;
};


} // namespace Meq

#endif

//# MeqExpr.h: The base class of an expression.
//#
//# Copyright (C) 2002
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

#if !defined(MNS_MEQEXPR_H)
#define MNS_MEQEXPR_H

// \file MNS/MeqExpr.h
// The base class of an expression

//# Includes
#include <Common/lofar_vector.h>
#include <BBS3/MNS/MeqRequestId.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqResultVec.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{


// This class is the (abstract) base class for an expression.

class MeqExprRep
{
public:
  // The default constructor.
  MeqExprRep()
    : itsCount   (0),
      itsNParents(0),
      itsResult  (0),
      itsResVec  (0),
      itsReqId   (InitMeqRequestId)
    {}

  virtual ~MeqExprRep();

  // Link to this node (incrementing the refcount).
  void link()
    { itsCount++; }

  // Link to this node (decrementing the refcount).
  // The object is deleted if the refcount gets zero.
  static void unlink (MeqExprRep* rep)
    { if (rep != 0  &&  --rep->itsCount == 0) delete rep; }

  // Increment nr of parents.
  void incrNParents()
    { itsNParents++; }

  // Get the single result of the expression for the given domain.
  // The return value is a reference to the true result. This can either
  // be a reference to the cached value (if the object maintains a cache)
  // or to the result object in the parameter list (if no cache).
  // If the result is stored in the cache, it is done in a thread-safe way.
  // Note that a cache is used if the expression has multiple parents.
  const MeqResult& getResultSynced (const MeqRequest& request,
				    MeqResult& result)
    { return itsReqId == request.getId()  ?
	*itsResult : calcResult(request,result); }

  // Get the actual result.
  // The default implementation throw an exception.
  virtual MeqResult getResult (const MeqRequest&);

  // Get the multi result of the expression for the given domain.
  // The default implementation calls getResult.
  const MeqResultVec& getResultVecSynced (const MeqRequest& request,
					  MeqResultVec& result)
    { return itsReqId == request.getId()  ?
	*itsResVec : calcResultVec(request,result); }
  virtual MeqResultVec getResultVec (const MeqRequest&);
  // </group>

private:
  // Forbid copy and assignment.
  MeqExprRep (const MeqExprRep&);
  MeqExprRep& operator= (const MeqExprRep&);

  // Calculate the actual result in a cache thread-safe way.
  // <group>
  const MeqResult& calcResult (const MeqRequest&, MeqResult&);
  const MeqResultVec& calcResultVec (const MeqRequest&, MeqResultVec&);
  // </group>

  int           itsCount;      //# Reference count
  int           itsNParents;   //# Nr of parents
  MeqResult*    itsResult;     //# Possibly cached result
  MeqResultVec* itsResVec;     //# Possibly cached result vector
  MeqRequestId  itsReqId;      //# Request-id of cached result.
};



class MeqExpr
{
public:
  // Construct from a rep object.
  // It takes over the pointer, so it takes care of deleting the object.
  MeqExpr (MeqExprRep* expr = 0)
    : itsRep(expr)
    { if (itsRep) itsRep->link(); }

  // Copy constructor (reference semantics).
  MeqExpr (const MeqExpr&);

  ~MeqExpr()
    { MeqExprRep::unlink (itsRep); }

  // Assignment (reference semantics).
  MeqExpr& operator= (const MeqExpr&);

  // Increment nr of parents.
  void incrNParents()
    { itsRep->incrNParents(); }

  // Get the result of the expression for the given domain.
  // getResult will throw an exception if the node has a multi result.
  // getResultVec will always succeed.
  // <group>
  const MeqResult getResult (const MeqRequest& request)
    { return itsRep->getResult (request); }
  const MeqResult& getResultSynced (const MeqRequest& request,
				    MeqResult& result)
    { return itsRep->getResultSynced (request, result); }
  const MeqResultVec getResultVec (const MeqRequest& request)
    { return itsRep->getResultVec (request); }
  const MeqResultVec& getResultVecSynced (const MeqRequest& request,
					  MeqResultVec& result)
    { return itsRep->getResultVecSynced (request, result); }
  // </group>

private:
  MeqExprRep* itsRep;
};



class MeqExprToComplex: public MeqExprRep
{
public:
  MeqExprToComplex (MeqExpr real, MeqExpr imag);

  virtual ~MeqExprToComplex();

  virtual MeqResult getResult (const MeqRequest&);

private:
  MeqExpr itsReal;
  MeqExpr itsImag;
};



class MeqExprAPToComplex: public MeqExprRep
{
public:
  MeqExprAPToComplex (MeqExpr ampl, MeqExpr phase);

  virtual ~MeqExprAPToComplex();

  virtual MeqResult getResult (const MeqRequest&);

  MeqExpr itsAmpl;
  MeqExpr itsPhase;
};

// @}

}

#endif

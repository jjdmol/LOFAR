//# MeqJonesExpr.h: The base class of a Jones matrix expression.
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

#if !defined(MNS_MEQJONESEXPR_H)
#define MNS_MEQJONESEXPR_H

// \file MNS/MeqJonesExpr.h
// The base class of a Jones matrix expression.

//# Includes
#include <BBS3/MNS/MeqJonesResult.h>
#include <BBS3/MNS/MeqRequest.h>


namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

// This class is the (abstract) base class for a Jones expression.

class MeqJonesExprRep
{
public:
  // The default constructor.
  MeqJonesExprRep()
    : itsCount   (0),
      itsNParents(0),
      itsResult  (0),
      itsReqId   (InitMeqRequestId)
    {}

  virtual ~MeqJonesExprRep();

  void link()
    { itsCount++; }

  static void unlink (MeqJonesExprRep* rep)
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
  const MeqJonesResult& getResultSynced (const MeqRequest& request,
					 MeqJonesResult& result)
    { return itsReqId == request.getId()  ?
	*itsResult : calcResult(request,result); }

  // Get the actual result.
  // The default implementation throw an exception.
  virtual MeqJonesResult getResult (const MeqRequest&) = 0;

private:
  // Forbid copy and assignment.
  MeqJonesExprRep (const MeqJonesExprRep&);
  MeqJonesExprRep& operator= (const MeqJonesExprRep&);

  // Calculate the actual result in a cache thread-safe way.
  const MeqJonesResult& calcResult (const MeqRequest&, MeqJonesResult&);

  int             itsCount;
  int             itsNParents;   //# Nr of parents
  MeqJonesResult* itsResult;     //# Possibly cached result
  MeqRequestId    itsReqId;      //# Request-id of cached result.
};



class MeqJonesExpr
{
public:
  // The default constructor.
  // It takes over the pointer, so it takes care of deleting the object.
  MeqJonesExpr (MeqJonesExprRep* expr = 0)
    { itsRep = expr; if (itsRep) itsRep->link(); }

  MeqJonesExpr (const MeqJonesExpr&);

  ~MeqJonesExpr()
    { MeqJonesExprRep::unlink (itsRep); }

  MeqJonesExpr& operator= (const MeqJonesExpr&);

  // Increment nr of parents.
  void incrNParents()
    { itsRep->incrNParents(); }

  // Get the result of the expression for the given domain.
  // <group>
  MeqJonesResult getResult (const MeqRequest& request)
    { return itsRep->getResult (request); }
  const MeqJonesResult& getResultSynced (const MeqRequest& request,
					MeqJonesResult& result)
    { return itsRep->getResultSynced (request, result); }
  // </group>

private:
  MeqJonesExprRep* itsRep;
};


// @}

}

#endif

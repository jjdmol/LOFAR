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
#include <BBS3/MNS/MeqExpr.h>
#include <BBS3/MNS/MeqJonesResult.h>
#include <BBS3/MNS/MeqRequest.h>
#include <Common/LofarLogger.h>


namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

// This class is the (abstract) base class for a Jones expression.

class MeqJonesExprRep : public MeqExprRep
{
public:
  // The default constructor.
  MeqJonesExprRep()
    : itsResult (0)
    {}

  virtual ~MeqJonesExprRep();

  // Get the single result of the expression for the given domain.
  // The return value is a reference to the true result. This can either
  // be a reference to the cached value (if the object maintains a cache)
  // or to the result object in the parameter list (if no cache).
  // If the result is stored in the cache, it is done in a thread-safe way.
  // Note that a cache is used if the expression has multiple parents.
  const MeqJonesResult& getJResultSynced (const MeqRequest& request,
					  MeqJonesResult& result)
    { return itsReqId == request.getId()  ?
	*itsResult : getJResult2(request, result); }

  // Get the actual result.
  // The default implementation throw an exception.
  virtual MeqJonesResult getJResult (const MeqRequest&) = 0;

  // Precalculate the result and store it in the cache.
  virtual void precalculate (const MeqRequest&);

private:
  // Helper function adding an extra check.
  const MeqJonesResult& getJResult2 (const MeqRequest& request,
				     MeqJonesResult& result)
    { DBGASSERT(itsNParents<=1); return result = getJResult(request); }

  // Forbid copy and assignment.
  MeqJonesExprRep (const MeqJonesExprRep&);
  MeqJonesExprRep& operator= (const MeqJonesExprRep&);


  MeqJonesResult* itsResult;     //# Possibly cached result
};



class MeqJonesExpr : public MeqExpr
{
public:
  // The default constructor.
  // It takes over the pointer, so it takes care of deleting the object.
  MeqJonesExpr (MeqJonesExprRep* expr = 0)
    : MeqExpr (expr),
      itsRep  (expr)
    {}

  MeqJonesExpr (const MeqJonesExpr& that)
    : MeqExpr (that),
      itsRep  (that.itsRep)
    {}

  ~MeqJonesExpr()
    {}

  MeqJonesExpr& operator= (const MeqJonesExpr& that)
  {
    MeqExpr::operator= (that);
    itsRep = that.itsRep;
    return *this;
  }

  // Get the result of the expression for the given domain.
  // <group>
  MeqJonesResult getResult (const MeqRequest& request)
    { return itsRep->getJResult (request); }
  const MeqJonesResult& getResultSynced (const MeqRequest& request,
					 MeqJonesResult& result)
    { return itsRep->getJResultSynced (request, result); }
  // </group>

private:
  MeqJonesExprRep* itsRep;
};


// @}

}

#endif

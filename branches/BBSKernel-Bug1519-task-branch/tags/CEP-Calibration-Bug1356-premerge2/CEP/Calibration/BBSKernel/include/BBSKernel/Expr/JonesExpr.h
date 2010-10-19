//# JonesExpr.h: The base class of a Jones matrix expression.
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

#if !defined(EXPR_JONESEXPR_H)
#define LOFAR_BBSKERNEL_EXPR_JONESEXPR_H

// \file
// The base class of a Jones matrix expression.

//# Includes
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/Request.h>
#include <Common/LofarLogger.h>


namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// This class is the (abstract) base class for a Jones expression.

class JonesExprRep : public ExprRep
{
public:
  // The default constructor.
  JonesExprRep()
    : itsResult (0)
    {}

  virtual ~JonesExprRep();

  // Get the single result of the expression for the given domain.
  // The return value is a reference to the true result. This can either
  // be a reference to the cached value (if the object maintains a cache)
  // or to the result object in the parameter list (if no cache).
  // If the result is stored in the cache, it is done in a thread-safe way.
  // Note that a cache is used if the expression has multiple parents.
  const JonesResult& getJResultSynced (const Request& request,
					  JonesResult& result)
    { return itsReqId == request.getId()  ?
	*itsResult : getJResult2(request, result); }

  // Get the actual result.
  // The default implementation throw an exception.
  virtual JonesResult getJResult (const Request&) = 0;

  // Precalculate the result and store it in the cache.
  virtual void precalculate (const Request&);

private:
  // Helper function adding an extra check.
  const JonesResult& getJResult2 (const Request& request,
				     JonesResult& result)
    ///#    { DBGASSERT(itsNParents<=1); return result = getJResult(request); }
    { return result = getJResult(request); }

  // Forbid copy and assignment.
  JonesExprRep (const JonesExprRep&);
  JonesExprRep& operator= (const JonesExprRep&);


  JonesResult* itsResult;     //# Possibly cached result
};



class JonesExpr : public Expr
{
public:
  // The default constructor.
  // It takes over the pointer, so it takes care of deleting the object.
  JonesExpr (JonesExprRep* expr = 0)
    : Expr (expr),
      itsRep  (expr)
    {}

  JonesExpr (const JonesExpr& that)
    : Expr (that),
      itsRep  (that.itsRep)
    {}

  ~JonesExpr()
    {}

  JonesExpr& operator= (const JonesExpr& that)
  {
    Expr::operator= (that);
    itsRep = that.itsRep;
    return *this;
  }

  // Get the result of the expression for the given domain.
  // <group>
  JonesResult getResult (const Request& request)
    { return itsRep->getJResult (request); }
  const JonesResult& getResultSynced (const Request& request,
					 JonesResult& result)
    { return itsRep->getJResultSynced (request, result); }
  // </group>

private:
  JonesExprRep* itsRep;
};


// @}

} // namespace BBS
} // namespace LOFAR

#endif

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
    : itsCount(0), itsLastReqId(InitMeqRequestId)
    {};

  virtual ~MeqJonesExprRep();

  void link()
    { itsCount++; }

  static void unlink (MeqJonesExprRep* rep)
    { if (rep != 0  &&  --rep->itsCount == 0) delete rep; }

  // Calculate the result of its members.
  virtual MeqJonesResult getResult (const MeqRequest&) = 0;

private:
  // Forbid copy and assignment.
  MeqJonesExprRep (const MeqJonesExprRep&);
  MeqJonesExprRep& operator= (const MeqJonesExprRep&);

  int          itsCount;
  MeqRequestId itsLastReqId;
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

  // Get the result of the expression for the given domain.
  MeqJonesResult getResult (const MeqRequest& request)
    { return itsRep->getResult (request); }

private:
  MeqJonesExprRep* itsRep;
};


// @}

}

#endif

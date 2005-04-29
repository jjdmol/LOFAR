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
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqResultVec.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward declarations
class MeqRequest;


// This class is the (abstract) base class for an expression.

class MeqExprRep
{
public:
  // The default constructor.
  MeqExprRep()
    : itsCount(0)
    {}

  virtual ~MeqExprRep();

  void link()
    { itsCount++; }

  static void unlink (MeqExprRep* rep)
    { if (rep != 0  &&  --rep->itsCount == 0) delete rep; }

  // Get the single result of the expression for the given domain.
  // The default implementation throw an exception.
  virtual MeqResult getResult (const MeqRequest&);

  // Get the multi result of the expression for the given domain.
  // The default implementation calls getResult.
  virtual MeqResultVec getResultVec (const MeqRequest& request);
  // </group>

private:
  // Forbid copy and assignment.
  MeqExprRep (const MeqExprRep&);
  MeqExprRep& operator= (const MeqExprRep&);

  int itsCount;
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

  // Get the result of the expression for the given domain.
  // getResult will throw an exception if the node has a multi result.
  // getResultVec will always succeed.
  // <group>
  MeqResult getResult (const MeqRequest& request)
    { return itsRep->getResult (request); }
  MeqResultVec getResultVec (const MeqRequest& request)
    { return itsRep->getResultVec (request); }
  // </group>

private:
  MeqExprRep* itsRep;
};



class MeqExprToComplex: public MeqExprRep
{
public:
  MeqExprToComplex (MeqExpr real, MeqExpr imag)
    : itsReal(real), itsImag(imag) {;}

  virtual ~MeqExprToComplex();

  virtual MeqResult getResult (const MeqRequest&);

private:
  MeqExpr itsReal;
  MeqExpr itsImag;
};



class MeqExprAPToComplex: public MeqExprRep
{
public:
  MeqExprAPToComplex (MeqExpr ampl, MeqExpr phase)
    : itsAmpl(ampl), itsPhase(phase) {;}

  virtual ~MeqExprAPToComplex();

  virtual MeqResult getResult (const MeqRequest&);

private:
  MeqExpr itsAmpl;
  MeqExpr itsPhase;
};

// @}

}

#endif

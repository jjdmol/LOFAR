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

//# Includes
#include <Common/lofar_vector.h>

namespace LOFAR {

//# Forward declarations
class MeqRequest;
class MeqResult;
class MeqMatrix;


// This class is the (abstract) base class for an expression.

class MeqExpr
{
public:
  // The default constructor.
  MeqExpr()
    {};

  virtual ~MeqExpr();

  // Get the result of the expression for the given domain.
  virtual MeqResult getResult (const MeqRequest&) = 0;

private:
};



class MeqExprToComplex: public MeqExpr
{
public:
  MeqExprToComplex (MeqExpr* real, MeqExpr* imag)
    : itsReal(real), itsImag(imag) {;}

  virtual ~MeqExprToComplex();

  virtual MeqResult getResult (const MeqRequest&);

private:
  MeqExpr* itsReal;
  MeqExpr* itsImag;
};



class MeqExprAPToComplex: public MeqExpr
{
public:
  MeqExprAPToComplex (MeqExpr* ampl, MeqExpr* phase)
    : itsAmpl(ampl), itsPhase(phase) {;}

  virtual ~MeqExprAPToComplex();

  virtual MeqResult getResult (const MeqRequest&);

private:
  MeqExpr* itsAmpl;
  MeqExpr* itsPhase;
};

}

#endif

//# TFExprPoly.h: The class of a polynomial expression.
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

#if !defined(MNS_TFEXPRPOLY_H)
#define MNS_TFEXPRPOLY_H

//# Includes
#include <MNS/TFExpr.h>
#include <Common/lofar_vector.h>


// This class is the class for a polynomial expression.

class TFExprPoly
{
public:
  // The expressions give the coefficients of the 2-dim polynomial.
  // nx and ny give the number of coefficients in x and y.
  TFExprPoly (const vector<TFExpr*>& coeff, int nx, int ny);

  virtual ~TFExprPoly();

  // Get the range of the expression for the given domain.
  virtual TFRange getRange (const TFRequest&);

private:
  vector<TFExpr*> itsCoeff;
  int             itsNx;
  int             itsNy;
};


#endif

//# TFExprPoly.cc: The class of a polynomial expression.
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

#include <MNS/TFExprPoly.h>
#include <MNS/TFRequest.h>
#include <MNS/TFDomain.h>
#include <MNS/TFRange.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>

TFExprPoly::TFExprPoly (const vector<TFExpr*>& coeff, int nx, int ny)
: itsCoeff (coeff),
  itsNx    (nx),
  itsNy    (ny)
{
  Assert (int(coeff.size()) == nx*ny);
}

TFExprPoly::~TFExprPoly()
{}

TFRange TFExprPoly::getRange (const TFRequest& request)
{
  const TFDomain& domain = request.domain();
  int ndx = domain.nx();
  int ndy = domain.ny();
  vector<TFRange> coeffRange(itsNx*itsNy);
  vector<double> coeff(itsNx*itsNy);
  // Evaluate the coefficients.
  for (int i=0; i<itsNx*itsNy; i++) {
    coeffRange[i] = itsCoeff[i]->getRange(request);
    coeff[i] = coeffRange[i].getValue().getDouble();
  }
  // Evaluate the expression.
  TFRange range(request.nspid());
  Matrix<double> result(ndx, ndy);
  for (int j=0; j<ndy; j++) {
    for (int i=0; i<ndx; i++) {
      double tmp = 0;
      const MnsMatrix& xterm = request.getCrossTerms (i, j);
      int inx = 0;
      for (int cy=0; cy<itsNy; cy++) {
	for (int cx=0; cx<itsNx; cx++) {
	  tmp += coeff[inx++] * xterm.getDouble(cx,cy);
	}
      }
      result(i,j) = tmp;
    }
  }
  range.setValue (result);
  // Evaluate (if needed) for the perturbed parameter values.
  vector<MnsMatrix*> valptr(coeff.size());
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval = false;
    double perturbation = 0;
    for (int i=0; i<int(coeff.size()); i++) {
      if (coeffRange[i].isDefined(spinx)) {
	eval = true;
	perturbation = coeffRange[i].getPerturbation(spinx);
	break;
      }
    }
    if (eval) {
      for (int i=0; i<itsNx*itsNy; i++) {
	coeff[i] = coeffRange[i].getPerturbedValue(spinx).getDouble();
      }
      Matrix<double> pert(ndx, ndy);
      for (int j=0; j<ndy; j++) {
	for (int i=0; i<ndx; i++) {
	  double tmp = 0;
	  const MnsMatrix& xterm = request.getCrossTerms (i, j);
	  int inx = 0;
	  for (int cy=0; cy<itsNy; cy++) {
	    for (int cx=0; cx<itsNx; cx++) {
	      tmp += coeff[inx++] * xterm.getDouble(cx,cy);
	    }
	  }
	  pert(i,j) = tmp;
	}
      }
      range.setPerturbedValue (spinx, pert);
      range.setPerturbation (spinx, perturbation);
    }
  }
  return range;
}

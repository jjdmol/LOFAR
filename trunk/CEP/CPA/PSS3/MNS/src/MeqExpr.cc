//# MeqExpr.cc: The base class of an expression.
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

#include <MNS/MeqExpr.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqRequest.h>

MeqExpr::~MeqExpr()
{}




MeqExprToComplex::~MeqExprToComplex()
{}

MeqResult MeqExprToComplex::getResult (const MeqRequest& request)
{
  MeqResult real = itsReal->getResult (request);
  MeqResult imag = itsImag->getResult (request);
  MeqResult result(request.nspid());
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    if (real.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
				tocomplex(real.getPerturbedValue(spinx),
					  imag.getPerturbedValue(spinx)));
      result.setPerturbation (spinx, real.getPerturbation(spinx));
    } else if (imag.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
				tocomplex(real.getPerturbedValue(spinx),
					  imag.getPerturbedValue(spinx)));
      result.setPerturbation (spinx, imag.getPerturbation(spinx));
    }
  }
  result.setValue (tocomplex(real.getValue(), imag.getValue()));
  return result;
}

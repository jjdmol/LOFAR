//# JonesMul3.cc: Calculate left*mid*right
//#
//# Copyright (C) 2005
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

#include <lofar_config.h>

#include <BBSKernel/Expr/JonesMul3.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

JonesMul3::JonesMul3 (const JonesExpr& left,
			    const JonesExpr& mid,
			    const JonesExpr& right)
: itsLeft  (left),
  itsMid   (mid),
  itsRight (right)
{
  addChild (itsLeft);
  addChild (itsMid);
  addChild (itsRight);
}

JonesMul3::~JonesMul3()
{}

JonesResult JonesMul3::getJResult (const Request& request)
{
  // Create the result object.
  JonesResult result(request.nspid());
  Result& result11 = result.result11();
  Result& result12 = result.result12();
  Result& result21 = result.result21();
  Result& result22 = result.result22();
  // Calculate the children.
  JonesResult leftBuf, midBuf, rightBuf;
  const JonesResult& left  = itsLeft.getResultSynced (request, leftBuf);
  const JonesResult& mid   = itsMid.getResultSynced (request, midBuf);
  const JonesResult& right = itsRight.getResultSynced (request, rightBuf);
  const Result& l11 = left.getResult11();
  const Result& l12 = left.getResult12();
  const Result& l21 = left.getResult21();
  const Result& l22 = left.getResult22();
  const Result& m11 = mid.getResult11();
  const Result& m12 = mid.getResult12();
  const Result& m21 = mid.getResult21();
  const Result& m22 = mid.getResult22();
  const Result& r11 = right.getResult11();
  const Result& r12 = right.getResult12();
  const Result& r21 = right.getResult21();
  const Result& r22 = right.getResult22();
  const Matrix& ml11 = l11.getValue();
  const Matrix& ml12 = l12.getValue();
  const Matrix& ml21 = l21.getValue();
  const Matrix& ml22 = l22.getValue();
  const Matrix& mm11 = m11.getValue();
  const Matrix& mm12 = m12.getValue();
  const Matrix& mm21 = m21.getValue();
  const Matrix& mm22 = m22.getValue();
  const Matrix& mr11 = r11.getValue();
  const Matrix& mr12 = r12.getValue();
  const Matrix& mr21 = r21.getValue();
  const Matrix& mr22 = r22.getValue();
  Matrix t11(ml11*mm11 + ml12*mm21);
  Matrix t12(ml11*mm12 + ml12*mm22);
  Matrix t21(ml21*mm11 + ml22*mm21);
  Matrix t22(ml21*mm12 + ml22*mm22);
  result11.setValue (t11*mr11 + t12*mr21);
  result12.setValue (t11*mr12 + t12*mr22);
  result21.setValue (t21*mr11 + t22*mr21);
  result22.setValue (t21*mr12 + t22*mr22);

  // Determine which values are perturbed and determine the perturbation.
  const ParmFunklet* perturbedParm;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval11 = false;
    bool eval12 = false;
    bool eval21 = false;
    bool eval22 = false;
    if (m11.isDefined(spinx)) {
      eval11 = true;
      perturbedParm = m11.getPerturbedParm(spinx);
    } else if (m12.isDefined(spinx)) {
      eval11 = true;
      perturbedParm = m12.getPerturbedParm(spinx);
    } else if (m21.isDefined(spinx)) {
      eval11 = true;
      perturbedParm = m21.getPerturbedParm(spinx);
    } else if (m22.isDefined(spinx)) {
      eval11 = true;
      perturbedParm = m22.getPerturbedParm(spinx);
    }
    if (eval11) {
      eval12 = eval21 = eval22 = true;
    } else {
      if (l11.isDefined(spinx)) {
	perturbedParm = l11.getPerturbedParm(spinx);
	eval11 = true;
	eval12 = true;
      } else if (l12.isDefined(spinx)) {
	perturbedParm = l12.getPerturbedParm(spinx);
	eval11 = true;
	eval12 = true;
      }
      if (l21.isDefined(spinx)) {
	perturbedParm = l21.getPerturbedParm(spinx);
	eval21 = true;
	eval22 = true;
      } else if (l22.isDefined(spinx)) {
	perturbedParm = l22.getPerturbedParm(spinx);
	eval21 = true;
	eval22 = true;
      }
      if (r11.isDefined(spinx)) {
	perturbedParm = r11.getPerturbedParm(spinx);
	eval11 = true;
	eval21 = true;
      } else if (r21.isDefined(spinx)) {
	perturbedParm = r21.getPerturbedParm(spinx);
	eval11 = true;
	eval21 = true;
      }
      if (r12.isDefined(spinx)) {
	perturbedParm = r12.getPerturbedParm(spinx);
	eval12 = true;
	eval22 = true;
      } else if (r22.isDefined(spinx)) {
	perturbedParm = r22.getPerturbedParm(spinx);
	eval12 = true;
	eval22 = true;
      }
    }
    if (eval11 || eval12 || eval21 || eval22) {
      const Matrix& ml11 = l11.getPerturbedValue(spinx);
      const Matrix& ml12 = l12.getPerturbedValue(spinx);
      const Matrix& ml21 = l21.getPerturbedValue(spinx);
      const Matrix& ml22 = l22.getPerturbedValue(spinx);
      const Matrix& mm11 = m11.getPerturbedValue(spinx);
      const Matrix& mm12 = m12.getPerturbedValue(spinx);
      const Matrix& mm21 = m21.getPerturbedValue(spinx);
      const Matrix& mm22 = m22.getPerturbedValue(spinx);
      const Matrix& mr11 = r11.getPerturbedValue(spinx);
      const Matrix& mr12 = r12.getPerturbedValue(spinx);
      const Matrix& mr21 = r21.getPerturbedValue(spinx);
      const Matrix& mr22 = r22.getPerturbedValue(spinx);
      if (eval11 || eval12) {
	Matrix t11(ml11*mm11 + ml12*mm21);
	Matrix t12(ml11*mm12 + ml12*mm22);
	if (eval11) { 
	  result11.setPerturbedParm (spinx, perturbedParm);
	  result11.setPerturbedValue (spinx, t11*mr11 + t12*mr21);
	}
	if (eval12) {
	  result12.setPerturbedParm (spinx, perturbedParm);
	  result12.setPerturbedValue (spinx, t11*mr12 + t12*mr22);
	}
      }
      if (eval21 || eval22) {
	Matrix t21(ml21*mm11 + ml22*mm21);
	Matrix t22(ml21*mm12 + ml22*mm22);
	if (eval21) {
	  result21.setPerturbedParm (spinx, perturbedParm);
	  result21.setPerturbedValue (spinx, t21*mr11 + t22*mr21);
	}
	if (eval22) {
	  result22.setPerturbedParm (spinx, perturbedParm);
	  result22.setPerturbedValue (spinx, t21*mr12 + t22*mr22);
	}
      }
    }
  }
  return result;
}

} // namespace BBS
} // namespace LOFAR

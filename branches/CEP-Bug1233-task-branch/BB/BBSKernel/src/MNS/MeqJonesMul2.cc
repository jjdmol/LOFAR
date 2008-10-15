//# JonesMul2.cc: Calculate left*right
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

#include <BBSKernel/MNS/MeqJonesMul2.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqJonesResult.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

JonesMul2::JonesMul2 (const JonesExpr& left,
			    const JonesExpr& right)
: itsLeft  (left),
  itsRight (right)
{
  addChild (itsLeft);
  addChild (itsRight);
}

JonesMul2::~JonesMul2()
{}

JonesResult JonesMul2::getJResult (const Request& request)
{
  // Create the result object.
  JonesResult result(request.nspid());
  Result& result11 = result.result11();
  Result& result12 = result.result12();
  Result& result21 = result.result21();
  Result& result22 = result.result22();
  // Calculate the children.
  JonesResult leftBuf, rightBuf;
  const JonesResult& left  = itsLeft.getResultSynced (request, leftBuf);
  const JonesResult& right = itsRight.getResultSynced (request, rightBuf);
  const Result& l11 = left.getResult11();
  const Result& l12 = left.getResult12();
  const Result& l21 = left.getResult21();
  const Result& l22 = left.getResult22();
  const Result& r11 = right.getResult11();
  const Result& r12 = right.getResult12();
  const Result& r21 = right.getResult21();
  const Result& r22 = right.getResult22();
  const Matrix& ml11 = l11.getValue();
  const Matrix& ml12 = l12.getValue();
  const Matrix& ml21 = l21.getValue();
  const Matrix& ml22 = l22.getValue();
  const Matrix& mr11 = r11.getValue();
  const Matrix& mr12 = r12.getValue();
  const Matrix& mr21 = r21.getValue();
  const Matrix& mr22 = r22.getValue();
  result11.setValue (ml11*mr11 + ml12*mr21);
  result12.setValue (ml11*mr12 + ml12*mr22);
  result21.setValue (ml21*mr11 + ml22*mr21);
  result22.setValue (ml21*mr12 + ml22*mr22);

  // Determine which values are perturbed and determine the perturbation.
  const ParmFunklet* perturbedParm;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval11 = false;
    bool eval12 = false;
    bool eval21 = false;
    bool eval22 = false;
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
    if (eval11 || eval12 || eval21 || eval22) {
      const Matrix& ml11 = l11.getPerturbedValue(spinx);
      const Matrix& ml12 = l12.getPerturbedValue(spinx);
      const Matrix& ml21 = l21.getPerturbedValue(spinx);
      const Matrix& ml22 = l22.getPerturbedValue(spinx);
      const Matrix& mr11 = r11.getPerturbedValue(spinx);
      const Matrix& mr12 = r12.getPerturbedValue(spinx);
      const Matrix& mr21 = r21.getPerturbedValue(spinx);
      const Matrix& mr22 = r22.getPerturbedValue(spinx);
      if (eval11) { 
	result11.setPerturbedParm (spinx, perturbedParm);
	result11.setPerturbedValue (spinx, ml11*mr11 + ml12*mr21);
      }
      if (eval12) {
	result12.setPerturbedParm (spinx, perturbedParm);
	result12.setPerturbedValue (spinx, ml11*mr12 + ml12*mr22);
      }
      if (eval21) {
	result21.setPerturbedParm (spinx, perturbedParm);
	result21.setPerturbedValue (spinx, ml21*mr11 + ml22*mr21);
      }
      if (eval22) {
	result22.setPerturbedParm (spinx, perturbedParm);
	result22.setPerturbedValue (spinx, ml21*mr12 + ml22*mr22);
      }
    }
  }
  return result;
}

} // namespace BBS
} // namespace LOFAR

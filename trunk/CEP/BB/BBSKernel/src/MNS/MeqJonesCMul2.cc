//# MeqJonesCMul2.cc: Calculate left*right
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

#include <BBS/MNS/MeqJonesCMul2.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqJonesResult.h>
#include <BBS/MNS/MeqMatrix.h>
#include <BBS/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR {

MeqJonesCMul2::MeqJonesCMul2 (const MeqJonesExpr& left,
			      const MeqJonesExpr& right)
: itsLeft  (left),
  itsRight (right)
{
  addChild (itsLeft);
  addChild (itsRight);
}

MeqJonesCMul2::~MeqJonesCMul2()
{}

MeqJonesResult MeqJonesCMul2::getJResult (const MeqRequest& request)
{
  // Create the result object.
  MeqJonesResult result(request.nspid());
  MeqResult& result11 = result.result11();
  MeqResult& result12 = result.result12();
  MeqResult& result21 = result.result21();
  MeqResult& result22 = result.result22();
  // Calculate the children.
  MeqJonesResult leftBuf, rightBuf;
  const MeqJonesResult& left  = itsLeft.getResultSynced (request, leftBuf);
  const MeqJonesResult& right = itsRight.getResultSynced (request, rightBuf);
  const MeqResult& l11 = left.getResult11();
  const MeqResult& l12 = left.getResult12();
  const MeqResult& l21 = left.getResult21();
  const MeqResult& l22 = left.getResult22();
  const MeqResult& r11 = right.getResult11();
  const MeqResult& r12 = right.getResult12();
  const MeqResult& r21 = right.getResult21();
  const MeqResult& r22 = right.getResult22();
  const MeqMatrix& ml11 = l11.getValue();
  const MeqMatrix& ml12 = l12.getValue();
  const MeqMatrix& ml21 = l21.getValue();
  const MeqMatrix& ml22 = l22.getValue();
  const MeqMatrix& mr11 = r11.getValue();
  const MeqMatrix& mr12 = r12.getValue();
  const MeqMatrix& mr21 = r21.getValue();
  const MeqMatrix& mr22 = r22.getValue();
  result11.setValue (ml11*conj(mr11) + ml12*conj(mr12));
  result12.setValue (ml11*conj(mr21) + ml12*conj(mr22));
  result21.setValue (ml21*conj(mr11) + ml22*conj(mr12));
  result22.setValue (ml21*conj(mr21) + ml22*conj(mr22));

  // Determine which values are perturbed and determine the perturbation.
  const MeqParmFunklet* perturbedParm;
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
    } else if (r12.isDefined(spinx)) {
      perturbedParm = r12.getPerturbedParm(spinx);
      eval11 = true;
      eval21 = true;
    }
    if (r21.isDefined(spinx)) {
      perturbedParm = r21.getPerturbedParm(spinx);
      eval12 = true;
      eval22 = true;
    } else if (r22.isDefined(spinx)) {
      perturbedParm = r22.getPerturbedParm(spinx);
      eval12 = true;
      eval22 = true;
    }
    if (eval11 || eval12 || eval21 || eval22) {
      const MeqMatrix& ml11 = l11.getValue();
      const MeqMatrix& ml12 = l12.getValue();
      const MeqMatrix& ml21 = l21.getValue();
      const MeqMatrix& ml22 = l22.getValue();
      const MeqMatrix& mr11 = r11.getValue();
      const MeqMatrix& mr12 = r12.getValue();
      const MeqMatrix& mr21 = r21.getValue();
      const MeqMatrix& mr22 = r22.getValue();
      if (eval11) { 
	result11.setPerturbedParm (spinx, perturbedParm);
	result11.setPerturbedValue (spinx, ml11*conj(mr11) + ml12*conj(mr12));
      }
      if (eval12) {
	result12.setPerturbedParm (spinx, perturbedParm);
	result12.setPerturbedValue (spinx, ml11*conj(mr21) + ml12*conj(mr22));
      }
      if (eval21) {
	result21.setPerturbedParm (spinx, perturbedParm);
	result21.setPerturbedValue (spinx, ml21*conj(mr11) + ml22*conj(mr12));
      }
      if (eval22) {
	result22.setPerturbedParm (spinx, perturbedParm);
	result22.setPerturbedValue (spinx, ml21*conj(mr21) + ml22*conj(mr22));
      }
    }
  }
  return result;
}

}

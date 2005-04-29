//# MeqJonesMul3.cc: Calculate left*mid*conj(right)
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
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqJonesMul3.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqJonesResult.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR {

MeqJonesMul3::MeqJonesMul3 (const MeqJonesExpr& left,
			    const MeqJonesExpr& mid,
			    const MeqJonesExpr& right)
: itsLeft  (left),
  itsMid   (mid),
  itsRight (right)
{}

MeqJonesMul3::~MeqJonesMul3()
{}

MeqJonesResult MeqJonesMul3::getResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  // Create the result object.
  MeqJonesResult result(request.nspid());
  MeqResult& result11 = result.result11();
  MeqResult& result12 = result.result12();
  MeqResult& result21 = result.result21();
  MeqResult& result22 = result.result22();
  // Calculate the children.
  MeqJonesResult left  = itsLeft.getResult (request);
  MeqJonesResult mid   = itsMid.getResult (request);
  MeqJonesResult right = itsRight.getResult (request);
  const MeqResult& l11 = left.getResult11();
  const MeqResult& l12 = left.getResult12();
  const MeqResult& l21 = left.getResult21();
  const MeqResult& l22 = left.getResult22();
  const MeqResult& m11 = mid.getResult11();
  const MeqResult& m12 = mid.getResult12();
  const MeqResult& m21 = mid.getResult21();
  const MeqResult& m22 = mid.getResult22();
  const MeqResult& r11 = right.getResult11();
  const MeqResult& r12 = right.getResult12();
  const MeqResult& r21 = right.getResult21();
  const MeqResult& r22 = right.getResult22();
  const MeqMatrix& ml11 = l11.getValue();
  const MeqMatrix& ml12 = l12.getValue();
  const MeqMatrix& ml21 = l21.getValue();
  const MeqMatrix& ml22 = l22.getValue();
  const MeqMatrix& mm11 = m11.getValue();
  const MeqMatrix& mm12 = m12.getValue();
  const MeqMatrix& mm21 = m21.getValue();
  const MeqMatrix& mm22 = m22.getValue();
  const MeqMatrix& mr11 = r11.getValue();
  const MeqMatrix& mr12 = r12.getValue();
  const MeqMatrix& mr21 = r21.getValue();
  const MeqMatrix& mr22 = r22.getValue();
  MeqMatrix t11(ml11*mm11 + ml12*mm21);
  MeqMatrix t12(ml11*mm12 + ml12*mm22);
  MeqMatrix t21(ml21*mm11 + ml22*mm21);
  MeqMatrix t22(ml21*mm12 + ml22*mm22);
  result11.setValue (t11*conj(mr11) + t12*conj(mr12));
  result12.setValue (t11*conj(mr21) + t12*conj(mr22));
  result21.setValue (t21*conj(mr11) + t22*conj(mr12));
  result22.setValue (t21*conj(mr21) + t22*conj(mr22));

  // Determine which values are perturbed and determine the perturbation.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval11 = false;
    bool eval12 = false;
    bool eval21 = false;
    bool eval22 = false;
    if (m11.isDefined(spinx)) {
      eval11 = true;
      perturbation = m11.getPerturbation(spinx);
    } else if (m12.isDefined(spinx)) {
      eval11 = true;
      perturbation = m12.getPerturbation(spinx);
    } else if (m21.isDefined(spinx)) {
      eval11 = true;
      perturbation = m21.getPerturbation(spinx);
    } else if (m22.isDefined(spinx)) {
      eval11 = true;
      perturbation = m22.getPerturbation(spinx);
    }
    if (eval11) {
      eval12 = eval21 = eval22 = true;
    } else {
      if (l11.isDefined(spinx)) {
	perturbation = l11.getPerturbation(spinx);
	eval11 = true;
	eval12 = true;
      } else if (l12.isDefined(spinx)) {
	perturbation = l12.getPerturbation(spinx);
	eval11 = true;
	eval12 = true;
      }
      if (l21.isDefined(spinx)) {
	perturbation = l21.getPerturbation(spinx);
	eval21 = true;
	eval22 = true;
      } else if (l22.isDefined(spinx)) {
	perturbation = l22.getPerturbation(spinx);
	eval21 = true;
	eval22 = true;
      }
      if (r11.isDefined(spinx)) {
	perturbation = r11.getPerturbation(spinx);
	eval11 = true;
	eval21 = true;
      } else if (r12.isDefined(spinx)) {
	perturbation = r12.getPerturbation(spinx);
	eval11 = true;
	eval21 = true;
      }
      if (r21.isDefined(spinx)) {
	perturbation = r21.getPerturbation(spinx);
	eval12 = true;
	eval22 = true;
      } else if (r22.isDefined(spinx)) {
	perturbation = r22.getPerturbation(spinx);
	eval12 = true;
	eval22 = true;
      }
    }
    if (eval11 || eval12 || eval21 || eval22) {
      const MeqMatrix& ml11 = l11.getPerturbedValue(spinx);
      const MeqMatrix& ml12 = l12.getPerturbedValue(spinx);
      const MeqMatrix& ml21 = l21.getPerturbedValue(spinx);
      const MeqMatrix& ml22 = l22.getPerturbedValue(spinx);
      const MeqMatrix& mm11 = m11.getPerturbedValue(spinx);
      const MeqMatrix& mm12 = m12.getPerturbedValue(spinx);
      const MeqMatrix& mm21 = m21.getPerturbedValue(spinx);
      const MeqMatrix& mm22 = m22.getPerturbedValue(spinx);
      const MeqMatrix& mr11 = r11.getPerturbedValue(spinx);
      const MeqMatrix& mr12 = r12.getPerturbedValue(spinx);
      const MeqMatrix& mr21 = r21.getPerturbedValue(spinx);
      const MeqMatrix& mr22 = r22.getPerturbedValue(spinx);
      if (eval11 || eval12) {
	MeqMatrix t11(ml11*mm11 + ml12*mm21);
	MeqMatrix t12(ml11*mm12 + ml12*mm22);
	if (eval11) { 
	  result11.setPerturbation (spinx, perturbation);
	  result11.setPerturbedValue (spinx, t11*conj(mr11) + t12*conj(mr12));
	}
	if (eval12) {
	  result12.setPerturbation (spinx, perturbation);
	  result12.setPerturbedValue (spinx, t11*conj(mr21) + t12*conj(mr22));
	}
      }
      if (eval21 || eval22) {
	MeqMatrix t21(ml21*mm11 + ml22*mm21);
	MeqMatrix t22(ml21*mm12 + ml22*mm22);
	if (eval21) {
	  result21.setPerturbation (spinx, perturbation);
	  result21.setPerturbedValue (spinx, t21*conj(mr11) + t22*conj(mr12));
	}
	if (eval22) {
	  result22.setPerturbation (spinx, perturbation);
	  result22.setPerturbedValue (spinx, t21*conj(mr21) + t22*conj(mr22));
	}
      }
    }
  }
  return result;
}

}

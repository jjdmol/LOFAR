//# MeqJonesInvert.cc: The inverse of a Jones matrix expression.
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

#include <BBS3/MNS/MeqJonesInvert.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqMatrixTmp.h>

// Inverse of a 2x2 matrix:
//
//        (a b)                      ( d -b)
// If A = (   ) then inverse(A)   =  (     ) / (ad-bc)
//        (c d)                      (-c  a)
//
// Note that:
//            (conj(a) conj(c))
//  conj(A) = (               )
//            (conj(b) conj(d))
//
// so  inverse(conj(A)) = conj(inverse(A))
//
// Also note that conj(AB) = conj(B)conj(A)

namespace LOFAR {

MeqJonesInvert::~MeqJonesInvert()
{}

MeqJonesResult MeqJonesInvert::getJResult (const MeqRequest& request)
{
  // Create the result object.
  MeqJonesResult result(request.nspid());
  MeqResult& result11 = result.result11();
  MeqResult& result12 = result.result12();
  MeqResult& result21 = result.result21();
  MeqResult& result22 = result.result22();
  // Calculate the children.
  MeqJonesResult buf;
  const MeqJonesResult& res = itsExpr.getResultSynced (request, buf);
  const MeqResult& r11 = res.getResult11();
  const MeqResult& r12 = res.getResult12();
  const MeqResult& r21 = res.getResult21();
  const MeqResult& r22 = res.getResult22();
  const MeqMatrix& mr11 = r11.getValue();
  const MeqMatrix& mr12 = r12.getValue();
  const MeqMatrix& mr21 = r21.getValue();
  const MeqMatrix& mr22 = r22.getValue();
  MeqMatrix t(1. / (mr11*mr22 - mr12*mr21));
  result11.setValue (mr22 * t);
  result12.setValue (mr12 * -t);
  result21.setValue (mr21 * -t);
  result22.setValue (mr11 * t);

  // Determine which values are perturbed and determine the perturbation.
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    double perturbation;
    bool eval = true;
    if (r11.isDefined(spinx)) {
      perturbation = r11.getPerturbation(spinx);
    } else if (r12.isDefined(spinx)) {
      perturbation = r12.getPerturbation(spinx);
    } else if (r21.isDefined(spinx)) {
      perturbation = r21.getPerturbation(spinx);
    } else if (r22.isDefined(spinx)) {
      perturbation = r22.getPerturbation(spinx);
    } else {
      eval = false;
    }
    if (eval) {
      const MeqMatrix& mr11 = r11.getPerturbedValue(spinx);
      const MeqMatrix& mr12 = r12.getPerturbedValue(spinx);
      const MeqMatrix& mr21 = r21.getPerturbedValue(spinx);
      const MeqMatrix& mr22 = r22.getPerturbedValue(spinx);
      MeqMatrix t(1. / (mr11*mr22 - mr12*mr21));
      result11.setPerturbedValue (spinx, mr11 * t);
      result12.setPerturbedValue (spinx, mr12 * -t);
      result21.setPerturbedValue (spinx, mr21 * -t);
      result22.setPerturbedValue (spinx, mr22 * t);
    }
  }
  return result;
}

}

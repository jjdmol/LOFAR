//# MeqStatExpr.cc: The Jones expression for a station
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

#include <Common/Profiling/PerfProfile.h>

#include <PSS3/MNS/MeqStatExpr.h>
#include <PSS3/MNS/MeqExpr.h>
#include <PSS3/MNS/MeqRequest.h>
#include <PSS3/MNS/MeqMatrix.h>
#include <PSS3/MNS/MeqMatrixTmp.h>

namespace LOFAR {

MeqStatExpr::MeqStatExpr (MeqExpr* faradayRotation,
			  MeqExpr* dipoleRotation,
			  MeqExpr* dipoleEllipticity,
			  MeqExpr* gain1,
			  MeqExpr* gain2)
: itsFarRot (faradayRotation),
  itsDipRot (dipoleRotation),
  itsDipEll (dipoleEllipticity),
  itsGain1  (gain1),
  itsGain2  (gain2)
{}

MeqStatExpr::~MeqStatExpr()
{}

void MeqStatExpr::calcResult (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  // Allocate the result objects.
  // At the end they will be stored in the base class object.
  MeqResult result11(request.nspid());
  MeqResult result12(request.nspid());
  MeqResult result21(request.nspid());
  MeqResult result22(request.nspid());
  // Get the values (also perturbed) for the expressions.
  MeqResult frot = itsFarRot->getResult (request);
  MeqResult drot = itsDipRot->getResult (request);
  MeqResult dell = itsDipEll->getResult (request);
  MeqResult g1  = itsGain1->getResult (request);
  MeqResult g2  = itsGain2->getResult (request);
  // Precalculate reused subexpressions.
  // They might also be reused in calculating the perturbed values,
  // so do not use a MeqMatrixTmp for them.
  MeqMatrix sinfrot = sin(frot.getValue());
  MeqMatrix cosfrot = cos(frot.getValue());
  MeqMatrix sindrot = sin(drot.getValue());
  MeqMatrix cosdrot = cos(drot.getValue());
  MeqMatrix sindell = sin(dell.getValue());
  MeqMatrix cosdell = cos(dell.getValue());
  // Multiple dell and drot matrices as:
  //        cde -sde       cdr isdr
  //        sde  cde      isdr  cdr
  // Precalculate the multiplications.
  // Thereafter multiple the result with the frot matrix
  //            cosfrot -sinfrot
  //            sinfrot  cosfrot
  // This is described in AIPS++ note 185.
  MeqMatrix cdecdr = cosdell * cosdrot;
  MeqMatrix sdesdr = sindell * sindrot;
  MeqMatrix cdesdr = cosdell * sindrot;
  MeqMatrix sdecdr = sindell * cosdrot;
  MeqMatrix d11 = tocomplex( cdecdr, -sdesdr);
  MeqMatrix d12 = tocomplex(-cdesdr,  sdecdr);
  MeqMatrix d21 = tocomplex( cdesdr,  sdecdr);
  MeqMatrix d22 = tocomplex( cdecdr,  sdesdr);
  MeqMatrix df11 = d11 * cosfrot + d12 * sinfrot;
  MeqMatrix df12 = d12 * cosfrot - d11 * sinfrot;
  MeqMatrix df21 = d21 * cosfrot + d22 * sinfrot;
  MeqMatrix df22 = d22 * cosfrot - d21 * sinfrot;
  // Calculate the final result.
  result11.setValue (g1.getValue() * df11);
  result12.setValue (g1.getValue() * df12);
  result21.setValue (g2.getValue() * df21);
  result22.setValue (g2.getValue() * df22);
  if (request.nspid() > 0) {
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      if (drot.isDefined(spinx)  ||  dell.isDefined(spinx)
      ||  frot.isDefined(spinx)
      ||  g1.isDefined(spinx)  || g2.isDefined(spinx)) {
	MeqMatrix psinfrot = sinfrot;
	MeqMatrix pcosfrot = cosfrot;
	MeqMatrix psindrot = sindrot;
	MeqMatrix pcosdrot = cosdrot;
	MeqMatrix psindell = sindell;
	MeqMatrix pcosdell = cosdell;
	MeqMatrix pd11 = d11;
	MeqMatrix pd12 = d12;
	MeqMatrix pd21 = d21;
	MeqMatrix pd22 = d22;
	MeqMatrix pdf11 = df11;
	MeqMatrix pdf12 = df12;
	MeqMatrix pdf21 = df21;
	MeqMatrix pdf22 = df22;
	MeqMatrix perturbation;
	bool eval = false;
	if (drot.isDefined(spinx)) {
	  eval = true;
	  pcosdrot = cos(drot.getPerturbedValue(spinx));
	  psindrot = sin(drot.getPerturbedValue(spinx));
	  perturbation = drot.getPerturbation(spinx);
	}
	if (dell.isDefined(spinx)) {
	  eval = true;
	  pcosdell = cos(dell.getPerturbedValue(spinx));
	  psindell = sin(dell.getPerturbedValue(spinx));
	  perturbation = dell.getPerturbation(spinx);
	}
	if (eval) {
	  MeqMatrix cdecdr = pcosdell * pcosdrot;
	  MeqMatrix sdesdr = psindell * psindrot;
	  MeqMatrix cdesdr = pcosdell * psindrot;
	  MeqMatrix sdecdr = psindell * pcosdrot;
	  pd11 = tocomplex( cdecdr, -sdesdr);
	  pd12 = tocomplex(-cdesdr,  sdecdr);
	  pd21 = tocomplex( cdesdr,  sdecdr);
	  pd22 = tocomplex( cdecdr,  sdesdr);
	}
	if (frot.isDefined(spinx)) {
	  eval = true;
	  pcosfrot = cos(frot.getPerturbedValue(spinx));
	  psinfrot = sin(frot.getPerturbedValue(spinx));
	  perturbation = frot.getPerturbation(spinx);
	}
	if (eval) {
	  pdf11 = pd11 * pcosfrot + pd12 * psinfrot;
	  pdf12 = pd12 * pcosfrot - pd11 * psinfrot;
	  pdf21 = pd21 * pcosfrot + pd22 * psinfrot;
	  pdf22 = pd22 * pcosfrot - pd21 * psinfrot;
	}
	bool evalg = eval;
	if (g1.isDefined(spinx)) {
	  evalg = true;
	  perturbation = g1.getPerturbation(spinx);
	}
	if (evalg) {
	  const MeqMatrix& pert = g1.getPerturbedValue(spinx);
	  result11.setPerturbedValue (spinx, pert * pdf11);
	  result12.setPerturbedValue (spinx, pert * pdf12);
	  result11.setPerturbation (spinx, perturbation);
	  result12.setPerturbation (spinx, perturbation);
	}
	evalg = eval;
	if (g2.isDefined(spinx)) {
	  evalg = true;
	  perturbation = g2.getPerturbation(spinx);
	}
	if (evalg) {
	  const MeqMatrix& pert = g2.getPerturbedValue(spinx);
	  result21.setPerturbedValue (spinx, pert * pdf21);
	  result22.setPerturbedValue (spinx, pert * pdf22);
	  result21.setPerturbation (spinx, perturbation);
	  result22.setPerturbation (spinx, perturbation);
	}
      }
    }
  }
  setResult11 (result11);
  setResult12 (result12);
  setResult21 (result21);
  setResult22 (result22);
}

}

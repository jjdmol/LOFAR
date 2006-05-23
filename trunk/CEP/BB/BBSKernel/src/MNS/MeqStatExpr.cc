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

#include <lofar_config.h>
#include <Common/Timer.h>

#include <BBS/MNS/MeqStatExpr.h>
#include <BBS/MNS/MeqExpr.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqMatrix.h>
#include <BBS/MNS/MeqMatrixTmp.h>

namespace LOFAR {

MeqStatExpr::MeqStatExpr (const MeqExpr& faradayRotation,
			  const MeqExpr& dipoleRotation,
			  const MeqExpr& dipoleEllipticity,
			  const MeqExpr& gain1,
			  const MeqExpr& gain2)
: itsFarRot (faradayRotation),
  itsDipRot (dipoleRotation),
  itsDipEll (dipoleEllipticity),
  itsGain1  (gain1),
  itsGain2  (gain2)
{
  addChild (itsFarRot);
  addChild (itsDipRot);
  addChild (itsDipEll);
  addChild (itsGain1);
  addChild (itsGain2);
}

MeqStatExpr::~MeqStatExpr()
{}

MeqJonesResult MeqStatExpr::getJResult (const MeqRequest& request)
{
  static NSTimer timer("MeqStatExpr::getResult", true);
  timer.start();
  
  // Allocate the result objects.
  // At the end they will be stored in the base class object.
  MeqJonesResult result(request.nspid());
  MeqResult& result11 = result.result11();
  MeqResult& result12 = result.result12();
  MeqResult& result21 = result.result21();
  MeqResult& result22 = result.result22();
  // Get the values (also perturbed) for the expressions.
  MeqResult frotBuf, drotBuf, dellBuf, g1Buf, g2Buf;
  const MeqResult& frot = itsFarRot.getResultSynced (request, frotBuf);
  const MeqResult& drot = itsDipRot.getResultSynced (request, drotBuf);
  const MeqResult& dell = itsDipEll.getResultSynced (request, dellBuf);
  const MeqResult& g1  = itsGain1.getResultSynced (request, g1Buf);
  const MeqResult& g2  = itsGain2.getResultSynced (request, g2Buf);
  // Precalculate reused subexpressions.
  // They might also be reused in calculating the perturbed values,
  // so do not use a MeqMatrixTmp for them.
  MeqMatrix sinfrot = sin(frot.getValue());
  MeqMatrix cosfrot = cos(frot.getValue());
  MeqMatrix sindrot = sin(drot.getValue());
  MeqMatrix cosdrot = cos(drot.getValue());
  MeqMatrix sindell = sin(dell.getValue());
  MeqMatrix cosdell = cos(dell.getValue());
  // Multiply dell and drot matrices as:
  //        cde -sde       cdr isdr
  //        sde  cde      isdr  cdr
  // Precalculate the multiplications.
  // Thereafter multiply the result with the frot matrix
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
	const MeqParmFunklet* perturbedParm;
	bool eval = false;
	if (drot.isDefined(spinx)) {
	  eval = true;
	  pcosdrot = cos(drot.getPerturbedValue(spinx));
	  psindrot = sin(drot.getPerturbedValue(spinx));
	  perturbedParm = drot.getPerturbedParm(spinx);
	}
	if (dell.isDefined(spinx)) {
	  eval = true;
	  pcosdell = cos(dell.getPerturbedValue(spinx));
	  psindell = sin(dell.getPerturbedValue(spinx));
	  perturbedParm = dell.getPerturbedParm(spinx);
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
	  perturbedParm = frot.getPerturbedParm(spinx);
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
	  perturbedParm = g1.getPerturbedParm(spinx);
	}
	if (evalg) {
	  const MeqMatrix& pert = g1.getPerturbedValue(spinx);
	  result11.setPerturbedValue (spinx, pert * pdf11);
	  result12.setPerturbedValue (spinx, pert * pdf12);
	  result11.setPerturbedParm (spinx, perturbedParm);
	  result12.setPerturbedParm (spinx, perturbedParm);
	}
	evalg = eval;
	if (g2.isDefined(spinx)) {
	  evalg = true;
	  perturbedParm = g2.getPerturbedParm(spinx);
	}
	if (evalg) {
	  const MeqMatrix& pert = g2.getPerturbedValue(spinx);
	  result21.setPerturbedValue (spinx, pert * pdf21);
	  result22.setPerturbedValue (spinx, pert * pdf22);
	  result21.setPerturbedParm (spinx, perturbedParm);
	  result22.setPerturbedParm (spinx, perturbedParm);
	}
      }
    }
  }
  timer.stop();
  return result;
}

}

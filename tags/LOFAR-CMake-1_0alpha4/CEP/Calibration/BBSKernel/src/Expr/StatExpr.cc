//# StatExpr.cc: The Jones expression for a station
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <Common/Timer.h>

#include <BBSKernel/Expr/StatExpr.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

StatExpr::StatExpr (const Expr& faradayRotation,
			  const Expr& dipoleRotation,
			  const Expr& dipoleEllipticity,
			  const Expr& gain1,
			  const Expr& gain2)
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

StatExpr::~StatExpr()
{}

JonesResult StatExpr::getJResult (const Request& request)
{
  static NSTimer timer("StatExpr::getResult", true);
  timer.start();
  
  // Allocate the result objects.
  // At the end they will be stored in the base class object.
  JonesResult result(request.nspid());
  Result& result11 = result.result11();
  Result& result12 = result.result12();
  Result& result21 = result.result21();
  Result& result22 = result.result22();
  // Get the values (also perturbed) for the expressions.
  Result frotBuf, drotBuf, dellBuf, g1Buf, g2Buf;
  const Result& frot = itsFarRot.getResultSynced (request, frotBuf);
  const Result& drot = itsDipRot.getResultSynced (request, drotBuf);
  const Result& dell = itsDipEll.getResultSynced (request, dellBuf);
  const Result& g1  = itsGain1.getResultSynced (request, g1Buf);
  const Result& g2  = itsGain2.getResultSynced (request, g2Buf);
  // Precalculate reused subexpressions.
  // They might also be reused in calculating the perturbed values,
  // so do not use a MatrixTmp for them.
  Matrix sinfrot = sin(frot.getValue());
  Matrix cosfrot = cos(frot.getValue());
  Matrix sindrot = sin(drot.getValue());
  Matrix cosdrot = cos(drot.getValue());
  Matrix sindell = sin(dell.getValue());
  Matrix cosdell = cos(dell.getValue());
  // Multiply dell and drot matrices as:
  //        cde -sde       cdr isdr
  //        sde  cde      isdr  cdr
  // Precalculate the multiplications.
  // Thereafter multiply the result with the frot matrix
  //            cosfrot -sinfrot
  //            sinfrot  cosfrot
  // This is described in AIPS++ note 185.
  Matrix cdecdr = cosdell * cosdrot;
  Matrix sdesdr = sindell * sindrot;
  Matrix cdesdr = cosdell * sindrot;
  Matrix sdecdr = sindell * cosdrot;
  Matrix d11 = tocomplex( cdecdr, -sdesdr);
  Matrix d12 = tocomplex(-cdesdr,  sdecdr);
  Matrix d21 = tocomplex( cdesdr,  sdecdr);
  Matrix d22 = tocomplex( cdecdr,  sdesdr);
  Matrix df11 = d11 * cosfrot + d12 * sinfrot;
  Matrix df12 = d12 * cosfrot - d11 * sinfrot;
  Matrix df21 = d21 * cosfrot + d22 * sinfrot;
  Matrix df22 = d22 * cosfrot - d21 * sinfrot;
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
	Matrix psinfrot = sinfrot;
	Matrix pcosfrot = cosfrot;
	Matrix psindrot = sindrot;
	Matrix pcosdrot = cosdrot;
	Matrix psindell = sindell;
	Matrix pcosdell = cosdell;
	Matrix pd11 = d11;
	Matrix pd12 = d12;
	Matrix pd21 = d21;
	Matrix pd22 = d22;
	Matrix pdf11 = df11;
	Matrix pdf12 = df12;
	Matrix pdf21 = df21;
	Matrix pdf22 = df22;
	const ParmFunklet* perturbedParm;
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
	  Matrix cdecdr = pcosdell * pcosdrot;
	  Matrix sdesdr = psindell * psindrot;
	  Matrix cdesdr = pcosdell * psindrot;
	  Matrix sdecdr = psindell * pcosdrot;
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
	  const Matrix& pert = g1.getPerturbedValue(spinx);
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
	  const Matrix& pert = g2.getPerturbedValue(spinx);
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

} // namespace BBS
} // namespace LOFAR

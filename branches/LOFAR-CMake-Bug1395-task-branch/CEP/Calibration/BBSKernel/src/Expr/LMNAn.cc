//# LMN.cc: Class holding the LMN values of a point source
//#
//# Copyright (C) 2005
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
#include <Common/Profiling/PerfProfile.h>

#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/PhaseRef.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <Common/LofarLogger.h>


namespace LOFAR
{
namespace BBS
{

LMN::LMN (PointSource* source)
: itsSource    (source)
{
  addChild (itsSource->getRa());
  addChild (itsSource->getDec());
}

ResultVec LMN::getResultVec (const Request& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  ResultVec result(3, request.nspid());
  Result& resL = result[0];
  Result& resM = result[1];
  Result& resN = result[2];
  Result raRes, deRes;
  const Result& rak  = itsSource->getRa().getResultSynced (request, raRes);
  const Result& deck = itsSource->getDec().getResultSynced (request, deRes);
  double refRa  = itsPhaseRef->getRa();
  double refDec = itsPhaseRef->getDec();
  double refSinDec = itsPhaseRef->getSinDec();
  double refCosDec = itsPhaseRef->getCosDec();
  Matrix cosdec = cos(deck.getValue());
  Matrix radiff = rak.getValue() - refRa;
  if (request.nspid() == 0) {
    Matrix lk = cosdec * sin(radiff);
    Matrix mk = sin(deck.getValue()) * refCosDec -
                   cosdec * refSinDec * cos(radiff);
    MatrixTmp nks = 1. - sqr(lk) - sqr(mk);
    ASSERTSTR (min(nks).getDouble() > 0, "source " << itsSource->getSourceNr()
	       << " too far from phaseref " << refRa << ", " << refDec);
    Matrix nk = sqrt(nks);
    resL.setValue (lk);
    resM.setValue (mk);
    resN.setValue (nk);
  } else {
    Matrix sinradiff = sin(radiff);
    Matrix cosradiff = cos(radiff);
    Matrix sindec = sin(deck.getValue());
    Matrix lk = cosdec * sinradiff;
    Matrix mk = sindec * refCosDec -
                   cosdec * refSinDec * cosradiff;
    MatrixTmp nks = 1. - sqr(lk) - sqr(mk);
    ASSERTSTR (min(nks).getDouble() > 0, "source " << itsSource->getSourceNr()
	       << " too far from phaseref " << refRa << ", " << refDec);
    Matrix nk = sqrt(nks);
    resL.setValue (lk);
    resM.setValue (mk);
    resN.setValue (nk);

    // Evaluate (if needed) for the perturbed parameter values.
    // l = cosdec*sinradiff
    // m = sindec*cosdec0 - cosdec*sindec0*cosradiff
    // l'= -sindec*dec'*sinradiff + cosdec*cosradiff*ra'
    // m'= cosdec*dec'*cosdec0 - (-sindec*dec'*sindec0*cosradiff
    //                            + cosdec*sindec0*-sinradiff*ra')
    //   = (cosdec*cosdec0 + sindec*sindec0*cosradiff) * dec'
    //     + cosdec*sindec0*sinradiff*ra'
    // precalculate:  c1 = cosdec*sindec0*sinradiff = l*sindec0
    //                c2 = cosdec*cosdec0 + sindec*sindec0*cosradiff
    //                c3 = cosdec*cosradiff
    //                c4 = -sindec*sinradiff
    // l'= c3*ra' + c4*dec'
    // m'= c1*ra' + c2*dec'
    // n = sqrt(1 - l^2 - m^2)
    // n'= 0.5 * 1/sqrt(1 - l^2 - m^2) * (-2*l*l' - 2*m*m')
    //   = (l*l' + m*m') / -n
    double perturbation;
    Matrix c1 = lk * refSinDec;
    Matrix c2 = cosdec * refCosDec + sindec * refSinDec * cosradiff;
    Matrix c3 = cosdec * cosradiff;
    Matrix c4 = -sindec * sinradiff;
    Matrix ln = -lk / n;
    Matrix mn = -mk / n;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      if (rak.isDefined(spinx)) {
	const Matrix& dra = raRes.getPerturbedValue (spinx);
	if (deck.isDefined(spinx)) {
	  // Both are defined, so we have to evaluate all.
	  const Matrix& ddec = decRes.getPerturbedValue (spinx);
	  Matrix dl = c3*dra + c4*ddec;
	  Matrix dm = c1*dra + c2*ddec;
	  resL.setPerturbedValue (spinx, dl);
	  resM.setPerturbedValue (spinx, dm);
	  resN.setPerturbedValue (spinx, ln*dl + mn*dm);
	} else {
	  // no derivative in dec, so ddec=0.
	  Matrix dl = c3*dra;
	  Matrix dm = c1*dra;
	  resL.setPerturbedValue (spinx, dl);
	  resM.setPerturbedValue (spinx, dm);
	  resN.setPerturbedValue (spinx, ln*dl + mn*dm);
	}
      } else if (deck.isDefined(spinx)) {
	// no derivative in ra, so dra=0.
	const Matrix& ddec = decRes.getPerturbedValue (spinx);
	Matrix dl = c4*ddec;
	Matrix dm = c2*ddec;
	resL.setPerturbedValue (spinx, dl);
	resM.setPerturbedValue (spinx, dm);
	resN.setPerturbedValue (spinx, ln*dl + mn*dm);
      }
    }
  }
  return result;
}

} // namespace BBS
} // namespace LOFAR

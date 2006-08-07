//# MeqLMN.cc: Class holding the LMN values of a point source
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

#include <BBS/MNS/MeqLMN.h>
#include <BBS/MNS/MeqSource.h>
#include <BBS/MNS/MeqPhaseRef.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>


namespace LOFAR {

MeqLMN::MeqLMN (MeqSource* source)
: itsSource    (source)
{
    addChild(itsSource->getRa());
    addChild(itsSource->getDec());
}

MeqLMN::~MeqLMN()
{
    removeChild(itsSource->getRa());
    removeChild(itsSource->getDec());
}

MeqResultVec MeqLMN::getResultVec (const MeqRequest& request)
{
  MeqResultVec result(3, request.nspid());
  MeqResult& resL = result[0];
  MeqResult& resM = result[1];
  MeqResult& resN = result[2];
  MeqResult raRes, deRes;
  const MeqResult& rak  = itsSource->getRa().getResultSynced (request, raRes);
  const MeqResult& deck = itsSource->getDec().getResultSynced (request, deRes);
  double refRa  = itsPhaseRef->getRa();
  double refDec = itsPhaseRef->getDec();
  double refSinDec = itsPhaseRef->getSinDec();
  double refCosDec = itsPhaseRef->getCosDec();
  MeqMatrix cosdec = cos(deck.getValue());
  MeqMatrix radiff = rak.getValue() - refRa;
  MeqMatrix lk = cosdec * sin(radiff);
  MeqMatrix mk = sin(deck.getValue()) * refCosDec -
                 cosdec * refSinDec * cos(radiff);
  MeqMatrixTmp nks = 1. - sqr(lk) - sqr(mk);
  ASSERTSTR (min(nks).getDouble() > 0, "source " << itsSource->getSourceNr()
         << " too far from phaseref " << refRa << ", " << refDec);
  MeqMatrix nk = sqrt(nks);
  resL.setValue (lk);
  resM.setValue (mk);
  resN.setValue (nk);

  // Evaluate (if needed) for the perturbed parameter values.
  const MeqParmFunklet* perturbedParm;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    MeqMatrix pcosdec = cosdec;
    MeqMatrix pradiff = radiff;
    bool eval = false;
    if (rak.isDefined(spinx)) {
      perturbedParm = rak.getPerturbedParm(spinx);
      pradiff = rak.getPerturbedValue(spinx) - refRa;
      eval = true;
    }
    if (deck.isDefined(spinx)) {
      perturbedParm = deck.getPerturbedParm(spinx);
      pcosdec = cos(deck.getPerturbedValue(spinx));
      eval = true;
    }
    if (eval) {
      MeqMatrix plk = pcosdec * sin(pradiff);
      MeqMatrix pmk = sin(deck.getPerturbedValue(spinx)) * refCosDec -
       pcosdec * refSinDec * cos(pradiff);
      MeqMatrixTmp nks = MeqMatrixTmp(1.) - sqr(plk) - sqr(pmk);
      ASSERTSTR (min(nks).getDouble() > 0, "perturbed source "
         << itsSource->getSourceNr()
         << " too far from phaseref " << refRa << ", " << refDec);
      MeqMatrix pnk = sqrt(nks);
      resL.setPerturbedValue (spinx, plk);
      resL.setPerturbedParm   (spinx, perturbedParm);
      resM.setPerturbedValue (spinx, pmk);
      resM.setPerturbedParm   (spinx, perturbedParm);
      resN.setPerturbedValue (spinx, pnk);
      resN.setPerturbedParm   (spinx, perturbedParm);
    }
  }
  return result;
}

MeqResultVec MeqLMN::getAnResultVec (const MeqRequest& request)
{
  MeqResultVec result(3, request.nspid());
  MeqResult& resL = result[0];
  MeqResult& resM = result[1];
  MeqResult& resN = result[2];
  MeqResult raRes, deRes;
  const MeqResult& rak  = itsSource->getRa().getResultSynced (request, raRes);
  const MeqResult& deck = itsSource->getDec().getResultSynced (request, deRes);
  double refRa  = itsPhaseRef->getRa();
  double refDec = itsPhaseRef->getDec();
  double refSinDec = itsPhaseRef->getSinDec();
  double refCosDec = itsPhaseRef->getCosDec();
  MeqMatrix cosdec = cos(deck.getValue());
  MeqMatrix radiff = rak.getValue() - refRa;
  if (request.nspid() == 0) {
    MeqMatrix lk = cosdec * sin(radiff);
    MeqMatrix mk = sin(deck.getValue()) * refCosDec -
                   cosdec * refSinDec * cos(radiff);
    MeqMatrixTmp nks = 1. - sqr(lk) - sqr(mk);
    ASSERTSTR (min(nks).getDouble() > 0, "source " << itsSource->getSourceNr()
           << " too far from phaseref " << refRa << ", " << refDec);
    MeqMatrix nk = sqrt(nks);
    resL.setValue (lk);
    resM.setValue (mk);
    resN.setValue (nk);
  } else {
    MeqMatrix sinradiff = sin(radiff);
    MeqMatrix cosradiff = cos(radiff);
    MeqMatrix sindec = sin(deck.getValue());
    MeqMatrix lk = cosdec * sinradiff;
    MeqMatrix mk = sindec * refCosDec -
                   cosdec * refSinDec * cosradiff;
    MeqMatrixTmp nks = 1. - sqr(lk) - sqr(mk);
    ASSERTSTR (min(nks).getDouble() > 0, "source " << itsSource->getSourceNr()
           << " too far from phaseref " << refRa << ", " << refDec);
    MeqMatrix nk = sqrt(nks);
    resL.setValue (lk);
    resM.setValue (mk);
    resN.setValue (nk);

    // Evaluate (if needed) for the parameter derivatives.
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
    MeqMatrix c1 = lk * refSinDec;
    MeqMatrix c2 = cosdec * refCosDec + sindec * refSinDec * cosradiff;
    MeqMatrix c3 = cosdec * cosradiff;
    MeqMatrix c4 = -sindec * sinradiff;
    MeqMatrix ln = -lk / nk;
    MeqMatrix mn = -mk / nk;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      if (rak.isDefined(spinx)) {
    const MeqMatrix& dra = raRes.getPerturbedValue (spinx);
    if (deck.isDefined(spinx)) {
      // Both are defined, so we have to evaluate all.
      const MeqMatrix& ddec = deRes.getPerturbedValue (spinx);
      MeqMatrix dl = c3*dra + c4*ddec;
      MeqMatrix dm = c1*dra + c2*ddec;
      resL.setPerturbedValue (spinx, dl);
      resM.setPerturbedValue (spinx, dm);
      resN.setPerturbedValue (spinx, ln*dl + mn*dm);
    } else {
      // no derivative in dec, so ddec=0.
      MeqMatrix dl = c3*dra;
      MeqMatrix dm = c1*dra;
      resL.setPerturbedValue (spinx, dl);
      resM.setPerturbedValue (spinx, dm);
      resN.setPerturbedValue (spinx, ln*dl + mn*dm);
    }
      } else if (deck.isDefined(spinx)) {
    // no derivative in ra, so dra=0.
    const MeqMatrix& ddec = deRes.getPerturbedValue (spinx);
    MeqMatrix dl = c4*ddec;
    MeqMatrix dm = c2*ddec;
    resL.setPerturbedValue (spinx, dl);
    resM.setPerturbedValue (spinx, dm);
    resN.setPerturbedValue (spinx, ln*dl + mn*dm);
      }
    }
  }
  return result;
}

}

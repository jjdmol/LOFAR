//# MeqPointSource.cc: Abstract base class for a list of sources
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

#include <BBS3/MNS/MeqPointSource.h>
#include <BBS3/MNS/MeqPhaseRef.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>


namespace LOFAR {

MeqPointSource::MeqPointSource()
: itsSourceNr (0),
  itsI        (0),
  itsQ        (0),
  itsU        (0),
  itsV        (0),
  itsRa       (0),
  itsDec      (0)
{}

MeqPointSource::MeqPointSource (const string& name,
				MeqExpr* fluxI, MeqExpr* fluxQ,
				MeqExpr* fluxU, MeqExpr* fluxV,
				MeqExpr* ra, MeqExpr* dec)
: itsName(name),
  itsI   (fluxI),
  itsQ   (fluxQ),
  itsU   (fluxU),
  itsV   (fluxV),
  itsRa  (ra),
  itsDec (dec),
  itsLastReqId (InitMeqRequestId)
{}

void MeqPointSource::calculate (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  itsL = MeqResult(request.nspid());
  itsM = MeqResult(request.nspid());
  itsN = MeqResult(request.nspid());
  const MeqResult& rak  = itsRa->getResult (request);
  const MeqResult& deck = itsDec->getResult (request);
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
  ASSERTSTR (min(nks).getDouble() > 0, "source " << itsSourceNr
	     << " too far from phaseref " << refRa << ", " << refDec);
  MeqMatrix nk = sqrt(nks);
  itsL.setValue (lk);
  itsM.setValue (mk);
  itsN.setValue (nk);

  // Evaluate (if needed) for the perturbed parameter values.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    MeqMatrix pcosdec = cosdec;
    MeqMatrix pradiff = radiff;
    bool eval = false;
    if (rak.isDefined(spinx)) {
      perturbation = rak.getPerturbation(spinx);
      pradiff = rak.getPerturbedValue(spinx) - refRa;
      eval = true;
    }
    if (deck.isDefined(spinx)) {
      perturbation = deck.getPerturbation(spinx);
      pcosdec = cos(deck.getPerturbedValue(spinx));
      eval = true;
    }
    if (eval) {
      MeqMatrix plk = pcosdec * sin(pradiff);
      MeqMatrix pmk = sin(deck.getPerturbedValue(spinx)) * refCosDec -
	   pcosdec * refSinDec * cos(pradiff);
      MeqMatrixTmp nks = MeqMatrixTmp(1.) - sqr(plk) - sqr(pmk);
      ASSERTSTR (min(nks).getDouble() > 0, "perturbed source " << itsSourceNr
		 << " too far from phaseref " << refRa << ", " << refDec);
      MeqMatrix pnk = sqrt(nks);
      itsL.setPerturbedValue (spinx, plk);
      itsL.setPerturbation   (spinx, perturbation);
      itsM.setPerturbedValue (spinx, pmk);
      itsM.setPerturbation   (spinx, perturbation);
      itsN.setPerturbedValue (spinx, pnk);
      itsN.setPerturbation   (spinx, perturbation);
    }
  }
  itsLastReqId = request.getId();
}

}

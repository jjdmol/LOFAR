//# MeqStatSources.cc: The precalculated source DFT exponents for a domain
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

#include <PerfProfile.h>

#include <MNS/MeqStatSources.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqStatUVW.h>
#include <MNS/MeqPhaseRef.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqDomain.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Mathematics/Constants.h>


MeqStatSources::MeqStatSources (MeqStatUVW* statUVW,
				vector<MeqPointSource>* sources)
: itsUVW       (statUVW),
  itsSources   (sources),
  itsLastReqId (InitMeqRequestId)
{}

void MeqStatSources::calculate (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  itsResults.resize (itsSources->size());
  const MeqResult& resU = itsUVW->getU(request);
  const MeqResult& resV = itsUVW->getV(request);
  const MeqResult& resW = itsUVW->getW(request);
  vector<MeqResult>::iterator iterRes = itsResults.begin();
  for (vector<MeqPointSource>::iterator iter=itsSources->begin();
       iter != itsSources->end();
       ++iter) {
    MeqPointSource& src = *iter;
    const MeqMatrix& u = resU.getValue();
    const MeqMatrix& v = resV.getValue();
    const MeqMatrix& w = resW.getValue();
    const MeqResult& lrk  = src.getL(request);
    const MeqResult& mrk  = src.getM(request);
    const MeqResult& nrk  = src.getN(request);
    MeqResult result(request.nspid());
    MeqMatrix r1 = u*lrk.getValue() + v*mrk.getValue() + w*nrk.getValue();
    MeqMatrix res = tocomplex(cos(r1), sin(r1));
    result.setValue (res);

    // Evaluate (if needed) for the perturbed parameter values.
    MeqMatrix perturbation;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      bool eval = false;
      if (lrk.isDefined(spinx)) {
	perturbation = lrk.getPerturbation(spinx);
	eval = true;
      }
      if (mrk.isDefined(spinx)) {
	perturbation = mrk.getPerturbation(spinx);
	eval = true;
      }
      if (nrk.isDefined(spinx)) {
	perturbation = nrk.getPerturbation(spinx);
	eval = true;
      }
      if (resU.isDefined(spinx)) {
	perturbation = resU.getPerturbation(spinx);
	eval = true;
      }
      if (resV.isDefined(spinx)) {
	perturbation = resV.getPerturbation(spinx);
	eval = true;
      }
      if (resW.isDefined(spinx)) {
	perturbation = resW.getPerturbation(spinx);
	eval = true;
      }
      if (eval) {
	r1 = resU.getPerturbedValue(spinx) * lrk.getPerturbedValue(spinx) +
	     resV.getPerturbedValue(spinx) * mrk.getPerturbedValue(spinx) +
	     resW.getPerturbedValue(spinx) * nrk.getPerturbedValue(spinx);
	res = tocomplex(cos(r1), sin(r1));
	result.setPerturbedValue (spinx, res);
	result.setPerturbation (spinx, perturbation);
      }
    }
    *iterRes = result;
    ++iterRes;
  }
  itsLastReqId = request.getId();
}

double MeqStatSources::getExponent (int sourceNr,
				    const MeqRequest& request)
{
  DbgAssert (request.nx() == 1);
  double u = itsUVW->getU(request).getValue().getDouble();
  double v = itsUVW->getV(request).getValue().getDouble();
  double w = itsUVW->getW(request).getValue().getDouble();
  MeqPointSource& src = (*itsSources)[sourceNr];
  double lk = src.getL(request).getValue().getDouble();
  double mk = src.getL(request).getValue().getDouble();
  double nk = src.getL(request).getValue().getDouble();
  return (u*lk + v*mk + w*nk);
}

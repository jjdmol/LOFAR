//# MeqStatSources.cc: The precalculated source DFT exponents for a station
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

#include <PSS3/MNS/MeqStatSources.h>
#include <PSS3/MNS/MeqStatUVW.h>
#include <PSS3/MNS/MeqPhaseRef.h>
#include <PSS3/MNS/MeqRequest.h>
#include <PSS3/MNS/MeqDomain.h>
#include <PSS3/MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <casa/BasicSL/Constants.h>


MeqStatSources::MeqStatSources (MeqStatUVW* statUVW,
				MeqSourceList* sources)
: itsUVW       (statUVW),
  itsSources   (sources),
  itsLastReqId (InitMeqRequestId)
{}

void MeqStatSources::calculate (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);
  const MeqDomain& domain = request.domain();
  // The exponent and its frequency delta are calculated.
  // However, the delta is only calculated if there are multiple channels.
  itsResults.resize (itsSources->size());
  bool calcDelta = request.ny() > 1;
  // Calculate 2pi/wavelength, where wavelength=c/freq.
  // Calculate it for the frequency step if needed.
  double df = request.stepY();
  double f0 = domain.startY() + df/2;
  MeqMatrix wavel0 (C::_2pi * f0 / C::c);
  MeqMatrix dwavel;
  if (calcDelta) {
    itsDeltas.resize (itsSources->size());
    dwavel = MeqMatrix (df / f0);
  } else {
    itsDeltas.resize (0);
  }
  // Get the UVW coordinates.
  const MeqResult& resU = itsUVW->getU(request);
  const MeqResult& resV = itsUVW->getV(request);
  const MeqResult& resW = itsUVW->getW(request);
  const MeqMatrix& u = resU.getValue();
  const MeqMatrix& v = resV.getValue();
  const MeqMatrix& w = resW.getValue();
  // Calculate the DFT contribution for this station for all sources.
  vector<MeqResult>::iterator iterRes = itsResults.begin();
  vector<MeqResult>::iterator iterDelta = itsDeltas.begin();
  int nrsrc = itsSources->size();
  for (int srcnr=0; srcnr<nrsrc; srcnr++) {
    MeqPointSource& src = (*itsSources)[srcnr];
    const MeqResult& lrk  = src.getL(request);
    const MeqResult& mrk  = src.getM(request);
    const MeqResult& nrk  = src.getN(request);
    MeqResult result(request.nspid());
    MeqMatrix r1 = (u*lrk.getValue() + v*mrk.getValue() + w*nrk.getValue()) *
                   wavel0;
    result.setValue (tocomplex(cos(r1), sin(r1)));
    MeqResult delta;
    if (calcDelta) {
      delta = MeqResult(request.nspid());
      r1 *= dwavel;
      delta.setValue (tocomplex(cos(r1), sin(r1)));
    }

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
	r1 = (resU.getPerturbedValue(spinx) * lrk.getPerturbedValue(spinx) +
	      resV.getPerturbedValue(spinx) * mrk.getPerturbedValue(spinx) +
	      resW.getPerturbedValue(spinx) * nrk.getPerturbedValue(spinx))
	  * wavel0;
	result.setPerturbedValue (spinx, tocomplex(cos(r1), sin(r1)));
	result.setPerturbation (spinx, perturbation);
	if (calcDelta) {
	  r1 *= dwavel;
	  delta.setPerturbedValue (spinx, tocomplex(cos(r1), sin(r1)));
	}
      }
    }
    *iterRes = result;
    ++iterRes;
    if (calcDelta) {
      *iterDelta = delta;
      ++iterDelta;
    }
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

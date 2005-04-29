//# MeqDFTPS.cc: Class doing the station DFT for a point source
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
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqDFTPS.h>
#include <BBS3/MNS/MeqStatUVW.h>
#include <BBS3/MNS/MeqLMN.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR {


MeqDFTPS::MeqDFTPS (MeqLMN* lmn, MeqStatUVW* uvw)
: itsLMN (lmn),
  itsUVW (uvw)
{}

MeqDFTPS::~MeqDFTPS()
{}

MeqResultVec MeqDFTPS::getResultVec (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  const MeqDomain& domain = request.domain();
  // The exponent and its frequency delta are calculated.
  // However, the delta is only calculated if there are multiple channels.
  MeqResultVec resultVec(2, request.nspid());
  bool calcDelta = request.nx() > 1;
  // Calculate 2pi/wavelength, where wavelength=c/freq.
  // Calculate it for the frequency step if needed.
  double df = request.stepX();
  double f0 = domain.startX() + df/2;
  MeqMatrix wavel0 (C::_2pi * f0 / C::c);
  MeqMatrix dwavel (df / f0);
  // Get the UVW coordinates.
  const MeqResult& resU = itsUVW->getU(request);
  const MeqResult& resV = itsUVW->getV(request);
  const MeqResult& resW = itsUVW->getW(request);
  const MeqMatrix& u = resU.getValue();
  const MeqMatrix& v = resV.getValue();
  const MeqMatrix& w = resW.getValue();
  // Calculate the DFT contribution for this station for the source.
  MeqResultVec lmn = itsLMN->getResultVec (request);
  const MeqResult& lrk  = lmn[0];
  const MeqResult& mrk  = lmn[1];
  const MeqResult& nrk  = lmn[2];
  MeqResult& result = resultVec[0];
  MeqMatrix r1 = (u*lrk.getValue() + v*mrk.getValue() +
		  w*(nrk.getValue() - 1.)) * wavel0;
  result.setValue (tocomplex(cos(r1), sin(r1)));
  if (calcDelta) {
    MeqResult& delta = resultVec[1];
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
    } else if (mrk.isDefined(spinx)) {
      perturbation = mrk.getPerturbation(spinx);
      eval = true;
    } else if (nrk.isDefined(spinx)) {
      perturbation = nrk.getPerturbation(spinx);
      eval = true;
    } else if (resU.isDefined(spinx)) {
      perturbation = resU.getPerturbation(spinx);
      eval = true;
    } else if (resV.isDefined(spinx)) {
      perturbation = resV.getPerturbation(spinx);
      eval = true;
    } else if (resW.isDefined(spinx)) {
      perturbation = resW.getPerturbation(spinx);
      eval = true;
    }
    if (eval) {
      r1 = (resU.getPerturbedValue(spinx) * lrk.getPerturbedValue(spinx) +
	    resV.getPerturbedValue(spinx) * mrk.getPerturbedValue(spinx) +
	    resW.getPerturbedValue(spinx) * 
	    (nrk.getPerturbedValue(spinx) - 1.)) * wavel0;
      result.setPerturbedValue (spinx, tocomplex(cos(r1), sin(r1)));
      result.setPerturbation (spinx, perturbation);
      if (calcDelta) {
	MeqResult& delta = resultVec[1];
	r1 *= dwavel;
	delta.setPerturbedValue (spinx, tocomplex(cos(r1), sin(r1)));
      }
    }
  }
  return resultVec;
}

}

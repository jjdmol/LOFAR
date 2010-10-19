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

#include <BBSKernel/MNS/MeqDFTPS.h>
#include <BBSKernel/MNS/MeqStatUVW.h>
#include <BBSKernel/MNS/MeqLMN.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqDFTPS::MeqDFTPS (const MeqExpr& lmn, MeqStatUVW* uvw)
: itsLMN (lmn),
  itsUVW (uvw)
{
  addChild (itsLMN);
}

MeqDFTPS::~MeqDFTPS()
{}

MeqResultVec MeqDFTPS::getResultVec (const MeqRequest& request)
{
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
  MeqResultVec lmnBuf;
  const MeqResultVec& lmn = itsLMN.getResultVecSynced (request, lmnBuf);
  const MeqResult& lrk  = lmn[0];
  const MeqResult& mrk  = lmn[1];
  const MeqResult& nrk  = lmn[2];
  MeqResult& result = resultVec[0];
  // Note that both UVW and LMN are scalars or vectors in time.
  // They always have nx==1 in their MeqMatrices.
  MeqMatrix r1 = (u*lrk.getValue() + v*mrk.getValue() +
          w*(nrk.getValue() - 1.)) * wavel0;
  result.setValue (tocomplex(cos(r1), sin(r1)));
  if (calcDelta) {
    MeqResult& delta = resultVec[1];
    r1 *= dwavel;
    delta.setValue (tocomplex(cos(r1), sin(r1)));
  }

  // Evaluate (if needed) for the perturbed parameter values.
  const MeqParmFunklet* perturbedParm;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval = false;
    if (lrk.isDefined(spinx)) {
      perturbedParm = lrk.getPerturbedParm(spinx);
      eval = true;
    } else if (mrk.isDefined(spinx)) {
      perturbedParm = mrk.getPerturbedParm(spinx);
      eval = true;
    } else if (nrk.isDefined(spinx)) {
      perturbedParm = nrk.getPerturbedParm(spinx);
      eval = true;
    } else if (resU.isDefined(spinx)) {
      perturbedParm = resU.getPerturbedParm(spinx);
      eval = true;
    } else if (resV.isDefined(spinx)) {
      perturbedParm = resV.getPerturbedParm(spinx);
      eval = true;
    } else if (resW.isDefined(spinx)) {
      perturbedParm = resW.getPerturbedParm(spinx);
      eval = true;
    }
    if (eval) {
      r1 = (resU.getPerturbedValue(spinx) * lrk.getPerturbedValue(spinx) +
        resV.getPerturbedValue(spinx) * mrk.getPerturbedValue(spinx) +
        resW.getPerturbedValue(spinx) *
        (nrk.getPerturbedValue(spinx) - 1.)) * wavel0;
      result.setPerturbedValue (spinx, tocomplex(cos(r1), sin(r1)));
      result.setPerturbedParm (spinx, perturbedParm);
      if (calcDelta) {
    MeqResult& delta = resultVec[1];
    r1 *= dwavel;
    delta.setPerturbedValue (spinx, tocomplex(cos(r1), sin(r1)));
      }
    }
  }
  return resultVec;
}

#ifdef EXPR_GRAPH
std::string MeqDFTPS::getLabel()
{
    return std::string("MeqDFTPS\\nStation DFT of a point source\\n" + itsUVW->getStation()->getName());
}
#endif

} // namespace BBS
} // namespace LOFAR

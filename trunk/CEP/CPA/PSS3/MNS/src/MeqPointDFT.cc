//# MeqPointDFT.cc: Class doing the DFT for a point source prediction
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

#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqPointDFT.h>
#include <MNS/MeqUVWPolc.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqMatrixTmp.h>
#include <aips/Measures/MDirection.h>
#include <aips/Mathematics/Constants.h>


MeqPointDFT::MeqPointDFT (const vector<MeqPointSource>& sources,
			  const MDirection& phaseRef,
			  MeqUVWPolc* uvw)
: itsSources (sources),
  itsUVW     (uvw)
{
  Quantum<Vector<Double> > angles = phaseRef.getAngle();
  itsRefRa  = angles.getBaseValue()(0);
  itsRefDec = angles.getBaseValue()(1);
}

vector<int> MeqPointDFT::ncells (const MeqPointSource&,
				 const MeqDomain&) const
{
  return vector<int> (2, 10);
}

MeqResult MeqPointDFT::getResult (const MeqRequest& request)
{
  MeqPointSource& src = itsSources[request.getSourceNr()];
  itsUVW->calcUVW (request);
  MeqMatrix u = itsUVW->getU().getValue();
  MeqMatrix v = itsUVW->getV().getValue();
  MeqMatrix w = itsUVW->getW().getValue();
  MeqResult rak  = src.getRa()->getResult(request);
  MeqResult deck = src.getDec()->getResult(request);
  MeqMatrix lk = rak.getValue() - itsRefRa;
  MeqMatrix mk = deck.getValue() - itsRefDec;
  MeqMatrix nk = sqrt(MeqMatrixTmp(1.) - sqr(lk) - sqr(mk));
  MeqResult result(request.nspid());
  result.setValue (exp(tocomplex(0, C::_2pi * (u*lk + v*mk + w*nk) / nk)));

  // Evaluate (if needed) for the perturbed parameter values.
  // Only RA and DEC can be perturbed.
  // In the future that might also be the case for UVW.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool evalr = rak.isDefined(spinx);
    bool evald = deck.isDefined(spinx);
    if (evalr || evald) {
      MeqMatrix lkp(lk);
      MeqMatrix mkp(mk);
      if (evalr) {
	lkp = rak.getPerturbedValue(spinx) - itsRefRa;
	perturbation = rak.getPerturbation(spinx);
      }
      if (evald) {
	lkp = deck.getPerturbedValue(spinx) - itsRefRa;
	perturbation = deck.getPerturbation(spinx);
      }
      nk = sqrt(1 - sqr(lkp)- sqr(mkp));
      result.setPerturbedValue
	(spinx, exp(tocomplex(0, C::_2pi * (u*lk + v*mk + w*nk) / nk)));
      result.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

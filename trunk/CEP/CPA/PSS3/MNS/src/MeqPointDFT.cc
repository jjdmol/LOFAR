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
#include <Common/Debug.h>
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
  cout << "MeqPointDFT phaseRef: " << itsRefRa << ' ' << itsRefDec << endl;
}

MeqPointDFT::~MeqPointDFT()
{}

vector<int> MeqPointDFT::ncells (const MeqPointSource&,
				 const MeqDomain&) const
{
  ///  return vector<int> (2, 10);
  return vector<int> (2, 2);
}

MeqResult MeqPointDFT::getResult (const MeqRequest& request)
{
  const MeqDomain& domain = request.domain();
  MeqPointSource& src = itsSources[request.getSourceNr()];
  itsUVW->calcUVW (request);
  MeqMatrix u = itsUVW->getU().getValue();
  MeqMatrix v = itsUVW->getV().getValue();
  MeqMatrix w = itsUVW->getW().getValue();
  MeqResult rak  = src.getRa()->getResult(request);
  MeqResult deck = src.getDec()->getResult(request);
  MeqMatrix lk = cos(deck.getValue()) * posdiff (rak.getValue(), itsRefRa);
  MeqMatrix mk = posdiff (deck.getValue(), itsRefDec);
  MeqMatrixTmp nks = MeqMatrixTmp(1.) - sqr(lk) - sqr(mk);
  AssertStr (min(nks).getDouble() > 0, "source " << request.getSourceNr()
	     << " too far from phaseref " << itsRefRa << ", " << itsRefDec);
  MeqMatrix nk = sqrt(nks);
  ///  cout << "MeqPointDFT u: " << u << endl;
    ///  cout << "MeqPointDFT v: " << v << endl;
    ///  cout << "MeqPointDFT w: " << w << endl;
    ///  cout << "MeqPointDFT lk: " << lk << endl;
    ///  cout << "MeqPointDFT mk: " << mk << endl;
    ///  cout << "MeqPointDFT nk: " << nk << endl;
  MeqResult result(request.nspid());
  MeqMatrixTmp tmp = u*lk + v*mk + w*nk;
  MeqMatrix res(complex<double>(0,0), request.nx(), request.ny(), false);
  const double* tmpdata = tmp.doubleStorage();
  complex<double>* resdata = res.dcomplexStorage();
  double freq = domain.startY();
  for (int j=0; j<request.ny(); j++) {
    double wavel = C::_2pi * freq / C::c;          // 2*pi/wavelength
    const double* tmpd = tmpdata;
    for (int i=0; i<request.nx(); i++) {
      *resdata++ = complex<double> (0, wavel * *tmpd++);
    }
    freq += request.stepY();
  }
  ///  cout << "Res: " << res << endl;
  result.setValue (exp(res) / nk);

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
      tmp = u*lkp + v*mkp + w*nk;
      resdata = res.dcomplexStorage();
      double freq = domain.startY();
      for (int j=0; j<request.ny(); j++) {
	double wavel = C::_2pi * freq / C::c;          // 2*pi/wavelength
	const double* tmpd = tmpdata;
	for (int i=0; i<request.nx(); i++) {
	  *resdata++ = complex<double> (0, *tmpd++ * wavel);
	}
	freq += request.stepY();
      }
      result.setPerturbedValue (spinx, exp(res) / nk);
      result.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

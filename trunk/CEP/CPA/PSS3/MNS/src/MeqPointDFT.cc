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

bool MeqPointDFT::doshow = false;


MeqPointDFT::MeqPointDFT (const vector<MeqPointSource>& sources,
			  const MDirection& phaseRef,
			  MeqUVWPolc* uvw)
: itsSources (sources),
  itsUVW     (uvw)
{
  Quantum<Vector<Double> > angles = phaseRef.getAngle();
  itsRefRa  = angles.getBaseValue()(0);
  itsRefDec = angles.getBaseValue()(1);
  itsCosRefDec = cos(itsRefDec);
  itsSinRefDec = sin(itsRefDec);
  // cout << "MeqPointDFT phaseRef: " << itsRefRa << ' ' << itsRefDec << endl;
}

MeqPointDFT::~MeqPointDFT()
{}

vector<int> MeqPointDFT::ncells (MeqPointSource& src,
				 const MeqDomain& domain)
{
  MeqRequest request (domain, 1, 1, 0);
  vector<int> ncell(2,1);
  double u = itsUVW->getU().getValue().getDouble();
  double v = itsUVW->getV().getValue().getDouble();
  double w = itsUVW->getW().getValue().getDouble();
  double rak  = src.getRa()->getResult(request).getValue().getDouble();
  double deck  = src.getDec()->getResult(request).getValue().getDouble();
  double cosdec = cos(deck);
  double radiff = rak - itsRefRa;
  double lk = cosdec * sin(radiff);
  double mk = sin(deck)*itsCosRefDec - cosdec*itsSinRefDec*cos(radiff);
  double nks = 1. - lk*lk - mk*mk;
  AssertStr (nks > 0, "source too far from phaseref "
             << itsRefRa << ", " << itsRefDec);
  double nk = sqrt(nks);
  double tmpphi = C::_2pi * abs(u*lk + v*mk + w*nk);
  double cellWidth = C::c / (2*tmpphi);
  ncell[1] = 2*(1 + int((domain.endY() - domain.startY()) / cellWidth));
    //  double stphi = 2 * tmpphi * domain.startY() / C::c;
    //  double endphi = 2 * tmpphi * domain.endY() / C::c;
    //  ncell[1] = 10 * (1 + int(abs(endphi - stphi)));
  if (doshow) {
    cout << "MeqPointDFT: " << u << ' ' << v << ' ' << w << ' ' << rak << ' ' << deck << ' ' << ncell[0] << ' ' << ncell[1] << endl;
  }
  return ncell;
}

MeqResult MeqPointDFT::getResult (const MeqRequest& request)
{
  const MeqDomain& domain = request.domain();
  MeqPointSource& src = itsSources[request.getSourceNr()];
  MeqMatrix u = itsUVW->getU().getValue();
  MeqMatrix v = itsUVW->getV().getValue();
  MeqMatrix w = itsUVW->getW().getValue();
  MeqResult rak  = src.getRa()->getResult(request);
  MeqResult deck = src.getDec()->getResult(request);
  MeqMatrix cosdec = cos(deck.getValue());
  MeqMatrix radiff = rak.getValue() - itsRefRa;
  MeqMatrix lk = cosdec * sin(radiff);
  MeqMatrix mk = sin(deck.getValue())*itsCosRefDec -
                 cosdec*itsSinRefDec*cos(radiff);
  MeqMatrixTmp nks = MeqMatrixTmp(1.) - sqr(lk) - sqr(mk);
  AssertStr (min(nks).getDouble() > 0, "source " << request.getSourceNr()
	     << " too far from phaseref " << itsRefRa << ", " << itsRefDec);
  MeqMatrix nk = sqrt(nks);
  ///  cout << "lk=" << lk << ", mk=" << mk << ", nk=" << nk << endl;
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
  // Note that exp(ix) = cos(x) + i*sin(x)
  for (int j=0; j<request.ny(); j++) {
    double wavel = C::_2pi * freq / C::c;          // 2*pi/wavelength
    const double* tmpd = tmpdata;
    for (int i=0; i<request.nx(); i++) {
      double val = wavel * *tmpd++;
      *resdata++ = complex<double> (cos(val), sin(val));
      ///      if (i==0) {
	///	cout << "abs dft " << abs(*(resdata-1)) << endl;
	///      }
    }
    freq += request.stepY();
  }
  ///  cout << "Res: " << res << endl;
  result.setValue (res/nk);

  // Evaluate (if needed) for the perturbed parameter values.
  // Only RA and DEC can be perturbed.
  // In the future that might also be the case for UVW.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    if (rak.isDefined(spinx)  ||  deck.isDefined(spinx)) {
      MeqMatrix pcosdec = cosdec;
      MeqMatrix pradiff = radiff;
      if (rak.isDefined(spinx)) {
	perturbation = rak.getPerturbation(spinx);
	pradiff = rak.getPerturbedValue(spinx) - itsRefRa;
      }
      if (deck.isDefined(spinx)) {
	perturbation = deck.getPerturbation(spinx);
	pcosdec = cos(deck.getPerturbedValue(spinx));
      }
      lk = pcosdec * sin(pradiff);
      mk = sin(deck.getPerturbedValue(spinx))*itsCosRefDec -
	   pcosdec*itsSinRefDec*cos(pradiff);
      MeqMatrixTmp nks = MeqMatrixTmp(1.) - sqr(lk) - sqr(mk);
      AssertStr (min(nks).getDouble() > 0, "source " << request.getSourceNr()
	     << " too far from phaseref " << itsRefRa << ", " << itsRefDec);
      nk = sqrt(nks);
      tmp = u*lk + v*mk + w*nk;
      tmpdata = tmp.doubleStorage();
      resdata = res.dcomplexStorage();
      double freq = domain.startY();
      for (int j=0; j<request.ny(); j++) {
	double wavel = C::_2pi * freq / C::c;          // 2*pi/wavelength
	const double* tmpd = tmpdata;
	for (int i=0; i<request.nx(); i++) {
	  double val = wavel * *tmpd++;
	  *resdata++ = complex<double> (cos(val), sin(val));
	}
	freq += request.stepY();
      }
      result.setPerturbedValue (spinx, res/nk);
      result.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

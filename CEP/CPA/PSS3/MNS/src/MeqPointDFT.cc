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

#include <MNS/MeqPointDFT.h>
#include <MNS/MeqStatSources.h>
#include <MNS/MeqStatUVW.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Mathematics/Constants.h>


bool MeqPointDFT::doshow = false;


MeqPointDFT::MeqPointDFT (MeqStatSources* left, MeqStatSources* right)
: itsLeft  (left),
  itsRight (right)
{}

MeqPointDFT::~MeqPointDFT()
{}

vector<int> MeqPointDFT::ncells (int sourceNr,
				 const MeqRequest& request)
{
  const MeqDomain& domain = request.domain();
  int ncell = 1;
  double tmpphi = C::_2pi * (itsLeft->getExponent(sourceNr, request) -
			     itsRight->getExponent(sourceNr, request));
  double cellWidth = C::c / (2*tmpphi);
  double step = request.stepY();
  double freq0 = domain.startY();
  double freq1 = freq0 + step;
  for (int i=0; i<request.ny(); i++) {
    //int nc = 2*(1 + int((freq1 - freq0) / cellWidth));
    int nc = (1 + int((freq1 - freq0) / cellWidth));
    if (nc > ncell) {
      ncell = nc;
    }
    freq0 = freq1;
    freq1 += step;
  }
  vector<int> nc(2,1);
  nc[1] = ncell;
  return nc;
}

MeqResult MeqPointDFT::getResult (const MeqRequest& request)
{
  const MeqDomain& domain = request.domain();
  MeqResult result(request.nspid());
  const MeqResult& left = itsLeft->getResult (request);
  const MeqResult& right = itsRight->getResult (request);
  const MeqResult& nk = itsLeft->getN (request);
  // dft = exp(2i.pi(ul+vm+wn)) / n                 (with u,v,w in wavelengths)
  //     = (exp(2i.pi((u1.l+v1.m+w1.n) - (u2.l+v2.m+w2.n))/wvl))/n (u,v,w in m)
  //     = ((exp(i(u1.l+v1.m+w1.n)) / exp(i(u2.l+v2.m+w2.m))) ^ (2.pi/wvl)) / n
  // So left and right return the exp values independent of wavelength.
  // Thereafter they are scaled to the freq domain by raising the values
  // for each time to the appropriate powers.
  // That is expensive. However, because the frequencies are regularly
  // strided, it is possible to use multiplication.
  // So it gets
  // dft(f0) = ((left/right)^(2pi.f0/c)) / n
  // dft(fj) = ((left/right)^(2pi.f0/c + 2pi.j.df/c)) / n
  //         = dft(fj-1) * (left/right)^(2pi.df/c)
  {
    const MeqResult& ul = itsLeft->uvw().getU(request);
    const MeqResult& vl = itsLeft->uvw().getV(request);
    const MeqResult& wl = itsLeft->uvw().getW(request);
    const MeqResult& ur = itsRight->uvw().getU(request);
    const MeqResult& vr = itsRight->uvw().getV(request);
    const MeqResult& wr = itsRight->uvw().getW(request);
    //cout << "U: " << ur.getValue() - ul.getValue() << endl;
    //cout << "V: " << vr.getValue() - vl.getValue() << endl;
    //cout << "W: " << wr.getValue() - wl.getValue() << endl;
    TRACER1 ("U: " << ur.getValue() - ul.getValue());
    TRACER1 ("V: " << vr.getValue() - vl.getValue());
    TRACER1 ("W: " << wr.getValue() - wl.getValue());
  }
  const complex<double>* tmpl = left.getValue().dcomplexStorage();
  const complex<double>* tmpr = right.getValue().dcomplexStorage();
  const double* tmpnk = nk.getValue().doubleStorage();
  double wavel0 = C::_2pi * domain.startY() / C::c;       // 2*pi/wavelength
  double dwavel = C::_2pi * request.stepY() / C::c;
  MeqMatrix res(complex<double>(0,0), request.nx(), request.ny(), false);
  complex<double>* resdata = res.dcomplexStorage();
  complex<double> dval, val0;
  for (int i=0; i<request.nx(); i++) {
    complex<double> val (tmpr[i] / tmpl[i]);
    if (nk.getValue().nx() == 1) {
      val0 = pow(val, wavel0) / tmpnk[0];
    } else {
      val0 = pow(val, wavel0) / tmpnk[i];
    }
    resdata[i] = val0;
    if (request.ny() > 1) {
      dval = pow(val, dwavel);
      for (int j=1; j<request.ny(); j++) {
	val0 *= dval;
	resdata[i + j*request.nx()] = val0;
      }
    }
  }
      ///      if (i==0) {
	///	cout << "abs dft " << abs(*(resdata-1)) << endl;
	///      }
  ///  cout << "Res: " << res << endl;
  result.setValue (res);

  // Evaluate (if needed) for the perturbed parameter values.
  // Note that we do not have to test for perturbed values in nk,
  // because the left and right value already depend on nk.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    if (left.isDefined(spinx)  ||  right.isDefined(spinx)) {
      if (left.isDefined(spinx)) {
	perturbation = left.getPerturbation(spinx);
      } else {
	perturbation = right.getPerturbation(spinx);
      }
      tmpl = left.getPerturbedValue(spinx).dcomplexStorage();
      tmpr = right.getPerturbedValue(spinx).dcomplexStorage();
      tmpnk = nk.getPerturbedValue(spinx).doubleStorage();
      for (int i=0; i<request.nx(); i++) {
	complex<double> val (tmpr[i] / tmpl[i]);
	if (nk.getValue().nx() == 1) {
	  val0 = pow(val, wavel0) / tmpnk[0];
	} else {
	  val0 = pow(val, wavel0) / tmpnk[i];
	}
	resdata[i] = val0;
	if (request.ny() > 1) {
	  dval = pow(val, dwavel);
	  for (int j=1; j<request.ny(); j++) {
	    val0 *= dval;
	    resdata[i + j*request.nx()] = val0;
	  }
	}
      }
      result.setPerturbedValue (spinx, res);
      result.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

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

#include <PSS3/MNS/MeqPointDFT.h>
#include <PSS3/MNS/MeqStatSources.h>
#include <PSS3/MNS/MeqStatUVW.h>
#include <PSS3/MNS/MeqRequest.h>
#include <PSS3/MNS/MeqResult.h>
#include <PSS3/MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR {

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
    int nc = 2*(1 + int((freq1 - freq0) / cellWidth));
    //int nc = (1 + int((freq1 - freq0) / cellWidth));
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
  // A delta in the station source predict is only available if multiple
  // frequency channels are used.
  bool multFreq = request.ny() > 1;
  MeqResult result(request.nspid());
  const MeqResult& left = itsLeft->getResult (request);
  const MeqResult& right = itsRight->getResult (request);
  const MeqResult& leftDelta = itsLeft->getDelta (request);
  const MeqResult& rightDelta = itsRight->getDelta (request);
  const MeqResult& nk = itsLeft->getN (request);
  // It is tried to compute the DFT as efficient as possible.
  // Therefore the baseline contribution is split into its antenna parts.
  // dft = exp(2i.pi(ul+vm+wn)) / n                 (with u,v,w in wavelengths)
  //     = (exp(2i.pi((u1.l+v1.m+w1.n) - (u2.l+v2.m+w2.n))/wvl))/n (u,v,w in m)
  //     = ((exp(i(u1.l+v1.m+w1.n)) / exp(i(u2.l+v2.m+w2.m))) ^ (2.pi/wvl)) / n
  // So left and right return the exp values independent of wavelength.
  // Thereafter they are scaled to the freq domain by raising the values
  // for each time to the appropriate powers.
  // Alas the rule
  //   x^(a*b) = (x^a)^b
  // which is valid for real numbers, is only valid for complex numbers
  // if b is an integer number.
  // Therefore the station calculations (in MeqStatSources) are done as
  // follows, where it should be noted that the frequencies are regularly
  // strided.
  //  f = f0 + k.df   (k = 0 ... nchan-1)
  //  s1 = (u1.l+v1.m+w1.n).2i.pi/c
  //  s2 = (u2.l+v2.m+w2.n).2i.pi/c
  //  dft = exp(s1(f0+k.df)) / exp(s2(f0+k.df)) / n
  //      = (exp(s1.f0)/exp(s2.f0)) . (exp(s1.k.df)/exp(s2.k.df)) / n
  //      = (exp(s1.f0)/exp(s2.f0)) . (exp(s1.df)/exp(s2.df))^k / n
  // In principle the power is expensive, but because the frequencies are
  // regularly strided, it is possible to use multiplication.
  // So it gets
  // dft(f0) = (exp(s1.f0)/exp(s2.f0)) / n
  // dft(fj) = dft(fj-1) * (exp(s1.df)/exp(s2.df))
  // Using a python script (tuvw.py) is is checked that this way of
  // calculation is accurate enough.
  // Another optimization can be achieved in the division of the two
  // complex numbers which can be turned into a cheaper multiplication.
  //  exp(x)/exp(y) = (cos(x) + i.sin(x)) / (cos(y) + i.sin(y))
  //                = (cos(x) + i.sin(x)) * (cos(y) - i.sin(y))
  {
    //const MeqResult& ul = itsLeft->uvw().getU(request);
    //const MeqResult& vl = itsLeft->uvw().getV(request);
    //const MeqResult& wl = itsLeft->uvw().getW(request);
    //const MeqResult& ur = itsRight->uvw().getU(request);
    //const MeqResult& vr = itsRight->uvw().getV(request);
    //const MeqResult& wr = itsRight->uvw().getW(request);
    //cout << "U: " << ur.getValue() - ul.getValue() << endl;
    //cout << "V: " << vr.getValue() - vl.getValue() << endl;
    //cout << "W: " << wr.getValue() - wl.getValue() << endl;
    //TRACER1 ("U: " << ur.getValue() - ul.getValue());
    //TRACER1 ("V: " << vr.getValue() - vl.getValue());
    //TRACER1 ("W: " << wr.getValue() - wl.getValue());
  }
  const complex<double>* tmpl = left.getValue().dcomplexStorage();
  const complex<double>* tmpr = right.getValue().dcomplexStorage();
  const complex<double>* deltal = 0;
  const complex<double>* deltar = 0;
  if (multFreq) {
    deltal = leftDelta.getValue().dcomplexStorage();
    deltar = rightDelta.getValue().dcomplexStorage();
  }
  MeqMatrix res(complex<double>(0,0), request.nx(), request.ny(), false);
  complex<double>* resdata = res.dcomplexStorage();
  complex<double> dval, val0;
  // Note that some optimization can be achieved here, because usually
  // request.nx()==1. 'Roll out' the loop by making a special case for nx==1
  // so it can use tmpnk[0], deltal[0], etc. and does not need nx in
  // the resdata addressing.
  // A similar optimization in the perturbed value calculation.
  const double* tmpnk = nk.getValue().doubleStorage();
  //if request.nx() == 1) {
  //  val0 = tmpr[0] * conj(tmpl[0]) / tmpnk[0]);
  //  resdata[0] = val0;
  //  if (multFreq) {
  //    dval = deltar[0] / deltal[0];
  //    for (int j=1; j<request.ny(); j++) {
  //	  val0 *= dval;
  //	  resdata[j] = val0;
  //    }
  //  }
  //} else {
  // nk can be a scalar or an array (in time axis), so set its step to 0
  // if it is a scalar.
  int stepnk = (nk.getValue().nx() > 1  ?  1 : 0);
  int nki = 0;
  for (int i=0; i<request.nx(); i++) {
    val0 = tmpr[i] * conj(tmpl[i]) / tmpnk[nki];
    nki += stepnk;
    resdata[i] = val0;
    if (multFreq) {
      dval = deltar[i] * conj(deltal[i]);
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
      if (multFreq) {
	deltal = leftDelta.getPerturbedValue(spinx).dcomplexStorage();
	deltar = rightDelta.getPerturbedValue(spinx).dcomplexStorage();
      }
      MeqMatrix pres(complex<double>(0,0), request.nx(), request.ny(), false);
      complex<double>* presdata = pres.dcomplexStorage();
      tmpnk = nk.getPerturbedValue(spinx).doubleStorage();
      nki = 0;
      for (int i=0; i<request.nx(); i++) {
	val0 = tmpr[i] * conj(tmpl[i]) / tmpnk[nki];
	nki += stepnk;
	presdata[i] = val0;
	if (multFreq) {
	  dval = deltar[i] * conj(deltal[i]);
	  for (int j=1; j<request.ny(); j++) {
	    val0 *= dval;
	    presdata[i + j*request.nx()] = val0;
	  }
	}
      }
      result.setPerturbedValue (spinx, pres);
      result.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

}

//# MeqBaseDFTPS.cc: Baseline prediction of a point source with linear polarisation
//#
//# Copyright (C) 2005
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

#include <BBS3/MNS/MeqBaseDFTPS.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR {

MeqBaseDFTPS::MeqBaseDFTPS (const MeqExpr& left, const MeqExpr& right,
			    MeqLMN* lmn)
  : itsLeft  (left),
    itsRight (right),
    itsLMN   (lmn)
{}

MeqBaseDFTPS::~MeqBaseDFTPS()
{}

MeqResult MeqBaseDFTPS::getResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  // We can only calculate for a single time bin.
  ASSERT (request.ny() == 1);
  MeqResult result(request.nspid());

  // Get N (for the division).
  // Assert it is a scalar value.
  MeqResultVec lmn = itsLMN->getResultVec (request);
  const MeqResult& nk = lmn[2];
  ASSERT (nk.getValue().nelements() == 1);
  // Calculate the left and right station Jones matrix elements.
  // A delta in the station source predict is only available if multiple
  // frequency channels are used.
  bool multFreq = request.nx() > 1;
  MeqResultVec resl = itsLeft.getResultVec (request);
  MeqResultVec resr = itsRight.getResultVec (request);
  const MeqResult& left = resl[0];
  const MeqResult& right = resr[0];
  const MeqResult& leftDelta = resl[1];
  const MeqResult& rightDelta = resr[1];

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
      //LOG_TRACE_FLOW ("U: " << ur.getValue() - ul.getValue());
      //LOG_TRACE_FLOW ("V: " << vr.getValue() - vl.getValue());
      //LOG_TRACE_FLOW ("W: " << wr.getValue() - wl.getValue());
    }
  dcomplex tmpl = left.getValue().getDComplex();
  dcomplex tmpr = right.getValue().getDComplex();
  dcomplex deltal;
  dcomplex deltar;
  if (multFreq) {
    deltal = leftDelta.getValue().getDComplex();
    deltar = rightDelta.getValue().getDComplex();
  }
  MeqMatrix res(makedcomplex(0,0), request.nx(), request.ny(), false);
  dcomplex* resdata = res.dcomplexStorage();
  dcomplex dval, val0;
  // We have to divide by N.
  // However, we divide by 2N to get the factor 0.5 needed in (I+Q)/2, etc.
  // in MeqBaseLinPS.
  double tmpnk = 2. * nk.getValue().getDouble();
  val0 = tmpr * conj(tmpl) / tmpnk;
  *resdata++ = val0;
  if (multFreq) {
    dval = deltar * conj(deltal);
    for (int j=1; j<request.nx(); j++) {
      val0 *= dval;
      *resdata++ = val0;
    }
  }
  result.setValue (res);

  // Evaluate (if needed) for the perturbed parameter values.
  // Note that we do not have to test for perturbed values in nk,
  // because the left and right value already depend on nk.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval = false;
    if (left.isDefined(spinx)) {
      eval = true;
      perturbation = left.getPerturbation(spinx);
    } else if (right.isDefined(spinx)) {
      eval = true;
      perturbation = right.getPerturbation(spinx);
    }
    if (eval) {
      tmpl = left.getPerturbedValue(spinx).getDComplex();
      tmpr = right.getPerturbedValue(spinx).getDComplex();
      if (multFreq) {
	deltal = leftDelta.getPerturbedValue(spinx).getDComplex();
	deltar = rightDelta.getPerturbedValue(spinx).getDComplex();
      }
      MeqMatrix pres(makedcomplex(0,0), request.nx(), 1, false);
      dcomplex* presdata = pres.dcomplexStorage();
      tmpnk = 2. * nk.getPerturbedValue(spinx).getDouble();
      val0 = tmpr * conj(tmpl) / tmpnk;
      *presdata++ = val0;
      if (multFreq) {
	dval = deltar * conj(deltal);
	for (int j=1; j<request.nx(); j++) {
	  val0 *= dval;
	  *presdata++ = val0;
	}
      }
      result.setPerturbedValue (spinx, pres);
      result.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

}

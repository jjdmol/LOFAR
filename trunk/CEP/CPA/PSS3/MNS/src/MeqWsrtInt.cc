//# MeqWsrtInt.cc: The integrated point source contribution
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

#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqWsrtPoint.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>

#include <MNS/MeqPointDFT.h>


MeqWsrtInt::MeqWsrtInt (MeqJonesExpr* vis, MeqJonesExpr* station1,
			MeqJonesExpr* station2)
: itsExpr  (vis),
  itsStat1 (station1),
  itsStat2 (station2)
{}

MeqWsrtInt::~MeqWsrtInt()
{}

void MeqWsrtInt::calcResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  // We can handle only 1 time at a time (for PSS-1 at least).
  // It makes life much easier.
  Assert (request.nx() == 1);
  // Create the result objects.
  setResult11 (MeqResult(request.nspid()));
  setResult12 (MeqResult(request.nspid()));
  setResult21 (MeqResult(request.nspid()));
  setResult22 (MeqResult(request.nspid()));
  // Allocate a complex matrix of the right size in the results.
  Matrix<complex<double> > mat(request.nx(), request.ny());
  result11().setValue (mat);
  result12().setValue (mat);
  result21().setValue (mat);
  result22().setValue (mat);
  // Calculate the contribution of all sources.
  // A cell may be split up in smaller subcells (both in time and
  // frequency direction), so the matrix returned might be bigger
  // and integration is required.
  itsExpr->calcResult (request);
  itsStat1->calcResult (request);
  itsStat2->calcResult (request);
  const MeqResult& xx = itsExpr->getResult11();
  const MeqResult& xy = itsExpr->getResult12();
  const MeqResult& yx = itsExpr->getResult21();
  const MeqResult& yy = itsExpr->getResult22();
  if (MeqPointDFT::doshow) {
    cout << "MeqWsrtInt " << request.nx() <<  ' ' << request.ny() << endl;
    cout << "MeqWsrtInt xx: " << xx.getValue() << endl;
  }
  // Integrate and normalize the results by adding the values and
  // dividing by the number of subcells used to predict a single cell.
  // The nr of subcells is the same for each cell.
  double nsubc = xx.getValue().nelements() / request.ny();
  if (MeqPointDFT::doshow) {
    cout << "Int: " << xx.getValue() << ' ' << xy.getValue() << ' '
	 << yx.getValue() << ' ' << yy.getValue() << endl;
  }
  MeqMatrix xxres;
  MeqMatrix xyres;
  MeqMatrix yxres;
  MeqMatrix yyres;
  double fact = 1./nsubc;
  if (nsubc == request.ny()) {
    xxres = xx.getValue();
    xyres = xy.getValue();
    yxres = yx.getValue();
    yyres = yy.getValue();
  } else {
    const complex<double>* xxc = xx.getValue().dcomplexStorage();
    const complex<double>* xyc = xy.getValue().dcomplexStorage();
    const complex<double>* yxc = yx.getValue().dcomplexStorage();
    const complex<double>* yyc = yy.getValue().dcomplexStorage();
    xxres = MeqMatrix(complex<double>(), 1, request.ny(), false);
    xyres = MeqMatrix(complex<double>(), 1, request.ny(), false);
    yxres = MeqMatrix(complex<double>(), 1, request.ny(), false);
    yyres = MeqMatrix(complex<double>(), 1, request.ny(), false);
    complex<double>* xxr = xxres.dcomplexStorage();
    complex<double>* xyr = xyres.dcomplexStorage();
    complex<double>* yxr = yxres.dcomplexStorage();
    complex<double>* yyr = yyres.dcomplexStorage();
#if 0
    for (int i=0; i<request.ny(); i++) {
      complex<double> sumxx;
      complex<double> sumxy;
      complex<double> sumyx;
      complex<double> sumyy;
      for (int j=0; j<nsubc; j++) {
	sumxx += *xxc++;
	sumxy += *xyc++;
	sumyx += *yxc++;
	sumyy += *yyc++;
      }
      xxr[i] = sumxx * fact;
      xyr[i] = sumxy * fact;
      yxr[i] = sumyx * fact;
      yyr[i] = sumyy * fact;
      if (MeqPointDFT::doshow) {
	cout << "MeqWsrtInt abs(sum): " << abs(sumxx) << ' ' << abs(sumxy) << ' ' << abs(sumyx) << ' ' << abs(sumyy) << endl;
      }
    }
#else
    int i,j;
    for (i=0; i<request.ny(); i++) {
      complex<double> sumxx;
      for (j=0; j<nsubc; j++) {
	sumxx += *xxc++;
      }
      xxr[i] = sumxx * fact;
    }
    for (i=0; i<request.ny(); i++) {
      complex<double> sumxy;
      for (j=0; j<nsubc; j++) {
	sumxy += *xyc++;
      }
      xyr[i] = sumxy * fact;
    }
    for (i=0; i<request.ny(); i++) {
      complex<double> sumyx;
      for (j=0; j<nsubc; j++) {
	sumyx += *yxc++;
      }
      yxr[i] = sumyx * fact;
    }
    for (i=0; i<request.ny(); i++) {
      complex<double> sumyy;
      for (j=0; j<nsubc; j++) {
	sumyy += *yyc++;
      }
      yyr[i] = sumyy * fact;
    }
#endif
  }
  // Now combine with the stations jones.
  MeqMatrix s11 = itsStat1->getResult11().getValue();
  MeqMatrix s12 = itsStat1->getResult12().getValue();
  MeqMatrix s21 = itsStat1->getResult21().getValue();
  MeqMatrix s22 = itsStat1->getResult22().getValue();
  MeqMatrix conj11 = conj(itsStat2->getResult11().getValue());
  MeqMatrix conj12 = conj(itsStat2->getResult12().getValue());
  MeqMatrix conj21 = conj(itsStat2->getResult21().getValue());
  MeqMatrix conj22 = conj(itsStat2->getResult22().getValue());
  result11().getValueRW() =
	s11 * conj11 * xxres +
	s11 * conj12 * xyres +
	s12 * conj11 * yxres +
	s12 * conj12 * yyres;
  result12().getValueRW() =
	s11 * conj21 * xxres +
	s11 * conj22 * xyres +
	s12 * conj21 * yxres +
	s12 * conj22 * yyres;
  result21().getValueRW() =
	s21 * conj11 * xxres +
	s21 * conj12 * xyres +
	s22 * conj11 * yxres +
	s22 * conj12 * yyres;
  result22().getValueRW() =
	s21 * conj21 * xxres +
	s21 * conj22 * xyres +
	s22 * conj21 * yxres +
	s22 * conj22 * yyres;

  // Determine which values are perturbed and determine the perturbation.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval = false;
    bool evalxx = false;
    bool evalxy = false;
    bool evalyx = false;
    bool evalyy = false;
    if (xx.isDefined(spinx)) {
      perturbation = xx.getPerturbation(spinx);
      eval = true;
    }
    if (xy.isDefined(spinx)) {
      perturbation = xy.getPerturbation(spinx);
      eval = true;
    }
    if (yx.isDefined(spinx)) {
      perturbation = yx.getPerturbation(spinx);
      eval = true;
    }
    if (yy.isDefined(spinx)) {
      perturbation = yy.getPerturbation(spinx);
      eval = true;
    }
    if (itsStat1->getResult11().isDefined(spinx)) {
      perturbation = itsStat1->getResult11().getPerturbation(spinx);
      evalxx = true;
      evalxy = true;
    }
    if (itsStat1->getResult12().isDefined(spinx)) {
      perturbation = itsStat1->getResult12().getPerturbation(spinx);
      evalxx = true;
      evalxy = true;
    }
    if (itsStat1->getResult21().isDefined(spinx)) {
      perturbation = itsStat1->getResult21().getPerturbation(spinx);
      evalyx = true;
      evalyy = true;
    }
    if (itsStat1->getResult22().isDefined(spinx)) {
      perturbation = itsStat1->getResult22().getPerturbation(spinx);
      evalyx = true;
      evalyy = true;
    }
    if (itsStat2->getResult11().isDefined(spinx)) {
      perturbation = itsStat2->getResult11().getPerturbation(spinx);
      evalxx = true;
      evalyx = true;
    }
    if (itsStat2->getResult12().isDefined(spinx)) {
      perturbation = itsStat2->getResult12().getPerturbation(spinx);
      evalxx = true;
      evalyx = true;
    }
    if (itsStat2->getResult21().isDefined(spinx)) {
      perturbation = itsStat2->getResult21().getPerturbation(spinx);
      evalxy = true;
      evalyy = true;
    }
    if (itsStat2->getResult22().isDefined(spinx)) {
      perturbation = itsStat2->getResult22().getPerturbation(spinx);
      evalxy = true;
      evalyy = true;
    }
    if (eval) {
      evalxx = true;
      evalxy = true;
      evalyx = true;
      evalyy = true;
    } else {
      eval = evalxx || evalxy || evalyx || evalyy;
    }
    if (evalxx) {
      result11().setPerturbation (spinx, perturbation);
      result11().setPerturbedValue (spinx, mat);
    }
    if (evalxy) {
      result12().setPerturbation (spinx, perturbation);
      result12().setPerturbedValue (spinx, mat);
    }
    if (evalyx) {
      result21().setPerturbation (spinx, perturbation);
      result21().setPerturbedValue (spinx, mat);
    }
    if (evalyy) {
      result22().setPerturbation (spinx, perturbation);
      result22().setPerturbedValue (spinx, mat);
    }
    if (eval) {
      MeqMatrix pxxres = xxres;
      MeqMatrix pxyres = xyres;
      MeqMatrix pyxres = yxres;
      MeqMatrix pyyres = yyres;
      MeqMatrix ps11 = s11;
      MeqMatrix ps12 = s12;
      MeqMatrix ps21 = s21;
      MeqMatrix ps22 = s22;
      MeqMatrix pconj11 = conj11;
      MeqMatrix pconj12 = conj12;
      MeqMatrix pconj21 = conj21;
      MeqMatrix pconj22 = conj22;
      if (xx.isDefined(spinx)) {
	if (nsubc == request.ny()) {
	  pxxres = xx.getPerturbedValue(spinx);
	} else {
	  const complex<double>* dc =
	    xx.getPerturbedValue(spinx).dcomplexStorage();
	  pxxres = MeqMatrix(complex<double>(), 1, request.ny(), false);
	  complex<double>* dr = pxxres.dcomplexStorage();
	  for (int i=0; i<request.ny(); i++) {
	    complex<double> dsum;
	    for (int j=0; j<nsubc; j++) {
	      dsum += *dc++;
	    }
	    dr[i] = dsum * fact;
	  }
	}
      }
      if (xy.isDefined(spinx)) {
	if (nsubc == request.ny()) {
	  pxxres = xy.getPerturbedValue(spinx);
	} else {
	  const complex<double>* dc =
	    xy.getPerturbedValue(spinx).dcomplexStorage();
	  pxyres = MeqMatrix(complex<double>(), 1, request.ny(), false);
	  complex<double>* dr = pxyres.dcomplexStorage();
	  for (int i=0; i<request.ny(); i++) {
	    complex<double> dsum;
	    for (int j=0; j<nsubc; j++) {
	      dsum += *dc++;
	    }
	    dr[i] = dsum * fact;
	  }
	}
      }
      if (yx.isDefined(spinx)) {
	if (nsubc == request.ny()) {
	  pxxres = yx.getPerturbedValue(spinx);
	} else {
	  const complex<double>* dc =
	    yx.getPerturbedValue(spinx).dcomplexStorage();
	  pyxres = MeqMatrix(complex<double>(), 1, request.ny(), false);
	  complex<double>* dr = pyxres.dcomplexStorage();
	  for (int i=0; i<request.ny(); i++) {
	    complex<double> dsum;
	    for (int j=0; j<nsubc; j++) {
	      dsum += *dc++;
	    }
	    dr[i] = dsum * fact;
	  }
	}
      }
      if (yy.isDefined(spinx)) {
	if (nsubc == request.ny()) {
	  pxxres = yy.getPerturbedValue(spinx);
	} else {
	  const complex<double>* dc =
	    yy.getPerturbedValue(spinx).dcomplexStorage();
	  pyyres = MeqMatrix(complex<double>(), 1, request.ny(), false);
	  complex<double>* dr = pyyres.dcomplexStorage();
	  for (int i=0; i<request.ny(); i++) {
	    complex<double> dsum;
	    for (int j=0; j<nsubc; j++) {
	      dsum += *dc++;
	    }
	    dr[i] = dsum * fact;
	  }
	}
      }
      if (itsStat1->getResult11().isDefined(spinx)) {
	ps11 = itsStat1->getResult11().getPerturbedValue(spinx);
      }
      if (itsStat1->getResult12().isDefined(spinx)) {
	ps12 = itsStat1->getResult12().getPerturbedValue(spinx);
      }
      if (itsStat1->getResult21().isDefined(spinx)) {
	ps21 = itsStat1->getResult21().getPerturbedValue(spinx);
      }
      if (itsStat1->getResult22().isDefined(spinx)) {
	ps22 = itsStat1->getResult22().getPerturbedValue(spinx);
      }
      if (itsStat2->getResult11().isDefined(spinx)) {
	pconj11 = conj(itsStat2->getResult11().getPerturbedValue(spinx));
      }
      if (itsStat2->getResult12().isDefined(spinx)) {
	pconj12 = conj(itsStat2->getResult12().getPerturbedValue(spinx));
      }
      if (itsStat2->getResult21().isDefined(spinx)) {
	pconj21 = conj(itsStat2->getResult21().getPerturbedValue(spinx));
      }
      if (itsStat2->getResult22().isDefined(spinx)) {
	pconj22 = conj(itsStat2->getResult22().getPerturbedValue(spinx));
      }
      if (evalxx) {
	result11().getPerturbedValueRW(spinx) =
	  ps11 * pconj11 * pxxres +
	  ps11 * pconj12 * pxyres +
	  ps12 * pconj11 * pyxres +
	  ps12 * pconj12 * pyyres;
      }
      if (evalxy) {
	result12().getPerturbedValueRW(spinx) =
	  ps11 * pconj21 * pxxres +
	  ps11 * pconj22 * pxyres +
	  ps12 * pconj21 * pyxres +
	  ps12 * pconj22 * pyyres;
      }
      if (evalyx) {
	result21().getPerturbedValueRW(spinx) =
	  ps21 * pconj11 * pxxres +
	  ps21 * pconj12 * pxyres +
	  ps22 * pconj11 * pyxres +
	  ps22 * pconj12 * pyyres;
      }
      if (evalyy) {
	result22().getPerturbedValueRW(spinx) =
	  ps21 * pconj21 * pxxres +
	  ps21 * pconj22 * pxyres +
	  ps22 * pconj21 * pyxres +
	  ps22 * pconj22 * pyyres;
      }
    }
  }
}

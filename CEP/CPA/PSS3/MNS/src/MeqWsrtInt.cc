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

#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqWsrtPoint.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqMatrix.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>

#include <MNS/MeqPointDFT.h>


MeqWsrtInt::MeqWsrtInt (MeqWsrtPoint* expr, MeqJonesExpr* station1,
			MeqJonesExpr* station2)
: itsExpr  (expr),
  itsStat1 (station1),
  itsStat2 (station2)
{}

MeqWsrtInt::~MeqWsrtInt()
{}

void MeqWsrtInt::calcResult (const MeqRequest& request)
{
  const MeqDomain& domain = request.domain();
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
  vector<bool> eval(request.nspid(), false);
  vector<bool> evalxx(request.nspid(), false);
  vector<bool> evalxy(request.nspid(), false);
  vector<bool> evalyx(request.nspid(), false);
  vector<bool> evalyy(request.nspid(), false);
  // Loop through all the bins in the domain and get the source
  // contribution for a single bin. That bin will be split up in
  // smaller cells, so we get back a Matrix of values.
  // Integrate all those cells to the single bin.
  double sty = domain.startY();
  bool first = true;
  int inx = 0;
  for (int iy=0; iy<request.ny(); iy++) {
    double stx = domain.startX();
    for (int ix=0; ix<request.nx(); ix++) {
      MeqDomain dom(stx, stx+request.stepX(), sty, sty+request.stepY());
      MeqRequest req(dom, 1, 1, request.nspid());
      itsExpr->calcResult (req);
      itsStat1->calcResult (req);
      itsStat2->calcResult (req);
      const MeqResult& xx = itsExpr->getResultXX();
      const MeqResult& xy = itsExpr->getResultXY();
      const MeqResult& yx = itsExpr->getResultYX();
      const MeqResult& yy = itsExpr->getResultYY();
      if (MeqPointDFT::doshow) {
	cout << stx << ' ' << request.stepX() << ' ' << sty << ' ' << request.stepY() << ' ' << request.nx() <<  ' ' << request.ny() << endl;
      cout << "MeqWsrtInt xx: " << xx.getValue() << endl;
      }
      // Integrate and normalize the results by adding the values
      // and dividing by the number of cells.
      //// with the surface of the x,y cells and adding them thereafter.
      ///double wx = request.stepX() / xx.getValue().nx();
	///double wy = request.stepY() / xx.getValue().ny();
	///double surface = wx*wy;
      double nc = xx.getValue().nelements();
      // The values also have to be divided by 2 as that is not
      // done in MeqWsrtPoint because it is cheaper to do it here.
      nc *= 2;
      ///      if (MeqPointDFT::doshow) {
	///      cout << "Int: " << xx.getValue() << ' ' << xy.getValue() << ' '
	///      	   << yx.getValue() << ' ' << yy.getValue() << endl;
	///      }
      complex<double> sumxx = sum(xx.getValue()).getDComplex() / nc;
      complex<double> sumxy = sum(xy.getValue()).getDComplex() / nc;
      complex<double> sumyx = sum(yx.getValue()).getDComplex() / nc;
      complex<double> sumyy = sum(yy.getValue()).getDComplex() / nc;
      if (MeqPointDFT::doshow) {
      cout << "MeqWsrtInt abs(sum): " << abs(sumxx) << ' ' << abs(sumxy) << ' ' << abs(sumyx) << ' ' << abs(sumyy) << endl;
      }
      // Now combine with the stations jones.
      complex<double> s11 = itsStat1->getResult11().getValue().getDComplex();
      complex<double> s12 = itsStat1->getResult12().getValue().getDComplex();
      complex<double> s21 = itsStat1->getResult21().getValue().getDComplex();
      complex<double> s22 = itsStat1->getResult22().getValue().getDComplex();
      complex<double> conj11 = conj(itsStat2->getResult11().getValue().getDComplex());
      complex<double> conj12 = conj(itsStat2->getResult12().getValue().getDComplex());
      complex<double> conj21 = conj(itsStat2->getResult21().getValue().getDComplex());
      complex<double> conj22 = conj(itsStat2->getResult22().getValue().getDComplex());
      result11().getValueRW().dcomplexStorage()[inx] =
	s11 * conj11 * sumxx +
	s11 * conj12 * sumxy +
	s12 * conj11 * sumyx +
	s12 * conj12 * sumyy;
      result12().getValueRW().dcomplexStorage()[inx] =
	s11 * conj21 * sumxx +
	s11 * conj22 * sumxy +
	s12 * conj21 * sumyx +
	s12 * conj22 * sumyy;
      result21().getValueRW().dcomplexStorage()[inx] =
	s21 * conj11 * sumxx +
	s21 * conj12 * sumxy +
	s22 * conj11 * sumyx +
	s22 * conj12 * sumyy;
      result22().getValueRW().dcomplexStorage()[inx] =
	s21 * conj21 * sumxx +
	s21 * conj22 * sumxy +
	s22 * conj21 * sumyx +
	s22 * conj22 * sumyy;

      // If first bin, determine which values are perturbed and
      // determine the perturbation.
      if (first) {
	MeqMatrix perturbation;
	for (int spinx=0; spinx<request.nspid(); spinx++) {
	  if (xx.isDefined(spinx)) {
	    perturbation = xx.getPerturbation(spinx);
	    eval[spinx] = true;
	  }
	  if (xy.isDefined(spinx)) {
	    perturbation = xy.getPerturbation(spinx);
	    eval[spinx] = true;
	  }
	  if (yx.isDefined(spinx)) {
	    perturbation = yx.getPerturbation(spinx);
	    eval[spinx] = true;
	  }
	  if (yy.isDefined(spinx)) {
	    perturbation = yy.getPerturbation(spinx);
	    eval[spinx] = true;
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    perturbation = itsStat1->getResult11().getPerturbation(spinx);
	    evalxx[spinx] = true;
	    evalxy[spinx] = true;
	  }
	  if (itsStat1->getResult12().isDefined(spinx)) {
	    perturbation = itsStat1->getResult12().getPerturbation(spinx);
	    evalxx[spinx] = true;
	    evalxy[spinx] = true;
	  }
	  if (itsStat1->getResult21().isDefined(spinx)) {
	    perturbation = itsStat1->getResult21().getPerturbation(spinx);
	    evalyx[spinx] = true;
	    evalyy[spinx] = true;
	  }
	  if (itsStat1->getResult22().isDefined(spinx)) {
	    perturbation = itsStat1->getResult22().getPerturbation(spinx);
	    evalyx[spinx] = true;
	    evalyy[spinx] = true;
	  }
	  if (itsStat2->getResult11().isDefined(spinx)) {
	    perturbation = itsStat2->getResult11().getPerturbation(spinx);
	    evalxx[spinx] = true;
	    evalyx[spinx] = true;
	  }
	  if (itsStat2->getResult12().isDefined(spinx)) {
	    perturbation = itsStat2->getResult12().getPerturbation(spinx);
	    evalxx[spinx] = true;
	    evalyx[spinx] = true;
	  }
	  if (itsStat2->getResult21().isDefined(spinx)) {
	    perturbation = itsStat2->getResult21().getPerturbation(spinx);
	    evalxy[spinx] = true;
	    evalyy[spinx] = true;
	  }
	  if (itsStat2->getResult22().isDefined(spinx)) {
	    perturbation = itsStat2->getResult22().getPerturbation(spinx);
	    evalxy[spinx] = true;
	    evalyy[spinx] = true;
	  }
	  if (eval[spinx]) {
	    evalxx[spinx] = true;
	    evalxy[spinx] = true;
	    evalyx[spinx] = true;
	    evalyy[spinx] = true;
	  } else {
	    eval[spinx] = evalxx[spinx] || evalxy[spinx] ||
	                  evalyx[spinx] || evalyy[spinx];
	  }
	  if (evalxx[spinx]) {
	    result11().setPerturbation (spinx, perturbation);
	    result11().setPerturbedValue (spinx, mat);
	  }
	  if (evalxy[spinx]) {
	    result12().setPerturbation (spinx, perturbation);
	    result12().setPerturbedValue (spinx, mat);
	  }
	  if (evalyx[spinx]) {
	    result21().setPerturbation (spinx, perturbation);
	    result21().setPerturbedValue (spinx, mat);
	  }
	  if (evalyy[spinx]) {
	    result22().setPerturbation (spinx, perturbation);
	    result22().setPerturbedValue (spinx, mat);
	  }
	}
	first = false;
      }
      for (int spinx=0; spinx<request.nspid(); spinx++) {
	if (eval[spinx]) {
	  complex<double> psumxx = sumxx;
	  complex<double> psumxy = sumxy;
	  complex<double> psumyx = sumyx;
	  complex<double> psumyy = sumyy;
	  complex<double> ps11 = s11;
	  complex<double> ps12 = s12;
	  complex<double> ps21 = s21;
	  complex<double> ps22 = s22;
	  complex<double> pconj11 = conj11;
	  complex<double> pconj12 = conj12;
	  complex<double> pconj21 = conj21;
	  complex<double> pconj22 = conj22;
	  if (xx.isDefined(spinx)) {
	    psumxx = sum(xx.getPerturbedValue(spinx)).getDComplex() / nc;
	  }
	  if (xy.isDefined(spinx)) {
	    psumxy = sum(xy.getPerturbedValue(spinx)).getDComplex() / nc;
	  }
	  if (yx.isDefined(spinx)) {
	    psumyx = sum(yx.getPerturbedValue(spinx)).getDComplex() / nc;
	  }
	  if (yy.isDefined(spinx)) {
	    psumyy = sum(yy.getPerturbedValue(spinx)).getDComplex() / nc;
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    ps11 = itsStat1->getResult11().getPerturbedValue(spinx).getDComplex();
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    ps12 = itsStat1->getResult12().getPerturbedValue(spinx).getDComplex();
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    ps21 = itsStat1->getResult21().getPerturbedValue(spinx).getDComplex();
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    ps22 = itsStat1->getResult22().getPerturbedValue(spinx).getDComplex();
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    pconj11 = conj(itsStat2->getResult11().getPerturbedValue(spinx).getDComplex());
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    pconj12 = conj(itsStat2->getResult12().getPerturbedValue(spinx).getDComplex());
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    pconj21 = conj(itsStat2->getResult21().getPerturbedValue(spinx).getDComplex());
	  }
	  if (itsStat1->getResult11().isDefined(spinx)) {
	    pconj22 = conj(itsStat2->getResult22().getPerturbedValue(spinx).getDComplex());
	  }
	  if (evalxx[spinx]) {
	    result11().getPerturbedValueRW(spinx).dcomplexStorage()[inx] =
	      ps11 * pconj11 * psumxx +
	      ps11 * pconj12 * psumxy +
	      ps12 * pconj11 * psumyx +
	      ps12 * pconj12 * psumyy;
	  }
	  if (evalxy[spinx]) {
	    result12().getPerturbedValueRW(spinx).dcomplexStorage()[inx] =
	      ps11 * pconj21 * psumxx +
	      ps11 * pconj22 * psumxy +
	      ps12 * pconj21 * psumyx +
	      ps12 * pconj22 * psumyy;
	  }
	  if (evalyx[spinx]) {
	    result21().getPerturbedValueRW(spinx).dcomplexStorage()[inx] =
	      ps21 * pconj11 * psumxx +
	      ps21 * pconj12 * psumxy +
	      ps22 * pconj11 * psumyx +
	      ps22 * pconj12 * psumyy;
	  }
	  if (evalyy[spinx]) {
	    result22().getPerturbedValueRW(spinx).dcomplexStorage()[inx] =
	      ps21 * pconj21 * psumxx +
	      ps21 * pconj22 * psumxy +
	      ps22 * pconj21 * psumyx +
	      ps22 * pconj22 * psumyy;
	  }
	}
      }
      stx += request.stepX();
      inx++;
    }
    sty += request.stepY();
  }
}

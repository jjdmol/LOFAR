//# MeqPolc.cc: Polynomial coefficients
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

#include <MNS/MeqPolc.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqResult.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>


MeqPolc::MeqPolc()
{}

void MeqPolc::setCoeff (const MeqMatrix& values)
{
  itsCoeff = values.clone();
  itsMask.resize (values.nelements());
  for (int i=0; i<values.nelements(); i++) {
    itsMask[i] = true;
  }
}

void MeqPolc::setCoeff (const MeqMatrix& values,
			const Matrix<Bool>& mask)
{
  Assert (values.nx()==mask.shape()(0) && values.ny()==mask.shape()(1));
  itsCoeff = values.clone();
  itsMask.resize (values.nelements());
  bool deleteM;
  const bool* mdata = mask.getStorage(deleteM);
  for (unsigned int i=0; i<mask.nelements(); i++) {
    itsMask[i] = mdata[i];
  }
  mask.freeStorage (mdata, deleteM);
}

MeqResult MeqPolc::getResult (const MeqRequest& request)
{
  if (itsCoeff.nelements() == 1) {
    MeqResult result(0);
    if (itsCoeff.isDouble()) {
      result.setValue (MeqMatrix(itsCoeff.getDouble()));
    } else {
      result.setValue (MeqMatrix(itsCoeff.getDComplex()));
    }
    return result;
  }
  const MeqDomain& domain = request.domain();
  Assert (domain.startX() >= itsDomain.startX());
  Assert (domain.startY() >= itsDomain.startY());
  Assert (domain.endX() <= itsDomain.endX());
  Assert (domain.endY() <= itsDomain.endY());
  double stepx = request.stepX() / itsDomain.scaleX();
  double stepy = request.stepY() / itsDomain.scaleY();
  double stx = itsDomain.normalizeX (domain.startX()) + stepx/2;
  double sty = itsDomain.normalizeY (domain.startY()) + stepy/2;
  int ndx = request.nx();
  int ndy = request.ny();
  int ncx = itsCoeff.nx();
  int ncy = itsCoeff.ny();
  MeqResult result(request.nspid());

  vector<MeqMatrix> allPowers(ndx*ndy);
  double powers[100];
  // Calculate all cross-terms for 2-dim polynomials with max. order 9.
  // Thus x, x^2 .. x^n, xy, x^2y .. x^ny, xy^2 .. x^ny^2 .. x^ny^n
  // Do that for all values in the domain.
  int inx = 0;
  double y = sty;
  for (int iy=0; iy<ndy; iy++) {
    double x = stx;
    for (int ix=0; ix<ndx; ix++) {
      double* powptr = powers;
      double yv = 1;
      for (int j=0; j<ncy; j++) {
	double xv = 1;
	for (int i=0; i<ncx; i++) {
	  *powptr++ = xv*yv;
	  xv *= x;
	}
	yv *= y;
      }
      allPowers[inx++] = MeqMatrix(powers, ncx, ncy);
      x += stepx;
    }
    y += stepy;
  }

  // Evaluate the expression as double or dcomplex.
  if (itsCoeff.isDouble()) {
    int inx=0;
    const double* coeff = itsCoeff.doubleStorage();
    Matrix<double> value(ndx, ndy);
    for (int j=0; j<ndy; j++) {
      for (int i=0; i<ndx; i++) {
	double tmp = 0;
	const double* xterm = allPowers[inx++].doubleStorage();
	for (int k=0; k<ncx*ncy; k++) {
	  tmp += coeff[k] * xterm[k];
	}
	value(i,j) = tmp;
      }
    }
    MeqMatrix mnsres(value);
    result.setValue (mnsres);
    // Evaluate (if needed) for the perturbed parameter values.
    for (unsigned int spinx=0; spinx<itsSpidInx.size(); spinx++) {
      if (itsSpidInx[spinx] >= 0) {
	double perturbation = 1e-6;
	if (abs(coeff[spinx]) > 1e-10) {
	  perturbation *= coeff[spinx];
	}
	MeqMatrix pres(value);
	double* presData = pres.doubleStorage();
	inx = 0;
	for (int j=0; j<ndy; j++) {
	  for (int i=0; i<ndx; i++) {
	    const double* xterm = allPowers[inx++].doubleStorage();
	    *presData++ += perturbation * xterm[spinx];
	  }
	}
	result.setPerturbedValue (spinx, pres);
	result.setPerturbation (spinx, perturbation);
      }
    }
  } else {
    int inx=0;
    const complex<double>* coeff = itsCoeff.dcomplexStorage();
    Matrix<complex<double> > value(ndx, ndy);
    for (int j=0; j<ndy; j++) {
      for (int i=0; i<ndx; i++) {
	complex<double> tmp = 0;
	const double* xterm = allPowers[inx++].doubleStorage();
	for (int k=0; k<ncx*ncy; k++) {
	  tmp += coeff[k] * xterm[k];
	}
	value(i,j) = tmp;
      }
    }
    MeqMatrix mnsres(value);
    result.setValue (mnsres);
    // Evaluate (if needed) for the perturbed parameter values.
    for (unsigned int spinx=0; spinx<itsSpidInx.size(); spinx++) {
      if (itsSpidInx[spinx] >= 0) {
	double realpert = 1e-6;
	double imagpert = 1e-6;
	if (abs(coeff[spinx].real()) > 1e-10) {
	  realpert *= coeff[spinx].real();
	}
	if (abs(coeff[spinx].imag()) > 1e-10) {
	  imagpert *= coeff[spinx].imag();
	}
	complex<double> perturbation(realpert, imagpert);
	MeqMatrix pres(value);
	complex<double>* presData = pres.dcomplexStorage();
	inx = 0;
	for (int j=0; j<ndy; j++) {
	  for (int i=0; i<ndx; i++) {
	    const double* xterm = allPowers[inx++].doubleStorage();
	    *presData++ += perturbation * xterm[spinx];
	  }
	}
	result.setPerturbedValue (spinx, pres);
	result.setPerturbation (spinx, perturbation);
      }
    }
  }
  return result;
}

int MeqPolc::makeSolvable (int spidIndex)
{
  Assert (itsSpidInx.size() == 0);
  itsSpidInx.resize (itsCoeff.nelements());
  int nr=0;
  for (int i=0; i<itsCoeff.nelements(); i++) {
    if (itsMask[i]) {
      itsSpidInx[i] = spidIndex++;
      nr++;
    } else {
      itsSpidInx[i] = -1;          // not solvable
    }
  }
  return nr;
}

void MeqPolc::update (const MeqMatrix& value)
{
  if (itsCoeff.isDouble()) {
    double* coeff = itsCoeff.doubleStorage();
    for (unsigned int i=0; i<itsSpidInx.size(); i++) {
      if (itsSpidInx[i] >= 0) {
	coeff[i] = value.getDouble (itsSpidInx[i], 0);
      }
    }
  } else {
    complex<double>* coeff = itsCoeff.dcomplexStorage();
    for (unsigned int i=0; i<itsSpidInx.size(); i++) {
      if (itsSpidInx[i] >= 0) {
	coeff[i] = value.getDComplex (itsSpidInx[i], 0);
      }
    }
  }
}

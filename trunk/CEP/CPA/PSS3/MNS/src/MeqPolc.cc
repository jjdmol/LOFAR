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
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>


MeqPolc::MeqPolc()
: itsMaxNrSpid(0)
{}

void MeqPolc::setCoeff (const MeqMatrix& values)
{
  itsCoeff = values.clone();
  itsMask.resize (values.nelements());
  for (int i=0; i<values.nelements(); i++) {
    itsMask[i] = true;
  }
  clearSolvable();
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
  clearSolvable();
}

MeqResult MeqPolc::getResult (const MeqRequest& request)
{
  // First check if the domain is valid.
  // Because the values are calculaed for the center of each cell,
  // it is only checked if the centers are in the polc domain.
  const MeqDomain& domain = request.domain();
  Assert (domain.startX() + request.stepX()/2 >= itsDomain.startX());
  Assert (domain.startY() + request.stepY()/2 >= itsDomain.startY());
  Assert (domain.endX() - request.stepX()/2 <= itsDomain.endX());
  Assert (domain.endY() - request.stepY()/2 <= itsDomain.endY());
  // Create the result object containing as many spids as needed for
  // this polynomial (but not more).
  MeqResult result(itsMaxNrSpid);
  // If there is only one coefficient, the polynomial is independent
  // of x and y.
  // So set the value to the coefficient and possibly set the perturbed value.
  // Make sure it is turned into a scalar value.
  if (itsCoeff.nelements() == 1) {
    if (itsCoeff.isDouble()) {
      result.setValue (MeqMatrix(itsCoeff.getDouble()));
      if (itsMaxNrSpid) {
	result.setPerturbedValue (itsSpidInx[0],
				  MeqMatrix(itsCoeff.getDouble()
					    + itsPerturbation.getDouble()));
      }
    } else { 
      result.setValue (MeqMatrix(itsCoeff.getDComplex()));
      if (itsMaxNrSpid) {
	result.setPerturbedValue (itsSpidInx[0],
				  MeqMatrix(itsCoeff.getDComplex()
					    + itsPerturbation.getDComplex()));
      }
   }
  } else {
    // The polynomial has multiple coefficients.
    // Get the step and start values in the normalized domain.
    double stepx = request.stepX() / itsDomain.scaleX();
    double stepy = request.stepY() / itsDomain.scaleY();
    double stx = itsDomain.normalizeX (domain.startX()) + stepx/2;
    double sty = itsDomain.normalizeY (domain.startY()) + stepy/2;
    // Get number of steps and coefficients in x and y.
    int ndx = request.nx();
    int ndy = request.ny();
    int ncx = itsCoeff.nx();
    int ncy = itsCoeff.ny();
    // Evaluate the expression as double or dcomplex.
    if (itsCoeff.isDouble()) {
      const double* coeffData = itsCoeff.doubleStorage();
      const double* pertData = 0;
      double* pertValPtr[100];
      if (itsMaxNrSpid) {
	pertData = itsPerturbation.doubleStorage();
	// Create the matrix for each perturbed value.
	// Keep a pointer to the internal matrix data.
	for (unsigned int i=0; i<itsSpidInx.size(); i++) {
	  if (itsSpidInx[i] >= 0) {
	    result.setPerturbedValue (itsSpidInx[i],
				      MeqMatrix(double(0), ndx, ndy));
	    pertValPtr[i] =
	      result.getPerturbedValueRW(itsSpidInx[i]).doubleStorage();
	  }
	}
      }
      // Create matrix for the value itself and keep a pointer to its data.
      result.setValue (MeqMatrix(double(0), ndx, ndy));
      double* value = result.getValueRW().doubleStorage();
      // Iterate over all cells in the domain.
      double valy = sty;
      for (int j=0; j<ndy; j++) {
	double valx = stx;
	for (int i=0; i<ndx; i++) {
	  const double* coeff = coeffData;
	  const double* pert  = pertData;
	  double total = 0;
	  if (ncx == 1) {
	    // Only 1 coefficient in X, it is independent of x.
	    // So only calculate for the Y values in the most efficient way.
	    total = coeff[ncy-1];
	    for (int iy=ncy-2; iy>=0; iy--) {
	      total *= valy;
	      total += coeff[iy];
	    }
	    if (itsMaxNrSpid) {
	      double powy = 1;
	      for (int iy=0; iy<ncy; iy++) {
		if (pertValPtr[iy]) {
		  *(pertValPtr[iy]) = total + pert[iy] * powy;
		  pertValPtr[iy]++;
		}
		powy *= valy;
	      }
	    }
	  } else {
	    double powy = 1;
	    for (int iy=0; iy<ncy; iy++) {
	      double tmp = coeff[ncx-1];
	      for (int ix=ncx-2; ix>=0; ix--) {
		tmp *= valx;
		tmp += coeff[ix];
	      }
	      total += tmp * powy;
	      powy *= valy;
	      coeff += ncx;
	    }
	    if (itsMaxNrSpid) {
	      double powersx[10];
	      double powx = 1;
	      for (int ix=0; ix<ncx; ix++) {
		powersx[ix] = powx;
		powx *= valx;
	      }
	      double powy = 1;
	      int ik = 0;
	      for (int iy=0; iy<ncy; iy++) {
		for (int ix=0; ix<ncx; ix++) {
		  if (pertValPtr[ik]) {
		    *(pertValPtr[ik]) = total + pert[ik] * powersx[ix] * powy;
		    pertValPtr[ik]++;
		  }
		  ik++;
		}
		powy *= valy;
	      }
	    }
	  }
	  *value++ = total;
	  valx += stepx;
	}
	valy += stepy;
      }
    } else {
      const complex<double>* coeffData = itsCoeff.dcomplexStorage();
      const complex<double>* pertData = 0;
      complex<double>* pertValPtr[100];
      if (itsMaxNrSpid) {
	pertData = itsPerturbation.dcomplexStorage();
	// Create the matrix for each perturbed value.
	// Keep a pointer to the internal matrix data.
	for (unsigned int i=0; i<itsSpidInx.size(); i++) {
	  if (itsSpidInx[i] >= 0) {
	    result.setPerturbedValue (itsSpidInx[i],
				      MeqMatrix(complex<double>(), ndx, ndy));
	    pertValPtr[i] =
	      result.getPerturbedValueRW(itsSpidInx[i]).dcomplexStorage();
	  }
	}
      }
      // Create matrix for the value itself and keep a pointer to its data.
      result.setValue (MeqMatrix(complex<double>(), ndx, ndy));
      complex<double>* value = result.getValueRW().dcomplexStorage();
      // Iterate over all cells in the domain.
      double valy = sty;
      for (int j=0; j<ndy; j++) {
	double valx = stx;
	for (int i=0; i<ndx; i++) {
	  const complex<double>* coeff = coeffData;
	  const complex<double>* pert  = pertData;
	  complex<double> total(0,0);
	  if (ncx == 1) {
	    // Only 1 coefficient in X, it is independent of x.
	    // So only calculate for the Y values in the most efficient way.
	    total = coeff[ncy-1];
	    for (int iy=ncy-2; iy>=0; iy--) {
	      total *= valy;
	      total += coeff[iy];
	    }
	    if (itsMaxNrSpid) {
	      double powy = 1;
	      for (int iy=0; iy<ncy; iy++) {
		if (pertValPtr[iy]) {
		  *(pertValPtr[iy]) = total + pert[iy] * powy;
		  pertValPtr[iy]++;
		}
		powy *= valy;
	      }
	    }
	  } else {
	    double powy = 1;
	    for (int iy=0; iy<ncy; iy++) {
	      complex<double> tmp = coeff[ncx-1];
	      for (int ix=ncx-2; ix>=0; ix--) {
		tmp *= valx;
		tmp += coeff[ix];
	      }
	      total += tmp * powy;
	      powy *= valy;
	      coeff += ncx;
	    }
	    if (itsMaxNrSpid) {
	      double powersx[10];
	      double powx = 1;
	      for (int ix=0; ix<ncx; ix++) {
		powersx[ix] = powx;
		powx *= valx;
	      }
	      double powy = 1;
	      int ik = 0;
	      for (int iy=0; iy<ncy; iy++) {
		for (int ix=0; ix<ncx; ix++) {
		  if (pertValPtr[ik]) {
		    *(pertValPtr[ik]) = total + pert[ik] * powersx[ix] * powy;
		    pertValPtr[ik]++;
		  }
		  ik++;
		}
		powy *= valy;
	      }
	    }
	  }
	  *value++ = total;
	  valx += stepx;
	}
	valy += stepy;
      }
    }
  }
  // Set the perturbations.
  if (itsMaxNrSpid) {
    if (itsCoeff.isDouble()) {
      const double* pert  = itsPerturbation.doubleStorage();
      for (unsigned int i=0; i<itsSpidInx.size(); i++) {
	if (itsSpidInx[i] >= 0) {
	  result.setPerturbation (itsSpidInx[i], pert[i]);
	}
      }
    } else {
      const complex<double>* pert  = itsPerturbation.dcomplexStorage();
      for (unsigned int i=0; i<itsSpidInx.size(); i++) {
	if (itsSpidInx[i] >= 0) {
	  result.setPerturbation (itsSpidInx[i], pert[i]);
	}
      }
    }
  }
  return result;
}

int MeqPolc::makeSolvable (int spidIndex)
{
  Assert (itsSpidInx.size() == 0);
  itsSpidInx.resize (itsCoeff.nelements());
  itsMaxNrSpid = 0;
  int nr=0;
  for (int i=0; i<itsCoeff.nelements(); i++) {
    if (itsMask[i]) {
      itsSpidInx[i] = spidIndex++;
      itsMaxNrSpid = spidIndex;
      nr++;
    } else {
      itsSpidInx[i] = -1;          // not solvable
    }
  }
  // Precalculate the perturbed coefficients.
  // The perbation is 10^-6 of the coefficient.
  // If the coefficient is too small, just take 10^-6.
  if (nr > 0) {
    itsPerturbation = itsCoeff.clone();
    if (itsCoeff.isDouble()) {
      const double* coeff = itsCoeff.doubleStorage();
      double* pert  = itsPerturbation.doubleStorage();
      for (int i=0; i<itsCoeff.nelements(); i++) {
	double perturbation = 1e-6;
	if (abs(coeff[i]) > 1e-10) {
	  perturbation *= coeff[i];
	}
	pert[i] = perturbation;
      }
    } else {
      const complex<double>* coeff = itsCoeff.dcomplexStorage();
      complex<double>* pert  = itsPerturbation.dcomplexStorage();
      for (int i=0; i<itsCoeff.nelements(); i++) {
	double realpert = 1e-6;
	double imagpert = 1e-6;
	if (abs(coeff[i].real()) > 1e-10) {
	  realpert *= coeff[i].real();
	}
	if (abs(coeff[i].imag()) > 1e-10) {
	  imagpert *= coeff[i].imag();
	}
	pert[i] = complex<double>(realpert, imagpert);
      }
    }
  }
  return nr;
}

void MeqPolc::clearSolvable()
{
  itsSpidInx.resize (0);
  itsMaxNrSpid = 0;
  itsPerturbation = MeqMatrix();
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

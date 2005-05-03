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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Profiling/PerfProfile.h>

#include <PSS3/MNS/MeqPolc.h>
#include <PSS3/MNS/MeqRequest.h>
#include <PSS3/MNS/MeqResult.h>
#include <PSS3/MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR {

double MeqPolc::theirPascal[10][10];
bool   MeqPolc::theirPascalFilled = false;


MeqPolc::MeqPolc()
: itsMaxNrSpid  (0),
  itsPertValue  (1e-6),
  itsIsRelPert  (true),
  itsX0         (0),
  itsY0         (0),
  itsNormalized (false)
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

void MeqPolc::setCoeffOnly (const MeqMatrix& values)
{
  itsCoeff = values.clone();
  clearSolvable();
}

MeqResult MeqPolc::getResult (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);
  Bool makeDiff = itsMaxNrSpid > 0  &&  request.nspid() > 0;
  // It is not checked if the domain is valid.
  // In that way any value can be used for the default domain [-1,1].
  // Because the values are calculated for the center of each cell,
  // it is only checked if the centers are in the polc domain.
  const MeqDomain& domain = request.domain();
  //Assert (domain.startX() + request.stepX()/2 >= itsDomain.startX());
  //Assert (domain.startY() + request.stepY()/2 >= itsDomain.startY());
  //Assert (domain.endX() - request.stepX()/2 <= itsDomain.endX());
  //Assert (domain.endY() - request.stepY()/2 <= itsDomain.endY());
  // Create the result object containing as many spids as needed for
  // this polynomial (but not more).
  //////  MeqResult result(itsMaxNrSpid);
  MeqResult result(request.nspid());
  // If there is only one coefficient, the polynomial is independent
  // of x and y.
  // So set the value to the coefficient and possibly set the perturbed value.
  // Make sure it is turned into a scalar value.
  if (itsCoeff.nelements() == 1) {
    result.setValue (MeqMatrix(itsCoeff.getDouble()));
    if (makeDiff) {
      result.setPerturbedValue (itsSpidInx[0],
				MeqMatrix(itsCoeff.getDouble()
					  + itsPerturbation.getDouble()));
      result.setPerturbation (itsSpidInx[0], itsPerturbation.getDouble());
// 	cout << "polc " << itsSpidInx[0] << ' ' << result.getValue()
// 	     << result.getPerturbedValue(itsSpidInx[0]) << itsPerturbation
// 	     << ' ' << itsCoeff << endl;
    }
  } else {
    // The polynomial has multiple coefficients.
    // Get the step and start values in the (un)normalized domain.
    double stepx, stepy, stx, sty;
    if (itsNormalized) {
      stepx = request.stepX() / itsDomain.scaleX();
      stepy = request.stepY() / itsDomain.scaleY();
      stx = itsDomain.normalizeX (domain.startX()) + stepx * .5;
      sty = itsDomain.normalizeY (domain.startY()) + stepy * .5;
    } else {
      stepx = request.stepX();
      stepy = request.stepY();
      stx = domain.startX() - itsX0 + stepx * .5;
      sty = domain.startY() - itsY0 + stepy * .5;
    }
    // Get number of steps and coefficients in x and y.
    int ndx = request.nx();
    int ndy = request.ny();
    int ncx = itsCoeff.nx();
    int ncy = itsCoeff.ny();
    // Evaluate the expression (as double).
    const double* coeffData = itsCoeff.doubleStorage();
    const double* pertData = 0;
    double* pertValPtr[100];
    if (makeDiff) {
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
	  if (makeDiff) {
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
	  if (makeDiff) {
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
    // Set the perturbations.
    if (makeDiff) {
      const double* pert  = itsPerturbation.doubleStorage();
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
  // Removed Assert, so the same parm can be set solvable multiple times
  // in a row.
  //  Assert (itsSpidInx.size() == 0);
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
  // The perturbation is absolute or a factor of the coefficient.
  // If the coefficient is too small, take absolute.
  if (nr > 0) {
    itsPerturbation = itsCoeff.clone();
    const double* coeff = itsCoeff.doubleStorage();
    double* pert  = itsPerturbation.doubleStorage();
    for (int i=0; i<itsCoeff.nelements(); i++) {
      double perturbation = itsPertValue;
      if (itsIsRelPert  &&  std::abs(coeff[i]) > 1e-10) {
	perturbation *= coeff[i];
      }
      pert[i] = perturbation;
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

void MeqPolc::getInitial (MeqMatrix& values) const
{
  double* data = values.doubleStorage();
  const double* coeff = itsCoeff.doubleStorage();
  for (unsigned int i=0; i<itsSpidInx.size(); i++) {
    if (itsSpidInx[i] >= 0) {
      Assert (itsSpidInx[i] < values.nx());
      data[itsSpidInx[i]] = coeff[i];
    }
  }
}

void MeqPolc::getCurrentValue (MeqMatrix& value, bool denorm) const
{
  if (denorm && isNormalized()) {
    value = denormalize(itsCoeff);
  } else {
    value = itsCoeff;
  }
}

void MeqPolc::update (const MeqMatrix& value)
{
  double* coeff = itsCoeff.doubleStorage();
  for (unsigned int i=0; i<itsSpidInx.size(); i++) {
    if (itsSpidInx[i] >= 0) {
      coeff[i] = value.getDouble (itsSpidInx[i], 0);
    }
  }
}


MeqMatrix MeqPolc::normalize (const MeqMatrix& coeff, const MeqDomain& domain)
{
  return normDouble (coeff,
		     domain.scaleX(), domain.scaleY(),
		     domain.offsetX()-itsX0, domain.offsetY()-itsY0);
}
  
MeqMatrix MeqPolc::denormalize (const MeqMatrix& coeff) const
{
  return normDouble (coeff,
		     1/itsDomain.scaleX(), 1/itsDomain.scaleY(),
		     (itsX0-itsDomain.offsetX())/itsDomain.scaleX(),
		     (itsY0-itsDomain.offsetY())/itsDomain.scaleY());
}
  
void MeqPolc::fillPascal()
{
  for (int j=0; j<10; j++) {
    theirPascal[j][0] = 1;
    for (int i=1; i<=j; i++) {
      theirPascal[j][i] = theirPascal[j-1][i-1] + theirPascal[j-1][i];
    }
  }
  theirPascalFilled = true;
}

MeqMatrix MeqPolc::normDouble (const MeqMatrix& coeff, double sx,
			       double sy, double ox, double oy)
{
  // Fill Pascal's triangle if not done yet.
  if (!theirPascalFilled) {
    fillPascal();
  }
  int nx = coeff.nx();
  int ny = coeff.ny();
  const double* pcold = coeff.doubleStorage();
  // Create vectors holding the powers of the scale and offset values.
  vector<double> sxp(nx);
  vector<double> syp(ny);
  vector<double> oxp(nx);
  vector<double> oyp(ny);
  sxp[0] = 1;
  oxp[0] = 1;
  for (int i=1; i<nx; i++) {
    sxp[i] = sxp[i-1] * sx;
    oxp[i] = oxp[i-1] * ox;
  }
  syp[0] = 1;
  oyp[0] = 1;
  for (int i=1; i<ny; i++) {
    syp[i] = syp[i-1] * sy;
    oyp[i] = oyp[i-1] * oy;
  }
  // Create the new coefficient matrix.
  // Create a vector to hold the terms of (sy+oy)^j
  MeqMatrix newc (double(0), nx, ny, true);
  double* pcnew = newc.doubleStorage();
  vector<double> psyp(ny);
  // Loop through all coefficients in the y direction.
  for (int j=0; j<ny; j++) {
    // Precalculate the terms of (sy+oy)^j
    for (int k=0; k<=j; k++) {
      psyp[k] = oyp[j-k] * syp[k] * theirPascal[j][k];
    }
    // Loop through all coefficients in the x direction.
    for (int i=0; i<nx; i++) {
      // Get original coefficient.
      double f = *pcold++;
      // Calculate all terms of (sx+ox)^i
      for (int k1=0; k1<=i; k1++) {
	double c = oxp[i-k1] * sxp[k1] * theirPascal[i][k1] * f;
	// Multiply each term with the precalculated terms of (sy+oy)^j
	// and add the result to the appropriate new coefficient.
	for (int k2=0; k2<=j; k2++) {
	  pcnew[k1 + k2*nx] += c * psyp[k2];
	}
      }
    }
  }
  return newc;
}

}

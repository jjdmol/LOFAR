//# MeqPolc.cc: Ordinary polynomial with coefficients valid for a given domain
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

#include <BBS3/MNS/MeqPolc.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR {


MeqPolc::~MeqPolc()
{}

MeqResult MeqPolc::getResult (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);
  bool makeDiff = itsMaxNrSpid > 0  &&  request.nspid() > 0;
  // It is not checked if the domain is valid.
  // In that way any value can be used for the default domain [-1,1].
  // Because the values are calculated for the center of each cell,
  // it is only checked if the centers are in the polc domain.
  const MeqDomain& domain = request.domain();
  //ASSERT (domain.startX() + request.stepX()/2 >= itsDomain.startX());
  //ASSERT (domain.startY() + request.stepY()/2 >= itsDomain.startY());
  //ASSERT (domain.endX() - request.stepX()/2 <= itsDomain.endX());
  //ASSERT (domain.endY() - request.stepY()/2 <= itsDomain.endY());
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
    // Get the step and start values in the domain.
    double stepx = request.stepX();
    double stepy = request.stepY();
    double stx = domain.startX() - itsX0 + stepx * .5;
    double sty = domain.startY() - itsY0 + stepy * .5;
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
      const double* pert = itsPerturbation.doubleStorage();
      for (unsigned int i=0; i<itsSpidInx.size(); i++) {
	if (itsSpidInx[i] >= 0) {
	  result.setPerturbation (itsSpidInx[i], pert[i]);
	}
      }
    }
  }
  return result;
}

}

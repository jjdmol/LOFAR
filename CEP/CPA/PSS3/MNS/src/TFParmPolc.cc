//# TFParmPolc.cc: Polynomial coefficients
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

#include <MNS/TFParmPolc.h>
#include <MNS/TFRequest.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>

TFParmPolc::TFParmPolc (unsigned int type, 
			unsigned int orderT, unsigned int orderF)
: TFParm      (type),
  itsNx       (orderT+1),
  itsNy       (orderF+1),
  itsCurCoeff ((orderT+1)*(orderF+1), 1),
  itsMask     ((orderT+1)*(orderF+1), true)
{
  itsInitialCoeff = itsCurCoeff;
}

TFParmPolc::TFParmPolc (unsigned int type, const Matrix<double>& values)
: TFParm  (type),
  itsNx   (values.shape()(0)),
  itsNy   (values.shape()(1)),
  itsMask (values.nelements(), true)
{
  itsCurCoeff.reserve (values.nelements());
  bool deleteD;
  const double* vdata = values.getStorage(deleteD);
  for (unsigned int i=0; i<values.nelements(); i++) {
    itsCurCoeff.push_back (vdata[i]);
  }
  values.freeStorage (vdata, deleteD);
  itsInitialCoeff = itsCurCoeff;
}

TFParmPolc::TFParmPolc (unsigned int type, const Matrix<double>& values,
			const Matrix<bool>& mask)
: TFParm (type),
  itsNx  (values.shape()(0)),
  itsNy  (values.shape()(1))
{
  Assert (values.shape().isEqual (mask.shape()));
  itsCurCoeff.reserve (values.nelements());
  itsMask.reserve (values.nelements());
  bool deleteD, deleteM;
  const double* vdata = values.getStorage(deleteD);
  const bool* mdata = mask.getStorage(deleteM);
  for (unsigned int i=0; i<values.nelements(); i++) {
    itsCurCoeff.push_back (vdata[i]);
    itsMask.push_back (mdata[i]);
  }
  values.freeStorage (vdata, deleteD);
  mask.freeStorage (mdata, deleteM);
  itsInitialCoeff = itsCurCoeff;
}

TFParmPolc::~TFParmPolc()
{}

void TFParmPolc::init (const TFDomain&)
{}

int TFParmPolc::setSolvable (int spidIndex)
{
  Assert (itsSpidInx.size() == 0);
  itsSpidInx.resize (itsCurCoeff.size());
  int nr=0;
  for (unsigned int i=0; i<itsCurCoeff.size(); i++) {
    if (itsMask[i]) {
      itsSpidInx[i] = spidIndex++;
      nr++;
    } else {
      itsSpidInx[i] = -1;          // not solvable
    }
  }
  return nr;
}

void TFParmPolc::clearSolvable()
{
  itsSpidInx.clear();
}


TFRange TFParmPolc::getRange (const TFRequest& request)
{
  const TFDomain& domain = request.domain();
  int ndx = domain.nx();
  int ndy = domain.ny();
  // Evaluate the expression.
  TFRange range(request.nspid());
  Matrix<double> result(ndx, ndy);
  for (int j=0; j<ndy; j++) {
    for (int i=0; i<ndx; i++) {
      double tmp = 0;
      const double* xterm = request.getCrossTerms(i,j).doubleStorage();
      for (int k=0; k<itsNx*itsNy; k++) {
	tmp += itsCurCoeff[k] * xterm[k];
      }
      result(i,j) = tmp;
    }
  }
  MnsMatrix mnsres(result);
  range.setValue (mnsres);
  // Evaluate (if needed) for the perturbed parameter values.
  vector<MnsMatrix*> valptr(itsCurCoeff.size());
  for (unsigned int spinx=0; spinx<itsSpidInx.size(); spinx++) {
    if (itsSpidInx[spinx] >= 0) {
      double perturbation = 1e-6;
      if (abs(itsCurCoeff[spinx]) > 1e-10) {
	perturbation = itsCurCoeff[spinx] * 1e-6;
      }
      MnsMatrix pres(result);
      double* presData = pres.doubleStorage();
      for (int j=0; j<ndy; j++) {
	for (int i=0; i<ndx; i++) {
	  const double* xterm = request.getCrossTerms(i,j).doubleStorage();
	  *presData++ += perturbation * xterm[spinx];
	}
      }
      range.setPerturbedValue (spinx, pres);
      range.setPerturbation (spinx, perturbation);
    }
  }
  return range;
}

void TFParmPolc::update (const MnsMatrix& value)
{
  for (unsigned int i=0; i<itsSpidInx.size(); i++) {
    if (itsSpidInx[i] >= 0) {
      itsCurCoeff[i] = value.getDouble (itsSpidInx[i], 0);
    }
  }
}

void TFParmPolc::save (const TFDomain&)
{
  itsInitialCoeff = itsCurCoeff;
}

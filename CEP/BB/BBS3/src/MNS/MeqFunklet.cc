//# MeqFunklet.cc: Parameter function with coefficients valid for a given domain
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

#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqFunklet.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR {


MeqFunklet::MeqFunklet()
: itsMaxNrSpid  (0),
  itsPertValue  (1e-6),
  itsIsRelPert  (true),
  itsX0         (0),
  itsY0         (0)
{}

MeqFunklet::~MeqFunklet()
{}

void MeqFunklet::setCoeff (const MeqMatrix& values)
{
  itsCoeff = values.clone();
  itsMask.resize (values.nelements());
  for (int i=0; i<values.nelements(); i++) {
    itsMask[i] = true;
  }
  clearSolvable();
}

void MeqFunklet::setCoeff (const MeqMatrix& values,
			   const Matrix<bool>& mask)
{
  ASSERT (values.nx()==mask.shape()(0) && values.ny()==mask.shape()(1));
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

void MeqFunklet::setCoeffOnly (const MeqMatrix& values)
{
  itsCoeff = values.clone();
  clearSolvable();
}

int MeqFunklet::makeSolvable (int spidIndex)
{
  // Removed ASSERT, so the same parm can be set solvable multiple times
  // in a row.
  //  ASSERT (itsSpidInx.size() == 0);
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

void MeqFunklet::clearSolvable()
{
  itsSpidInx.resize (0);
  itsMaxNrSpid = 0;
  itsPerturbation = MeqMatrix();
}

void MeqFunklet::update (const MeqMatrix& value)
{
  double* coeff = itsCoeff.doubleStorage();
  const double* vals = value.doubleStorage();
  ASSERT (value.nelements() == itsCoeff.nelements());
  for (int i=0; i<value.nelements(); ++i) {
    coeff[i] = vals[i];
  }
}

void MeqFunklet::update (const vector<double>& values)
{
  double* coeff = itsCoeff.doubleStorage();
  for (unsigned int i=0; i<itsSpidInx.size(); i++) {
    if (itsSpidInx[i] >= 0) {
      DBGASSERT (itsSpidInx[i] < int(values.size()));
      coeff[i] = values[itsSpidInx[i]];
    }
  }
}

}

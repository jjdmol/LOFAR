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

#include <lofar_config.h>

#include <BBS/MNS/MeqFunklet.h>
#include <BBS/MNS/MeqPolc.h>
#include <BBS/MNS/MeqTabular.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR {


MeqFunklet::MeqFunklet()
: itsNrScid (0)
{
  setDomain (MeqDomain(0,1,0,1));
}

MeqFunklet::MeqFunklet (const ParmDB::ParmValue& pvalue)
: itsNrScid (0),
  itsParmValue (pvalue.clone())
{
  const ParmDB::ParmValueRep& pval = pvalue.rep();
  itsDomain = MeqDomain(pval.itsDomain);
  if (pval.itsShape.size() != 0) {
    ASSERT (pval.itsShape.size() == 2);
    int nx = pval.itsShape[0];
    int ny = pval.itsShape[1];
    itsCoeff = MeqMatrix (&(pval.itsCoeff[0]), nx, ny);
  }
}

MeqFunklet::MeqFunklet (const MeqFunklet& that)
: itsCoeff     (that.itsCoeff.clone()),
  itsCoeffPert (that.itsCoeffPert.clone()),
  itsDomain    (that.itsDomain),
  itsNrScid    (that.itsNrScid),
  itsScidInx   (that.itsScidInx),
  itsParmValue (that.itsParmValue.clone())
{}

MeqFunklet::~MeqFunklet()
{}

MeqFunklet& MeqFunklet::operator= (const MeqFunklet& that)
{
  if (this != &that) {
    itsCoeff     = that.itsCoeff.clone();
    itsCoeffPert = that.itsCoeffPert.clone();
    itsDomain    = that.itsDomain;
    itsNrScid    = that.itsNrScid;
    itsScidInx   = that.itsScidInx;
    itsParmValue = that.itsParmValue.clone();
  }
  return *this;
}

MeqFunklet* MeqFunklet::make (const ParmDB::ParmValue& pvalue,
			      const string& name)
{
  ASSERTSTR (pvalue.rep().itsShape.size()==2,
	     "No 2-dim funklet found for parameter " << name);
  if (pvalue.rep().itsType=="polc") {
    return new MeqPolc(pvalue);
  } else if (pvalue.rep().itsType=="tabular") {
    return new MeqTabular(pvalue);
  }
  ASSERTSTR (false,
	     "Unknown funklet found for parameter " << name);
}

int MeqFunklet::makeSolvable (int scidIndex)
{
  // Ignore the call if done multiple times.
  if (itsNrScid > 0) {
    return 0;
  }
  itsScidInx = scidIndex;
  itsNrScid  = 0;
  for (int i=0; i<itsCoeff.nelements(); i++) {
    if (isCoeffSolvable(i)) {
      itsNrScid++;
    }
  }
  // Precalculate the perturbed coefficients.
  // The perturbation is absolute or a factor of the coefficient.
  // If the coefficient is too small, take absolute.
  if (itsNrScid > 0) {
    itsCoeffPert = itsCoeff.clone();
    const double* coeff = itsCoeff.doubleStorage();
    double* pert = itsCoeffPert.doubleStorage();
    for (int i=0; i<itsCoeffPert.nelements(); i++) {
      double perturbation = itsParmValue.rep().itsPerturbation;
      if (itsParmValue.rep().itsIsRelPert  &&  std::abs(coeff[i]) > 1e-10) {
	perturbation *= coeff[i];
      }
      pert[i] = perturbation;
    }
  }
  return itsNrScid;
}

void MeqFunklet::clearSolvable()
{
  itsNrScid    = 0;
  itsScidInx   = -1;
  itsCoeffPert = MeqMatrix();
}

void MeqFunklet::setDomain (const MeqDomain& domain)
{
  itsDomain = domain;
  itsParmValue.rep().setDomain (ParmDB::ParmDomain(domain.startX(),
						   domain.endX(),
						   domain.startY(),
						   domain.endY()));
}

void MeqFunklet::setCoeff (const MeqMatrix& value, const bool* mask)
{
  itsCoeff = value.clone();
  vector<int> shp(2);
  shp[0] = value.nx();
  shp[1] = value.ny();
  if (mask == 0) {
    itsParmValue.rep().setCoeff (value.doubleStorage(), shp);
  } else {
    itsParmValue.rep().setCoeff (value.doubleStorage(), mask, shp);
  }
}

void MeqFunklet::update (const MeqMatrix& value)
{
  double* coeff = itsCoeff.doubleStorage();
  const double* vals = value.doubleStorage();
  ASSERT (value.nelements() == itsCoeff.nelements());
  for (int i=0; i<value.nelements(); ++i) {
    coeff[i] = vals[i];
    itsParmValue.rep().itsCoeff[i] = vals[i];
  }
}

void MeqFunklet::update (const vector<double>& values)
{
  double* coeff = itsCoeff.doubleStorage();
  int inx = itsScidInx;
  for (int i=0; i<itsCoeff.nelements(); i++) {
    if (isCoeffSolvable(i)) {
      DBGASSERT (inx < int(values.size()));
      coeff[i] = values[inx];
      itsParmValue.rep().itsCoeff[i] = values[inx];
      inx++;
    }
  }
}

}

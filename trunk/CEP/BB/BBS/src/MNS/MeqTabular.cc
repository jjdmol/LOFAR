//# MeqTabular.cc:  A tabular parameter value
//#
//# Copyright (C) 2006
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

#include <BBS/MNS/MeqTabular.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqResult.h>
#include <BBS/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR {


MeqTabular::MeqTabular (const ParmDB::ParmValue& pvalue)
: MeqFunklet (pvalue)
{
  const ParmDB::ParmValueRep& pval = pvalue.rep();
  ASSERTSTR (pval.itsType == "tabular",
	     "Funklet in ParmValue is not of type 'tabular'");
}

MeqTabular::~MeqTabular()
{}

MeqTabular* MeqTabular::clone() const
{
  return new MeqTabular(*this);
}

MeqResult MeqTabular::getResult (const MeqRequest& request,
				 int nrpert, int pertInx)
{
  ASSERTSTR (nrpert == 0,
	     "A tabular parameter value cannot be solvable");
  // It is not checked if the domain is valid.
  // In that way any value can be used for the default domain [-1,1].
  // Because the values are calculated for the center of each cell,
  // it is only checked if the centers are in the tabular domain.
  const MeqDomain& reqDomain = request.domain();
  MeqResult result(request.nspid());
  // If there is only one value, the value is independent of x and y.
  // Make sure it is turned into a scalar value.
  if (itsCoeff.nelements() == 1) {
    result.setValue (MeqMatrix(itsCoeff.getDouble()));
  } else if (itsCoeff.ny() == 1) {
    // The tabular has multiple values in frequency only.
    // Get number of steps and values in x and y.
    int ndx = request.nx();
    int ndy = request.ny();
    int ncx = itsCoeff.nx();
    int ncy = itsCoeff.ny();
    // Get the step and start values in the domain.
    double stepdx = (reqDomain.endX() - reqDomain.startX()) / ndx;
    double stepcx = (domain().endX() - domain().startX()) / ncx;
    // Evaluate the expression (as double).
    const double* coeffData = itsCoeff.doubleStorage();
    // Create matrix for the value itself and keep a pointer to its data.
    result.setValue (MeqMatrix(double(0), ndx, ndy));
    double* value = result.getValueRW().doubleStorage();
    // Iterate over all cells in the frequency domain.
    double valx = reqDomain.startX();
    for (int i=0; i<ndx; i++) {
      valx += stepdx;
    }
  }
  return result;
}

MeqResult MeqTabular::getAnResult (const MeqRequest& request,
				   int nrpert, int pertInx)
{
  return getResult (request, nrpert, pertInx);
}

}

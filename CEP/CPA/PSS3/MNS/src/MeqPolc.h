//# MeqPolc.h: Ordinary polynomial with coefficients valid for a given domain.
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

#if !defined(MNS_MEQPOLC_H)
#define MNS_MEQPOLC_H

//# Includes
#include <MNS/MeqDomain.h>
#include <MNS/MeqMatrix.h>
#include <Common/lofar_vector.h>

//# Forward declarations
class MeqRequest;
class MeqResult;


// This class contains an ordinary 2-dim with real or complex coefficients.
// It is valid for the given domain only.
// The domain is scaled between -1 and 1 to avoid large values for
// the high order terms. The coefficients are valid for the scaled
// domain.
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.

class MeqPolc
{
public:
  // Create an empty 2-dim polynomial.
  // By default a relative perturbation of 10^-6 is used.
  MeqPolc();

  // Calculate the value and possible perturbations.
  MeqResult getResult (const MeqRequest&);

  // Get number of coefficients.
  int ncoeff() const
    { return itsCoeff.nelements(); }

  // Get the coefficients.
  const MeqMatrix& getCoeff() const
    { return itsCoeff; }

  // Get the mask.
  const Matrix<bool>& mask() const;

  // Set the coefficients. The mask is set to all true.
  void setCoeff (const MeqMatrix& coeff);

  // Set the coefficients and mask.
  void setCoeff (const MeqMatrix& coeff, const Matrix<bool>& mask);

  // Get the domain.
  const MeqDomain& domain() const
    { return itsDomain; }

  // Set the domain.
  void setDomain (const MeqDomain& domain)
    { itsDomain = domain; }

  void setPerturbation (double perturbation = 1e-6,
			bool isRelativePerturbation = true)
    { itsPertValue = perturbation; itsIsRelPert = isRelativePerturbation; }

  // Make the polynomial non-solvable.
  void clearSolvable();

  // Make the parameters solvable, thus perturbed values have to be calculated.
  // spidIndex is the index of the first spid of this polc.
  // It returns the number of spids in this polc.
  int makeSolvable (int spidIndex);

  // Get the current values of the solvable parameter and store them
  // in the argument.
  void getInitial (MeqMatrix& values) const;

  // Get the current value of the solvable parameter and store it
  // in the argument.
  void getCurrentValue(MeqMatrix& value) const;

  // Update the solvable parameters with the new values.
  void update (const MeqMatrix& value);

private:
  MeqMatrix    itsCoeff;
  MeqMatrix    itsPerturbation;
  MeqDomain    itsDomain;
  vector<bool> itsMask;
  vector<int>  itsSpidInx;     //# -1 is not solvable
  int          itsMaxNrSpid;
  double       itsPertValue;
  bool         itsIsRelPert;   //# true = perturbation is relative
};


#endif

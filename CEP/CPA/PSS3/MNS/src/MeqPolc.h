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


// This class contains an ordinary 2-dim with real coefficients.
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

  // Set the coefficients. The mask is set to all true.
  void setCoeff (const MeqMatrix& coeff);

  // Set the coefficients and mask.
  void setCoeff (const MeqMatrix& coeff, const casa::Matrix<bool>& mask);

  // Set the coefficients only. The mask is left alone.
  void setCoeffOnly (const MeqMatrix& coeff);

  // Get the domain.
  const MeqDomain& domain() const
    { return itsDomain; }

  // Set the domain.
  void setDomain (const MeqDomain& domain)
    { itsDomain = domain; }

  // Get the perturbation.
  double getPerturbation() const
    { return itsPertValue; }
  bool isRelativePerturbation() const
    { return itsIsRelPert; }

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
  void getCurrentValue (MeqMatrix& value, bool denormalize) const;

  // Update the solvable parameters with the new values.
  void update (const MeqMatrix& value);

  // Set the original simulation coefficients.
  void setSimCoeff (const MeqMatrix& coeff)
    { itsSimCoeff = coeff; }

  // Set the perturbation of the simulation coefficients.
  void setPertSimCoeff (const MeqMatrix& coeff)
    { itsPertSimCoeff = coeff; }

  // Get the original simulation coefficients.
  const MeqMatrix& getSimCoeff() const
    { return itsSimCoeff; }

  // Get the perturbation of the simulation coefficients.
  const MeqMatrix& getPertSimCoeff() const
    { return itsPertSimCoeff; }

  // Set the zero-point of the function.
  void setX0 (double x0)
    { itsX0 = x0; }
  void setY0 (double y0)
    { itsY0 = y0; }

  // Get the zero-point of the function.
  double getX0() const
    { return itsX0; }
  double getY0() const
    { return itsY0; }

  // Tell if the coefficients have to be normalized.
  void setNormalize (bool normalize)
    { itsNormalized = normalize; }

  // Tell if the coefficients are normalized.
  bool isNormalized() const
    { return itsNormalized; }

  // Normalize the coefficients for the given domain.
  MeqMatrix normalize (const MeqMatrix& coeff, const MeqDomain&);

  // Denormalize the coefficients.
  MeqMatrix denormalize (const MeqMatrix& coeff) const;

  // (De)normalize real coefficients.
  static MeqMatrix normDouble (const MeqMatrix& coeff, double sx,
			       double sy, double ox, double oy);

private:
  // Fill Pascal's triangle.
  static void fillPascal();

  MeqMatrix    itsCoeff;
  MeqMatrix    itsSimCoeff;
  MeqMatrix    itsPertSimCoeff;
  MeqMatrix    itsPerturbation;
  MeqDomain    itsDomain;
  vector<bool> itsMask;
  vector<int>  itsSpidInx;     //# -1 is not solvable
  int          itsMaxNrSpid;
  double       itsPertValue;
  bool         itsIsRelPert;   //# true = perturbation is relative
  double       itsX0;
  double       itsY0;
  bool         itsNormalized;  //# true = coefficients normalized to domain

  //# Pascal's triangle for the binomial coefficients needed when normalizing.
  static double theirPascal[10][10];
  static bool   theirPascalFilled;
};


#endif

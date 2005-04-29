//# MeqFunklet.h: Parameter function with coefficients valid for a given domain
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

#ifndef MNS_MEQFUNKLET_H
#define MNS_MEQFUNKLET_H

// \file MNS/MeqFunklet.h
// Parameter function with coeffients valid for a given domain

//# Includes
#include <BBS3/MNS/MeqDomain.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <Common/lofar_vector.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward declarations
class MeqRequest;
class MeqResult;


// This class contains a 2-dim function with real coefficients.
// It is valid for the given domain only.
// The coefficients are numbered 0..N with the frequency as the most rapidly
// varying axis.

class MeqFunklet
{
public:
  // Create an empty 2-dim funklet.
  // By default a relative perturbation of 10^-6 is used.
  MeqFunklet();

  virtual ~MeqFunklet();

  // Calculate the value and possible perturbations.
  virtual MeqResult getResult (const MeqRequest&) = 0;

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

  // Make the coefficients non-solvable.
  void clearSolvable();

  // Make the coefficients solvable, thus perturbed values have to be
  // calculated.
  // spidIndex is the index of the first spid of this funklet.
  // It returns the number of spids in this funklet.
  int makeSolvable (int spidIndex);

  // Get the current value of the coefficients and store it
  // in the argument.
  void getCurrentValue (MeqMatrix& value) const
    { value = itsCoeff; }

  // Update the solvable coefficients with the new values taken at
  // the spid index from the vector.
  void update (const vector<double>& values);

  // Update the solvable coefficients with the new values.
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

  // Set the zero-points of the funklet.
  void setX0 (double x0)
    { itsX0 = x0; }
  void setY0 (double y0)
    { itsY0 = y0; }

  // Set the scales of the funklet.
  void setXScale (double xScale)
    { itsXScale = xScale; }
  void setYScale (double yScale)
    { itsYScale = yScale; }

  // Get the zero-points of the funklet.
  double getX0() const
    { return itsX0; }
  double getY0() const
    { return itsY0; }

  // Get the scales of the funklet.
  double getXScale() const
    { return itsXScale; }
  double getYScale() const
    { return itsYScale; }

  // Set the ID of the funklet in the database table.
  void setID (int id)
    { itsID = id; }

  // Get the ID of the funklet in the database table.
  int getID() const
    { return itsID; }

protected:
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
  double       itsXScale;
  double       itsYScale;
  int          itsID;          //# ID of funklet in database
                               //#   -1 = might be new
                               //#   -2 = certainly new
};

// @}

}

#endif

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

// \file
// Parameter function with coeffients valid for a given domain

//# Includes
#include <BBSKernel/MNS/MeqDomain.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <ParmDB/ParmValue.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
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

  // Create the funklet from a parm value.
  explicit MeqFunklet (const LOFAR::ParmDB::ParmValue& pvalue);

  // Copy constructor.
  MeqFunklet (const MeqFunklet&);

  virtual ~MeqFunklet();

  // Assignment.
  MeqFunklet& operator= (const MeqFunklet&);

  // Clone the funklet.
  virtual MeqFunklet* clone() const = 0;

  // Create the correct derived funklet from a ParmValue.
  // The caller is responsible for deleting the newly created object.
  static MeqFunklet* make (const LOFAR::ParmDB::ParmValue&, const std::string& name);

  // Calculate the value and possible perturbations.
  virtual MeqResult getResult (const MeqRequest&,
			       int nrpert, int pertInx) = 0;

  // Get number of coefficients.
  int ncoeff() const
    { return itsCoeff.nelements(); }

  // Set the coefficients and optional mask.
  void setCoeff (const MeqMatrix& value, const bool* mask = 0);

  // Get the coefficients.
  const MeqMatrix& getCoeff() const
    { return itsCoeff; }

  // Get the mask.
  const vector<bool>& getSolvMask() const
    { return itsParmValue.rep().itsSolvMask; }

  // Get the perturbation factor.
  double getPerturbation() const
    { return itsParmValue.rep().itsPerturbation; }

  // Get the perturbation of the i-th coefficient.
  double getPerturbation (int i) const
    { return itsCoeffPert.getDouble (i,0); }

  // Set the domain.
  void setDomain (const MeqDomain&);

  // Get the domain.
  const MeqDomain& domain() const
    { return itsDomain; }

  // Make the coefficients non-solvable.
  void clearSolvable();

  // Make the coefficients solvable, thus perturbed values have to be
  // calculated.
  // scidIndex is the index of the first scid of this funklet.
  // It returns the number of scids in this funklet.
  int makeSolvable (int scidIndex);

  // Get the number of scids and the scid index.
  // <group>
  int nscid() const
    { return itsNrScid; }
  int scidInx() const
    { return itsScidInx; }
  // </group>

  // Get the current value of the coefficients and store it
  // in the argument.
  void getCurrentValue (MeqMatrix& value) const
    { value = itsCoeff; }

  // Get the parm value.
  const LOFAR::ParmDB::ParmValue& getParmValue() const
    { return itsParmValue; }

  // Set to be a new parm value (i.e. a new record in the parm table).
  void setNewParm()
    { itsParmValue.setNewParm(); }

  // Update the solvable coefficients with the new values taken at
  // the scid index from the vector.
  void update (const vector<double>& coeff);

  // Update the solvable coefficients with the new values taken from the vector
  // starting from the given offset.
  void update (const vector<double>& coeff, size_t offset);

  // Update the solvable coefficients with the new values.
  void update (const MeqMatrix& value);


protected:
  bool isCoeffSolvable (int coeffnr) const
    { return itsParmValue.rep().itsSolvMask[coeffnr]; }

  MeqMatrix    itsCoeff;
  MeqMatrix    itsCoeffPert;
  MeqDomain    itsDomain;
  int          itsNrScid;
  int          itsScidInx;           //# Index of first solvable coeff.
  LOFAR::ParmDB::ParmValue itsParmValue;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

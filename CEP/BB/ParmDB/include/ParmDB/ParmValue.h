//# ParmValue.h: Value of a parm for a given domain
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

#ifndef LOFAR_PARMDB_PARMVALUE_H
#define LOFAR_PARMDB_PARMVALUE_H

// \file
// Ordinary polynomial with coefficients valid for a given domain

//# Includes
#include <ParmDB/ParmDomain.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace ParmDB {

// \ingroup ParmDB
// @{

// This class defines a polynomial as a specialization of a funklet.
//
// It scales the X and Y values between 0 and 1. So the value to use for X is:
//        (X - domain.startX()) / (domain.endX() - domain.startX())
// Y is to be calculated similarly.

class ParmValueRep
{
public:
  // Create a default ParmValue object.
  ParmValueRep();

  // Copy (except itsCount).
  void copy (const ParmValueRep&);

  // Set the funklet type and possible constants needed.
  // <group>
  void setType (const std::string& type)
    { itsType = type; itsConstants = std::vector<double>(); }
  void setType (const std::string& type, const std::vector<double>& constants)
    { itsType = type; itsConstants = constants; }
  // </group>

  // Set the coefficients for a funklet with the given shape.
  // The solvableMask tells which coefficients are solvable
  // (false is no solvable).
  // If no mask is given, a default is used where the right lower half
  // of the mask array is set to false.
  // <group>
  void setCoeff (const double* coeff, const std::vector<int>& shape);
  void setCoeff (const double* coeff, const bool* solvableMask,
		 const std::vector<int>& shape);
  // </group>

  // Set the domain. It initializes offset and scale such that the domain
  // is scaled to 0:1. If needed, the user can later overwrite the offset
  // and scale.
  void setDomain (const ParmDomain&);

  // Set the perturbation to be used when calculating numerical derivatives.
  void setPerturbation (double perturbation, bool isRelative=true)
    { itsPerturbation = perturbation; itsIsRelPert = isRelative; }

  // Helper function to make vectors for the common case of 2-dim parms.
  // <group>
  static std::vector<int> makeShape (int nx, int ny);
  static std::vector<double> makeVecDouble (double x, double y);
  // </group>

  std::string         itsType;        //# funklet type or expression
  std::vector<double> itsConstants;   //# possible funklet constants
  std::vector<double> itsCoeff;
  std::vector<bool>   itsSolvMask;    //# false = coefficient is not solvable
  std::vector<int>    itsShape;       //# shape of coeff and mask
  double              itsPerturbation;
  bool                itsIsRelPert;   //# true = perturbation is relative
  ParmDomain          itsDomain;
  std::vector<double> itsOffset;
  std::vector<double> itsScale;
  double         itsWeight;
  int            itsID;        //# ID if result of a refit
  int            itsParentID;  //# ID of refit result (in old parm records)
  int            itsDBTabRef;  //# Ref to table in database
                               //#   -3 = might be new record; no check partial
                               //#   -2 = might be new record; check on partial
                               //#   -1 = certainly new record
                               //#    0 = read from normal table
                               //#    1 = read from old table
  int            itsDBRowRef;  //# Ref to parmrecord in table (e.g. rownr)
  int            itsDBSeqNr;   //# The ParmDB seqnr

private:
  // Copy and assignment cannot be done.
  // <group>
  ParmValueRep (const ParmValueRep&);
  ParmValueRep& operator= (const ParmValueRep&);
  // </group>

  // Get the length of the coefficients array/
  int getLength (const std::vector<int>& shape) const;

  int itsCount;         //# reference count

public:
  // Reference counting functions.
  // <group>
  ParmValueRep* link()
    { itsCount++; return this; }
  static void unlink (ParmValueRep* rep)
    { if (--(rep->itsCount) <= 0) delete rep; }
  // </group>
};


class ParmValue
{
public:
  ParmValue();

  // Copy with reference semantics.
  // <group>
  ParmValue (const ParmValue&);
  ParmValue& operator= (const ParmValue&);
  // </group>

  ~ParmValue();

  // Copy semantics.
  ParmValue clone() const;

  // Set it to be a new parm value.
  // It is not checked if there is already a parm value with an
  // overlapping domain.
  void setNewParm()
    { rep().itsDBTabRef = -1; }

  // Set that it is unknown if the parm value is new or not.
  // If the domain matches exactly with a domain in the table it is existing.
  // Otherwise it is new. In that case it can be checked if the parm is
  // really new, thus if no partial overlap with an existing domain occurs.
  // Optionally it can be checked 
  void setUnknownParm (bool checkPartialOverlap)
    { rep().itsDBTabRef = (checkPartialOverlap ? -2 : -3); }

  // Get access to the actual data.
  ParmValueRep& rep()
    { return *itsRep; }
  const ParmValueRep& rep() const
    { return *itsRep; }

private:
  ParmValueRep* itsRep;
};



class ParmValueSetRep
{
public:
  explicit ParmValueSetRep (const std::string& parmName)
    : itsName(parmName), itsCount(1)
  {}

  std::string            itsName;
  std::vector<ParmValue> itsValue;

private:
  // Copies cannot be made.
  // <group>
  ParmValueSetRep (const ParmValueSetRep&);
  ParmValueSetRep& operator= (const ParmValueSetRep&);
  // </group>

  int itsCount;         //# reference count

public:
  // Reference counting functions.
  // <group>
  ParmValueSetRep* link()
    { itsCount++; return this; }
  static void unlink (ParmValueSetRep* rep)
    { if (--(rep->itsCount) <= 0) delete rep; }
  // </group>
};


class ParmValueSet
{
public:
  ParmValueSet (const std::string& parmName = "");
  ParmValueSet (const ParmValueSet&);
  ParmValueSet& operator= (const ParmValueSet&);
  ~ParmValueSet();

  // Get access to the actual data.
  const std::string& getName() const
    { return itsRep->itsName; }
  std::vector<ParmValue>& getValues()
    { return itsRep->itsValue; }
  const std::vector<ParmValue>& getValues() const
    { return itsRep->itsValue; }

private:
  ParmValueSetRep* itsRep;
};

// @}

} // namespace ParmDB
} // namespace LOFAR

#endif

//# TFRange.h: The range of an expression for a domain.
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

#if !defined(MNS_TFRANGE_H)
#define MNS_TFRANGE_H

//# Includes
#include <MNS/MnsMatrix.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>

// This class represents a range of a domain which an expression has been
// evaluated.

class TFRangeRep
{
public:
  // Create a time,frequency range for the given number of spids.
  TFRangeRep (int nspid);

  ~TFRangeRep();

  TFRangeRep* link()
    { itsCount++; return this; }

  static void unlink (TFRangeRep*);

  // Get the value.
  const MnsMatrix& getValue()
    { return itsValue; }

  // Is the i-th perturbed value defined?
  bool isDefined (int i) const
    { return itsPerturbedValues[i] != 0; }

  // Get the i-th perturbed value.
  const MnsMatrix& getPerturbedValue (int i) const
    { return (itsPerturbedValues[i]  ?  *(itsPerturbedValues[i]) : itsValue); }

  // Set the i-th perturbed parameter.
  double getPerturbation (int i)
    { return itsPerturbation[i]; }

  // Set the value.
  void setValue (const MnsMatrix&);
  
  // Set the i-th perturbed value.
  void setPerturbedValue (int i, const MnsMatrix&);

  // Set the i-th perturbed parameter.
  void setPerturbation (int i, double value)
    { itsPerturbation[i] = value; }

  void show (ostream&) const;

private:
  // Forbid copy and assignment.
  TFRangeRep (const TFRangeRep&);
  TFRangeRep& operator= (const TFRangeRep&);

  int       itsCount;
  MnsMatrix itsValue;
  vector<MnsMatrix*> itsPerturbedValues;
  vector<double>     itsPerturbation;
};



class TFRange
{
public:
  // Create a time,frequency range for the given number of parameters.
  TFRange (int nspid = 0);

  TFRange (const TFRange&);

  ~TFRange()
    { TFRangeRep::unlink (itsRep); }

  TFRange& operator= (const TFRange&);

  // Get the value.
  const MnsMatrix& getValue()
    { return itsRep->getValue(); }

  // Is the i-th perturbed value defined?
  bool isDefined (int i) const
    { return itsRep->isDefined(i); }

  // Get the i-th perturbed value.
  const MnsMatrix& getPerturbedValue (int i) const
    { return itsRep->getPerturbedValue(i); }

  // Get the i-th perturbed parameter.
  double getPerturbation (int i)
    { return itsRep->getPerturbation (i); }

  // Set the value.
  void setValue (const MnsMatrix& value)
    { itsRep->setValue (value); }
  
  // Set the i-th perturbed value.
  void setPerturbedValue (int i, const MnsMatrix& value)
    { itsRep->setPerturbedValue (i, value); }

  // Set the i-th perturbed parameter.
  void setPerturbation (int i, double value)
    { itsRep->setPerturbation (i, value); }

  friend ostream& operator<< (ostream& os, const TFRange& range)
    { range.itsRep->show (os); return os; }

private:
  TFRangeRep* itsRep;
};


#endif

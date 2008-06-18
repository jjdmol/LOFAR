//# MeqResult.h: The result of an expression for a domain.
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

#if !defined(MNS_MEQRESULT_H)
#define MNS_MEQRESULT_H

// \file
// The result of an expression for a domain.

//# Includes
#include <BBSKernel/MNS/MeqMatrix.h>
//#include <BBSKernel/MNS/Pool.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/lofar_iostream.h>

// This class represents a result of a domain for which an expression
// has been evaluated.

namespace LOFAR
{
namespace BBS
{

//# Forward Declarations
class MeqParmFunklet;

// \ingroup BBSKernel
// \addtogroup MNS
// @{


class MeqResultRep
{
public:
  // Create a time,frequency result for the given number of spids.
  explicit MeqResultRep (int nspid);

  ~MeqResultRep();

  MeqResultRep* link()
    { itsCount++; return this; }

  static void unlink (MeqResultRep* rep)
    { if (rep != 0 && --rep->itsCount == 0) delete rep; }

//  static void *operator new(size_t size);
//  static void operator delete(void *rep);

  // Get the value.
  const MeqMatrix& getValue() const
    { return itsValue; }
  MeqMatrix& getValueRW()
    { return itsValue; }

  // Is the i-th perturbed value defined?
  bool isDefined (int i) const
    { return itsPerturbedValues != 0 && /*i<itsNspid &&*/ itsPerturbedValues->find(i) != itsPerturbedValues->end(); }

  // Get the i-th perturbed value.
  const MeqMatrix& getPerturbedValue (int i) // const
    {
      if (itsPerturbedValues == 0) return itsValue;
      map<int, MeqMatrix>::iterator it = itsPerturbedValues->find(i);
      return it == itsPerturbedValues->end() ? itsValue : it->second;
    }

  MeqMatrix& getPerturbedValueRW (int i)
    {
      if (itsPerturbedValues == 0) itsPerturbedValues = new map<int,MeqMatrix>;
      return (*itsPerturbedValues)[i];
    }

  // Get the i-th parameter perturbation info.
  const MeqParmFunklet* getPerturbedParm (int i) const
    {
      DBGASSERT (itsPerturbedParms != 0);
      map<int,const MeqParmFunklet*>::iterator it = itsPerturbedParms->find(i);
      DBGASSERT (it != itsPerturbedParms->end());
      return it->second;
    }

  // Get the perturbation for j-th domain in the i-th perturbed value.
  double getPerturbation (int i, int j) const;

  // Set the value.
  // The current value is replaced by the new one.
  void setValue (const MeqMatrix& value)
    { itsValue = value; }

  // Set the value with a given type and shape.
  // It won't change if the current value type and shape match.
  double* setDoubleFormat (int nx, int ny)
    { return itsValue.setDoubleFormat (nx, ny); }

  // Remove all perturbed values.
  void clear();

  int nperturbed() const
    { return itsNspid; }

  // Set the i-th perturbed value.
  void setPerturbedValue (int i, const MeqMatrix&);

  // Set the i-th perturbed parameter.
  void setPerturbedParm (int i, const MeqParmFunklet* parm);

  void show (ostream&) const;

private:
  // Forbid copy and assignment.
  MeqResultRep (const MeqResultRep&);
  MeqResultRep& operator= (const MeqResultRep&);

  int			           itsCount;
  int			           itsNspid;
  MeqMatrix		           itsValue;
  map<int, MeqMatrix>*	           itsPerturbedValues;
  map<int, const MeqParmFunklet*>* itsPerturbedParms;
};



class MeqResult
{
public:
  // Create a null object.
  MeqResult()
    : itsRep(0) {}

  // Create a time,frequency result for the given number of parameters.
  explicit MeqResult (int nspid);

  MeqResult (const MeqResult&);

  ~MeqResult()
    { MeqResultRep::unlink (itsRep); }

  MeqResult& operator= (const MeqResult&);

  // Get the value.
  const MeqMatrix& getValue() const
    { return itsRep->getValue(); }
  MeqMatrix& getValueRW()
    { return itsRep->getValueRW(); }

  // Is the i-th perturbed value defined?
  bool isDefined (int i) const
    { return itsRep->isDefined(i); }

  // Get the i-th perturbed value.
  const MeqMatrix& getPerturbedValue (int i) const
    { return itsRep->getPerturbedValue(i); }
  MeqMatrix& getPerturbedValueRW (int i)
    { return itsRep->getPerturbedValueRW(i); }

  // Get the i-th perturbed parameter.
  const MeqParmFunklet* getPerturbedParm (int i) const
    { return itsRep->getPerturbedParm (i); }

  // Get the perturbation for j-th domain in the i-th perturbed value
  double getPerturbation (int i, int j=0) const
    { return itsRep->getPerturbation (i, j); }

  // Set the value.
  void setValue (const MeqMatrix& value)
    { itsRep->setValue (value); }
  
  // Set the value with a given type and shape.
  // It won't change if the current value type and shape matches.
  // It returns a pointer to the internal storage.
  double* setDoubleFormat (int nx, int ny)
    { return itsRep->setDoubleFormat (nx, ny); }

  // Remove all perturbed values.
  void clear()
    { itsRep->clear(); }

  int nperturbed() const
    { return itsRep->nperturbed(); }

  // Set the i-th perturbed value.
  void setPerturbedValue (int i, const MeqMatrix& value)
    { itsRep->setPerturbedValue (i, value); }

  // Set the i-th perturbed parameter.
  void setPerturbedParm (int i, const MeqParmFunklet* parm)
    { itsRep->setPerturbedParm (i, parm); }

  friend ostream& operator<< (ostream& os, const MeqResult& result)
    { result.itsRep->show (os); return os; }

private:
  MeqResultRep* itsRep;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

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

//# Includes
#include <PSS3/MNS/MeqMatrix.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>

// This class represents a result of a domain for which an expression
// has been evaluated.

class MeqResultRep
{
public:
  // Create a time,frequency result for the given number of spids.
  explicit MeqResultRep (int nspid);

  ~MeqResultRep();

  MeqResultRep* link()
    { itsCount++; return this; }

  static void unlink (MeqResultRep* rep)
    { if (rep != 0  &&  --rep->itsCount == 0) delete rep; }

  // Get the value.
  const MeqMatrix& getValue() const
    { return itsValue; }
  MeqMatrix& getValueRW()
    { return itsValue; }

  // Is the i-th perturbed value defined?
  bool isDefined (int i) const
    { return i<int(itsPerturbedValues.size()) && itsPerturbedValues[i] != 0; }

  // Get the i-th perturbed value.
  const MeqMatrix& getPerturbedValue (int i) const
    { return (isDefined(i)  ?  *(itsPerturbedValues[i]) : itsValue); }
  MeqMatrix& getPerturbedValueRW (int i)
    { return *(itsPerturbedValues[i]); }

  // Get the i-th perturbed parameter.
  const MeqMatrix& getPerturbation (int i) const
    { return (isDefined(i)  ?  *(itsPerturbation[i]) : itsDefPert); }

  // Set the value.
  // The current value is replaced by the new one.
  void setValue (const MeqMatrix& value)
    { itsValue = value; }

  // Set the value with a given type and shape.
  // It won't change if the current value type and shape match.
  double* setDouble (int nx, int ny)
    { return itsValue.setDouble (nx, ny); }
  complex<double>* setDComplex (int nx, int ny)
    { return itsValue.setDComplex (nx, ny); }

  // Remove all perturbed values.
  void clear();

  int nperturbed() const
    { return itsPerturbation.size(); }

  // Set the i-th perturbed value.
  void setPerturbedValue (int i, const MeqMatrix&);

  // Set the i-th perturbed value with a given type and shape.
  // It won't change if the current value type and shape matches.
  double* setPerturbedDouble (int i, int nx, int ny)
    { if (!isDefined(i)) {
        itsPerturbedValues[i] = new MeqMatrix();
      }
      return itsPerturbedValues[i]->setDouble (nx, ny);
    } 
  complex<double>* setPerturbedDComplex (int i, int nx, int ny)
    { if (!isDefined(i)) {
        itsPerturbedValues[i] = new MeqMatrix();
      }
      return itsPerturbedValues[i]->setDComplex (nx, ny);
    } 
  
  // Set the i-th perturbed parameter.
  void setPerturbation (int i, const MeqMatrix& value)
    { itsPerturbation[i] = new MeqMatrix(value); }
  void setPerturbation (int i, const double& value)
    { itsPerturbation[i] = new MeqMatrix(value); }
  void setPerturbation (int i, const complex<double>& value)
    { itsPerturbation[i] = new MeqMatrix(value); }

  void show (ostream&) const;

  static int nctor;
  static int ndtor;

private:
  // Forbid copy and assignment.
  MeqResultRep (const MeqResultRep&);
  MeqResultRep& operator= (const MeqResultRep&);

  int       itsCount;
  MeqMatrix itsValue;
  MeqMatrix itsDefPert;
  ///  int       itsFirstPert;     // spid of first perturbed value
  vector<MeqMatrix*> itsPerturbedValues;
  vector<MeqMatrix*> itsPerturbation;
};



class MeqResult
{
public:
  // Create a time,frequency result for the given number of parameters.
  explicit MeqResult (int nspid = 0);

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
  const MeqMatrix& getPerturbation (int i) const
    { return itsRep->getPerturbation (i); }

  // Set the value.
  void setValue (const MeqMatrix& value)
    { itsRep->setValue (value); }
  
  // Set the value with a given type and shape.
  // It won't change if the current value type and shape matches.
  // It returns a pointer to the internal storage.
  double* setDouble (int nx, int ny)
    { return itsRep->setDouble (nx, ny); }
  complex<double>* setDComplex (int nx, int ny)
    { return itsRep->setDComplex (nx, ny); }

  // Remove all perturbed values.
  void clear()
    { itsRep->clear(); }

  int nperturbed() const
    { return itsRep->nperturbed(); }

  // Set the i-th perturbed value.
  void setPerturbedValue (int i, const MeqMatrix& value)
    { itsRep->setPerturbedValue (i, value); }

  // Set the i-th perturbed value with a given type and shape.
  // It won't change if the current value type and shape matches.
  double* setPerturbedDouble (int i, int nx, int ny)
    { return itsRep->setPerturbedDouble (i, nx, ny); }
  complex<double>* setPerturbedDComplex (int i, int nx, int ny)
    { return itsRep->setPerturbedDComplex (i, nx, ny); }

  // Set the i-th perturbed parameter.
  void setPerturbation (int i, const MeqMatrix& value)
    { itsRep->setPerturbation (i, value); }
  void setPerturbation (int i, double value)
    { itsRep->setPerturbation (i, value); }
  void setPerturbation (int i, const complex<double>& value)
    { itsRep->setPerturbation (i, value); }

  friend ostream& operator<< (ostream& os, const MeqResult& result)
    { result.itsRep->show (os); return os; }

private:
  MeqResultRep* itsRep;
};


#endif

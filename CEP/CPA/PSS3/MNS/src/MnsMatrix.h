//# MnsMatrix.h: Matrix for Mns
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

#if !defined(MNS_MNSVECTOR_H)
#define MNS_MNSVECTOR_H


//# Includes
#include <MNS/MnsMatrixRep.h>

//# Forward Declarations
class MnsMatrixTmp;
template<class T> class Matrix;


class MnsMatrix
{
public:
  // A null vector (i.e. no vector assigned yet).
  // This can be used to clear the 'cache' in the Mns.
  MnsMatrix()
    : itsRep(0) {}

  // Create a scalar MnsMatrix.
  // <group>
  MnsMatrix (double);
  MnsMatrix (complex<double>);
  // <group>

  // Create a MnsMatrix from a value array.
  // <group>
  MnsMatrix (const double* values, int nx, int ny);
  MnsMatrix (const complex<double>* values, int nx, int ny);
  MnsMatrix (const Matrix<double>&);
  MnsMatrix (const Matrix<complex<double> >&);
  // </group>

  // Create a MnsMatrix from a MnsMatrixRep.
  // It takes over the pointer and deletes it in the destructor.
  MnsMatrix (MnsMatrixRep* rep)
    : itsRep (rep->link()) {}

  // Create a MnsMatrix from a temporary one (reference semantics).
  MnsMatrix (const MnsMatrixTmp& that);

  // Copy constructor (reference semantics).
  MnsMatrix (const MnsMatrix& that);

  ~MnsMatrix()
    { MnsMatrixRep::unlink (itsRep); }

  MnsMatrix& operator= (const MnsMatrix& other);
  MnsMatrix& operator= (const MnsMatrixTmp& other);

  int nx() const
    { return itsRep->nx(); }

  int ny() const
    { return itsRep->ny(); }

  int nelements() const
    { return itsRep->nelements(); }

  int elemLength() const
    { return itsRep->elemLength(); }

  Bool isNull() const
    { return (itsRep == 0); }

  void show (ostream& os) const
    { itsRep->show (os); }

  Matrix<double> getDoubleMatrix() const;

  const double* doubleStorage() const
    { return itsRep->doubleStorage(); }

  double* doubleStorage()
    { return (double*)(itsRep->doubleStorage()); }

  double getDouble (int x, int y) const
    { return itsRep->getDouble (x, y); }

  double getDouble() const
    { return itsRep->getDouble (0, 0); }

  complex<double> getDComplex() const
    { return itsRep->getDComplex (0, 0); }

  MnsMatrixRep* rep() const
    { return itsRep; }

  MnsMatrixTmp operator+ (const MnsMatrix& right) const;
  MnsMatrixTmp operator+ (const MnsMatrixTmp& right) const;

  MnsMatrixTmp operator- (const MnsMatrix& right) const;
  MnsMatrixTmp operator- (const MnsMatrixTmp& right) const;

  MnsMatrixTmp operator* (const MnsMatrix& right) const;
  MnsMatrixTmp operator* (const MnsMatrixTmp& right) const;

  MnsMatrixTmp operator/ (const MnsMatrix& right) const;
  MnsMatrixTmp operator/ (const MnsMatrixTmp& right) const;

  MnsMatrixTmp operator-() const;

  MnsMatrixTmp sin() const;
  MnsMatrixTmp cos() const;
  MnsMatrixTmp exp() const;
  MnsMatrixTmp conj() const; 
  MnsMatrixTmp min() const;
  MnsMatrixTmp max() const;
  MnsMatrixTmp mean() const;
  MnsMatrixTmp sum() const;


private:
  MnsMatrixRep* itsRep;
};


inline ostream& operator<< (ostream& os, const MnsMatrix& vec)
  { vec.show (os); return os; }


#endif

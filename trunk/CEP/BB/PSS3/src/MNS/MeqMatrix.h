//# MeqMatrix.h: Matrix for Mns
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

#if !defined(MNS_MEQMATRIX_H)
#define MNS_MEQMATRIX_H


//# Includes
#include <PSS3/MNS/MeqMatrixRep.h>

//# Forward Declarations
namespace casa {
template<class T> class Matrix;
}
namespace LOFAR {
class BlobOStream;
class BlobIStream;
class MeqMatrixTmp;
}

namespace LOFAR {

class MeqMatrix
{
public:
  // A null vector (i.e. no vector assigned yet).
  // This can be used to clear the 'cache' in the Mns.
  MeqMatrix()
    : itsRep(0) {}

  // Create a scalar MeqMatrixTmp.
  // <group>
  explicit MeqMatrix (double value);
  explicit MeqMatrix (dcomplex value);
  // <group>

  // Create a MeqMatrix of given size.
  // If the init flag is true, the matrix is initialized to the given value.
  // Otherwise the value only indicates the type of matrix to be created.
  // <group>
  MeqMatrix (double, int nx, int ny, bool init=true);
  MeqMatrix (dcomplex, int nx, int ny, bool init=true);
  // <group>

  // Create a MeqMatrix from a value array.
  // <group>
  MeqMatrix (const double* values, int nx, int ny);
  MeqMatrix (const dcomplex* values, int nx, int ny);
  MeqMatrix (const casa::Matrix<double>&);
  MeqMatrix (const casa::Matrix<dcomplex>&);
  // </group>

  // Create a MeqMatrix from a MeqMatrixRep.
  // It takes over the pointer and deletes it in the destructor.
  explicit MeqMatrix (MeqMatrixRep* rep)
    : itsRep (rep->link()) {}

  // Create a MeqMatrix from a temporary one (reference semantics).
  MeqMatrix (const MeqMatrixTmp& that);

  // Copy constructor (reference semantics).
  MeqMatrix (const MeqMatrix& that);

  ~MeqMatrix()
    { MeqMatrixRep::unlink (itsRep); }

  // Assignment (reference semantics).
  MeqMatrix& operator= (const MeqMatrix& other);
  MeqMatrix& operator= (const MeqMatrixTmp& other);

  // Clone the matrix (copy semantics).
  MeqMatrix clone() const
    { return MeqMatrix (itsRep->clone()); }

  // Change the type/or shape if different.
  double* setDouble (int nrx, int nry)
    { if (itsRep == 0  ||  !isDouble()  ||  nrx != nx()  ||  nry != ny()) {
        setDMat (nrx, nry);
      }
      return doubleStorage();
    }
  dcomplex* setDComplex (int nrx, int nry)
    { if (itsRep == 0  ||  isDouble()  ||  nrx != nx()  ||  nry != ny()) {
        setDCMat (nrx, nry);
      }
      return dcomplexStorage();
    }
  void setDMat (int nx, int ny);
  void setDCMat (int nx, int ny);

  int nx() const
    { return itsRep->nx(); }

  int ny() const
    { return itsRep->ny(); }

  int nelements() const
    { return itsRep->nelements(); }

  int elemLength() const
    { return itsRep->elemLength(); }

  casa::Bool isNull() const
    { return (itsRep == 0); }

  void show (ostream& os) const
    { itsRep->show (os); }

  bool isDouble() const
    { return itsRep->isDouble(); }

  casa::Matrix<double> getDoubleMatrix() const;
  casa::Matrix<dcomplex> getDComplexMatrix() const;

  const double* doubleStorage() const
    { return itsRep->doubleStorage(); }

  double* doubleStorage()
    { return (double*)(itsRep->doubleStorage()); }

  const dcomplex* dcomplexStorage() const
    { return itsRep->dcomplexStorage(); }

  dcomplex* dcomplexStorage()
    { return (dcomplex*)(itsRep->dcomplexStorage()); }

  double getDouble (int x, int y) const
    { return itsRep->getDouble (x, y); }

  double getDouble() const
    { return itsRep->getDouble (0, 0); }

  dcomplex getDComplex (int x, int y) const
    { return itsRep->getDComplex (x, y); }

  dcomplex getDComplex() const
    { return itsRep->getDComplex (0, 0); }

  MeqMatrixRep* rep() const
    { return itsRep; }

  void operator+= (const MeqMatrix& right);
  void operator+= (const MeqMatrixTmp& right);

  void operator-= (const MeqMatrix& right);
  void operator-= (const MeqMatrixTmp& right);

  void operator*= (const MeqMatrix& right);
  void operator*= (const MeqMatrixTmp& right);

  void operator/= (const MeqMatrix& right);
  void operator/= (const MeqMatrixTmp& right);

  MeqMatrixTmp operator+ (const MeqMatrix& right) const;
  MeqMatrixTmp operator+ (const MeqMatrixTmp& right) const;

  MeqMatrixTmp operator- (const MeqMatrix& right) const;
  MeqMatrixTmp operator- (const MeqMatrixTmp& right) const;

  MeqMatrixTmp operator* (const MeqMatrix& right) const;
  MeqMatrixTmp operator* (const MeqMatrixTmp& right) const;

  MeqMatrixTmp operator/ (const MeqMatrix& right) const;
  MeqMatrixTmp operator/ (const MeqMatrixTmp& right) const;

  MeqMatrixTmp operator-() const;

  friend MeqMatrixTmp posdiff (const MeqMatrix&, const MeqMatrix&);
  friend MeqMatrixTmp posdiff (const MeqMatrix&, const MeqMatrixTmp&);
  friend MeqMatrixTmp tocomplex (const MeqMatrix&, const MeqMatrix&);
  friend MeqMatrixTmp tocomplex (const MeqMatrix&, const MeqMatrixTmp&);
  friend MeqMatrixTmp sin (const MeqMatrix&);
  friend MeqMatrixTmp cos (const MeqMatrix&);
  friend MeqMatrixTmp exp (const MeqMatrix&);
  friend MeqMatrixTmp sqr (const MeqMatrix&);
  friend MeqMatrixTmp sqrt(const MeqMatrix&);
  friend MeqMatrixTmp conj(const MeqMatrix&);
  friend MeqMatrixTmp min (const MeqMatrix&);
  friend MeqMatrixTmp max (const MeqMatrix&);
  friend MeqMatrixTmp mean(const MeqMatrix&);
  friend MeqMatrixTmp sum (const MeqMatrix&);


private:
  MeqMatrixRep* itsRep;
};


inline ostream& operator<< (ostream& os, const MeqMatrix& vec)
  { vec.show (os); return os; }

LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& os, const MeqMatrix& vec);

LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& os, MeqMatrix& vec);

}

#endif

//# Matrix.h: Matrix for Mns
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#if !defined(EXPR_MATRIX_H)
#define EXPR_MATRIX_H

// \file
// Matrix for Mns

//# Includes
#include <BBSKernel/Expr/MatrixRep.h>

//# Forward Declarations
//#namespace casa
//#{
//#    template<class T> class Matrix;
//#}

namespace LOFAR
{
    class BlobOStream;
    class BlobIStream;

    namespace BBS
    {
        class MatrixTmp;
    }
}

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class Matrix
{
public:
  // A null vector (i.e. no vector assigned yet).
  // This can be used to clear the 'cache' in the Mns.
  Matrix()
    : itsRep(0) {}

  // @{
  // Create a scalar Matrix.
  explicit Matrix (double value);
  explicit Matrix (dcomplex value);
  // @}

  // Create a Matrix of given size.
  // If the init flag is true, the matrix is initialized to the given value.
  // Otherwise the value only indicates the type of matrix to be created.
  // @{
  Matrix (double, int nx, int ny, bool init=true);
  Matrix (dcomplex, int nx, int ny, bool init=true);
  // @}

  // Create a Matrix from a value array.
  // @{
  Matrix (const double* values, int nx, int ny);
  Matrix (const dcomplex* values, int nx, int ny);
//#  Matrix (const casa::Matrix<double>&);
//#  Matrix (const casa::Matrix<dcomplex>&);
  // @}

  // Create a Matrix from a MatrixRep.
  // It takes over the pointer and deletes it in the destructor.
  explicit Matrix (MatrixRep* rep)
    : itsRep (rep->link()) {}

  // Create a Matrix from a temporary one (reference semantics).
  Matrix (const MatrixTmp& that);

  // Copy constructor (reference semantics).
  Matrix (const Matrix& that);

  ~Matrix()
    { MatrixRep::unlink (itsRep); }

  // Assignment (reference semantics).
  Matrix& operator= (const Matrix& other);
  Matrix& operator= (const MatrixTmp& other);

  // Clone the matrix (copy semantics).
  Matrix clone() const;

  // Change the type/or shape if different.
  double* setDoubleFormat (int nrx, int nry)
    { if (itsRep == 0  ||  isComplex()  ||  nrx != nx()  ||  nry != ny()) {
        setDMat (nrx, nry);
      }
      return doubleStorage();
    }
#if 0
  dcomplex* setDComplexFormat (int nrx, int nry)
    { if (itsRep == 0  ||  !isComplex()  ||  nrx != nx()  ||  nry != ny()) {
        setDCMat (nrx, nry);
      }
      return dcomplexStorage();
    }
#endif
  void setDMat (int nx, int ny);
  void setDCMat (int nx, int ny);

  int nx() const
    { return itsRep->nx(); }

  int ny() const
    { return itsRep->ny(); }

  int nelements() const
    { return itsRep->nelements(); }

  bool isNull() const
    { return (itsRep == 0); }

  void show (ostream& os) const
    { itsRep->show (os); }

  bool isArray() const
    { return itsRep->isArray(); }

  bool isComplex() const
    { return itsRep->isComplex(); }

//#  casa::Matrix<double> getDoubleMatrix() const;
//#  casa::Matrix<dcomplex> getDComplexMatrix() const;

  const double* doubleStorage() const
    { return itsRep->doubleStorage(); }

  double* doubleStorage()
    { return (double*)(itsRep->doubleStorage()); }

  void dcomplexStorage(const double *&realPtr, const double *&imagPtr) const
    { itsRep->dcomplexStorage(realPtr, imagPtr); }

  void dcomplexStorage(double *&realPtr, double *&imagPtr)
    { itsRep->dcomplexStorage((const double *&) realPtr, (const double *&) imagPtr); }

  double getDouble (int x, int y) const
    { return itsRep->getDouble (x, y); }

  double getDouble() const
    { return itsRep->getDouble (0, 0); }

  dcomplex getDComplex (int x, int y) const
    { return itsRep->getDComplex (x, y); }

  dcomplex getDComplex() const
    { return itsRep->getDComplex (0, 0); }

  MatrixRep* rep() const
    { return itsRep; }

  void operator+= (const Matrix& right);
  void operator+= (const MatrixTmp& right);

  void operator-= (const Matrix& right);
  void operator-= (const MatrixTmp& right);

  void operator*= (const Matrix& right);
  void operator*= (const MatrixTmp& right);

  void operator/= (const Matrix& right);
  void operator/= (const MatrixTmp& right);

  MatrixTmp operator+ (const Matrix& right) const;
  MatrixTmp operator+ (const MatrixTmp& right) const;

  MatrixTmp operator- (const Matrix& right) const;
  MatrixTmp operator- (const MatrixTmp& right) const;

  MatrixTmp operator* (const Matrix& right) const;
  MatrixTmp operator* (const MatrixTmp& right) const;

  MatrixTmp operator/ (const Matrix& right) const;
  MatrixTmp operator/ (const MatrixTmp& right) const;

  MatrixTmp operator-() const;

  void fillRowWithProducts(dcomplex v0, dcomplex factor, int row)
    {
       itsRep->fillRowWithProducts(v0, factor, row);
    }

  friend MatrixTmp posdiff (const Matrix&, const Matrix&);
  friend MatrixTmp posdiff (const Matrix&, const MatrixTmp&);
  friend MatrixTmp tocomplex (const Matrix&, const Matrix&);
  friend MatrixTmp tocomplex (const Matrix&, const MatrixTmp&);
  friend MatrixTmp sin (const Matrix&);
  friend MatrixTmp cos (const Matrix&);
  friend MatrixTmp log (const Matrix&);
  friend MatrixTmp exp (const Matrix&);
  friend MatrixTmp log10 (const Matrix&);
  friend MatrixTmp pow10 (const Matrix&);
  friend MatrixTmp sqr (const Matrix&);
  friend MatrixTmp sqrt(const Matrix&);
  friend MatrixTmp conj(const Matrix&);
  friend MatrixTmp min (const Matrix&);
  friend MatrixTmp max (const Matrix&);
  friend MatrixTmp mean(const Matrix&);
  friend MatrixTmp sum (const Matrix&);


private:
  MatrixRep* itsRep;
};


inline ostream& operator<< (ostream& os, const Matrix& vec)
  { vec.show (os); return os; }

//#LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& os, const Matrix& vec);

//#LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& os, Matrix& vec);

// @}

} // namespace BBS
} // namespace LOFAR


#endif

//# Matrix.cc: Matrix for Mns
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

#include <lofar_config.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/MatrixRealSca.h>
#include <BBSKernel/Expr/MatrixComplexSca.h>
#include <BBSKernel/Expr/MatrixRealArr.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>
//#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

Matrix::Matrix (double value)
{
    MatrixRealSca* v = new MatrixRealSca (value);
    itsRep = v->link();
}

Matrix::Matrix (dcomplex value)
{
    MatrixComplexSca* v = new MatrixComplexSca (value);
    itsRep = v->link();
}

Matrix::Matrix (double value, int nx, int ny, bool init)
{
    MatrixRealArr* v = MatrixRealArr::allocate(nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

Matrix::Matrix (dcomplex value, int nx, int ny, bool init)
{
    MatrixComplexArr* v = MatrixComplexArr::allocate (nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

Matrix::Matrix (const double* values, int nx, int ny)
{
    MatrixRealArr* v = MatrixRealArr::allocate(nx, ny);
    v->set (values);
    itsRep = v->link();
}

Matrix::Matrix (const dcomplex* values, int nx, int ny)
{
    MatrixComplexArr* v = MatrixComplexArr::allocate(nx, ny);
    v->set (values);
    itsRep = v->link();
}
/*
Matrix::Matrix (const Matrix<double>& array)
{
    bool deleteIt;
    const double* values = array.getStorage (deleteIt);
    MatrixRealArr* v = MatrixRealArr::allocate(array.shape()(0),
						     array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}

Matrix::Matrix (const Matrix<dcomplex >& array)
{
    bool deleteIt;
    const dcomplex* values = array.getStorage (deleteIt);
    MatrixComplexArr* v = MatrixComplexArr::allocate (array.shape()(0),
							    array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}
*/
Matrix::Matrix (const Matrix& that)
: itsRep (that.itsRep)
{
    if (itsRep != 0) {
	itsRep->link();
    }
}

Matrix::Matrix (const MatrixTmp& that)
: itsRep (that.rep()->link()) {}

Matrix& Matrix::operator= (const Matrix& that)
{
    if (this != &that) {
        MatrixRep::unlink (itsRep);
	itsRep = that.itsRep;
	if (itsRep != 0) {
	    itsRep->link();
	}
    }
    return *this;
}
Matrix& Matrix::operator= (const MatrixTmp& that)
{
    MatrixRep::unlink (itsRep);
    itsRep = that.rep()->link();
    return *this;
}

Matrix Matrix::clone() const
{
  if (itsRep == 0) {
    return Matrix();
  }
  return Matrix (itsRep->clone());
}

void Matrix::setDMat (int nx, int ny)
{
    MatrixRep::unlink (itsRep);
    if (nx == 1 && ny == 1) {
        itsRep = new MatrixRealSca (0.);
    } else {
        itsRep = MatrixRealArr::allocate(nx, ny);
    }
    itsRep->link();
}
void Matrix::setDCMat (int nx, int ny)
{
    MatrixRep::unlink (itsRep);
    if (nx == 1 && ny == 1) {
        itsRep = new MatrixComplexSca (dcomplex());
    } else {
        itsRep = MatrixComplexArr::allocate (nx, ny);
    }
    itsRep->link();
}

//Matrix<double> Matrix::getDoubleMatrix() const
//{
//  return Matrix<double> (IPosition(2,nx(),ny()), doubleStorage());
//}

//Matrix<dcomplex > Matrix::getDComplexMatrix() const
//{
//  Matrix<dcomplex > mat(nx(), ny());
//  for (int i1=0; i1<ny(); i1++) {
//    for (int i0=0; i0<nx(); i0++) {
//      mat(i0,i1) = getDComplex(i0,i1);
//    }
//  }
//  return mat;
//}


void Matrix::operator+= (const Matrix& right)
{
  MatrixRep* res = itsRep->add (*right.itsRep, False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator+= (const MatrixTmp& right)
{
  MatrixRep* res = itsRep->add (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator-= (const Matrix& right)
{
  MatrixRep* res = itsRep->subtract (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator-= (const MatrixTmp& right)
{
  MatrixRep* res = itsRep->subtract (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator*= (const Matrix& right)
{
  MatrixRep* res = itsRep->multiply (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator*= (const MatrixTmp& right)
{
  MatrixRep* res = itsRep->multiply (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator/= (const Matrix& right)
{
  MatrixRep* res = itsRep->divide (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}
void Matrix::operator/= (const MatrixTmp& right)
{
  MatrixRep* res = itsRep->divide (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}

MatrixTmp Matrix::operator+ (const Matrix& right) const
{
    return MatrixTmp(*this) + right;
    //return MatrixTmp(rep()->add(*right.rep(), false));
}
MatrixTmp Matrix::operator+ (const MatrixTmp& right) const
{
    return (MatrixTmp&)right + *this;
}

MatrixTmp Matrix::operator- (const Matrix& right) const
{
    return MatrixTmp(*this) - right;
}
MatrixTmp Matrix::operator- (const MatrixTmp& right) const
{
    return MatrixTmp(*this) - right;
}

MatrixTmp Matrix::operator* (const Matrix& right) const
{
    return MatrixTmp(*this) * right;
}
MatrixTmp Matrix::operator* (const MatrixTmp& right) const
{
    return (MatrixTmp&)right * *this;
}

MatrixTmp Matrix::operator/ (const Matrix& right) const
{
    return MatrixTmp(*this) / right;
}
//# This could possibly be speeded up by using right as the result.
//# It requires a special divide function in MatrixRep and derived classes.
//# The same is true for operator-.
//# (Alternatively one could do  "(-right) + this".)
MatrixTmp Matrix::operator/ (const MatrixTmp& right) const
{
    return MatrixTmp(*this) / right;
}

MatrixTmp Matrix::operator-() const
{
    return MatrixTmp(*this).operator-();
}

MatrixTmp posdiff (const Matrix& left, const Matrix& right)
{
    return left.itsRep->posdiff(*right.itsRep);
}
MatrixTmp posdiff (const Matrix& left, const MatrixTmp& right)
{
    return left.itsRep->posdiff(*right.rep());
}
MatrixTmp tocomplex (const Matrix& left, const Matrix& right)
{
    return left.itsRep->tocomplex(*right.itsRep);
}
MatrixTmp tocomplex (const Matrix& left, const MatrixTmp& right)
{
    return left.itsRep->tocomplex(*right.rep());
}
MatrixTmp min (const Matrix& left, const Matrix& right)
{
    return left.itsRep->min(*right.itsRep);
}
MatrixTmp min (const Matrix& left, const MatrixTmp& right)
{
    return left.itsRep->min(*right.rep());
}
MatrixTmp max (const Matrix& left, const Matrix& right)
{
    return left.itsRep->max(*right.itsRep);
}
MatrixTmp max (const Matrix& left, const MatrixTmp& right)
{
    return left.itsRep->max(*right.rep());
}
MatrixTmp abs (const Matrix& arg)
{
    return abs(MatrixTmp(arg));
}
MatrixTmp sin (const Matrix& arg)
{
    return sin(MatrixTmp(arg));
}
MatrixTmp cos(const Matrix& arg)
{
    return cos(MatrixTmp(arg));
}
MatrixTmp exp(const Matrix& arg)
{
    return exp(MatrixTmp(arg));
}
MatrixTmp sqr(const Matrix& arg)
{
    return sqr(MatrixTmp(arg));
}
MatrixTmp sqrt(const Matrix& arg)
{
    return sqrt(MatrixTmp(arg));
}
MatrixTmp conj(const Matrix& arg)
{
    return conj(MatrixTmp(arg));
}
MatrixTmp min(const Matrix& arg)
{
    return arg.itsRep->min();
}
MatrixTmp max(const Matrix& arg)
{
    return arg.itsRep->max();
}
MatrixTmp mean(const Matrix& arg)
{
    return arg.itsRep->mean();
}
MatrixTmp sum(const Matrix& arg)
{
    return arg.itsRep->sum();
}

/*
LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs, const Matrix& vec)
{
  bs.putStart ("Matrix", 1);
#if 0
  bs<<(vec.rep() == 0);
  if (vec.rep() != 0) {
    bs << vec.isDouble() << (vec.nelements()==1);
    if (vec.isDouble()) {
      if (vec.nelements() == 1) {
	bs << vec.getDouble();
      } else {
	bs << vec.getDoubleMatrix();
      }
    } else {
      if (vec.nelements() == 1) {
	bs << vec.getDComplex();
      } else {
	bs << vec.getDComplexMatrix();
      }
    }
  }
#else
  MatrixRep *rep = vec.rep();
  bs << (rep == 0);
  if (rep != 0) {
    bs << (unsigned char) rep->type;
    switch (rep->type) {
      case MatrixRep::RealScalar :
	bs << vec.getDouble();
	break;

      case MatrixRep::RealArray :
	bs << vec.getDoubleMatrix();
	break;

      case MatrixRep::ComplexScalar :
	bs << vec.getDComplex();
	break;

      case MatrixRep::ComplexArray :
	bs << vec.getDComplexMatrix();
	break;
    }
  }
#endif
  bs.putEnd();
  return bs;
}

LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs, Matrix& vec)
{
  bs.getStart ("Matrix");
#if 0
  bool isRepNull, isDouble, isScalar;
  bs >> isRepNull;
  if (isRepNull) {
    vec = Matrix();
  } else {
    bs >> isDouble >> isScalar;
    if (isDouble) {
      if (isScalar) {
	double val;
	bs >> val;
	vec = Matrix(val);
      } else {
	Matrix<double> mat;
	bs >> mat;
	vec = Matrix(mat);
      }
    } else {
      if (isScalar) {
	dcomplex val;
	bs >> val;
	vec = Matrix(val);
      } else {
	Matrix<dcomplex > mat;
	bs >> mat;
	vec = Matrix(mat);
      }
    }
  }
#else
  bool isRepNull;
  bs >> isRepNull;
  if (isRepNull) {
    vec = Matrix();
  } else {
    double rval;
    Matrix<double> rmat;
    dcomplex cval;
    Matrix<dcomplex > cmat;
    unsigned char type;

    bs >> type;
    switch (type) {
      case MatrixRep::RealScalar :
	bs >> rval;
	vec = Matrix(rval);
	break;

      case MatrixRep::RealArray :
	bs >> rmat;
	vec = Matrix(rmat);
	break;

      case MatrixRep::ComplexScalar :
	bs >> cval;
	vec = Matrix(cval);
	break;

      case MatrixRep::ComplexArray :
	bs >> cmat;
	vec = Matrix(cmat);
	break;
    }
  }
#endif

  bs.getEnd();
  return bs;
}
*/
} // namespace BBS
} // namespace LOFAR
/*
//# Instantiate the AIPS++ templates needed for Matrix<dcomplex>
//# This is needed because dcomplex is usually not the same as casa::DComplex.
//# The inclusion of the other .cc files is needed for the automatic
//# instantiation of the templates used by Matrix.
#ifdef AIPS_NO_TEMPLATE_SRC
#include <casa/Arrays/Matrix.cc>
#include <casa/Arrays/Vector.cc>
#include <casa/Arrays/Array.cc>
#include <casa/Arrays/MaskedArray.cc>
#include <casa/Utilities/Copy.cc>
#include <casa/Utilities/CountedPtr.cc>
template class Matrix<LOFAR::dcomplex>;
#endif
*/

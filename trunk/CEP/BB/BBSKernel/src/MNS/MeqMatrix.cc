//# MeqMatrix.cc: Matrix for Mns
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
#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <BBSKernel/MNS/MeqMatrixRealSca.h>
#include <BBSKernel/MNS/MeqMatrixComplexSca.h>
#include <BBSKernel/MNS/MeqMatrixRealArr.h>
#include <BBSKernel/MNS/MeqMatrixComplexArr.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqMatrix::MeqMatrix (double value)
{
    MeqMatrixRealSca* v = new MeqMatrixRealSca (value);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (dcomplex value)
{
    MeqMatrixComplexSca* v = new MeqMatrixComplexSca (value);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (double value, int nx, int ny, bool init)
{
    MeqMatrixRealArr* v = MeqMatrixRealArr::allocate(nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (dcomplex value, int nx, int ny, bool init)
{
    MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (const double* values, int nx, int ny)
{
    MeqMatrixRealArr* v = MeqMatrixRealArr::allocate(nx, ny);
    v->set (values);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (const dcomplex* values, int nx, int ny)
{
    MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate(nx, ny);
    v->set (values);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (const Matrix<double>& array)
{
    bool deleteIt;
    const double* values = array.getStorage (deleteIt);
    MeqMatrixRealArr* v = MeqMatrixRealArr::allocate(array.shape()(0),
						     array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}

MeqMatrix::MeqMatrix (const Matrix<dcomplex >& array)
{
    bool deleteIt;
    const dcomplex* values = array.getStorage (deleteIt);
    MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (array.shape()(0),
							    array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}

MeqMatrix::MeqMatrix (const MeqMatrix& that)
: itsRep (that.itsRep)
{
    if (itsRep != 0) {
	itsRep->link();
    }
}

MeqMatrix::MeqMatrix (const MeqMatrixTmp& that)
: itsRep (that.rep()->link()) {}

MeqMatrix& MeqMatrix::operator= (const MeqMatrix& that)
{
    if (this != &that) {
        MeqMatrixRep::unlink (itsRep);
	itsRep = that.itsRep;
	if (itsRep != 0) {
	    itsRep->link();
	}
    }
    return *this;
}
MeqMatrix& MeqMatrix::operator= (const MeqMatrixTmp& that)
{
    MeqMatrixRep::unlink (itsRep);
    itsRep = that.rep()->link();
    return *this;
}

MeqMatrix MeqMatrix::clone() const
{
  if (itsRep == 0) {
    return MeqMatrix();
  }
  return MeqMatrix (itsRep->clone());
}

void MeqMatrix::setDMat (int nx, int ny)
{
    MeqMatrixRep::unlink (itsRep);
    if (nx == 1 && ny == 1) {
        itsRep = new MeqMatrixRealSca (0.);
    } else {
        itsRep = MeqMatrixRealArr::allocate(nx, ny);
    }
    itsRep->link();
}
void MeqMatrix::setDCMat (int nx, int ny)
{
    MeqMatrixRep::unlink (itsRep);
    if (nx == 1 && ny == 1) {
        itsRep = new MeqMatrixComplexSca (dcomplex());
    } else {
        itsRep = MeqMatrixComplexArr::allocate (nx, ny);
    }
    itsRep->link();
}

Matrix<double> MeqMatrix::getDoubleMatrix() const
{
  return Matrix<double> (IPosition(2,nx(),ny()), doubleStorage());
}

Matrix<dcomplex > MeqMatrix::getDComplexMatrix() const
{
  Matrix<dcomplex > mat(nx(), ny());
  for (int i1=0; i1<ny(); i1++) {
    for (int i0=0; i0<nx(); i0++) {
      mat(i0,i1) = getDComplex(i0,i1);
    }
  }
  return mat;
}


void MeqMatrix::operator+= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->add (*right.itsRep, False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator+= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->add (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator-= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->subtract (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator-= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->subtract (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator*= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->multiply (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator*= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->multiply (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator/= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->divide (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator/= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->divide (*right.rep(), False);
  ASSERTSTR (res == itsRep, "Mismatching types");
}   

MeqMatrixTmp MeqMatrix::operator+ (const MeqMatrix& right) const
{
    return MeqMatrixTmp(*this) + right;
    //return MeqMatrixTmp(rep()->add(*right.rep(), false));
}   
MeqMatrixTmp MeqMatrix::operator+ (const MeqMatrixTmp& right) const
{
    return (MeqMatrixTmp&)right + *this;
}

MeqMatrixTmp MeqMatrix::operator- (const MeqMatrix& right) const
{
    return MeqMatrixTmp(*this) - right;
}   
MeqMatrixTmp MeqMatrix::operator- (const MeqMatrixTmp& right) const
{
    return MeqMatrixTmp(*this) - right;
}

MeqMatrixTmp MeqMatrix::operator* (const MeqMatrix& right) const
{
    return MeqMatrixTmp(*this) * right;
}   
MeqMatrixTmp MeqMatrix::operator* (const MeqMatrixTmp& right) const
{
    return (MeqMatrixTmp&)right * *this;
}

MeqMatrixTmp MeqMatrix::operator/ (const MeqMatrix& right) const
{
    return MeqMatrixTmp(*this) / right;
}
//# This could possibly be speeded up by using right as the result.
//# It requires a special divide function in MeqMatrixRep and derived classes.
//# The same is true for operator-.
//# (Alternatively one could do  "(-right) + this".)
MeqMatrixTmp MeqMatrix::operator/ (const MeqMatrixTmp& right) const
{
    return MeqMatrixTmp(*this) / right;
}

MeqMatrixTmp MeqMatrix::operator-() const
{
    return MeqMatrixTmp(*this).operator-();
}

MeqMatrixTmp posdiff (const MeqMatrix& left, const MeqMatrix& right)
{
    return left.itsRep->posdiff(*right.itsRep);
}
MeqMatrixTmp posdiff (const MeqMatrix& left, const MeqMatrixTmp& right)
{
    return left.itsRep->posdiff(*right.rep());
}
MeqMatrixTmp tocomplex (const MeqMatrix& left, const MeqMatrix& right)
{
    return left.itsRep->tocomplex(*right.itsRep);
}
MeqMatrixTmp tocomplex (const MeqMatrix& left, const MeqMatrixTmp& right)
{
    return left.itsRep->tocomplex(*right.rep());
}
MeqMatrixTmp sin (const MeqMatrix& arg)
{
    return sin(MeqMatrixTmp(arg));
}
MeqMatrixTmp cos(const MeqMatrix& arg)
{
    return cos(MeqMatrixTmp(arg));
}
MeqMatrixTmp exp(const MeqMatrix& arg)
{
    return exp(MeqMatrixTmp(arg));
}
MeqMatrixTmp sqr(const MeqMatrix& arg)
{
    return sqr(MeqMatrixTmp(arg));
}
MeqMatrixTmp sqrt(const MeqMatrix& arg)
{
    return sqrt(MeqMatrixTmp(arg));
}
MeqMatrixTmp conj(const MeqMatrix& arg)
{
    return conj(MeqMatrixTmp(arg));
}
MeqMatrixTmp min(const MeqMatrix& arg)
{
    return arg.itsRep->min();
}
MeqMatrixTmp max(const MeqMatrix& arg)
{
    return arg.itsRep->max();
}
MeqMatrixTmp mean(const MeqMatrix& arg)
{
    return arg.itsRep->mean();
}
MeqMatrixTmp sum(const MeqMatrix& arg)
{
    return arg.itsRep->sum();
}


LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs, const MeqMatrix& vec)
{
  bs.putStart ("MeqMatrix", 1);
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
  MeqMatrixRep *rep = vec.rep();
  bs << (rep == 0);
  if (rep != 0) {
    bs << (unsigned char) rep->type;
    switch (rep->type) {
      case MeqMatrixRep::RealScalar :
	bs << vec.getDouble();
	break;

      case MeqMatrixRep::RealArray :
	bs << vec.getDoubleMatrix();
	break;

      case MeqMatrixRep::ComplexScalar :
	bs << vec.getDComplex();
	break;

      case MeqMatrixRep::ComplexArray :
	bs << vec.getDComplexMatrix();
	break;
    }
  }
#endif
  bs.putEnd();
  return bs;
}

LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs, MeqMatrix& vec)
{
  bs.getStart ("MeqMatrix");
#if 0
  bool isRepNull, isDouble, isScalar;
  bs >> isRepNull;
  if (isRepNull) {
    vec = MeqMatrix();
  } else {
    bs >> isDouble >> isScalar;
    if (isDouble) {
      if (isScalar) {
	double val;
	bs >> val;
	vec = MeqMatrix(val);
      } else {
	Matrix<double> mat;
	bs >> mat;
	vec = MeqMatrix(mat);
      }
    } else {
      if (isScalar) {
	dcomplex val;
	bs >> val;
	vec = MeqMatrix(val);
      } else {
	Matrix<dcomplex > mat;
	bs >> mat;
	vec = MeqMatrix(mat);
      }
    }
  }
#else
  bool isRepNull;
  bs >> isRepNull;
  if (isRepNull) {
    vec = MeqMatrix();
  } else {
    double rval;
    Matrix<double> rmat;
    dcomplex cval;
    Matrix<dcomplex > cmat;
    unsigned char type;

    bs >> type;
    switch (type) {
      case MeqMatrixRep::RealScalar :
	bs >> rval;
	vec = MeqMatrix(rval);
	break;

      case MeqMatrixRep::RealArray :
	bs >> rmat;
	vec = MeqMatrix(rmat);
	break;

      case MeqMatrixRep::ComplexScalar :
	bs >> cval;
	vec = MeqMatrix(cval);
	break;

      case MeqMatrixRep::ComplexArray :
	bs >> cmat;
	vec = MeqMatrix(cmat);
	break;
    }
  }
#endif

  bs.getEnd();
  return bs;
}

} // namespace BBS
} // namespace LOFAR

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

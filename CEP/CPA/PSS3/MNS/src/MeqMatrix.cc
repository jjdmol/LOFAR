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


#include <MNS/MeqMatrix.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqMatrixRealSca.h>
#include <MNS/MeqMatrixComplexSca.h>
#include <MNS/MeqMatrixRealArr.h>
#include <MNS/MeqMatrixComplexArr.h>
#include <Common/BlobArray.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/Debug.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

MeqMatrix::MeqMatrix (double value)
{
    MeqMatrixRealSca* v = new MeqMatrixRealSca (value);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (complex<double> value)
{
    MeqMatrixComplexSca* v = new MeqMatrixComplexSca (value);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (double value, int nx, int ny, bool init)
{
    MeqMatrixRealArr* v = new MeqMatrixRealArr (nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (complex<double> value, int nx, int ny, bool init)
{
    MeqMatrixComplexArr* v = MeqMatrixComplexArr::poolNew (nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (const double* values, int nx, int ny)
{
    MeqMatrixRealArr* v = new MeqMatrixRealArr (nx, ny);
    v->set (values);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (const complex<double>* values, int nx, int ny)
{
    MeqMatrixComplexArr* v = MeqMatrixComplexArr::poolNew(nx, ny);
    v->set (values);
    itsRep = v->link();
}

MeqMatrix::MeqMatrix (const Matrix<double>& array)
{
    bool deleteIt;
    const double* values = array.getStorage (deleteIt);
    MeqMatrixRealArr* v = new MeqMatrixRealArr (array.shape()(0),
						array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}

MeqMatrix::MeqMatrix (const Matrix<complex<double> >& array)
{
    bool deleteIt;
    const complex<double>* values = array.getStorage (deleteIt);
    MeqMatrixComplexArr* v = MeqMatrixComplexArr::poolNew (array.shape()(0),
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
    MeqMatrixRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
	itsRep->link();
    }
    return *this;
}
MeqMatrix& MeqMatrix::operator= (const MeqMatrixTmp& that)
{
    MeqMatrixRep::unlink (itsRep);
    itsRep = that.rep()->link();
    return *this;
}

void MeqMatrix::setDMat (int nx, int ny)
{
    MeqMatrixRep::unlink (itsRep);
    itsRep = 0;
    if (nx == 1  &&  ny == 1) {
        itsRep = new MeqMatrixRealSca (0.);
    } else {
        itsRep = new MeqMatrixRealArr (nx, ny);
    }
    itsRep->link();
}
void MeqMatrix::setDCMat (int nx, int ny)
{
    MeqMatrixRep::unlink (itsRep);
    itsRep = 0;
    if (nx == 1  &&  ny == 1) {
        itsRep = new MeqMatrixComplexSca (complex<double>());
    } else {
        itsRep = MeqMatrixComplexArr::poolNew (nx, ny);
    }
    itsRep->link();
}

Matrix<double> MeqMatrix::getDoubleMatrix() const
{
  return Matrix<double> (IPosition(2,nx(),ny()), doubleStorage());
}

Matrix<complex<double> > MeqMatrix::getDComplexMatrix() const
{
  Matrix<complex<double> > mat(nx(), ny());
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
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator+= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->add (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator-= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->subtract (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator-= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->subtract (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator*= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->multiply (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator*= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->multiply (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator/= (const MeqMatrix& right)
{
  MeqMatrixRep* res = itsRep->divide (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   
void MeqMatrix::operator/= (const MeqMatrixTmp& right)
{
  MeqMatrixRep* res = itsRep->divide (*right.rep(), False);
  AssertStr (res == itsRep, "Mismatching types");
}   

MeqMatrixTmp MeqMatrix::operator+ (const MeqMatrix& right) const
{
    return MeqMatrixTmp(*this) + right;
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
  bs.putEnd();
  return bs;
}

LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs, MeqMatrix& vec)
{
  bs.getStart ("MeqMatrix");
  bool isDouble, isScalar;
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
      complex<double> val;
      bs >> val;
      vec = MeqMatrix(val);
    } else {
      Matrix<complex<double> > mat;
      bs >> mat;
      vec = MeqMatrix(mat);
    }
  }
  bs.getEnd();
  return bs;
}

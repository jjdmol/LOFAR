//# MnsMatrix.cc: Matrix for Mns
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


#include <MNS/MnsMatrix.h>
#include <MNS/MnsMatrixTmp.h>
#include <MNS/MnsMatrixRealSca.h>
#include <MNS/MnsMatrixComplexSca.h>
#include <MNS/MnsMatrixRealArr.h>
#include <MNS/MnsMatrixComplexArr.h>
#include <aips/Arrays/Matrix.h>


MnsMatrix::MnsMatrix (double value)
{
    MnsMatrixRealSca* v = new MnsMatrixRealSca (value);
    itsRep = v->link();
}

MnsMatrix::MnsMatrix (complex<double> value)
{
    MnsMatrixComplexSca* v = new MnsMatrixComplexSca (value);
    itsRep = v->link();
}

MnsMatrix::MnsMatrix (const double* values, int nx, int ny)
{
    MnsMatrixRealArr* v = new MnsMatrixRealArr (nx, ny);
    v->set (values);
    itsRep = v->link();
}

MnsMatrix::MnsMatrix (const complex<double>* values, int nx, int ny)
{
    MnsMatrixComplexArr* v = new MnsMatrixComplexArr (nx, ny);
    v->set (values);
    itsRep = v->link();
}

MnsMatrix::MnsMatrix (const Matrix<double>& array)
{
    bool deleteIt;
    const double* values = array.getStorage (deleteIt);
    MnsMatrixRealArr* v = new MnsMatrixRealArr (array.shape()(0),
						array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}

MnsMatrix::MnsMatrix (const Matrix<complex<double> >& array)
{
    bool deleteIt;
    const complex<double>* values = array.getStorage (deleteIt);
    MnsMatrixComplexArr* v = new MnsMatrixComplexArr (array.shape()(0),
						      array.shape()(1));
    v->set (values);
    itsRep = v->link();
    array.freeStorage (values, deleteIt);
}

MnsMatrix::MnsMatrix (const MnsMatrix& that)
: itsRep (that.itsRep)
{
    if (itsRep != 0) {
	itsRep->link();
    }
}

MnsMatrix::MnsMatrix (const MnsMatrixTmp& that)
: itsRep (that.rep()->link()) {}

MnsMatrix& MnsMatrix::operator= (const MnsMatrix& that)
{
    MnsMatrixRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
	itsRep->link();
    }
    return *this;
}
MnsMatrix& MnsMatrix::operator= (const MnsMatrixTmp& that)
{
    MnsMatrixRep::unlink (itsRep);
    itsRep = that.rep()->link();
    return *this;
}

Matrix<double> MnsMatrix::getDoubleMatrix() const
{
  return Matrix<double> (IPosition(2,nx(),ny()), doubleStorage());
}


MnsMatrixTmp MnsMatrix::operator+ (const MnsMatrix& right) const
{
    return MnsMatrixTmp(*this) + right;
}   
MnsMatrixTmp MnsMatrix::operator+ (const MnsMatrixTmp& right) const
{
    return (MnsMatrixTmp&)right + *this;
}

MnsMatrixTmp MnsMatrix::operator- (const MnsMatrix& right) const
{
    return MnsMatrixTmp(*this) - right;
}   
MnsMatrixTmp MnsMatrix::operator- (const MnsMatrixTmp& right) const
{
    return MnsMatrixTmp(*this) - right;
}

MnsMatrixTmp MnsMatrix::operator* (const MnsMatrix& right) const
{
    return MnsMatrixTmp(*this) * right;
}   
MnsMatrixTmp MnsMatrix::operator* (const MnsMatrixTmp& right) const
{
    return (MnsMatrixTmp&)right * *this;
}

MnsMatrixTmp MnsMatrix::operator/ (const MnsMatrix& right) const
{
    return MnsMatrixTmp(*this) / right;
}
//# This could possibly be speeded up by using right as the result.
//# It requires a special divide function in MnsMatrixRep and derived classes.
//# The same is true for operator-.
//# (Alternatively one could do  "(-right) + this".)
MnsMatrixTmp MnsMatrix::operator/ (const MnsMatrixTmp& right) const
{
    return MnsMatrixTmp(*this) / right;
}

MnsMatrixTmp MnsMatrix::operator-() const
{
    return MnsMatrixTmp(*this).operator-();
}

MnsMatrixTmp MnsMatrix::sin() const
{
    return MnsMatrixTmp(*this).sin();
}
MnsMatrixTmp MnsMatrix::cos() const
{
    return MnsMatrixTmp(*this).cos();
}
MnsMatrixTmp MnsMatrix::exp() const
{
    return MnsMatrixTmp(*this).exp();
}
MnsMatrixTmp MnsMatrix::conj() const
{
    return MnsMatrixTmp(*this).conj();
}
MnsMatrixTmp MnsMatrix::min() const
{
    return itsRep->min();
}
MnsMatrixTmp MnsMatrix::max() const
{
    return itsRep->max();
}
MnsMatrixTmp MnsMatrix::mean() const
{
    return itsRep->mean();
}
MnsMatrixTmp MnsMatrix::sum() const
{
    return itsRep->sum();
}

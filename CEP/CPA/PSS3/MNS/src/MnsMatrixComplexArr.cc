//# MnsMatrixComplexArr.cc: Temporary matrix for Mns
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


#include <MNS/MnsMatrixRealSca.h>
#include <MNS/MnsMatrixRealArr.h>
#include <MNS/MnsMatrixComplexSca.h>
#include <MNS/MnsMatrixComplexArr.h>
#include <Common/Debug.h>


MnsMatrixComplexArr::MnsMatrixComplexArr (int nx, int ny)
: MnsMatrixRep (nx, ny, sizeof(complex<double>))
{
  itsValue = new complex<double>[nelements()];
}

MnsMatrixComplexArr::~MnsMatrixComplexArr()
{
  delete [] itsValue;
}

MnsMatrixRep* MnsMatrixComplexArr::clone() const
{
  MnsMatrixComplexArr* v = new MnsMatrixComplexArr (nx(), ny());
  memcpy (v->itsValue, itsValue, sizeof(complex<double>) * nelements());
  return v;
}

void MnsMatrixComplexArr::set (complex<double> value)
{
  for (int i=0; i<nelements(); i++) {
    itsValue[i] = value;
  }
}

void MnsMatrixComplexArr::show (ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << itsValue[i];
  }
  os << ']';
}

MnsMatrixRep* MnsMatrixComplexArr::add (MnsMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixComplexArr::subtract (MnsMatrixRep& right,
					     bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixComplexArr::multiply (MnsMatrixRep& right,
					     bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixComplexArr::divide (MnsMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

double MnsMatrixComplexArr::getDouble (int, int) const
{
  AssertMsg (false, "MnsMatrix: dcomplex->double conversion not possible");
}
complex<double> MnsMatrixComplexArr::getDComplex (int x, int y) const
{
  return itsValue[offset(x,y)];
}


#define MNSMATRIXCOMPLEXARR_OP(NAME, OP, OP2) \
MnsMatrixRep* MnsMatrixComplexArr::NAME (MnsMatrixRealSca& left, \
					 bool rightTmp) \
{ \
  MnsMatrixComplexArr* v = this; \
  if (!rightTmp) { \
    v = (MnsMatrixComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValue[i] = left.itsValue OP2 v->itsValue[i]; \
  } \
  return v; \
} \
MnsMatrixRep* MnsMatrixComplexArr::NAME (MnsMatrixComplexSca& left, \
					 bool rightTmp) \
{ \
  MnsMatrixComplexArr* v = this; \
  if (!rightTmp) { \
    v = (MnsMatrixComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValue[i] = left.itsValue OP2 v->itsValue[i]; \
  } \
  return &left; \
} \
MnsMatrixRep* MnsMatrixComplexArr::NAME (MnsMatrixRealArr& left,  \
					 bool rightTmp) \
{ \
  Assert (nelements() == left.nelements()); \
  MnsMatrixComplexArr* v = this; \
  if (!rightTmp) { \
    v = (MnsMatrixComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValue[i] = left.itsValue[i] OP2 v->itsValue[i]; \
  } \
  return v; \
} \
MnsMatrixRep* MnsMatrixComplexArr::NAME (MnsMatrixComplexArr& left, \
					 bool) \
{ \
  Assert (nelements() == left.nelements()); \
  for (int i=0; i<left.nelements(); i++) { \
    left.itsValue[i] OP itsValue[i]; \
  } \
  return &left; \
}

MNSMATRIXCOMPLEXARR_OP(addRep,+=,+);
MNSMATRIXCOMPLEXARR_OP(subRep,-=,-);
MNSMATRIXCOMPLEXARR_OP(mulRep,*=,*);
MNSMATRIXCOMPLEXARR_OP(divRep,/=,/);


void MnsMatrixComplexArr::negate()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = -(itsValue[i]);
  }
}

void MnsMatrixComplexArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::sin(itsValue[i]);
  }
}

void MnsMatrixComplexArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::cos(itsValue[i]);
  }
}

void MnsMatrixComplexArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::exp(itsValue[i]);
  }
}

void MnsMatrixComplexArr::conj()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::conj(itsValue[i]);
  }
}

MnsMatrixRep* MnsMatrixComplexArr::min()
{
  complex<double> val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    double absval = abs(val);
    for (int i=1; i<n; i++) {
      double av = abs(itsValue[i]);
      if (av < absval) {
	val = itsValue[i];
	absval = av;
      }
    }
  }
  return new MnsMatrixComplexSca (val);
}
MnsMatrixRep* MnsMatrixComplexArr::max()
{
  complex<double> val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    double absval = abs(val);
    for (int i=1; i<n; i++) {
      double av = abs(itsValue[i]);
      if (av > absval) {
	val = itsValue[i];
	absval = av;
      }
    }
  }
  return new MnsMatrixComplexSca (val);
}
MnsMatrixRep* MnsMatrixComplexArr::mean()
{
  complex<double> sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MnsMatrixComplexSca (sum/n);
}
MnsMatrixRep* MnsMatrixComplexArr::sum()
{
  complex<double> sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MnsMatrixComplexSca (sum);
}

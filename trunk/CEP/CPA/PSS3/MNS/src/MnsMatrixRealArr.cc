//# MnsMatrixRealArr.cc: Temporary matrix for Mns
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


MnsMatrixRealArr::MnsMatrixRealArr (int nx, int ny)
: MnsMatrixRep (nx, ny, sizeof(double))
{
  itsValue = new double[nelements()];
}

MnsMatrixRealArr::~MnsMatrixRealArr()
{
  delete [] itsValue;
}

MnsMatrixRep* MnsMatrixRealArr::clone() const
{
  MnsMatrixRealArr* v = new MnsMatrixRealArr (nx(), ny());
  memcpy (v->itsValue, itsValue, sizeof(double) * nelements());
  return v;
}

void MnsMatrixRealArr::set (double value)
{
  for (int i=0; i<nelements(); i++) {
    itsValue[i] = value;
  }
}

void MnsMatrixRealArr::show (ostream& os) const
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

MnsMatrixRep* MnsMatrixRealArr::add (MnsMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixRealArr::subtract (MnsMatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixRealArr::multiply (MnsMatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixRealArr::divide (MnsMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

const double* MnsMatrixRealArr::doubleStorage() const
{
  return itsValue;
}
double MnsMatrixRealArr::getDouble (int x, int y) const
{
  return itsValue[offset(x,y)];
}
complex<double> MnsMatrixRealArr::getDComplex (int x, int y) const
{
  return itsValue[offset(x,y)];
}


#define MNSMATRIXREALARR_OP(NAME, OP, OP2) \
MnsMatrixRep* MnsMatrixRealArr::NAME (MnsMatrixRealSca& left, \
				      bool rightTmp) \
{ \
  MnsMatrixRealArr* v = this; \
  if (!rightTmp) { \
    v = (MnsMatrixRealArr*)clone(); \
  } \
  double* value = v->itsValue; \
  double lvalue = left.itsValue; \
  double* end = value + nelements(); \
  while (value < end) { \
    *value = lvalue OP2 *value; \
    value++; \
  } \
  return v; \
} \
MnsMatrixRep* MnsMatrixRealArr::NAME (MnsMatrixComplexSca& left, \
				      bool) \
{ \
  MnsMatrixComplexArr* v = new MnsMatrixComplexArr (left.nx(), left.ny()); \
  complex<double>* value = v->itsValue; \
  double* value2 = itsValue; \
  complex<double> lvalue = left.itsValue; \
  int n = nelements(); \
  for (int i=0; i<n; i++) { \
    *value++ = lvalue OP2 *value2++; \
  } \
  return v; \
} \
MnsMatrixRep* MnsMatrixRealArr::NAME (MnsMatrixRealArr& left,  \
				      bool) \
{ \
  Assert (nelements() == left.nelements()); \
  double* value = left.itsValue; \
  double* value2 = itsValue; \
  double* end = value + nelements(); \
  while (value < end) { \
    *value++ OP *value2++; \
  } \
  return &left; \
} \
MnsMatrixRep* MnsMatrixRealArr::NAME (MnsMatrixComplexArr& left, \
				      bool) \
{ \
  Assert (nelements() == left.nelements()); \
  complex<double>* value = left.itsValue; \
  double* value2 = itsValue; \
  int n = left.nelements(); \
  for (int i=0; i<n; i++) { \
    *value++ OP *value2++; \
  } \
  return &left; \
}

MNSMATRIXREALARR_OP(addRep,+=,+);
MNSMATRIXREALARR_OP(subRep,-=,-);
MNSMATRIXREALARR_OP(mulRep,*=,*);
MNSMATRIXREALARR_OP(divRep,/=,/);


void MnsMatrixRealArr::negate()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = -(itsValue[i]);
  }
}

void MnsMatrixRealArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::sin(itsValue[i]);
  }
}

void MnsMatrixRealArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::cos(itsValue[i]);
  }
}

void MnsMatrixRealArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::exp(itsValue[i]);
  }
}

void MnsMatrixRealArr::conj()
{}

MnsMatrixRep* MnsMatrixRealArr::min()
{
  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    for (int i=1; i<n; i++) {
      if (itsValue[i] < val) {
	val = itsValue[i];
      }
    }
  }
  return new MnsMatrixRealSca (val);
}
MnsMatrixRep* MnsMatrixRealArr::max()
{
  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    for (int i=1; i<n; i++) {
      if (itsValue[i] > val) {
	val = itsValue[i];
      }
    }
  }
  return new MnsMatrixRealSca (val);
}
MnsMatrixRep* MnsMatrixRealArr::mean()
{
  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MnsMatrixRealSca (sum/n);
}
MnsMatrixRep* MnsMatrixRealArr::sum()
{
  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MnsMatrixRealSca (sum);
}

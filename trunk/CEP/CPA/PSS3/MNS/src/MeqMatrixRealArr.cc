//# MeqMatrixRealArr.cc: Temporary matrix for Mns
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


#include <MNS/MeqMatrixRealSca.h>
#include <MNS/MeqMatrixRealArr.h>
#include <MNS/MeqMatrixComplexSca.h>
#include <MNS/MeqMatrixComplexArr.h>
#include <Common/Debug.h>


MeqMatrixRealArr::MeqMatrixRealArr (int nx, int ny)
: MeqMatrixRep (nx, ny, sizeof(double))
{
  itsValue = new double[nelements()];
}

MeqMatrixRealArr::~MeqMatrixRealArr()
{
  delete [] itsValue;
}

MeqMatrixRep* MeqMatrixRealArr::clone() const
{
  MeqMatrixRealArr* v = new MeqMatrixRealArr (nx(), ny());
  memcpy (v->itsValue, itsValue, sizeof(double) * nelements());
  return v;
}

void MeqMatrixRealArr::set (double value)
{
  for (int i=0; i<nelements(); i++) {
    itsValue[i] = value;
  }
}

void MeqMatrixRealArr::show (ostream& os) const
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

MeqMatrixRep* MeqMatrixRealArr::add (MeqMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::subtract (MeqMatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::multiply (MeqMatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::divide (MeqMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::tocomplex (MeqMatrixRep& right)
{
  return right.complexRep (*this);
}

bool MeqMatrixRealArr::isDouble() const
{
  return true;
}
const double* MeqMatrixRealArr::doubleStorage() const
{
  return itsValue;
}
double MeqMatrixRealArr::getDouble (int x, int y) const
{
  return itsValue[offset(x,y)];
}
complex<double> MeqMatrixRealArr::getDComplex (int x, int y) const
{
  return itsValue[offset(x,y)];
}


#define MNSMATRIXREALARR_OP(NAME, OP, OP2) \
MeqMatrixRep* MeqMatrixRealArr::NAME (MeqMatrixRealSca& left, \
				      bool rightTmp) \
{ \
  MeqMatrixRealArr* v = this; \
  if (!rightTmp) { \
    v = (MeqMatrixRealArr*)clone(); \
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
MeqMatrixRep* MeqMatrixRealArr::NAME (MeqMatrixComplexSca& left, \
				      bool) \
{ \
  MeqMatrixComplexArr* v = new MeqMatrixComplexArr (left.nx(), left.ny()); \
  complex<double>* value = v->itsValue; \
  double* value2 = itsValue; \
  complex<double> lvalue = left.itsValue; \
  int n = nelements(); \
  for (int i=0; i<n; i++) { \
    *value++ = lvalue OP2 *value2++; \
  } \
  return v; \
} \
MeqMatrixRep* MeqMatrixRealArr::NAME (MeqMatrixRealArr& left,  \
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
MeqMatrixRep* MeqMatrixRealArr::NAME (MeqMatrixComplexArr& left, \
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

MeqMatrixRep* MeqMatrixRealArr::complexRep (MeqMatrixRealSca& left)
{
  MeqMatrixComplexArr* v = new MeqMatrixComplexArr (nx(), ny());
  complex<double>* value = v->itsValue;
  double* rvalue = itsValue;
  double  lvalue = left.itsValue;
  int n = nelements();
  for (int i=0; i<n; i++) {
    value[i] = complex<double> (lvalue, rvalue[i]);
  }
  return v;
}
MeqMatrixRep* MeqMatrixRealArr::complexRep (MeqMatrixRealArr& left)
{
  Assert (nelements() == left.nelements());
  MeqMatrixComplexArr* v = new MeqMatrixComplexArr (nx(), ny());
  complex<double>* value = v->itsValue;
  double* rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = nelements();
  for (int i=0; i<n; i++) {
    value[i] = complex<double> (lvalue[i], rvalue[i]);
  }
  return v;
}


MeqMatrixRep* MeqMatrixRealArr::negate()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = -(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::sin(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::cos(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::exp(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::sqr()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] *= itsValue[i];
  }
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::sqrt()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::sqrt(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::conj()
{
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::min()
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
  return new MeqMatrixRealSca (val);
}
MeqMatrixRep* MeqMatrixRealArr::max()
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
  return new MeqMatrixRealSca (val);
}
MeqMatrixRep* MeqMatrixRealArr::mean()
{
  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MeqMatrixRealSca (sum/n);
}
MeqMatrixRep* MeqMatrixRealArr::sum()
{
  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MeqMatrixRealSca (sum);
}

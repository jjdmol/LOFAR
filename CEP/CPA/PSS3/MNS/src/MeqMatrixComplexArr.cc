//# MeqMatrixComplexArr.cc: Temporary matrix for Mns
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
#include <iomanip>


MeqMatrixComplexArr::MeqMatrixComplexArr (int nx, int ny)
: MeqMatrixRep (nx, ny, sizeof(complex<double>))
{
  itsValue = new complex<double>[nelements()];
}

MeqMatrixComplexArr::~MeqMatrixComplexArr()
{
  delete [] itsValue;
}

MeqMatrixRep* MeqMatrixComplexArr::clone() const
{
  MeqMatrixComplexArr* v = new MeqMatrixComplexArr (nx(), ny());
  memcpy (v->itsValue, itsValue, sizeof(complex<double>) * nelements());
  return v;
}

void MeqMatrixComplexArr::set (complex<double> value)
{
  for (int i=0; i<nelements(); i++) {
    itsValue[i] = value;
  }
}

void MeqMatrixComplexArr::show (ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << '(' << setprecision(12) << itsValue[i].real()
       << ',' << setprecision(12) << itsValue[i].imag() << ')';
  }
  os << ']';
}

MeqMatrixRep* MeqMatrixComplexArr::add (MeqMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixComplexArr::subtract (MeqMatrixRep& right,
					     bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixComplexArr::multiply (MeqMatrixRep& right,
					     bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixComplexArr::divide (MeqMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

const complex<double>* MeqMatrixComplexArr::dcomplexStorage() const
{
  return itsValue;
}
double MeqMatrixComplexArr::getDouble (int x, int y) const
{
  AssertMsg (itsValue[offset(x,y)].imag()==0,
	     "MeqMatrix: dcomplex->double conversion not possible");
  return itsValue[offset(x,y)].real();
}
complex<double> MeqMatrixComplexArr::getDComplex (int x, int y) const
{
  return itsValue[offset(x,y)];
}


#define MNSMATRIXCOMPLEXARR_OP(NAME, OP, OP2) \
MeqMatrixRep* MeqMatrixComplexArr::NAME (MeqMatrixRealSca& left, \
					 bool rightTmp) \
{ \
  MeqMatrixComplexArr* v = this; \
  if (!rightTmp) { \
    v = (MeqMatrixComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValue[i] = left.itsValue OP2 v->itsValue[i]; \
  } \
  return v; \
} \
MeqMatrixRep* MeqMatrixComplexArr::NAME (MeqMatrixComplexSca& left, \
					 bool rightTmp) \
{ \
  MeqMatrixComplexArr* v = this; \
  if (!rightTmp) { \
    v = (MeqMatrixComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValue[i] = left.itsValue OP2 v->itsValue[i]; \
  } \
  return v; \
} \
MeqMatrixRep* MeqMatrixComplexArr::NAME (MeqMatrixRealArr& left,  \
					 bool rightTmp) \
{ \
  Assert (nelements() == left.nelements()); \
  MeqMatrixComplexArr* v = this; \
  if (!rightTmp) { \
    v = (MeqMatrixComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValue[i] = left.itsValue[i] OP2 v->itsValue[i]; \
  } \
  return v; \
} \
MeqMatrixRep* MeqMatrixComplexArr::NAME (MeqMatrixComplexArr& left, \
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


MeqMatrixRep* MeqMatrixComplexArr::negate()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = -(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::sin(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::cos(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::exp(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::sqr()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] *= itsValue[i];
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::sqrt()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::sqrt(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::conj()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = ::conj(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::min()
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
  return new MeqMatrixComplexSca (val);
}
MeqMatrixRep* MeqMatrixComplexArr::max()
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
  return new MeqMatrixComplexSca (val);
}
MeqMatrixRep* MeqMatrixComplexArr::mean()
{
  complex<double> sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MeqMatrixComplexSca (sum/n);
}
MeqMatrixRep* MeqMatrixComplexArr::sum()
{
  complex<double> sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MeqMatrixComplexSca (sum);
}

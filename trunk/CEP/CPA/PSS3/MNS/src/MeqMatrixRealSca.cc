//# MeqMatrixRealSca.cc: Temporary matrix for Mns
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
#include <aips/Mathematics/Math.h>
#include <Common/lofar_iostream.h>


MeqMatrixRealSca::~MeqMatrixRealSca()
{}

MeqMatrixRep* MeqMatrixRealSca::clone() const
{
  return new MeqMatrixRealSca (itsValue);
}

void MeqMatrixRealSca::show (ostream& os) const
{
  os << itsValue;
}

MeqMatrixRep* MeqMatrixRealSca::add (MeqMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealSca::subtract (MeqMatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealSca::multiply (MeqMatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealSca::divide (MeqMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealSca::tocomplex (MeqMatrixRep& right)
{
  return right.complexRep (*this);
}

bool MeqMatrixRealSca::isDouble() const
{
  return true;
}
const double* MeqMatrixRealSca::doubleStorage() const
{
  return &itsValue;
}
double MeqMatrixRealSca::getDouble (int, int) const
{
  return itsValue;
}
complex<double> MeqMatrixRealSca::getDComplex (int, int) const
{
  return itsValue;
}


#define MNSMATRIXREALSCA_OP(NAME, OP, OPX) \
MeqMatrixRep* MeqMatrixRealSca::NAME (MeqMatrixRealSca& left, \
				      bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MeqMatrixRep* MeqMatrixRealSca::NAME (MeqMatrixComplexSca& left, \
				      bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MeqMatrixRep* MeqMatrixRealSca::NAME (MeqMatrixRealArr& left,  \
				      bool) \
{ \
  double* value = left.itsValue; \
  double* end = value + left.nelements(); \
  while (value < end) { \
    *value++ OP itsValue; \
  } \
  return &left; \
} \
MeqMatrixRep* MeqMatrixRealSca::NAME (MeqMatrixComplexArr& left, \
				      bool) \
{ \
  complex<double>* value = left.itsValue; \
  int n = left.nelements(); \
  for (int i=0; i<n; i++) { \
    *value OP itsValue; \
    value++; \
  } \
  return &left; \
}

MNSMATRIXREALSCA_OP(addRep,+=,'+');
MNSMATRIXREALSCA_OP(subRep,-=,'-');
MNSMATRIXREALSCA_OP(mulRep,*=,'*');
MNSMATRIXREALSCA_OP(divRep,/=,'/');

MeqMatrixRep* MeqMatrixRealSca::complexRep (MeqMatrixRealSca& left)
{
  return new MeqMatrixComplexSca (complex<double> (left.itsValue, itsValue));
}
MeqMatrixRep* MeqMatrixRealSca::complexRep (MeqMatrixRealArr& left)
{
  MeqMatrixComplexArr* v = new MeqMatrixComplexArr (left.nx(), left.ny());
  complex<double>* value = v->itsValue;
  double  rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    value[i] = complex<double> (lvalue[i], rvalue);
  }
  return v;
}


MeqMatrixRep* MeqMatrixRealSca::negate()
{
  itsValue = -itsValue;
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::sin()
{
  itsValue = ::sin(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::cos()
{
  itsValue = ::cos(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::exp()
{
  itsValue = ::exp(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::sqr()
{
  itsValue *= itsValue;
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::sqrt()
{
  itsValue = ::sqrt(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::conj()
{
  return this;
}

MeqMatrixRep* MeqMatrixRealSca::min()
{
  return this;
}
MeqMatrixRep* MeqMatrixRealSca::max()
{
  return this;
}
MeqMatrixRep* MeqMatrixRealSca::mean()
{
  return this;
}
MeqMatrixRep* MeqMatrixRealSca::sum()
{
  return this;
}

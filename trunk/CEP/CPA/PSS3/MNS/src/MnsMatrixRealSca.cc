//# MnsMatrixRealSca.cc: Temporary matrix for Mns
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
#include <aips/Mathematics/Math.h>
#include <Common/lofar_iostream.h>


MnsMatrixRealSca::~MnsMatrixRealSca()
{}

MnsMatrixRep* MnsMatrixRealSca::clone() const
{
  return new MnsMatrixRealSca (itsValue);
}

void MnsMatrixRealSca::show (ostream& os) const
{
  os << itsValue;
}

MnsMatrixRep* MnsMatrixRealSca::add (MnsMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixRealSca::subtract (MnsMatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixRealSca::multiply (MnsMatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixRealSca::divide (MnsMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

double MnsMatrixRealSca::getDouble (int, int) const
{
  return itsValue;
}
complex<double> MnsMatrixRealSca::getDComplex (int, int) const
{
  return itsValue;
}


#define MNSMATRIXREALSCA_OP(NAME, OP, OPX) \
MnsMatrixRep* MnsMatrixRealSca::NAME (MnsMatrixRealSca& left, \
				      bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MnsMatrixRep* MnsMatrixRealSca::NAME (MnsMatrixComplexSca& left, \
				      bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MnsMatrixRep* MnsMatrixRealSca::NAME (MnsMatrixRealArr& left,  \
				      bool) \
{ \
  double* value = left.itsValue; \
  double* end = value + left.nelements(); \
  while (value < end) { \
    *value++ OP itsValue; \
  } \
  return &left; \
} \
MnsMatrixRep* MnsMatrixRealSca::NAME (MnsMatrixComplexArr& left, \
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


void MnsMatrixRealSca::negate()
{
  itsValue = -itsValue;
}

void MnsMatrixRealSca::sin()
{
  itsValue = ::sin(itsValue);
}

void MnsMatrixRealSca::cos()
{
  itsValue = ::cos(itsValue);
}

void MnsMatrixRealSca::exp()
{
  itsValue = ::exp(itsValue);
}

void MnsMatrixRealSca::conj()
{}

MnsMatrixRep* MnsMatrixRealSca::min()
{
  return this;
}
MnsMatrixRep* MnsMatrixRealSca::max()
{
  return this;
}
MnsMatrixRep* MnsMatrixRealSca::mean()
{
  return this;
}
MnsMatrixRep* MnsMatrixRealSca::sum()
{
  return this;
}

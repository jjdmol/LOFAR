//# MnsMatrixComplexSca.cc: Temporary matrix for Mns
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


MnsMatrixComplexSca::~MnsMatrixComplexSca()
{}

MnsMatrixRep* MnsMatrixComplexSca::clone() const
{
  return new MnsMatrixComplexSca (itsValue);
}

void MnsMatrixComplexSca::show (ostream& os) const
{
  os << itsValue;
}

MnsMatrixRep* MnsMatrixComplexSca::add (MnsMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixComplexSca::subtract (MnsMatrixRep& right,
					     bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixComplexSca::multiply (MnsMatrixRep& right,
					     bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MnsMatrixRep* MnsMatrixComplexSca::divide (MnsMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

double MnsMatrixComplexSca::getDouble (int, int) const
{
  AssertMsg (false, "MnsMatrix: dcomplex->double conversion not possible");
}
complex<double> MnsMatrixComplexSca::getDComplex (int, int) const
{
  return itsValue;
}


#define MNSMATRIXCOMPLEXSCA_OP(NAME, OP, OP2) \
MnsMatrixRep* MnsMatrixComplexSca::NAME (MnsMatrixRealSca& left, \
					 bool rightTmp) \
{ \
  MnsMatrixComplexSca* v = this; \
  if (!rightTmp) { \
    v = new MnsMatrixComplexSca (itsValue); \
  } \
  v->itsValue = left.itsValue OP2 v->itsValue; \
  return v; \
} \
MnsMatrixRep* MnsMatrixComplexSca::NAME (MnsMatrixComplexSca& left, \
					 bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MnsMatrixRep* MnsMatrixComplexSca::NAME (MnsMatrixRealArr& left,  \
					 bool) \
{ \
  MnsMatrixComplexArr* v = new MnsMatrixComplexArr (left.nx(), left.ny()); \
  for (int i=0; i<left.nelements(); i++) { \
    v->itsValue[i] = left.itsValue[i] OP2 itsValue; \
  } \
  return v; \
} \
MnsMatrixRep* MnsMatrixComplexSca::NAME (MnsMatrixComplexArr& left, \
					 bool) \
{ \
  for (int i=0; i<left.nelements(); i++) { \
    left.itsValue[i] OP itsValue; \
  } \
  return &left; \
}

MNSMATRIXCOMPLEXSCA_OP(addRep,+=,+);
MNSMATRIXCOMPLEXSCA_OP(subRep,-=,-);
MNSMATRIXCOMPLEXSCA_OP(mulRep,*=,*);
MNSMATRIXCOMPLEXSCA_OP(divRep,/=,/);


void MnsMatrixComplexSca::negate()
{
  itsValue = -itsValue;
}

void MnsMatrixComplexSca::sin()
{
  itsValue = ::sin(itsValue);
}

void MnsMatrixComplexSca::cos()
{
  itsValue = ::cos(itsValue);
}

void MnsMatrixComplexSca::exp()
{
  itsValue = ::exp(itsValue);
}

void MnsMatrixComplexSca::conj()
{
  itsValue = ::conj(itsValue);
}

MnsMatrixRep* MnsMatrixComplexSca::min()
{
  return this;
}
MnsMatrixRep* MnsMatrixComplexSca::max()
{
  return this;
}
MnsMatrixRep* MnsMatrixComplexSca::mean()
{
  return this;
}
MnsMatrixRep* MnsMatrixComplexSca::sum()
{
  return this;
}

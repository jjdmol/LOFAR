//# VellsComplexSca.cc: Temporary vells for Meq
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


#include <MEQ/VellsRealSca.h>
#include <MEQ/VellsRealArr.h>
#include <MEQ/VellsComplexSca.h>
#include <MEQ/VellsComplexArr.h>
#include <Common/Debug.h>

namespace MEQ {

VellsComplexSca::~VellsComplexSca()
{}

VellsRep* VellsComplexSca::clone() const
{
  return new VellsComplexSca (*itsValuePtr);
}

void VellsComplexSca::show (std::ostream& os) const
{
  os << *itsValuePtr;
}

VellsRep* VellsComplexSca::add (VellsRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
VellsRep* VellsComplexSca::subtract (VellsRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
VellsRep* VellsComplexSca::multiply (VellsRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
VellsRep* VellsComplexSca::divide (VellsRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
VellsRep* VellsComplexSca::pow (VellsRep& right, bool rightTmp)
{
  return right.powRep (*this, rightTmp);
}

complex<double>* VellsComplexSca::complexStorage()
{
  return itsValuePtr;
}


#define MEQVELLSCOMPLEXSCA_OP(NAME, OP, OP2) \
VellsRep* VellsComplexSca::NAME (VellsRealSca& left, bool rightTmp) \
{ \
  VellsComplexSca* v = this; \
  if (!rightTmp) { \
    v = new VellsComplexSca (*itsValuePtr); \
  } \
  *v->itsValuePtr = *left.itsValuePtr OP2 *v->itsValuePtr; \
  return v; \
} \
VellsRep* VellsComplexSca::NAME (VellsComplexSca& left, bool) \
{ \
  *left.itsValuePtr OP *itsValuePtr; \
  return &left; \
} \
VellsRep* VellsComplexSca::NAME (VellsRealArr& left, bool) \
{ \
  VellsComplexArr* v = new VellsComplexArr (left.nx(), left.ny()); \
  for (int i=0; i<left.nelements(); i++) { \
    v->itsValuePtr[i] = left.itsValuePtr[i] OP2 *itsValuePtr; \
  } \
  return v; \
} \
VellsRep* VellsComplexSca::NAME (VellsComplexArr& left, bool) \
{ \
  for (int i=0; i<left.nelements(); i++) { \
    left.itsValuePtr[i] OP *itsValuePtr; \
  } \
  return &left; \
}

MEQVELLSCOMPLEXSCA_OP(addRep,+=,+);
MEQVELLSCOMPLEXSCA_OP(subRep,-=,-);
MEQVELLSCOMPLEXSCA_OP(mulRep,*=,*);
MEQVELLSCOMPLEXSCA_OP(divRep,/=,/);

VellsRep* VellsComplexSca::powRep (VellsRealSca& left, bool rightTmp)
{
  VellsComplexSca* v = this;
  if (!rightTmp) {
    v = new VellsComplexSca (*itsValuePtr);
  }
  *v->itsValuePtr = std::pow(*left.itsValuePtr, *v->itsValuePtr);
  return v;
}
VellsRep* VellsComplexSca::powRep (VellsComplexSca& left, bool)
{
  *left.itsValuePtr = std::pow(*left.itsValuePtr, *itsValuePtr);
  return &left;
}
VellsRep* VellsComplexSca::powRep (VellsRealArr& left, bool)
{
  VellsComplexArr* v = new VellsComplexArr (left.nx(), left.ny());
  for (int i=0; i<left.nelements(); i++) {
    v->itsValuePtr[i] = std::pow(left.itsValuePtr[i], *itsValuePtr);
  }
  return v;
}
VellsRep* VellsComplexSca::powRep (VellsComplexArr& left, bool)
{
  for (int i=0; i<left.nelements(); i++) {
    left.itsValuePtr[i] = std::pow(left.itsValuePtr[i], *itsValuePtr);
  }
  return &left;
}


VellsRep* VellsComplexSca::negate()
{
  *itsValuePtr = -*itsValuePtr;
  return this;
}

VellsRep* VellsComplexSca::sin()
{
  *itsValuePtr = ::sin(*itsValuePtr);
  return this;
}

VellsRep* VellsComplexSca::cos()
{
  *itsValuePtr = ::cos(*itsValuePtr);
  return this;
}

VellsRep* VellsComplexSca::exp()
{
  *itsValuePtr = ::exp(*itsValuePtr);
  return this;
}

VellsRep* VellsComplexSca::sqr()
{
  *itsValuePtr *= *itsValuePtr;
  return this;
}

VellsRep* VellsComplexSca::sqrt()
{
  *itsValuePtr = ::sqrt(*itsValuePtr);
  return this;
}

VellsRep* VellsComplexSca::conj()
{
  *itsValuePtr = ::conj(*itsValuePtr);
  return this;
}

VellsRep* VellsComplexSca::min()
{
  return this;
}
VellsRep* VellsComplexSca::max()
{
  return this;
}
VellsRep* VellsComplexSca::mean()
{
  return this;
}
VellsRep* VellsComplexSca::sum()
{
  return this;
}

} // namespace MEQ

//# VellsRealSca.cc: Temporary vells for Meq
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
#include <aips/Mathematics/Math.h>
#include <aips/Mathematics/Constants.h>

namespace MEQ {

VellsRealSca::~VellsRealSca()
{}

VellsRep* VellsRealSca::clone() const
{
  return new VellsRealSca (*itsValuePtr);
}

void VellsRealSca::show (std::ostream& os) const
{
  os << *itsValuePtr;
}

VellsRep* VellsRealSca::add (VellsRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
VellsRep* VellsRealSca::subtract (VellsRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
VellsRep* VellsRealSca::multiply (VellsRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
VellsRep* VellsRealSca::divide (VellsRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
VellsRep* VellsRealSca::posdiff (VellsRep& right)
{
  return right.posdiffRep (*this);
}
VellsRep* VellsRealSca::tocomplex (VellsRep& right)
{
  return right.tocomplexRep (*this);
}
VellsRep* VellsRealSca::pow (VellsRep& right, bool rightTmp)
{
  return right.powRep (*this, rightTmp);
}

bool VellsRealSca::isReal() const
{
  return true;
}
double* VellsRealSca::realStorage()
{
  return itsValuePtr;
}


#define MEQVELLSREALSCA_OP(NAME, OP, OPX) \
VellsRep* VellsRealSca::NAME (VellsRealSca& left, bool) \
{ \
  *left.itsValuePtr OP *itsValuePtr; \
  return &left; \
} \
VellsRep* VellsRealSca::NAME (VellsComplexSca& left, bool) \
{ \
  *left.itsValuePtr OP *itsValuePtr; \
  return &left; \
} \
VellsRep* VellsRealSca::NAME (VellsRealArr& left, bool) \
{ \
  double* value = left.itsValuePtr; \
  double* end = value + left.nelements(); \
  while (value < end) { \
    *value++ OP *itsValuePtr; \
  } \
  return &left; \
} \
VellsRep* VellsRealSca::NAME (VellsComplexArr& left, bool) \
{ \
  complex<double>* value = left.itsValuePtr; \
  int n = left.nelements(); \
  for (int i=0; i<n; i++) { \
    *value OP *itsValuePtr; \
    value++; \
  } \
  return &left; \
}

MEQVELLSREALSCA_OP(addRep,+=,'+');
MEQVELLSREALSCA_OP(subRep,-=,'-');
MEQVELLSREALSCA_OP(mulRep,*=,'*');
MEQVELLSREALSCA_OP(divRep,/=,'/');

VellsRep* VellsRealSca::posdiffRep (VellsRealSca& left)
{
  double diff = *left.itsValuePtr - *itsValuePtr;
  if (diff < -1 * C::pi) {
    diff += C::_2pi;
  }
  if (diff > C::pi) {
    diff -= C::_2pi;
  }
  return new VellsRealSca (diff);
}
VellsRep* VellsRealSca::posdiffRep (VellsRealArr& left)
{
  VellsRealArr* v = new VellsRealArr (left.nx(), left.ny());
  double* value = v->itsValuePtr;
  double  rvalue = *itsValuePtr;
  double* lvalue = left.itsValuePtr;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double diff = lvalue[i] - rvalue;
    if (diff < -1 * C::pi) {
      diff += C::_2pi;
    }
    if (diff > C::pi) {
      diff -= C::_2pi;
    }
    value[i] = diff;
  }
  return v;
}

VellsRep* VellsRealSca::tocomplexRep (VellsRealSca& left)
{
  return new VellsComplexSca (complex<double> (*left.itsValuePtr,
					       *itsValuePtr));
}
VellsRep* VellsRealSca::tocomplexRep (VellsRealArr& left)
{
  VellsComplexArr* v = new VellsComplexArr (left.nx(), left.ny());
  complex<double>* value = v->itsValuePtr;
  double  rvalue = *itsValuePtr;
  double* lvalue = left.itsValuePtr;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    value[i] = complex<double> (lvalue[i], rvalue);
  }
  return v;
}

VellsRep* VellsRealSca::powRep (VellsRealSca& left, bool)
{
  *left.itsValuePtr = std::pow (*left.itsValuePtr, *itsValuePtr);
  return &left;
}
VellsRep* VellsRealSca::powRep (VellsComplexSca& left, bool)
{
  *left.itsValuePtr = std::pow(*left.itsValuePtr, *itsValuePtr);
  return &left;
}
VellsRep* VellsRealSca::powRep (VellsRealArr& left, bool)
{
  double* value = left.itsValuePtr;
  double* end = value + left.nelements();
  while (value < end) {
    *value = std::pow(*value, *itsValuePtr);
    value++;
  }
  return &left;
}
VellsRep* VellsRealSca::powRep (VellsComplexArr& left, bool)
{
  complex<double>* value = left.itsValuePtr;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    *value = std::pow(*value, *itsValuePtr);
    value++;
  }
  return &left;
}


VellsRep* VellsRealSca::negate()
{
  *itsValuePtr = -*itsValuePtr;
  return this;
}

VellsRep* VellsRealSca::sin()
{
  *itsValuePtr = ::sin(*itsValuePtr);
  return this;
}

VellsRep* VellsRealSca::cos()
{
  *itsValuePtr = ::cos(*itsValuePtr);
  return this;
}

VellsRep* VellsRealSca::exp()
{
  *itsValuePtr = ::exp(*itsValuePtr);
  return this;
}

VellsRep* VellsRealSca::sqr()
{
  *itsValuePtr *= *itsValuePtr;
  return this;
}

VellsRep* VellsRealSca::sqrt()
{
  *itsValuePtr = ::sqrt(*itsValuePtr);
  return this;
}

VellsRep* VellsRealSca::conj()
{
  return this;
}

VellsRep* VellsRealSca::min()
{
  return this;
}
VellsRep* VellsRealSca::max()
{
  return this;
}
VellsRep* VellsRealSca::mean()
{
  return this;
}
VellsRep* VellsRealSca::sum()
{
  return this;
}

} // namespace MEQ

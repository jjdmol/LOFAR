//# VellsRealArr.cc: Temporary vells for Meq
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
#include <aips/Mathematics/Constants.h>

namespace MEQ {

VellsRealArr::VellsRealArr (int nx, int ny)
: VellsRep   (nx, ny),
  itsIsOwner (true)
{
  itsValuePtr = new double[nelements()];
}

VellsRealArr::VellsRealArr (double* value, int nx, int ny)
: VellsRep    (nx, ny),
  itsValuePtr (value),
  itsIsOwner  (false)
{}

VellsRealArr::~VellsRealArr()
{
  if (itsIsOwner) {
    delete [] itsValuePtr;
  }
}

VellsRep* VellsRealArr::clone() const
{
  VellsRealArr* v = new VellsRealArr (nx(), ny());
  memcpy (v->itsValuePtr, itsValuePtr, sizeof(double) * nelements());
  return v;
}

void VellsRealArr::set (double value)
{
  for (int i=0; i<nelements(); i++) {
    itsValuePtr[i] = value;
  }
}

void VellsRealArr::show (std::ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << itsValuePtr[i];
  }
  os << ']';
}

VellsRep* VellsRealArr::add (VellsRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
VellsRep* VellsRealArr::subtract (VellsRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
VellsRep* VellsRealArr::multiply (VellsRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
VellsRep* VellsRealArr::divide (VellsRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
VellsRep* VellsRealArr::posdiff (VellsRep& right)
{
  return right.posdiffRep (*this);
}
VellsRep* VellsRealArr::tocomplex (VellsRep& right)
{
  return right.tocomplexRep (*this);
}
VellsRep* VellsRealArr::pow (VellsRep& right, bool rightTmp)
{
  return right.powRep (*this, rightTmp);
}

bool VellsRealArr::isReal() const
{
  return true;
}
double* VellsRealArr::realStorage()
{
  return itsValuePtr;
}


#define MEQVELLSREALARR_OP(NAME, OP, OP2) \
VellsRep* VellsRealArr::NAME (VellsRealSca& left, bool rightTmp) \
{ \
  VellsRealArr* v = this; \
  if (!rightTmp) { \
    v = (VellsRealArr*)clone(); \
  } \
  double* value = v->itsValuePtr; \
  double lvalue = *left.itsValuePtr; \
  double* end = value + nelements(); \
  while (value < end) { \
    *value = lvalue OP2 *value; \
    value++; \
  } \
  return v; \
} \
VellsRep* VellsRealArr::NAME (VellsComplexSca& left, bool) \
{ \
  VellsComplexArr* v = new VellsComplexArr (nx(), ny()); \
  complex<double>* value = v->itsValuePtr; \
  double* value2 = itsValuePtr; \
  complex<double> lvalue = *left.itsValuePtr; \
  int n = nelements(); \
  for (int i=0; i<n; i++) { \
    *value++ = lvalue OP2 *value2++; \
  } \
  return v; \
} \
VellsRep* VellsRealArr::NAME (VellsRealArr& left, bool) \
{ \
  Assert (nelements() == left.nelements()); \
  double* value = left.itsValuePtr; \
  double* value2 = itsValuePtr; \
  double* end = value + nelements(); \
  while (value < end) { \
    *value++ OP *value2++; \
  } \
  return &left; \
} \
VellsRep* VellsRealArr::NAME (VellsComplexArr& left, bool) \
{ \
  Assert (nelements() == left.nelements()); \
  complex<double>* value = left.itsValuePtr; \
  double* value2 = itsValuePtr; \
  int n = left.nelements(); \
  for (int i=0; i<n; i++) { \
    *value++ OP *value2++; \
  } \
  return &left; \
}

MEQVELLSREALARR_OP(addRep,+=,+);
MEQVELLSREALARR_OP(subRep,-=,-);
MEQVELLSREALARR_OP(mulRep,*=,*);
MEQVELLSREALARR_OP(divRep,/=,/);

VellsRep* VellsRealArr::posdiffRep (VellsRealSca& left)
{
  VellsRealArr* v = new VellsRealArr (nx(), ny());
  double* value = v->itsValuePtr;
  double* rvalue = itsValuePtr;
  double  lvalue = *left.itsValuePtr;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double diff = lvalue - rvalue[i];
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
VellsRep* VellsRealArr::posdiffRep (VellsRealArr& left)
{
  Assert (nelements() == left.nelements());
  VellsRealArr* v = new VellsRealArr (nx(), ny());
  double* value = v->itsValuePtr;
  double* rvalue = itsValuePtr;
  double* lvalue = left.itsValuePtr;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double diff = lvalue[i] - rvalue[i];
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

VellsRep* VellsRealArr::tocomplexRep (VellsRealSca& left)
{
  VellsComplexArr* v = new VellsComplexArr (nx(), ny());
  complex<double>* value = v->itsValuePtr;
  double* rvalue = itsValuePtr;
  double  lvalue = *left.itsValuePtr;
  int n = nelements();
  for (int i=0; i<n; i++) {
    value[i] = complex<double> (lvalue, rvalue[i]);
  }
  return v;
}
VellsRep* VellsRealArr::tocomplexRep (VellsRealArr& left)
{
  Assert (nelements() == left.nelements());
  VellsComplexArr* v = new VellsComplexArr (nx(), ny());
  complex<double>* value = v->itsValuePtr;
  double* rvalue = itsValuePtr;
  double* lvalue = left.itsValuePtr;
  int n = nelements();
  for (int i=0; i<n; i++) {
    value[i] = complex<double> (lvalue[i], rvalue[i]);
  }
  return v;
}

VellsRep* VellsRealArr::powRep (VellsRealSca& left, bool rightTmp)
{
  VellsRealArr* v = this;
  if (!rightTmp) {
    v = (VellsRealArr*)clone();
  }
  double* value = v->itsValuePtr;
  double lvalue = *left.itsValuePtr;
  double* end = value + nelements();
  while (value < end) {
    *value = std::pow(lvalue, *value);
    value++;
  }
  return v;
}
VellsRep* VellsRealArr::powRep (VellsComplexSca& left, bool)
{
  VellsComplexArr* v = new VellsComplexArr (nx(), ny());
  complex<double>* value = v->itsValuePtr;
  double* value2 = itsValuePtr;
  complex<double> lvalue = *left.itsValuePtr;
  int n = nelements();
  for (int i=0; i<n; i++) {
    *value++ = std::pow(lvalue, *value2++);
  }
  return v;
}
VellsRep* VellsRealArr::powRep (VellsRealArr& left, bool)
{
  Assert (nelements() == left.nelements());
  double* value = left.itsValuePtr;
  double* value2 = itsValuePtr;
  double* end = value + nelements();
  while (value < end) {
    *value = std::pow(*value, *value2++);
    value++;
  }
  return &left;
}
VellsRep* VellsRealArr::powRep (VellsComplexArr& left, bool)
{
  Assert (nelements() == left.nelements());
  complex<double>* value = left.itsValuePtr;
  double* value2 = itsValuePtr;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    *value = std::pow(*value, *value2++);
  }
  return &left;
}


VellsRep* VellsRealArr::negate()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = -(itsValuePtr[i]);
  }
  return this;
}

VellsRep* VellsRealArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::sin(itsValuePtr[i]);
  }
  return this;
}

VellsRep* VellsRealArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::cos(itsValuePtr[i]);
  }
  return this;
}

VellsRep* VellsRealArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::exp(itsValuePtr[i]);
  }
  return this;
}

VellsRep* VellsRealArr::sqr()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] *= itsValuePtr[i];
  }
  return this;
}

VellsRep* VellsRealArr::sqrt()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::sqrt(itsValuePtr[i]);
  }
  return this;
}

VellsRep* VellsRealArr::conj()
{
  return this;
}

VellsRep* VellsRealArr::min()
{
  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValuePtr[0];
    for (int i=1; i<n; i++) {
      if (itsValuePtr[i] < val) {
	val = itsValuePtr[i];
      }
    }
  }
  return new VellsRealSca (val);
}
VellsRep* VellsRealArr::max()
{
  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValuePtr[0];
    for (int i=1; i<n; i++) {
      if (itsValuePtr[i] > val) {
	val = itsValuePtr[i];
      }
    }
  }
  return new VellsRealSca (val);
}
VellsRep* VellsRealArr::mean()
{
  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValuePtr[i];
  }
  return new VellsRealSca (sum/n);
}
VellsRep* VellsRealArr::sum()
{
  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValuePtr[i];
  }
  return new VellsRealSca (sum);
}

} // namespace MEQ

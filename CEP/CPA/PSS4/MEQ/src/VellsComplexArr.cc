//# VellsComplexArr.cc: Temporary vells for Meq
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

#include <Common/Profiling/PerfProfile.h>

#include <MEQ/VellsRealSca.h>
#include <MEQ/VellsRealArr.h>
#include <MEQ/VellsComplexSca.h>
#include <MEQ/VellsComplexArr.h>
#include <Common/Debug.h>
#include <iomanip>

namespace MEQ {

MEQ::VellsComplexArr::VellsComplexArr (int nx, int ny)
: VellsRep    (nx, ny),
  itsIsOwner  (true)
{
  itsValuePtr = new complex<double>[nelements()];
}

MEQ::VellsComplexArr::VellsComplexArr (complex<double>* value, int nx, int ny)
: VellsRep    (nx, ny),
  itsValuePtr (value),
  itsIsOwner  (false)
{}

MEQ::VellsComplexArr::~VellsComplexArr()
{
  if (itsIsOwner) {
    delete [] itsValuePtr;
  }
}

VellsRep* MEQ::VellsComplexArr::clone() const
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  VellsComplexArr* v = new VellsComplexArr (nx(), ny());
  memcpy (v->itsValuePtr, itsValuePtr, sizeof(complex<double>) * nelements());
  return v;
}

void MEQ::VellsComplexArr::set (complex<double> value)
{
  for (int i=0; i<nelements(); i++) {
    itsValuePtr[i] = value;
  }
}

void MEQ::VellsComplexArr::show (std::ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << '(' << std::setprecision(12) << itsValuePtr[i].real()
       << ',' << std::setprecision(12) << itsValuePtr[i].imag() << ')';
  }
  os << ']';
}


VellsRep* MEQ::VellsComplexArr::add (VellsRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
VellsRep* MEQ::VellsComplexArr::subtract (VellsRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
VellsRep* MEQ::VellsComplexArr::multiply (VellsRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
VellsRep* MEQ::VellsComplexArr::divide (VellsRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
VellsRep* MEQ::VellsComplexArr::pow (VellsRep& right, bool rightTmp)
{
  return right.powRep (*this, rightTmp);
}

complex<double>* MEQ::VellsComplexArr::complexStorage()
{
  return itsValuePtr;
}


#define MEQVELLSCOMPLEXARR_OP(NAME, OP, OP2) \
VellsRep* MEQ::VellsComplexArr::NAME (VellsRealSca& left, bool rightTmp) \
{ \
  VellsComplexArr* v = this; \
  if (!rightTmp) { \
    v = (VellsComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValuePtr[i] = *left.itsValuePtr OP2 v->itsValuePtr[i]; \
  } \
  return v; \
} \
VellsRep* MEQ::VellsComplexArr::NAME (VellsComplexSca& left, bool rightTmp) \
{ \
  VellsComplexArr* v = this; \
  if (!rightTmp) { \
    v = (VellsComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValuePtr[i] = *left.itsValuePtr OP2 v->itsValuePtr[i]; \
  } \
  return v; \
} \
VellsRep* MEQ::VellsComplexArr::NAME (VellsRealArr& left, bool rightTmp) \
{ \
  Assert (nelements() == left.nelements()); \
  VellsComplexArr* v = this; \
  if (!rightTmp) { \
    v = (VellsComplexArr*)clone(); \
  } \
  for (int i=0; i<nelements(); i++) { \
    v->itsValuePtr[i] = left.itsValuePtr[i] OP2 v->itsValuePtr[i]; \
  } \
  return v; \
} \
VellsRep* MEQ::VellsComplexArr::NAME (VellsComplexArr& left, bool) \
{ \
  Assert (nelements() == left.nelements()); \
  for (int i=0; i<left.nelements(); i++) { \
    left.itsValuePtr[i] OP itsValuePtr[i]; \
  } \
  return &left; \
}

MEQVELLSCOMPLEXARR_OP(addRep,+=,+);
MEQVELLSCOMPLEXARR_OP(subRep,-=,-);
MEQVELLSCOMPLEXARR_OP(mulRep,*=,*);
MEQVELLSCOMPLEXARR_OP(divRep,/=,/);

VellsRep* MEQ::VellsComplexArr::powRep (VellsRealSca& left, bool rightTmp)
{
  VellsComplexArr* v = this;
  if (!rightTmp) {
    v = (VellsComplexArr*)clone();
  }
  for (int i=0; i<nelements(); i++) {
    v->itsValuePtr[i] = std::pow(*left.itsValuePtr, v->itsValuePtr[i]);
  }
  return v;
}
VellsRep* MEQ::VellsComplexArr::powRep (VellsComplexSca& left, bool rightTmp)
{
  VellsComplexArr* v = this;
  if (!rightTmp) {
    v = (VellsComplexArr*)clone();
  }
  for (int i=0; i<nelements(); i++) {
    v->itsValuePtr[i] = std::pow(*left.itsValuePtr, v->itsValuePtr[i]);
  }
  return v;
}
VellsRep* MEQ::VellsComplexArr::powRep (VellsRealArr& left, bool rightTmp)
{
  Assert (nelements() == left.nelements());
  VellsComplexArr* v = this;
  if (!rightTmp) {
    v = (VellsComplexArr*)clone();
  }
  for (int i=0; i<nelements(); i++) {
    v->itsValuePtr[i] = std::pow(left.itsValuePtr[i], v->itsValuePtr[i]);
  }
  return v;
}
VellsRep* MEQ::VellsComplexArr::powRep (VellsComplexArr& left, bool)
{
  Assert (nelements() == left.nelements());
  for (int i=0; i<left.nelements(); i++) {
    left.itsValuePtr[i] = std::pow(left.itsValuePtr[i], itsValuePtr[i]);
  }
  return &left;
}


VellsRep* MEQ::VellsComplexArr::negate()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = -(itsValuePtr[i]);
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::sin(itsValuePtr[i]);
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::cos(itsValuePtr[i]);
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::exp(itsValuePtr[i]);
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::sqr()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] *= itsValuePtr[i];
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::sqrt()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::sqrt(itsValuePtr[i]);
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::conj()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValuePtr[i] = ::conj(itsValuePtr[i]);
  }
  return this;
}

VellsRep* MEQ::VellsComplexArr::min()
{
  complex<double> val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValuePtr[0];
    double absval = abs(val);
    for (int i=1; i<n; i++) {
      double av = abs(itsValuePtr[i]);
      if (av < absval) {
	val = itsValuePtr[i];
	absval = av;
      }
    }
  }
  return new VellsComplexSca (val);
}
VellsRep* MEQ::VellsComplexArr::max()
{
  complex<double> val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValuePtr[0];
    double absval = abs(val);
    for (int i=1; i<n; i++) {
      double av = abs(itsValuePtr[i]);
      if (av > absval) {
	val = itsValuePtr[i];
	absval = av;
      }
    }
  }
  return new VellsComplexSca (val);
}
VellsRep* MEQ::VellsComplexArr::mean()
{
  complex<double> sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValuePtr[i];
  }
  return new VellsComplexSca (sum/double(n));
}
VellsRep* MEQ::VellsComplexArr::sum()
{
  complex<double> sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValuePtr[i];
  }
  return new VellsComplexSca (sum);
}

} // namespace MEQ

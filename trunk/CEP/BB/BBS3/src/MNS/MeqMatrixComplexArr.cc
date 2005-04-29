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

#include <lofar_config.h>
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqMatrixRealSca.h>
#include <BBS3/MNS/MeqMatrixRealArr.h>
#include <BBS3/MNS/MeqMatrixComplexSca.h>
#include <BBS3/MNS/MeqMatrixComplexArr.h>
#include <Common/LofarLogger.h>
#include <iomanip>

namespace LOFAR {

// Allocation will be done from the pool containing matrices of theirNElements
// or less elements.
std::deque<MeqMatrixComplexArr*> MeqMatrixComplexArr::theirPool;
int MeqMatrixComplexArr::theirNElements = 0;

// To ensure 8-byte alignment of the data, round the size
// of the header up to the nearest multiple of 8 bytes.
// Add 8 extra bytes to make alignement on 16 bytes possible (for SSE).
size_t MeqMatrixComplexArr::theirHeaderSize =
    ((sizeof(MeqMatrixComplexArr) >> 3) << 3)
  + ((sizeof(MeqMatrixComplexArr) & 0x7)? 8 : 0)
  + 8;

MeqMatrixComplexArr::MeqMatrixComplexArr (int nx, int ny)
: MeqMatrixRep (nx, ny)
{
  // data is found after the header
  // malloc ensures it is aligned on 8 byte, but we want it on 16 byte.
  double* ptr1 = (double*)(((char*)this) + theirHeaderSize);
  if ((ptrdiff_t)(ptr1) & 0x000F) ptr1++;
  itsValue = (dcomplex*)(ptr1);
}

MeqMatrixComplexArr::~MeqMatrixComplexArr()
{
  //delete [] itsValue;   // free is done by deallocate
}

MeqMatrixRep* MeqMatrixComplexArr::clone() const
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);
  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  memcpy (v->itsValue, itsValue, sizeof(dcomplex) * nelements());
  return v;
}

void MeqMatrixComplexArr::set (dcomplex value)
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
    os << '(' << std::setprecision(12) << real(itsValue[i])
       << ',' << std::setprecision(12) << imag(itsValue[i]) << ')';
  }
  os << ']';
}

MeqMatrixComplexArr* MeqMatrixComplexArr::allocate (int nx, int ny)
{
  MeqMatrixComplexArr* newArr = 0;
  if (nx * ny <= theirNElements) {
    if (theirPool.empty()) {
      // Allocate memory for the header and the maximum amount of data
      // (theirNElements). Only nx * ny elements will be used, but the
      // array can be reused for an array of size up to theirNElements.
      newArr = (MeqMatrixComplexArr*)malloc(theirHeaderSize +
					    (theirNElements * sizeof(dcomplex)));
      // placement new to call constructor
      newArr = new (newArr) MeqMatrixComplexArr(nx, ny);
      newArr->setInPool(true);
      newArr->setIsMalloced(true);
    } else {
      // Get an array from the pool.
      newArr = theirPool.back();
      newArr = new (newArr) MeqMatrixComplexArr(nx, ny);
      ///      newArr->setNXY (nx, ny);
      newArr->setInPool(true);
      newArr->setIsMalloced(true);
      theirPool.pop_back();
      MeqMatrixRep::nreused++;
    }
  } else {
    // Array is larger than arrays in pool.
    // So allocate it separately.
    // Still use malloc to get enough memory for alignment.
    newArr = (MeqMatrixComplexArr*)malloc(theirHeaderSize +
					  (nx * ny * sizeof(dcomplex)));
    // placement new
    newArr = new (newArr) MeqMatrixComplexArr(nx, ny);
    // set inPool to false so this matrix will be delete'd.
    newArr->setInPool(false);
    newArr->setIsMalloced(true);
  }
  return newArr;
}

void MeqMatrixComplexArr::deallocate()
{
  ASSERT(isMalloced());
  if (inPool()) {
    theirPool.push_front(this);
  } else {
    MeqMatrixRep::ndeleted++;
    free(this);
  }
}

void MeqMatrixComplexArr::poolActivate(int nelements)
{
  if (nelements != theirNElements) {
    poolDeactivate();
    theirNElements = nelements;
  }
}

void MeqMatrixComplexArr::poolDeactivate()
{
  // Free all objects remaining in the pool and clear the pool.
  deque<MeqMatrixComplexArr*>::iterator pos;
  for (pos = theirPool.begin(); pos < theirPool.end(); ++pos) {
    free(*pos);
    MeqMatrixRep::ndeleted++;
  }
  theirPool.clear();
  // Setting theirNElements to zero will result in no pool usage;
  // allocate will simply 'new' memory, deallocate will 'delete' it.
  theirNElements = 0;
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

const dcomplex* MeqMatrixComplexArr::dcomplexStorage() const
{
  return itsValue;
}
double MeqMatrixComplexArr::getDouble (int x, int y) const
{
  ASSERTSTR (imag(itsValue[offset(x,y)])==0,
	     "MeqMatrix: dcomplex->double conversion not possible");
  return real(itsValue[offset(x,y)]);
}
dcomplex MeqMatrixComplexArr::getDComplex (int x, int y) const
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
  ASSERT (nelements() == left.nelements()); \
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
  ASSERT (nelements() == left.nelements()); \
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
    itsValue[i] = -1. * itsValue[i];
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::sin()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = LOFAR::sin(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = LOFAR::cos(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = LOFAR::exp(itsValue[i]);
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
    itsValue[i] = LOFAR::sqrt(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::conj()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = LOFAR::conj(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::min()
{
  dcomplex val = makedcomplex(0,0);
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
  dcomplex val = makedcomplex(0,0);
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
  dcomplex sum = makedcomplex(0,0);
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MeqMatrixComplexSca (sum/double(n));
}
MeqMatrixRep* MeqMatrixComplexArr::sum()
{
  dcomplex sum = makedcomplex(0,0);
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  return new MeqMatrixComplexSca (sum);
}

}

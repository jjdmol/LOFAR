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

#include <Common/Profiling/PerfProfile.h>

#include <MNS/MeqMatrixRealSca.h>
#include <MNS/MeqMatrixRealArr.h>
#include <MNS/MeqMatrixComplexSca.h>
#include <MNS/MeqMatrixComplexArr.h>
#include <Common/Debug.h>
#include <iomanip>

std::deque<MeqMatrixComplexArr*> MeqMatrixComplexArr::theirPool;

// allocation will be done from the pool for matrices of theirNElements
// or smaller matrices
int MeqMatrixComplexArr::theirNElements = 0;

// to ensure 8-byte alignment of the data round the size
// of the header up to the nearest multiple of 8 bytes
size_t MeqMatrixComplexArr::theirHeaderSize =
    ((sizeof(MeqMatrixComplexArr) >> 3) << 3)
  + ((sizeof(MeqMatrixComplexArr) & 0x7)? 8 : 0);

MeqMatrixComplexArr::MeqMatrixComplexArr (int nx, int ny)
: MeqMatrixRep (nx, ny, sizeof(complex<double>))
{
  // data is found after the header
  itsValue = (complex<double>*)(((char*)this) + theirHeaderSize);
}

MeqMatrixComplexArr::~MeqMatrixComplexArr()
{
  //delete [] itsValue;
}

MeqMatrixRep* MeqMatrixComplexArr::clone() const
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  MeqMatrixComplexArr* v;
  
  v = MeqMatrixComplexArr::poolNew (nx(), ny());
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
    os << '(' << std::setprecision(12) << itsValue[i].real()
       << ',' << std::setprecision(12) << itsValue[i].imag() << ')';
  }
  os << ']';
}

void MeqMatrixComplexArr::poolActivate(int nelements)
{
  theirNElements = nelements;
}

MeqMatrixComplexArr* MeqMatrixComplexArr::poolNew(int nx, int ny)
{
  MeqMatrixComplexArr* newArr = 0;

  if (nx * ny <= theirNElements)
  {
    if (theirPool.empty())
    {
      // allocate memory for the header and the maximum amount of data (theirNElements)
      // only nx * ny elements will be used, but the array can be reused for an array
      // of size theirNElements
      newArr = (MeqMatrixComplexArr*)malloc(theirHeaderSize +
					    (theirNElements * sizeof(complex<double>)));

      // placement new to call constructor
      newArr = new (newArr) MeqMatrixComplexArr(nx, ny);
      newArr->setInPool(true);
      newArr->setIsMalloced(true);
    }
    else
    {
      newArr = theirPool.back();
      newArr = new (newArr) MeqMatrixComplexArr(nx, ny);
      newArr->setInPool(true);
      newArr->setIsMalloced(true);

      theirPool.pop_back();
    }
  }
  else
  {
    // allocate enough memory
    newArr = (MeqMatrixComplexArr*)malloc(theirHeaderSize +
					  (nx * ny * sizeof(complex<double>)));
    // placement new
    newArr = new (newArr) MeqMatrixComplexArr(nx, ny);

    // set inPool to false so this matrix will be delete'd.
    newArr->setInPool(false);
    newArr->setIsMalloced(true);
  }

  return newArr;
}

void MeqMatrixComplexArr::poolDelete()
{
  Assert(inPool() || isMalloced());
  if (inPool())
  {
    theirPool.push_front(this);
  }
  else // isMalloced
  {
    free(this);
  }
}

void MeqMatrixComplexArr::poolDeactivate()
{
  // free all objects remaining in the pool and clear the pool
  deque<MeqMatrixComplexArr*>::iterator pos;
  for (pos = theirPool.begin(); pos < theirPool.end(); ++pos)
  {
    free(*pos);
  }
  theirPool.clear();

  // setting theirNElements to zero will result in no pool usage;
  // poolNew will simply 'new' memory, poolDelete will 'delete' it.
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
    itsValue[i] = std::sin(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::cos()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::cos(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::exp()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::exp(itsValue[i]);
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
    itsValue[i] = std::sqrt(itsValue[i]);
  }
  return this;
}

MeqMatrixRep* MeqMatrixComplexArr::conj()
{
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::conj(itsValue[i]);
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
  return new MeqMatrixComplexSca (sum/double(n));
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

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

#include <PSS3/MNS/MeqMatrixRealSca.h>
#include <PSS3/MNS/MeqMatrixRealArr.h>
#include <PSS3/MNS/MeqMatrixComplexSca.h>
#include <PSS3/MNS/MeqMatrixComplexArr.h>
#include <Common/Debug.h>
#include <iomanip>


#if defined __SSE2__
#include <xmmintrin.h>
#include <emmintrin.h>
#endif


namespace LOFAR {

std::deque<MeqMatrixComplexArr*> MeqMatrixComplexArr::theirPool;

// allocation will be done from the pool for matrices of theirNElements
// or smaller matrices
int MeqMatrixComplexArr::theirNElements = 0;

// to ensure 16-byte alignment of the data round the size
// of the header up to the nearest multiple of 16 bytes
#define MEQ_MATRIX_COMPLEX_ARR_HEADER_SIZE ((sizeof(MeqMatrixComplexArr) + 15) & ~15)

MeqMatrixComplexArr::MeqMatrixComplexArr (int nx, int ny)
: MeqMatrixRep (nx, ny, sizeof(dcomplex))
{
  // data is found after the header
  itsValue = (dcomplex*)(((char*)this) + MEQ_MATRIX_COMPLEX_ARR_HEADER_SIZE);
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

void MeqMatrixComplexArr::poolActivate(int nelements)
{
  theirNElements = nelements;
}

MeqMatrixComplexArr* MeqMatrixComplexArr::poolNew(int nx, int ny)
{
  MeqMatrixComplexArr* newArr = 0;

#if 1
  if (nx * ny <= theirNElements)
  {
    if (theirPool.empty())
    {
      // allocate memory for the header and the maximum amount of data (theirNElements)
      // only nx * ny elements will be used, but the array can be reused for an array
      // of size theirNElements
      newArr = (MeqMatrixComplexArr*) new char [MEQ_MATRIX_COMPLEX_ARR_HEADER_SIZE +
					    (theirNElements * sizeof(dcomplex))];
    }
    else
    {
      newArr = theirPool.back();
      theirPool.pop_back();
    }
    // placement new to call constructor
    new (newArr) MeqMatrixComplexArr(nx, ny);
    newArr->setInPool(true);
  }
  else
#endif
  {
    // allocate enough memory
    newArr = (MeqMatrixComplexArr*) new char [MEQ_MATRIX_COMPLEX_ARR_HEADER_SIZE +
					  (nx * ny * sizeof(dcomplex))];
    // placement new
    new (newArr) MeqMatrixComplexArr(nx, ny);

    // set inPool is cleared by constructor so this matrix will be delete'd.
    // newArr->setInPool(false);
  }

  newArr->setIsMalloced(true);
  return newArr;
}

void MeqMatrixComplexArr::poolDelete()
{
#if 1
  Assert(inPool() || isMalloced());
  if (inPool())
  {
    theirPool.push_front(this);
  }
  else // isMalloced
#endif
  {
    delete (char *) this;
  }
}

void MeqMatrixComplexArr::poolDeactivate()
{
  // free all objects remaining in the pool and clear the pool
  deque<MeqMatrixComplexArr*>::iterator pos;
  for (pos = theirPool.begin(); pos < theirPool.end(); ++pos)
  {
    delete (char *) *pos;
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

const dcomplex* MeqMatrixComplexArr::dcomplexStorage() const
{
  return itsValue;
}
double MeqMatrixComplexArr::getDouble (int x, int y) const
{
  AssertMsg (imag(itsValue[offset(x,y)])==0,
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

#if !defined __SSE2__
MNSMATRIXCOMPLEXARR_OP(addRep,+=,+);
MNSMATRIXCOMPLEXARR_OP(subRep,-=,-);
MNSMATRIXCOMPLEXARR_OP(mulRep,*=,*);
#endif
MNSMATRIXCOMPLEXARR_OP(divRep,/=,/);


#if defined __SSE2__

MeqMatrixRep *MeqMatrixComplexArr::addRep(MeqMatrixRealSca& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d src   = _mm_load_sd(&left.itsValue);
  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_add_pd(src, dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::addRep(MeqMatrixComplexSca& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d src   = _mm_loadu_pd((double *) &left.itsValue);
  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_add_pd(src, dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::addRep(MeqMatrixRealArr& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_add_pd(_mm_load_sd(left.itsValue + i), dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::addRep(MeqMatrixComplexArr& left, bool rightTmp)
{
  __m128d *src  = (__m128d *) itsValue;
  __m128d *dest = (__m128d *) left.itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_add_pd(dest[i], src[i]);

  return &left;
}

MeqMatrixRep *MeqMatrixComplexArr::subRep(MeqMatrixRealSca& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d src   = _mm_load_sd(&left.itsValue);
  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_sub_pd(src, dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::subRep(MeqMatrixComplexSca& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d src   = _mm_loadu_pd((double *) &left.itsValue);
  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_sub_pd(src, dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::subRep(MeqMatrixRealArr& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_sub_pd(_mm_load_sd(left.itsValue + i), dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::subRep(MeqMatrixComplexArr& left, bool rightTmp)
{
  __m128d *src  = (__m128d *) itsValue;
  __m128d *dest = (__m128d *) left.itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_sub_pd(dest[i], src[i]);

  return &left;
}

MeqMatrixRep *MeqMatrixComplexArr::mulRep(MeqMatrixRealSca& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d src   = _mm_load1_pd(&left.itsValue);
  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_mul_pd(src, dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::mulRep(MeqMatrixComplexSca& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d *dest = (__m128d *) v->itsValue;
  __m128d ar    = _mm_load1_pd((double *) &left.itsValue);
  __m128d ai    = _mm_load1_pd((double *) &left.itsValue + 1);
  size_t  count = nelements() - 1;

  for (size_t i = 0; i < count; i += 2) {
    __m128d br = _mm_unpacklo_pd(dest[i], dest[i+1]);
    __m128d bi = _mm_unpackhi_pd(dest[i], dest[i+1]);
    __m128d cr = _mm_sub_pd(_mm_mul_pd(ar, br), _mm_mul_pd(ai, bi));
    __m128d ci = _mm_add_pd(_mm_mul_pd(ar, bi), _mm_mul_pd(ai, br));

    dest[i]    = _mm_unpacklo_pd(cr,ci);
    dest[i+1]  = _mm_unpackhi_pd(cr,ci);
  }

  if ((count & 1) == 0) // uneven array length
    v->itsValue[count] *= left.itsValue;

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::mulRep(MeqMatrixRealArr& left, bool rightTmp)
{
  MeqMatrixComplexArr* v = this;

  if (!rightTmp)
    v = (MeqMatrixComplexArr*) clone();

  __m128d *dest = (__m128d *) v->itsValue;
  size_t  count = nelements();

  for (size_t i = 0; i < count; i ++)
    dest[i] = _mm_mul_pd(_mm_load1_pd(left.itsValue + i), dest[i]);

  return v;
}

MeqMatrixRep *MeqMatrixComplexArr::mulRep(MeqMatrixComplexArr& left, bool rightTmp)
{
  __m128d *src  = (__m128d *) itsValue;
  __m128d *dest = (__m128d *) left.itsValue;
  size_t  count = nelements() - 1;

  for (size_t i = 0; i < count; i += 2) {
    __m128d ar = _mm_unpacklo_pd(src[i], src[i+1]);
    __m128d ai = _mm_unpackhi_pd(src[i], src[i+1]);
    __m128d br = _mm_unpacklo_pd(dest[i], dest[i+1]);
    __m128d bi = _mm_unpackhi_pd(dest[i], dest[i+1]);
    __m128d cr = _mm_sub_pd(_mm_mul_pd(ar, br), _mm_mul_pd(ai, bi));
    __m128d ci = _mm_add_pd(_mm_mul_pd(ar, bi), _mm_mul_pd(ai, br));

    dest[i]    = _mm_unpacklo_pd(cr,ci);
    dest[i+1]  = _mm_unpackhi_pd(cr,ci);
  }

  if ((count & 1) == 0) // uneven array length
    left.itsValue[count] *= itsValue[count];

  return &left;
}

#endif


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

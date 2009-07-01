//# MatrixComplexArr.cc: Temporary matrix for Mns
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

#undef TIMER

#include <lofar_config.h>

#include <BBSKernel/Expr/MatrixRealSca.h>
#include <BBSKernel/Expr/MatrixRealArr.h>
#include <BBSKernel/Expr/MatrixComplexSca.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>
#include <BBSKernel/Expr/Pool.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

#if defined TIMER
#include <Common/Timer.h>
#endif

#if defined __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif

namespace LOFAR
{
namespace BBS
{

// Allocation will be done from the pool containing matrices of poolNElements
// or less elements.
//static Pool<MatrixComplexArr>    pool;
//#pragma omp threadprivate(pool)
//static size_t           poolArraySize;
//static int              poolNElements = 0;

MatrixComplexArr::MatrixComplexArr (int nx, int ny)
: MatrixRep (nx, ny, ComplexArray)
{
  // data is found after the header
  // align itsReal and itsImag to 16 bytes
  ptrdiff_t ptr = (ptrdiff_t(this) + sizeof(MatrixComplexArr) + 15) & ~15;
  itsReal = (double *) ptr;
  itsImag = (double *) ptr + ((nelements() + 1) & ~1);
}

MatrixComplexArr::~MatrixComplexArr()
{
}

MatrixRep* MatrixComplexArr::clone() const
{
#if defined TIMER
  static NSTimer timer("clone CA", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate(nx(), ny());

#if defined __SSE2__
  __m128d *src_r = (__m128d *) itsReal, *src_i = (__m128d *) itsImag;
  __m128d *dst_r = (__m128d *) v->itsReal, *dst_i = (__m128d *) v->itsImag;
  int n = (nelements() + 1) / 2;

  for (int i = 0; i < n; i ++) {
    dst_r[i] = src_r[i];
    dst_i[i] = src_i[i];
  }
#else
  memcpy (v->itsReal, itsReal, sizeof(double) * nelements());
  memcpy (v->itsImag, itsImag, sizeof(double) * nelements());
#endif

#if defined TIMER
  timer.stop();
#endif

  return v;
}

void MatrixComplexArr::set (dcomplex value)
{
#if defined TIMER
  static NSTimer timer("set CA", true);
  timer.start();
#endif

#if defined __SSE2__
  __m128d *dst_r = (__m128d *) itsReal, *dst_i = (__m128d *) itsImag;
  __m128d re = _mm_set1_pd(real(value)), im = _mm_set1_pd(imag(value));

  int n = (nelements() + 1) / 2;
  for (int i = 0; i < n; i ++) {
    dst_r[i] = re;
    dst_i[i] = im;
  }
#else
  for (int i=0; i<nelements(); i++) {
    itsReal[i] = real(value);
    itsImag[i] = imag(value);
  }
#endif

#if defined TIMER
  timer.stop();
#endif
}

void MatrixComplexArr::show (ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << '(' << setprecision(12) << itsReal[i]
       << ',' << setprecision(12) << itsImag[i] << ')';
  }
  os << ']';
}

size_t MatrixComplexArr::memSize(int nelements)
{
  return ((sizeof(MatrixComplexArr) + 7) & ~7) + 16 +
     2 * sizeof(double) * ((nelements + 1) & ~1);
}

void *MatrixComplexArr::operator new(size_t, int nx, int ny)
{
#if defined TIMER
  static NSTimer timer("new CA", true);
  timer.start();
#endif

  void *ptr;

//  if (nx * ny == poolNElements)
//  {
    // Allocate memory from the pool.
//    ptr = pool.allocate(poolArraySize);
//  }
//  else
  {
    // The number of elements in the matrix is non-standard -> allocate it separately.
    // Still use new to get enough memory for alignment.
    ptr = malloc(memSize(nx * ny));
  }

#if defined TIMER
  timer.stop();
#endif

  return ptr;
}


void MatrixComplexArr::operator delete(void *ptr)
{
#if defined TIMER
  static NSTimer timer("delete CA", true);
  timer.start();
#endif

//  if(poolNElements == 0 || ((MatrixComplexArr *) ptr)->nelements() != poolNElements)
  {
    // Pool is deactivated or the number of elements in the matrix is non-standard.
    // -> use standard free()
    free(ptr);
  }
//  else
//  {
    // Return memory to the pool.
//    pool.deallocate((MatrixComplexArr *) ptr);
//  }

#if defined TIMER
  timer.stop();
#endif
}

/*
void MatrixComplexArr::poolActivate(int nelements)
{
  //cerr << "MatrixComplexArr::poolActivate(" << nelements << ")\n";
  if (nelements != poolNElements) {
    poolDeactivate();
    poolNElements = nelements;
    poolArraySize = memSize(nelements);
  }
}

void MatrixComplexArr::poolDeactivate()
{
  // Free all objects remaining in the pool and clear the pool.
#pragma omp parallel
  pool.clear();
  // Setting poolNElements to zero will result in no pool usage;
  // allocate will simply 'new' memory, deallocate will 'delete' it.
  poolNElements = 0;
}
*/

MatrixRep* MatrixComplexArr::add (MatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MatrixRep* MatrixComplexArr::subtract (MatrixRep& right,
                         bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MatrixRep* MatrixComplexArr::multiply (MatrixRep& right,
                         bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MatrixRep* MatrixComplexArr::divide (MatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

void MatrixComplexArr::dcomplexStorage(const double *&realPtr, const double *&imagPtr) const
{
  realPtr = itsReal;
  imagPtr = itsImag;
}

double MatrixComplexArr::getDouble (int x, int y) const
{
  ASSERTSTR (itsImag[offset(x,y)]==0,
         "Matrix: dcomplex->double conversion not possible");
  return itsReal[offset(x,y)];
}

dcomplex MatrixComplexArr::getDComplex (int x, int y) const
{
  int off = offset(x,y);
  return makedcomplex(itsReal[off], itsImag[off]);
}


MatrixRep* MatrixComplexArr::addRep(MatrixRealSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("add CA RS", true);
  timer.start();
#endif

  MatrixComplexArr* v = this;

  if (!rightTmp) {
    v = MatrixComplexArr::allocate(nx(), ny());
    memcpy(v->itsImag, itsImag, nelements() * sizeof(double));
  }

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = itsReal[i] + left.itsValue;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::addRep(MatrixComplexSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("add CA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = itsReal[i] + left_r;
    v->itsImag[i] = itsImag[i] + left_i;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::addRep(MatrixRealArr& left, bool rightTmp)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("add CA RA", true);
  timer.start();
#endif

  MatrixComplexArr* v = this;

  if (!rightTmp) {
    v = MatrixComplexArr::allocate(nx(), ny());
    memcpy(v->itsImag, itsImag, nelements() * sizeof(double));
  }

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = itsReal[i] + left.itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}


MatrixRep* MatrixComplexArr::addRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("add CA CA", true);
  timer.start();
#endif

#if defined __SSE2__
  __m128d *dst_r = (__m128d *) left.itsReal, *dst_i = (__m128d *) left.itsImag;
  __m128d *src_r = (__m128d *) itsReal, *src_i = (__m128d *) itsImag;

  int n = (nelements() + 1) / 2;
  for (int i = 0; i < n; i ++) {
    dst_r[i] = _mm_add_pd(src_r[i], dst_r[i]);
    dst_i[i] = _mm_add_pd(src_i[i], dst_i[i]);
  }
#else
  for (int i = 0; i < nelements(); i ++) {
    left.itsReal[i] += itsReal[i];
    left.itsImag[i] += itsImag[i];
  }
#endif

#if defined TIMER
  timer.stop();
#endif

  return &left;
}


MatrixRep* MatrixComplexArr::subRep(MatrixRealSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("sub CA RS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = left.itsValue - itsReal[i];
    v->itsImag[i] = - itsImag[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::subRep(MatrixComplexSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("sub CA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

  double left_r = real(left.itsValue), left_i = imag(left.itsValue);

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = left_r - itsReal[i];
    v->itsImag[i] = left_i - itsImag[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::subRep(MatrixRealArr& left, bool rightTmp)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("sub CA RA", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = left.itsValue[i] - itsReal[i];
    v->itsImag[i] = - itsImag[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}


MatrixRep* MatrixComplexArr::subRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("sub CA CA", true);
  timer.start();
#endif

  for (int i = 0; i < nelements(); i ++) {
    left.itsReal[i] -= itsReal[i];
    left.itsImag[i] -= itsImag[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}


MatrixRep* MatrixComplexArr::mulRep(MatrixRealSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("mul CA RS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = itsReal[i] * left.itsValue;
    v->itsImag[i] = itsImag[i] * left.itsValue;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::mulRep(MatrixComplexSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("mul CA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

#if defined __SSE2__
  __m128d left_r = _mm_set1_pd(real(left.itsValue));
  __m128d left_i = _mm_set1_pd(imag(left.itsValue));
  __m128d *dst_r = (__m128d *) v->itsReal, *dst_i = (__m128d *) v->itsImag;
  __m128d *src_r = (__m128d *) itsReal, *src_i = (__m128d *) itsImag;
  int n = (nelements() + 1) / 2;

  for (int i = 0; i < n; i ++) {
    __m128d re = src_r[i], im = src_i[i];
    dst_r[i] = _mm_sub_pd(_mm_mul_pd(re, left_r), _mm_mul_pd(im, left_i));
    dst_i[i] = _mm_add_pd(_mm_mul_pd(re, left_i), _mm_mul_pd(im, left_r));
  }
#else
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);

  for (int i = 0; i < nelements(); i ++) {
    double re = itsReal[i], im = itsImag[i];
    v->itsReal[i] = re * left_r - im * left_i;
    v->itsImag[i] = re * left_i + im * left_r;
  }
#endif

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::mulRep(MatrixRealArr& left, bool rightTmp)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("mul CA RA", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

#if defined __SSE2__
  __m128d *left_r = (__m128d *) left.itsValue;
  __m128d *dst_r = (__m128d *) v->itsReal, *dst_i = (__m128d *) v->itsImag;
  __m128d *src_r = (__m128d *) itsReal, *src_i = (__m128d *) itsImag;
  int n = (nelements() + 1) / 2;

  for (int i = 0; i < n; i ++) {
    __m128d left_v = left_r[i];
    dst_r[i] = _mm_mul_pd(src_r[i], left_v);
    dst_i[i] = _mm_mul_pd(src_i[i], left_v);
  }
#else
  for (int i = 0; i < nelements(); i ++) {
    v->itsReal[i] = itsReal[i] * left.itsValue[i];
    v->itsImag[i] = itsImag[i] * left.itsValue[i];
  }
#endif

#if defined TIMER
  timer.stop();
#endif

  return v;
}


MatrixRep* MatrixComplexArr::mulRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("mul CA CA", true);
  timer.start();
#endif

#if defined __SSE2__
  __m128d *dst_r = (__m128d *) left.itsReal, *dst_i = (__m128d *) left.itsImag;
  __m128d *src_r = (__m128d *) itsReal, *src_i = (__m128d *) itsImag;

  int n = (nelements() + 1) / 2;
  for (int i = 0; i < n; i ++) {
    __m128d left_r = dst_r[i], left_i = dst_i[i];
    __m128d re = src_r[i], im = src_i[i];

    dst_r[i] = _mm_sub_pd(_mm_mul_pd(left_r, re), _mm_mul_pd(left_i, im));
    dst_i[i] = _mm_add_pd(_mm_mul_pd(left_r, im), _mm_mul_pd(left_i, re));
  }
#else
  for (int i = 0; i < nelements(); i ++) {
    double left_r = left.itsReal[i], left_i = left.itsImag[i];
    double re = itsReal[i], im = itsImag[i];

    left.itsReal[i] = left_r * re - left_i * im;
    left.itsImag[i] = left_r * im + left_i * re;
  }
#endif

#if defined TIMER
  timer.stop();
#endif

  return &left;
}


MatrixRep* MatrixComplexArr::divRep(MatrixRealSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("div CA RS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());
  double x_r = left.itsValue;

  for (int i = 0; i < nelements(); i ++) {
    double y_r = itsReal[i], y_i = itsImag[i];
    double tmp = x_r / (y_r * y_r + y_i * y_i);

    v->itsReal[i] = y_r * tmp;
    v->itsImag[i] = - y_i * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::divRep(MatrixComplexSca& left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("div CA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());
  double x_r = real(left.itsValue), x_i = imag(left.itsValue);

  for (int i = 0; i < nelements(); i ++) {
    double y_r = itsReal[i], y_i = itsImag[i];
    double tmp = 1.0 / (y_r * y_r + y_i * y_i);

    v->itsReal[i] = (x_r * y_r + x_i * y_i) * tmp;
    v->itsImag[i] = (x_i * y_r - x_r * y_i) * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixComplexArr::divRep(MatrixRealArr& left, bool rightTmp)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("div CA RA", true);
  timer.start();
#endif

  MatrixComplexArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    double y_r = itsReal[i], y_i = itsImag[i];
    double x_r = left.itsValue[i];
    double tmp = x_r / (y_r * y_r + y_i * y_i);

    v->itsReal[i] = y_r * tmp;
    v->itsImag[i] = - y_i * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}


MatrixRep* MatrixComplexArr::divRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("div CA CA", true);
  timer.start();
#endif

  for (int i = 0; i < nelements(); i ++) {
    double y_r = itsReal[i], y_i = itsImag[i];
    double x_r = left.itsReal[i], x_i = left.itsImag[i];
    double tmp = 1.0 / (y_r * y_r + y_i * y_i);

    left.itsReal[i] = (x_r * y_r + x_i * y_i) * tmp;
    left.itsImag[i] = (x_i * y_r - x_r * y_i) * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}


MatrixRep* MatrixComplexArr::negate()
{
#if defined TIMER
  static NSTimer timer("negate CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsReal[i] = -itsReal[i];
    itsImag[i] = -itsImag[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::sin()
{
#if defined TIMER
  static NSTimer timer("sin CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    dcomplex value = LOFAR::sin(makedcomplex(itsReal[i], itsImag[i]));
    itsReal[i] = real(value);
    itsImag[i] = imag(value);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::cos()
{
#if defined TIMER
  static NSTimer timer("cos CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    dcomplex value = LOFAR::cos(makedcomplex(itsReal[i], itsImag[i]));
    itsReal[i] = real(value);
    itsImag[i] = imag(value);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::log()
{
#if defined TIMER
  static NSTimer timer("log CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    dcomplex value = LOFAR::log(makedcomplex(itsReal[i], itsImag[i]));
    itsReal[i] = real(value);
    itsImag[i] = imag(value);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::exp()
{
#if defined TIMER
  static NSTimer timer("exp CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    dcomplex value = LOFAR::exp(makedcomplex(itsReal[i], itsImag[i]));
    itsReal[i] = real(value);
    itsImag[i] = imag(value);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::sqr()
{
#if defined TIMER
  static NSTimer timer("sqr CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    double re = itsReal[i], im = itsImag[i];
    itsReal[i] = re * re - im * im;
    itsImag[i] = 2 * re * im;
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::sqrt()
{
#if defined TIMER
  static NSTimer timer("sqrt CA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    dcomplex value = LOFAR::sqrt(makedcomplex(itsReal[i], itsImag[i]));
    itsReal[i] = real(value);
    itsImag[i] = imag(value);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::conj()
{
#if defined TIMER
  static NSTimer timer("conj CA  ", true);
  timer.start();
#endif

#if defined __SSE2__
  __m128d minus_one = _mm_set1_pd(-1.0);
  __m128d *dst_i = (__m128d *) itsImag;
  int n = (nelements() + 1) / 2;

  for (int i = 0; i < n; i ++) {
    dst_i[i] = _mm_mul_pd(minus_one, dst_i[i]);
  }
#else
  int n = nelements();
  for (int i=0; i<n; i++) {
    itsImag[i] = -itsImag[i];
  }
#endif

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixComplexArr::min()
{
#if defined TIMER
  static NSTimer timer("min CA", true);
  timer.start();
#endif

  double re = 0, im = 0;
  int n = nelements();
  if (n > 0) {
    re = itsReal[0], im = itsImag[0];
    double absval_sqr = re * re + im * im;
    for (int i=1; i<n; i++) {
      double avs = itsReal[i] * itsReal[i] + itsImag[i] * itsImag[i];
      if (avs < absval_sqr) {
    re = itsReal[i], im = itsImag[i];
    absval_sqr = avs;
      }
    }
  }

  MatrixRep *result = new MatrixComplexSca (makedcomplex(re, im));

#if defined TIMER
  timer.stop();
#endif

  return result;
}

MatrixRep* MatrixComplexArr::max()
{
#if defined TIMER
  static NSTimer timer("max CA", true);
  timer.start();
#endif

  double re = 0, im = 0;
  int n = nelements();
  if (n > 0) {
    re = itsReal[0], im = itsImag[0];
    double absval_sqr = re * re + im * im;
    for (int i=1; i<n; i++) {
      double avs = itsReal[i] * itsReal[i] + itsImag[i] * itsImag[i];
      if (avs > absval_sqr) {
    re = itsReal[i], im = itsImag[i];
    absval_sqr = avs;
      }
    }
  }

  MatrixRep *result = new MatrixComplexSca (makedcomplex(re, im));

#if defined TIMER
  timer.stop();
#endif

  return result;
}

MatrixRep* MatrixComplexArr::mean()
{
#if defined TIMER
  static NSTimer timer("mean CA", true);
  timer.start();
#endif

  double sum_r = 0, sum_i = 0;

  int n = nelements();
  for (int i = 0; i < n; i ++) {
    sum_r += itsReal[i];
    sum_i += itsImag[i];
  }

  MatrixRep *result = new MatrixComplexSca (makedcomplex(sum_r / n, sum_i / n));

#if defined TIMER
  timer.stop();
#endif

  return result;
}

MatrixRep* MatrixComplexArr::sum()
{
#if defined TIMER
  static NSTimer timer("sum CA", true);
  timer.start();
#endif

  double sum_r = 0, sum_i = 0;

  int n = nelements();
  for (int i = 0; i < n; i ++) {
    sum_r += itsReal[i];
    sum_i += itsImag[i];
  }

  MatrixRep *result = new MatrixComplexSca (makedcomplex(sum_r, sum_i));

#if defined TIMER
  timer.stop();
#endif

  return result;
}

void MatrixComplexArr::fillRowWithProducts(dcomplex v0, dcomplex factor,
                          int row)
{
#if defined TIMER
  static NSTimer timer("fillWithProducts CA", true);
  timer.start();
#endif
  int nr = nx();
  int start = row*nr;
#if defined __SSE2__
  // The array always starts on a correct SSE boundary.
  // However, start may not be a multiple of 2.
  // If not, do the first one without SSE and adjust.
  if (start%2 != 0) {
    itsReal[start] = real(v0);
    itsImag[start] = imag(v0);
    v0 *= factor;
    start++;
    nr--;
  }
  // Now do the rest with SSE.
  // Note that nr might not be a multiple of 2, so one value too much might
  // be stored. This is not a problem as long as the array is filled
  // sequentially (which it is normally).
  // The array always has a multiple of 2 values, so the last chunk will
  // also behave correctly.
  if (nr > 0) {
    dcomplex v1        = v0 * factor;
    __m128d  v01_r     = _mm_set_pd(real(v1), real(v0));
    __m128d  v01_i     = _mm_set_pd(imag(v1), imag(v0));

    dcomplex factor2   = factor * factor;
    __m128d  factor2_r = _mm_set1_pd(real(factor2));
    __m128d  factor2_i = _mm_set1_pd(imag(factor2));

    __m128d *dst_r = (__m128d *) (itsReal+start);
    __m128d *dst_i = (__m128d *) (itsImag+start);

    for (int i = 0, n = (nr - 1) / 2;; i ++) {
      dst_r[i] = v01_r, dst_i[i] = v01_i;

      if (i == n) break;

      __m128d old_v01_r = v01_r;

      v01_r = _mm_sub_pd(_mm_mul_pd(v01_r, factor2_r),
             _mm_mul_pd(v01_i, factor2_i));
      v01_i = _mm_add_pd(_mm_mul_pd(old_v01_r, factor2_i),
             _mm_mul_pd(v01_i, factor2_r));
    }
  }
#else
  double v0_r = real(v0), v0_i = imag(v0);
  double factor_r = real(factor), factor_i = imag(factor);

  for (int i = start, n = start + nr - 1;; i ++) {
    itsReal[i] = v0_r, itsImag[i] = v0_i;

    if (i == n) break;

    v0_r = v0_r * factor_r - v0_i * factor_i;
    v0_i = itsReal[i] * factor_i + v0_i * factor_r;
  }
#endif

#if defined TIMER
  timer.stop();
#endif
}

} // namespace BBS
} // namespace LOFAR

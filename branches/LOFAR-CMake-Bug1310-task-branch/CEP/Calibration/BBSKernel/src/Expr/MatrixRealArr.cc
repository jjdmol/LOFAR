//# MatrixRealArr.cc: Temporary matrix for Mns
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
#include <casa/BasicSL/Constants.h>
#include <cmath>

#if defined TIMER
#include <Common/Timer.h>
#endif

#if defined __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif

using namespace casa;

namespace LOFAR
{
namespace BBS
{

// Allocation will be done from the pool containing matrices of poolNElements
// or less elements.
//static Pool<MatrixRealArr> pool;
//#pragma omp threadprivate(pool)
//static size_t         poolArraySize;
//static int            poolNElements = 0;


MatrixRealArr::MatrixRealArr (int nx, int ny)
: MatrixRep (nx, ny, RealArray)
{
  // data is found after the header, aligned at 16 bytes
  itsValue = (double *) ((ptrdiff_t(this) + sizeof(MatrixRealArr) + 15) & ~15);
}

MatrixRealArr::~MatrixRealArr()
{
}


size_t MatrixRealArr::memSize(int nelements) {
  return ((sizeof(MatrixRealArr) + 7) & ~7) + 16 + sizeof(double)*nelements;
}

void *MatrixRealArr::operator new(size_t, int nx, int ny)
{
#if defined TIMER
  static NSTimer timer("new RA", true);
  timer.start();
#endif

  void *ptr;

//  if (nx * ny == poolNElements)
//  {
//    // Allocate memory from the pool.
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

void MatrixRealArr::operator delete(void *ptr)
{
#if defined TIMER
  static NSTimer timer("delete RA", true);
  timer.start();
#endif

//  if(poolNElements == 0 || ((MatrixRealArr *) ptr)->nelements() != poolNElements)
  {
    // Pool is deactivated or the number of elements in the matrix is non-standard.
    // -> use standard free()
    free(ptr);
  }
//  else
//  {
    // Return memory to the pool.
//    pool.deallocate((MatrixRealArr *) ptr);
//  }

#if defined TIMER
  timer.stop();
#endif
}

/*
void MatrixRealArr::poolActivate(int nelements)
{
  //std::cerr << "MatrixRealArr::poolActivate(" << nelements << ")\n";
  if (nelements != poolNElements) {
    poolDeactivate();
    poolNElements = nelements;
    poolArraySize = memSize(nelements);
  }
}

void MatrixRealArr::poolDeactivate()
{
  // Free all objects remaining in the pool and clear the pool.
#pragma omp parallel
  pool.clear();
  // Setting poolNElements to zero will result in no pool usage;
  // allocate will simply 'new' memory, deallocate will 'delete' it.
  poolNElements = 0;
}
*/

MatrixRep* MatrixRealArr::clone() const
{
#if defined TIMER
  static NSTimer timer("clone RA", true);
  timer.start();
#endif

  MatrixRealArr* v = allocate(nx(), ny());

#if defined __SSE2__
  __m128d *src = (__m128d *) itsValue, *dst = (__m128d *) v->itsValue;
  int count = (nelements() + 1) / 2;

  for (int i = 0; i < count; i ++)
    dst[i] = src[i];
#else
  memcpy (v->itsValue, itsValue, sizeof(double) * nelements());
#endif

#if defined TIMER
  timer.stop();
#endif

  return v;
}

void MatrixRealArr::set (double value)
{
#if defined TIMER
  static NSTimer timer("set RA", true);
  timer.start();
#endif

  for (int i=0; i<nelements(); i++) {
    itsValue[i] = value;
  }

#if defined TIMER
  timer.stop();
#endif
}

void MatrixRealArr::show (ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << itsValue[i];
  }
  os << ']';
}

MatrixRep* MatrixRealArr::add (MatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MatrixRep* MatrixRealArr::subtract (MatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MatrixRep* MatrixRealArr::multiply (MatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MatrixRep* MatrixRealArr::divide (MatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
MatrixRep* MatrixRealArr::posdiff (MatrixRep& right)
{
  return right.posdiffRep (*this);
}
MatrixRep* MatrixRealArr::tocomplex (MatrixRep& right)
{
  return right.tocomplexRep (*this);
}

const double* MatrixRealArr::doubleStorage() const
{
  return itsValue;
}
double MatrixRealArr::getDouble (int x, int y) const
{
  return itsValue[offset(x,y)];
}
dcomplex MatrixRealArr::getDComplex (int x, int y) const
{
  return makedcomplex(itsValue[offset(x,y)], 0);
}


MatrixRep *MatrixRealArr::addRep(MatrixRealSca &left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("add RA RS", true);
  timer.start();
#endif

  MatrixRealArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsValue[i] = left.itsValue + itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep *MatrixRealArr::addRep(MatrixRealArr &left, bool)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("add RA RA", true);
  timer.start();
#endif

  for (int i = 0; i < nelements(); i ++) {
    left.itsValue[i] += itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixRealArr::addRep(MatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("add RA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate(nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left_r + itsValue[i];
    v->itsImag[i] = left_i;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixRealArr::addRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("add RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] += itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep *MatrixRealArr::subRep(MatrixRealSca &left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("sub RA RS", true);
  timer.start();
#endif

  MatrixRealArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsValue[i] = left.itsValue - itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep *MatrixRealArr::subRep(MatrixRealArr &left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("sub RA RA", true);
  timer.start();
#endif

  for (int i = 0; i < nelements(); i ++) {
    left.itsValue[i] -= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixRealArr::subRep(MatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("sub RA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate(nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left_r - itsValue[i];
    v->itsImag[i] = left_i;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixRealArr::subRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("sub RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] -= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep *MatrixRealArr::mulRep(MatrixRealSca &left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("mul RA RS", true);
  timer.start();
#endif

  MatrixRealArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsValue[i] = left.itsValue * itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep *MatrixRealArr::mulRep(MatrixRealArr &left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("mul RA RA", true);
  timer.start();
#endif

  for (int i = 0; i < nelements(); i ++) {
    left.itsValue[i] *= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixRealArr::mulRep(MatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("mul RA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate (nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left_r * itsValue[i];
    v->itsImag[i] = left_i * itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixRealArr::mulRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("mul RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] *= itsValue[i];
    left.itsImag[i] *= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep *MatrixRealArr::divRep(MatrixRealSca &left, bool rightTmp)
{
#if defined TIMER
  static NSTimer timer("div RA RS", true);
  timer.start();
#endif

  MatrixRealArr* v = rightTmp ? this : allocate(nx(), ny());

  for (int i = 0; i < nelements(); i ++) {
    v->itsValue[i] = left.itsValue / itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep *MatrixRealArr::divRep(MatrixRealArr &left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("div RA RA", true);
  timer.start();
#endif

  for (int i = 0; i < nelements(); i ++) {
    left.itsValue[i] /= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixRealArr::divRep(MatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("div RA CS", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate(nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    double tmp = 1.0 / itsValue[i];
    v->itsReal[i] = left_r * tmp;
    v->itsImag[i] = left_i * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixRealArr::divRep(MatrixComplexArr& left, bool)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("div RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double tmp = 1.0 / itsValue[i];
    left.itsReal[i] *= tmp;
    left.itsImag[i] *= tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixRealArr::posdiffRep (MatrixRealSca& left)
{
#if defined TIMER
  static NSTimer timer("posdiffRep RA RS", true);
  timer.start();
#endif

  MatrixRealArr* v = allocate(nx(), ny());
  double* value = v->itsValue;
  double* rvalue = itsValue;
  double  lvalue = left.itsValue;
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

#if defined TIMER
  timer.stop();
#endif

  return v;
}
MatrixRep* MatrixRealArr::posdiffRep (MatrixRealArr& left)
{
  DBGASSERT (left.nelements() == nelements());
#if defined TIMER
  static NSTimer timer("posdiffRep RA RA", true);
  timer.start();
#endif

  DBGASSERT (nelements() == left.nelements());
  MatrixRealArr* v = allocate(nx(), ny());
  double* value = v->itsValue;
  double* rvalue = itsValue;
  double* lvalue = left.itsValue;
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

#if defined TIMER
  timer.stop();
#endif

  return v;
}

MatrixRep* MatrixRealArr::tocomplexRep (MatrixRealSca& left)
{
#if defined TIMER
  static NSTimer timer("tocomplex RA RS", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate(nx(), ny());
  double* rvalue = itsValue;
  double  lvalue = left.itsValue;
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = lvalue;
    v->itsImag[i] = rvalue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}
MatrixRep* MatrixRealArr::tocomplexRep (MatrixRealArr& left)
{
  DBGASSERT (nelements() == left.nelements());
#if defined TIMER
  static NSTimer timer("tocomplex RA RA", true);
  timer.start();
#endif

  MatrixComplexArr* v = MatrixComplexArr::allocate(nx(), ny());
  double* rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = lvalue[i];
    v->itsImag[i] = rvalue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return v;
}


MatrixRep* MatrixRealArr::negate()
{
#if defined TIMER
  static NSTimer timer("negate RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = -(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixRealArr::sin()
{
#if defined TIMER
  static NSTimer timer("sin RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::sin(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixRealArr::cos()
{
#if defined TIMER
  static NSTimer timer("cos RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::cos(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixRealArr::exp()
{
#if defined TIMER
  static NSTimer timer("exp RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::exp(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixRealArr::sqr()
{
#if defined TIMER
  static NSTimer timer("sqr RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] *= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixRealArr::sqrt()
{
#if defined TIMER
  static NSTimer timer("sqrt RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::sqrt(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif

  return this;
}

MatrixRep* MatrixRealArr::conj()
{
  return this;
}

MatrixRep* MatrixRealArr::min()
{
#if defined TIMER
  static NSTimer timer("min RA", true);
  timer.start();
#endif

  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    for (int i=1; i<n; i++) {
      if (itsValue[i] < val) {
    val = itsValue[i];
      }
    }
  }
  MatrixRep *result = new MatrixRealSca (val);

#if defined TIMER
  timer.stop();
#endif

  return result;
}


MatrixRep* MatrixRealArr::max()
{
#if defined TIMER
  static NSTimer timer("max RA", true);
  timer.start();
#endif

  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    for (int i=1; i<n; i++) {
      if (itsValue[i] > val) {
    val = itsValue[i];
      }
    }
  }
  MatrixRep *result = new MatrixRealSca (val);

#if defined TIMER
  timer.stop();
#endif

  return result;
}


MatrixRep* MatrixRealArr::mean()
{
#if defined TIMER
  static NSTimer timer("mean RA", true);
  timer.start();
#endif

  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  MatrixRep *result = new MatrixRealSca (sum/n);

#if defined TIMER
  timer.stop();
#endif

  return result;
}


MatrixRep* MatrixRealArr::sum()
{
#if defined TIMER
  static NSTimer timer("sum RA", true);
  timer.start();
#endif

  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  MatrixRep *result = new MatrixRealSca (sum);

#if defined TIMER
  timer.stop();
#endif

  return result;
}

} // namespace BBS
} // namespace LOFAR

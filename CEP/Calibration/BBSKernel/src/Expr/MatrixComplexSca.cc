//# MatrixComplexSca.cc: Temporary matrix for Mns
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#undef TIMER

#include <lofar_config.h>
#include <BBSKernel/Expr/MatrixRealSca.h>
#include <BBSKernel/Expr/MatrixRealArr.h>
#include <BBSKernel/Expr/MatrixComplexSca.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>
#include <Common/LofarLogger.h>

#if defined TIMER
#include <Common/Timer.h>
#endif

namespace LOFAR
{
namespace BBS
{

MatrixComplexSca::~MatrixComplexSca()
{}

MatrixRep* MatrixComplexSca::clone() const
{
  return new MatrixComplexSca (itsValue);
}

void MatrixComplexSca::show (ostream& os) const
{
  os << itsValue;
}

MatrixRep* MatrixComplexSca::add (MatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MatrixRep* MatrixComplexSca::subtract (MatrixRep& right,
					     bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MatrixRep* MatrixComplexSca::multiply (MatrixRep& right,
					     bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MatrixRep* MatrixComplexSca::divide (MatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
MatrixRep* MatrixComplexSca::min (MatrixRep& right)
{
  return MatrixRep::min(right);
}
MatrixRep* MatrixComplexSca::max (MatrixRep& right)
{
  return MatrixRep::max(right);
}

void MatrixComplexSca::dcomplexStorage(const double *&realPtr, const double *&imagPtr) const
{
  realPtr = (double *) &itsValue;
  imagPtr = realPtr + 1;
}

double MatrixComplexSca::getDouble (int, int) const
{
  ASSERTSTR (imag(itsValue)==0,
	     "Matrix: dcomplex->double conversion not possible");
  return real(itsValue);
}

dcomplex MatrixComplexSca::getDComplex (int, int) const
{
  return itsValue;
}


#define ExprMATRIXCOMPLEXSCA_OP(NAME, OP, OP2) \
MatrixRep* MatrixComplexSca::NAME (MatrixRealSca& left, \
					 bool rightTmp) \
{ \
  MatrixComplexSca* v = this; \
  if (!rightTmp) { \
    v = new MatrixComplexSca (itsValue); \
  } \
  v->itsValue = left.itsValue OP2 v->itsValue; \
  return v; \
} \
MatrixRep* MatrixComplexSca::NAME (MatrixComplexSca& left, \
					 bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
}

ExprMATRIXCOMPLEXSCA_OP(addRep,+=,+);
ExprMATRIXCOMPLEXSCA_OP(subRep,-=,-);
ExprMATRIXCOMPLEXSCA_OP(mulRep,*=,*);
ExprMATRIXCOMPLEXSCA_OP(divRep,/=,/);

MatrixRep* MatrixComplexSca::addRep(MatrixRealArr& left, bool)
{
  MatrixComplexArr* v = MatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    v->itsReal[i] = left.itsValue[i] + re;
    v->itsImag[i] = im;
  }
  return v;
}

MatrixRep* MatrixComplexSca::addRep(MatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("add CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    left.itsReal[i] += re;
    left.itsImag[i] += im;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixComplexSca::subRep(MatrixRealArr& left, bool)
{
  MatrixComplexArr* v = MatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    v->itsReal[i] = left.itsValue[i] - re;
    v->itsImag[i] = im;
  }
  return v;
}

MatrixRep* MatrixComplexSca::subRep(MatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("sub CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    left.itsReal[i] -= re;
    left.itsImag[i] -= im;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixComplexSca::mulRep(MatrixRealArr& left, bool)
{
  MatrixComplexArr* v = MatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    v->itsReal[i] = left.itsValue[i] * re;
    v->itsImag[i] = left.itsValue[i] * im;
  }
  return v;
}

MatrixRep* MatrixComplexSca::mulRep(MatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("mul CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    double left_r = left.itsReal[i], left_i = left.itsImag[i];
    left.itsReal[i] = re * left_r - im * left_i;
    left.itsImag[i] = re * left_i + im * left_r;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixComplexSca::divRep(MatrixRealArr& left, bool)
{
  MatrixComplexArr* v = MatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);
  double tmp = re * re + im * im;

  for (int i=0; i<left.nelements(); i++) {
    double t = left.itsValue[i] / tmp;
    v->itsReal[i] = re * t;
    v->itsImag[i] = - im * t;
  }
  return v;
}

MatrixRep* MatrixComplexSca::divRep(MatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("div CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);
  double tmp = 1.0 / (re * re + im * im);

  for (int i=0; i<left.nelements(); i++) {
    double left_r = left.itsReal[i], left_i = left.itsImag[i];
    left.itsReal[i] = (left_r * re + left_i * im) * tmp;
    left.itsImag[i] = (left_i * im - left_r * re) * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MatrixRep* MatrixComplexSca::negate()
{
  itsValue = -1. * itsValue;
  return this;
}

MatrixRep* MatrixComplexSca::abs()
{
  return new MatrixRealSca(LOFAR::abs(itsValue));
}

MatrixRep* MatrixComplexSca::sin()
{
  itsValue = LOFAR::sin(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::cos()
{
  itsValue = LOFAR::cos(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::log()
{
  itsValue = LOFAR::log(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::exp()
{
  itsValue = LOFAR::exp(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::log10()
{
  itsValue = LOFAR::log10(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::pow10()
{
  itsValue = LOFAR::pow(10.0, itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::sqr()
{
  itsValue *= itsValue;
  return this;
}

MatrixRep* MatrixComplexSca::sqrt()
{
  itsValue = LOFAR::sqrt(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::conj()
{
  itsValue = LOFAR::conj(itsValue);
  return this;
}

MatrixRep* MatrixComplexSca::min()
{
  return this;
}
MatrixRep* MatrixComplexSca::max()
{
  return this;
}
MatrixRep* MatrixComplexSca::mean()
{
  return this;
}
MatrixRep* MatrixComplexSca::sum()
{
  return this;
}

} // namespace BBS
} // namespace LOFAR

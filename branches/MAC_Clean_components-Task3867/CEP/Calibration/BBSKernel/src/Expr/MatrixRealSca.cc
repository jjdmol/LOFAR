//# MatrixRealSca.cc: Temporary matrix for Mns
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

#include <lofar_config.h>
#include <BBSKernel/Expr/MatrixRealSca.h>
#include <BBSKernel/Expr/MatrixRealArr.h>
#include <BBSKernel/Expr/MatrixComplexSca.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MatrixRealSca::~MatrixRealSca()
{}

MatrixRep* MatrixRealSca::clone() const
{
  return new MatrixRealSca (itsValue);
}

void MatrixRealSca::show (ostream& os) const
{
  os << itsValue;
}

MatrixRep* MatrixRealSca::add (MatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MatrixRep* MatrixRealSca::subtract (MatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MatrixRep* MatrixRealSca::multiply (MatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MatrixRep* MatrixRealSca::divide (MatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
MatrixRep* MatrixRealSca::pow (MatrixRep& right, bool rightTmp)
{
  return right.powRep (*this, rightTmp);
}
MatrixRep* MatrixRealSca::posdiff (MatrixRep& right)
{
  return right.posdiffRep (*this);
}
MatrixRep* MatrixRealSca::tocomplex (MatrixRep& right)
{
  return right.tocomplexRep (*this);
}
MatrixRep* MatrixRealSca::min (MatrixRep& right)
{
  return right.minRep (*this);
}
MatrixRep* MatrixRealSca::max (MatrixRep& right)
{
  return right.maxRep (*this);
}
MatrixRep* MatrixRealSca::atan2 (MatrixRep& right)
{
  return right.atan2Rep (*this);
}

const double* MatrixRealSca::doubleStorage() const
{
  return &itsValue;
}
double MatrixRealSca::getDouble (int, int) const
{
  return itsValue;
}
dcomplex MatrixRealSca::getDComplex (int, int) const
{
  return makedcomplex(itsValue, 0);
}


#define ExprMATRIXREALSCA_OP(NAME, OP, OPX) \
MatrixRep* MatrixRealSca::NAME (MatrixRealSca& left, \
                      bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MatrixRep* MatrixRealSca::NAME (MatrixComplexSca& left, \
                      bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
} \
MatrixRep* MatrixRealSca::NAME (MatrixRealArr& left,  \
                      bool) \
{ \
  double* value = left.itsValue; \
  double* end = value + left.nelements(); \
  while (value < end) { \
    *value++ OP itsValue; \
  } \
  return &left; \
}

ExprMATRIXREALSCA_OP(addRep,+=,'+');
ExprMATRIXREALSCA_OP(subRep,-=,'-');
ExprMATRIXREALSCA_OP(mulRep,*=,'*');
ExprMATRIXREALSCA_OP(divRep,/=,'/');

MatrixRep *MatrixRealSca::addRep(MatrixComplexArr &left, bool)
{
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] += itsValue;
  }
  return &left;
}

MatrixRep *MatrixRealSca::subRep(MatrixComplexArr &left, bool)
{
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] -= itsValue;
  }
  return &left;
}

MatrixRep *MatrixRealSca::mulRep(MatrixComplexArr &left, bool)
{
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] *= itsValue;
    left.itsImag[i] *= itsValue;
  }
  return &left;
}

MatrixRep *MatrixRealSca::divRep(MatrixComplexArr &left, bool)
{
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] /= itsValue;
    left.itsImag[i] /= itsValue;
  }
  return &left;
}

MatrixRep* MatrixRealSca::powRep(MatrixRealSca& left, bool rightTmp)
{
  MatrixRealSca *v = rightTmp ? this : new MatrixRealSca(itsValue);
  v->itsValue = std::pow(left.itsValue, itsValue);
  return v;
}
MatrixRep* MatrixRealSca::powRep(MatrixComplexSca& left, bool)
{
  return new MatrixComplexSca(std::pow(left.itsValue, itsValue));
}
MatrixRep* MatrixRealSca::powRep(MatrixRealArr& left, bool)
{
  MatrixRealArr *v = MatrixRealArr::allocate(left.nx(), left.ny());
  int n = v->nelements();
  for (int i = 0; i < n; ++i) {
    v->itsValue[i] = std::pow(left.itsValue[i], itsValue);
  }
  return v;
}
MatrixRep *MatrixRealSca::powRep(MatrixComplexArr &left, bool)
{
  MatrixComplexArr *v = MatrixComplexArr::allocate(left.nx(), left.ny());
  int n = v->nelements();
  for (int i = 0; i < n; ++i) {
    dcomplex value = std::pow(makedcomplex(left.itsReal[i], left.itsImag[i]),
        itsValue);
    v->itsReal[i] = real(value);
    v->itsImag[i] = imag(value);
  }
  return v;
}

MatrixRep* MatrixRealSca::posdiffRep (MatrixRealSca& left)
{
  double diff = left.itsValue - itsValue;
  if (diff < -1 * C::pi) {
    diff += C::_2pi;
  }
  if (diff > C::pi) {
    diff -= C::_2pi;
  }
  return new MatrixRealSca (diff);
}
MatrixRep* MatrixRealSca::posdiffRep (MatrixRealArr& left)
{
  MatrixRealArr* v = MatrixRealArr::allocate(left.nx(), left.ny());
  double* value = v->itsValue;
  double  rvalue = itsValue;
  double* lvalue = left.itsValue;
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

MatrixRep* MatrixRealSca::tocomplexRep (MatrixRealSca& left)
{
  return new MatrixComplexSca (makedcomplex (left.itsValue, itsValue));
}
MatrixRep* MatrixRealSca::tocomplexRep (MatrixRealArr& left)
{
  MatrixComplexArr* v = MatrixComplexArr::allocate (left.nx(), left.ny());
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left.itsValue[i];
    v->itsImag[i] = itsValue;
  }
  return v;
}

MatrixRep* MatrixRealSca::minRep (MatrixRealSca& left)
{
  return new MatrixRealSca (LOFAR::min(left.itsValue, itsValue));
}
MatrixRep* MatrixRealSca::minRep (MatrixRealArr& left)
{
  MatrixRealArr* v = MatrixRealArr::allocate(left.nx(), left.ny());
  double* value = v->itsValue;
  double  rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    value[i] = LOFAR::min(lvalue[i], rvalue);
  }
  return v;
}

MatrixRep* MatrixRealSca::maxRep (MatrixRealSca& left)
{
  return new MatrixRealSca (LOFAR::max(left.itsValue, itsValue));
}
MatrixRep* MatrixRealSca::maxRep (MatrixRealArr& left)
{
  MatrixRealArr* v = MatrixRealArr::allocate(left.nx(), left.ny());
  double* value = v->itsValue;
  double  rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    value[i] = LOFAR::max(lvalue[i], rvalue);
  }
  return v;
}

MatrixRep* MatrixRealSca::atan2Rep (MatrixRealSca& left)
{
  return new MatrixRealSca (std::atan2(left.itsValue, itsValue));
}
MatrixRep* MatrixRealSca::atan2Rep (MatrixRealArr& left)
{
  MatrixRealArr* v = MatrixRealArr::allocate(left.nx(), left.ny());
  double* value = v->itsValue;
  double  rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    value[i] = std::atan2(lvalue[i], rvalue);
  }
  return v;
}

MatrixRep* MatrixRealSca::negate()
{
  itsValue = -itsValue;
  return this;
}

MatrixRep* MatrixRealSca::abs()
{
  itsValue = std::abs(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::sin()
{
  itsValue = std::sin(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::cos()
{
  itsValue = std::cos(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::asin()
{
  itsValue = std::asin(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::acos()
{
  itsValue = std::acos(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::log()
{
  itsValue = std::log(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::exp()
{
  itsValue = std::exp(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::log10()
{
  itsValue = std::log10(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::pow10()
{
  itsValue = std::pow(10.0, itsValue);
  return this;
}

MatrixRep* MatrixRealSca::sqr()
{
  itsValue *= itsValue;
  return this;
}

MatrixRep* MatrixRealSca::sqrt()
{
  itsValue = std::sqrt(itsValue);
  return this;
}

MatrixRep* MatrixRealSca::conj()
{
  return this;
}

MatrixRep* MatrixRealSca::min()
{
  return this;
}
MatrixRep* MatrixRealSca::max()
{
  return this;
}
MatrixRep* MatrixRealSca::mean()
{
  return this;
}
MatrixRep* MatrixRealSca::sum()
{
  return this;
}

} // namespace BBS
} // namespace LOFAR

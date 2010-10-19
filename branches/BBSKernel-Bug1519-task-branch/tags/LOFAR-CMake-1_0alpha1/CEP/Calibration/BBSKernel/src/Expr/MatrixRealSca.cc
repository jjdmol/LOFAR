//# MatrixRealSca.cc: Temporary matrix for Mns
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
#include <BBSKernel/Expr/MatrixRealSca.h>
#include <BBSKernel/Expr/MatrixRealArr.h>
#include <BBSKernel/Expr/MatrixComplexSca.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <Common/lofar_iostream.h>

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
MatrixRep* MatrixRealSca::posdiff (MatrixRep& right)
{
  return right.posdiffRep (*this);
}
MatrixRep* MatrixRealSca::tocomplex (MatrixRep& right)
{
  return right.tocomplexRep (*this);
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


MatrixRep* MatrixRealSca::negate()
{
  itsValue = -itsValue;
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

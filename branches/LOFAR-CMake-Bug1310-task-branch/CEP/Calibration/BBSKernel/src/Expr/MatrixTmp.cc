//# MatrixTmp.cc: Matrix for Mns
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
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/MatrixRealArr.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>


namespace LOFAR
{
namespace BBS
{

MatrixTmp::MatrixTmp (double value, int nx, int ny, bool init)
{
    MatrixRealArr* v = MatrixRealArr::allocate(nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

MatrixTmp::MatrixTmp (dcomplex value, int nx, int ny, bool init)
{
    MatrixComplexArr* v = MatrixComplexArr::allocate (nx, ny);
    if (init) {
      v->set (value);
    }
    itsRep = v->link();
}

MatrixTmp& MatrixTmp::operator= (const MatrixTmp& that)
{
  if (this != &that) {
    MatrixRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

MatrixTmp MatrixTmp::operator-() const
{
  itsRep->negate();
  return itsRep;
}

MatrixTmp posdiff (const MatrixTmp& left, const Matrix& right)
{
    return left.itsRep->posdiff(*right.rep());
}
MatrixTmp posdiff (const MatrixTmp& left, const MatrixTmp& right)
{
    return left.itsRep->posdiff(*right.rep());
}
MatrixTmp tocomplex (const MatrixTmp& left, const Matrix& right)
{
    return left.itsRep->tocomplex(*right.rep());
}
MatrixTmp tocomplex (const MatrixTmp& left, const MatrixTmp& right)
{
    return left.itsRep->tocomplex(*right.rep());
}
MatrixTmp min (const MatrixTmp& left, const Matrix& right)
{
    return left.itsRep->min(*right.rep());
}
MatrixTmp min (const MatrixTmp& left, const MatrixTmp& right)
{
    return left.itsRep->min(*right.rep());
}
MatrixTmp max (const MatrixTmp& left, const Matrix& right)
{
    return left.itsRep->max(*right.rep());
}
MatrixTmp max (const MatrixTmp& left, const MatrixTmp& right)
{
    return left.itsRep->max(*right.rep());
}
MatrixTmp abs (const MatrixTmp& arg)
{
  return arg.itsRep->abs();
}
MatrixTmp sin (const MatrixTmp& arg)
{
  return arg.itsRep->sin();
}
MatrixTmp cos (const MatrixTmp& arg)
{
  return arg.itsRep->cos();
}
MatrixTmp log (const MatrixTmp& arg)
{
  return arg.itsRep->log();
}
MatrixTmp exp (const MatrixTmp& arg)
{
  return arg.itsRep->exp();
}
MatrixTmp log10 (const MatrixTmp& arg)
{
  return arg.itsRep->log10();
}
MatrixTmp pow10 (const MatrixTmp& arg)
{
  return arg.itsRep->pow10();
}
MatrixTmp sqr(const MatrixTmp& arg)
{
  return arg.itsRep->sqr();
}
MatrixTmp sqrt(const MatrixTmp& arg)
{
  return arg.itsRep->sqrt();
}
MatrixTmp conj (const MatrixTmp& arg)
{
  return arg.itsRep->conj();
}
MatrixTmp min (const MatrixTmp& arg)
{
  return arg.itsRep->min();
}
MatrixTmp max (const MatrixTmp& arg)
{
  return arg.itsRep->max();
}
MatrixTmp mean (const MatrixTmp& arg)
{
  return arg.itsRep->mean();
}
MatrixTmp sum (const MatrixTmp& arg)
{
  return arg.itsRep->sum();
}

} // namespace BBS
} // namespace LOFAR

//# MatrixRealArr.h: Temporary matrix for Mns
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

#ifndef LOFAR_BBSKERNEL_EXPR_MATRIXREALARR_H
#define LOFAR_BBSKERNEL_EXPR_MATRIXREALARR_H

// \file
// Temporary matrix for Mns

//# Includes
#include <BBSKernel/Expr/MatrixRep.h>
#include <cstring>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class MatrixRealArr : public MatrixRep
{
friend class MatrixRealSca;
friend class MatrixComplexSca;
friend class MatrixComplexArr;

private:
  MatrixRealArr (int nx, int ny);   // use allocate(nx, ny) instead

public:
  virtual ~MatrixRealArr();

  void *operator new(size_t size, int nx, int ny);
  void operator delete(void *ptr);

  inline static MatrixRealArr *allocate(int nx, int ny) {
    return new (nx, ny) MatrixRealArr(nx, ny);
  }

//  static void poolActivate(int nelements);
//  static void poolDeactivate();

  virtual MatrixRep* clone() const;

  void set (double value);

  void set (int x, int y, double value)
    { itsValue[offset(x,y)] = value; }

  void set (const double* values)
    { memcpy (itsValue, values, nelements() * sizeof(double)); }

  virtual void show (ostream& os) const;

  virtual const double* doubleStorage() const;
  virtual double getDouble (int x, int y) const;
  virtual dcomplex getDComplex (int x, int y) const;

  virtual MatrixRep* add      (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* subtract (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* multiply (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* divide   (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* pow      (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* posdiff  (MatrixRep& right);
  virtual MatrixRep* tocomplex(MatrixRep& right);
  virtual MatrixRep* min      (MatrixRep& right);
  virtual MatrixRep* max      (MatrixRep& right);
  virtual MatrixRep* atan2    (MatrixRep& right);

private:
  static size_t memSize(int nelements);

  virtual MatrixRep* addRep (MatrixRealSca& left, bool rightTmp);
  virtual MatrixRep* addRep (MatrixComplexSca& left, bool rightTmp);
  virtual MatrixRep* addRep (MatrixRealArr& left, bool rightTmp);
  virtual MatrixRep* addRep (MatrixComplexArr& left, bool rightTmp);

  virtual MatrixRep* subRep (MatrixRealSca& left, bool rightTmp);
  virtual MatrixRep* subRep (MatrixRealArr& left, bool rightTmp);
  virtual MatrixRep* subRep (MatrixComplexSca& left, bool rightTmp);
  virtual MatrixRep* subRep (MatrixComplexArr& left, bool rightTmp);

  virtual MatrixRep* mulRep (MatrixRealSca& left, bool rightTmp);
  virtual MatrixRep* mulRep (MatrixRealArr& left, bool rightTmp);
  virtual MatrixRep* mulRep (MatrixComplexSca& left, bool rightTmp);
  virtual MatrixRep* mulRep (MatrixComplexArr& left, bool rightTmp);

  virtual MatrixRep* divRep (MatrixRealSca& left, bool rightTmp);
  virtual MatrixRep* divRep (MatrixRealArr& left, bool rightTmp);
  virtual MatrixRep* divRep (MatrixComplexSca& left, bool rightTmp);
  virtual MatrixRep* divRep (MatrixComplexArr& left, bool rightTmp);

  virtual MatrixRep* powRep (MatrixRealSca& left, bool rightTmp);
  virtual MatrixRep* powRep (MatrixRealArr& left, bool rightTmp);
  virtual MatrixRep* powRep (MatrixComplexSca& left, bool rightTmp);
  virtual MatrixRep* powRep (MatrixComplexArr& left, bool rightTmp);

  virtual MatrixRep* posdiffRep (MatrixRealSca& left);
  virtual MatrixRep* posdiffRep (MatrixRealArr& left);

  virtual MatrixRep* tocomplexRep (MatrixRealSca& left);
  virtual MatrixRep* tocomplexRep (MatrixRealArr& left);

  virtual MatrixRep* minRep (MatrixRealSca& left);
  virtual MatrixRep* minRep (MatrixRealArr& left);

  virtual MatrixRep* maxRep (MatrixRealSca& left);
  virtual MatrixRep* maxRep (MatrixRealArr& left);

  virtual MatrixRep* atan2Rep (MatrixRealSca& left);
  virtual MatrixRep* atan2Rep (MatrixRealArr& left);

  virtual MatrixRep* negate();

  virtual MatrixRep* abs();
  virtual MatrixRep* sin();
  virtual MatrixRep* cos();
  virtual MatrixRep* asin();
  virtual MatrixRep* acos();
  virtual MatrixRep* log();
  virtual MatrixRep* exp();
  virtual MatrixRep* log10();
  virtual MatrixRep* pow10();
  virtual MatrixRep* sqr();
  virtual MatrixRep* sqrt();
  virtual MatrixRep* conj();
  virtual MatrixRep* min();
  virtual MatrixRep* max();
  virtual MatrixRep* mean();
  virtual MatrixRep* sum();

  double* itsValue;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

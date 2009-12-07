//# MatrixRep.h: Temporary matrix for Mns
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

#ifndef LOFAR_BBSKERNEL_EXPR_MATRIXREP_H
#define LOFAR_BBSKERNEL_EXPR_MATRIXREP_H

// \file
// Temporary matrix for Mns

//# Includes
#include <Common/lofar_complex.h>
//#include <Common/lofar_iostream.h>
#include <iostream>
#include <casa/iosfwd.h>

#if defined _OPENMP
#include <omp.h>
#endif

namespace LOFAR
{
namespace BBS
{
using LOFAR::dcomplex;

// \addtogroup Expr
// @{

//# Forward Declarations
class MatrixRealSca;
class MatrixComplexSca;
class MatrixRealArr;
class MatrixComplexArr;

class MatrixRep
{
public:
  enum type {
    RealScalar     = 0,  // don't change; they may be interpreted as bitfields
    RealArray      = 1,
    ComplexScalar  = 2,
    ComplexArray   = 3
  };

  MatrixRep (int nx, int ny, enum type type)
    : type(type),
      itsCount     (0),
      itsNx        (nx),
      itsNy        (ny),
      itsLength    (nx*ny)
    {
    }

  virtual ~MatrixRep();

  virtual MatrixRep* clone() const = 0;

  MatrixRep* link()
    {
      itsCount++;
      return this;
    }

  static void unlink (MatrixRep* rep)
    { if (rep != 0 && -- rep->itsCount == 0) delete rep; }

  int nx() const
    { return itsNx; }

  int ny() const
    { return itsNy; }

  int nelements() const
    { return itsLength; }

  virtual void show (std::ostream& os) const = 0;

  bool isComplex() const
    { return (type & 2) != 0; }

  bool isArray() const
    { return (type & 1) != 0; }

  virtual const double* doubleStorage() const;
  virtual void dcomplexStorage(const double *&realPtr, const double *&imagPtr) const;
  virtual double getDouble (int x, int y) const = 0;
  virtual dcomplex getDComplex (int x, int y) const = 0;

  virtual MatrixRep* add      (MatrixRep& right, bool rightTmp) = 0;
  virtual MatrixRep* subtract (MatrixRep& right, bool rightTmp) = 0;
  virtual MatrixRep* multiply (MatrixRep& right, bool rightTmp) = 0;
  virtual MatrixRep* divide   (MatrixRep& right, bool rightTmp) = 0;
  virtual MatrixRep* posdiff  (MatrixRep& right);
  virtual MatrixRep* tocomplex(MatrixRep& right);
  virtual MatrixRep* min      (MatrixRep& right);
  virtual MatrixRep* max      (MatrixRep& right);

  virtual MatrixRep* addRep (MatrixRealSca& left, bool rightTmp) = 0;
  virtual MatrixRep* addRep (MatrixRealArr& left, bool rightTmp) = 0;
  virtual MatrixRep* addRep (MatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MatrixRep* addRep (MatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MatrixRep* subRep (MatrixRealSca& left, bool rightTmp) = 0;
  virtual MatrixRep* subRep (MatrixRealArr& left, bool rightTmp) = 0;
  virtual MatrixRep* subRep (MatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MatrixRep* subRep (MatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MatrixRep* mulRep (MatrixRealSca& left, bool rightTmp) = 0;
  virtual MatrixRep* mulRep (MatrixRealArr& left, bool rightTmp) = 0;
  virtual MatrixRep* mulRep (MatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MatrixRep* mulRep (MatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MatrixRep* divRep (MatrixRealSca& left, bool rightTmp) = 0;
  virtual MatrixRep* divRep (MatrixRealArr& left, bool rightTmp) = 0;
  virtual MatrixRep* divRep (MatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MatrixRep* divRep (MatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MatrixRep* posdiffRep (MatrixRealSca& left);
  virtual MatrixRep* posdiffRep (MatrixRealArr& left);

  virtual MatrixRep* tocomplexRep (MatrixRealSca& left);
  virtual MatrixRep* tocomplexRep (MatrixRealArr& left);

  virtual MatrixRep* minRep (MatrixRealSca& left);
  virtual MatrixRep* minRep (MatrixRealArr& left);

  virtual MatrixRep* maxRep (MatrixRealSca& left);
  virtual MatrixRep* maxRep (MatrixRealArr& left);

  virtual MatrixRep* negate() = 0;

  virtual MatrixRep* abs() = 0;
  virtual MatrixRep* sin() = 0;
  virtual MatrixRep* cos() = 0;
  virtual MatrixRep* log() = 0;
  virtual MatrixRep* exp() = 0;
  virtual MatrixRep* log10() = 0;
  virtual MatrixRep* pow10() = 0;
  virtual MatrixRep* sqr() = 0;
  virtual MatrixRep* sqrt() = 0;
  virtual MatrixRep* conj() = 0;
  virtual MatrixRep* min() = 0;
  virtual MatrixRep* max() = 0;
  virtual MatrixRep* mean() = 0;
  virtual MatrixRep* sum() = 0;

  virtual void fillRowWithProducts(dcomplex v0, dcomplex factor, int row);

  const enum type type;

protected:
  inline int offset (int x, int y) const
    { return y*itsNx + x; }

private:
  int  itsCount;
  int  itsNx;
  int  itsNy;
  int  itsLength;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

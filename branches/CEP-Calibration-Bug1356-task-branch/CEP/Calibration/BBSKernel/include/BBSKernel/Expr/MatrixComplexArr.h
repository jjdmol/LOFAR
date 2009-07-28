//# MatrixComplexArr.h: Temporary matrix for Mns
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

#if !defined(EXPR_MATRIXCOMPLEXARR_H)
#define EXPR_MATRIXCOMPLEXARR_H

// \file
// Temporary matrix for Mns

//# Includes
#include <BBSKernel/Expr/MatrixRep.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_stack.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class MatrixComplexArr : public MatrixRep
{
friend class MatrixRealSca;
friend class MatrixRealArr;
friend class MatrixComplexSca;

private:
  MatrixComplexArr(int nx, int ny); // use "allocate" instead

public:
  static size_t clone_count;

  virtual ~MatrixComplexArr();

  virtual MatrixRep* clone() const;

  void set (dcomplex value);

  void set (int x, int y, dcomplex value)
    { int off = offset(x,y);
      itsReal[off] = real(value);
      itsImag[off] = imag(value);
    }

  void set (const dcomplex* values)
    {
      for (int i = 0; i < nelements(); i ++) {
	itsReal[i] = real(values[i]);
	itsImag[i] = imag(values[i]);
      }
    }

  virtual void show (ostream& os) const;

  void *operator new(size_t, int nx, int ny);
  void operator delete(void *);

  inline static MatrixComplexArr* allocate(int nx, int ny) {
    return new (nx, ny) MatrixComplexArr(nx, ny);
  }

//  static void poolActivate(int nelements);
//  static void poolDeactivate();

  virtual MatrixRep* add      (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* subtract (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* multiply (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* divide   (MatrixRep& right, bool rightTmp);

  // Put here to avoid incorrect g++ warning about these function being hidden
  // by the functions of the same name without arguments (i.e. min() and max()).
  // @{
  virtual MatrixRep* min      (MatrixRep& right);
  virtual MatrixRep* max      (MatrixRep& right);
  // @}

  virtual void dcomplexStorage(const double *&realPtr, const double *&imagPtr) const;
  virtual double getDouble (int x, int y) const;
  virtual dcomplex getDComplex (int x, int y) const;

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

  virtual MatrixRep* negate();

  virtual MatrixRep* abs();
  virtual MatrixRep* sin();
  virtual MatrixRep* cos();
  virtual MatrixRep* log();
  virtual MatrixRep* exp();
  virtual MatrixRep* sqr();
  virtual MatrixRep* sqrt();
  virtual MatrixRep* conj();
  virtual MatrixRep* min();
  virtual MatrixRep* max();
  virtual MatrixRep* mean();
  virtual MatrixRep* sum();

  virtual void fillRowWithProducts(dcomplex v0, dcomplex factor, int row);

  double *itsReal, *itsImag;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

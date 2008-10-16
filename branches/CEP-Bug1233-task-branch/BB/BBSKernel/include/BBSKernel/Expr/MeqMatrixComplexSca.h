//# MatrixComplexSca.h: Temporary matrix for Mns
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

#if !defined(MNS_MEQMATRIXCOMPLEXSCA_H)
#define MNS_MEQMATRIXCOMPLEXSCA_H

// \file
// Temporary matrix for Mns

//# Includes
#include <BBSKernel/MNS/MeqMatrixRep.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup MNS
// @{

class MatrixComplexSca : public MatrixRep
{
friend class MatrixRealSca;
friend class MatrixRealArr;
friend class MatrixComplexArr;

public:
  MatrixComplexSca (dcomplex value)
    : MatrixRep(1,1,ComplexScalar), itsValue (value) {}

  virtual ~MatrixComplexSca();

  virtual MatrixRep* clone() const;

  virtual void show (ostream& os) const;

  virtual MatrixRep* add      (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* subtract (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* multiply (MatrixRep& right, bool rightTmp);
  virtual MatrixRep* divide   (MatrixRep& right, bool rightTmp);

  virtual void dcomplexStorage(const double *&realPtr, const double *&imagPtr) const;
  virtual double getDouble (int x, int y) const;
  virtual dcomplex getDComplex (int x, int y) const;

private:
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

  virtual MatrixRep* sin();
  virtual MatrixRep* cos();
  virtual MatrixRep* exp();
  virtual MatrixRep* sqr();
  virtual MatrixRep* sqrt();
  virtual MatrixRep* conj();
  virtual MatrixRep* min();
  virtual MatrixRep* max();
  virtual MatrixRep* mean();
  virtual MatrixRep* sum();


  dcomplex itsValue;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif

//# MnsMatrixComplexArr.h: Temporary matrix for Mns
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

#if !defined(MNS_MNSMATRIXCOMPLEXARR_H)
#define MNS_MNSMATRIXCOMPLEXARR_H


//# Includes
#include <MNS/MnsMatrixRep.h>
#include <Common/lofar_complex.h>


class MnsMatrixComplexArr : public MnsMatrixRep
{
friend class MnsMatrixRealSca;
friend class MnsMatrixRealArr;
friend class MnsMatrixComplexSca;

public:
  MnsMatrixComplexArr (int nx, int ny);

  virtual ~MnsMatrixComplexArr();

  virtual MnsMatrixRep* clone() const;

  void set (complex<double> value);

  void set (int x, int y, complex<double> value)
    { itsValue[offset(x,y)] = value; }

  void set (const complex<double>* values)
    { memcpy (itsValue, values, nelements() * sizeof(complex<double>)); }

  virtual void show (ostream& os) const;

  virtual MnsMatrixRep* add      (MnsMatrixRep& right, bool rightTmp);
  virtual MnsMatrixRep* subtract (MnsMatrixRep& right, bool rightTmp);
  virtual MnsMatrixRep* multiply (MnsMatrixRep& right, bool rightTmp);
  virtual MnsMatrixRep* divide   (MnsMatrixRep& right, bool rightTmp);

  virtual double getDouble (int x, int y) const;
  virtual complex<double> getDComplex (int x, int y) const;

private:
  virtual MnsMatrixRep* addRep (MnsMatrixRealSca& left, bool rightTmp);
  virtual MnsMatrixRep* addRep (MnsMatrixComplexSca& left, bool rightTmp);
  virtual MnsMatrixRep* addRep (MnsMatrixRealArr& left, bool rightTmp);
  virtual MnsMatrixRep* addRep (MnsMatrixComplexArr& left, bool rightTmp);

  virtual MnsMatrixRep* subRep (MnsMatrixRealSca& left, bool rightTmp);
  virtual MnsMatrixRep* subRep (MnsMatrixRealArr& left, bool rightTmp);
  virtual MnsMatrixRep* subRep (MnsMatrixComplexSca& left, bool rightTmp);
  virtual MnsMatrixRep* subRep (MnsMatrixComplexArr& left, bool rightTmp);

  virtual MnsMatrixRep* mulRep (MnsMatrixRealSca& left, bool rightTmp);
  virtual MnsMatrixRep* mulRep (MnsMatrixRealArr& left, bool rightTmp);
  virtual MnsMatrixRep* mulRep (MnsMatrixComplexSca& left, bool rightTmp);
  virtual MnsMatrixRep* mulRep (MnsMatrixComplexArr& left, bool rightTmp);

  virtual MnsMatrixRep* divRep (MnsMatrixRealSca& left, bool rightTmp);
  virtual MnsMatrixRep* divRep (MnsMatrixRealArr& left, bool rightTmp);
  virtual MnsMatrixRep* divRep (MnsMatrixComplexSca& left, bool rightTmp);
  virtual MnsMatrixRep* divRep (MnsMatrixComplexArr& left, bool rightTmp);

  virtual void negate();

  virtual void sin();
  virtual void cos();
  virtual void exp();
  virtual void conj();
  virtual MnsMatrixRep* min();
  virtual MnsMatrixRep* max();
  virtual MnsMatrixRep* mean();
  virtual MnsMatrixRep* sum();


  complex<double>* itsValue;
};


#endif

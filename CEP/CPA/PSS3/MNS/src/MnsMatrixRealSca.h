//# MnsMatrixRealSca.h: Temporary matrix for Mns
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

#if !defined(MNS_MNSMATRIXREALSCA_H)
#define MNS_MNSMATRIXREALSCA_H


//# Includes
#include <MNS/MnsMatrixRep.h>


class MnsMatrixRealSca : public MnsMatrixRep
{
friend class MnsMatrixRealArr;
friend class MnsMatrixComplexSca;
friend class MnsMatrixComplexArr;

public:
  MnsMatrixRealSca (double value)
    : MnsMatrixRep(1, 1, sizeof(double)), itsValue (value) {}

  virtual ~MnsMatrixRealSca();

  virtual MnsMatrixRep* clone() const;

  virtual void show (ostream& os) const;

  virtual MnsMatrixRep* add      (MnsMatrixRep& right, Bool rightTmp);
  virtual MnsMatrixRep* subtract (MnsMatrixRep& right, Bool rightTmp);
  virtual MnsMatrixRep* multiply (MnsMatrixRep& right, Bool rightTmp);
  virtual MnsMatrixRep* divide   (MnsMatrixRep& right, Bool rightTmp);

  virtual double getDouble (int x, int y) const;
  virtual complex<double> getDComplex (int x, int y) const;

private:
  virtual MnsMatrixRep* addRep (MnsMatrixRealSca& left, Bool rightTmp);
  virtual MnsMatrixRep* addRep (MnsMatrixComplexSca& left, Bool rightTmp);
  virtual MnsMatrixRep* addRep (MnsMatrixRealArr& left, Bool rightTmp);
  virtual MnsMatrixRep* addRep (MnsMatrixComplexArr& left, Bool rightTmp);

  virtual MnsMatrixRep* subRep (MnsMatrixRealSca& left, Bool rightTmp);
  virtual MnsMatrixRep* subRep (MnsMatrixRealArr& left, Bool rightTmp);
  virtual MnsMatrixRep* subRep (MnsMatrixComplexSca& left, Bool rightTmp);
  virtual MnsMatrixRep* subRep (MnsMatrixComplexArr& left, Bool rightTmp);

  virtual MnsMatrixRep* mulRep (MnsMatrixRealSca& left, Bool rightTmp);
  virtual MnsMatrixRep* mulRep (MnsMatrixRealArr& left, Bool rightTmp);
  virtual MnsMatrixRep* mulRep (MnsMatrixComplexSca& left, Bool rightTmp);
  virtual MnsMatrixRep* mulRep (MnsMatrixComplexArr& left, Bool rightTmp);

  virtual MnsMatrixRep* divRep (MnsMatrixRealSca& left, Bool rightTmp);
  virtual MnsMatrixRep* divRep (MnsMatrixRealArr& left, Bool rightTmp);
  virtual MnsMatrixRep* divRep (MnsMatrixComplexSca& left, Bool rightTmp);
  virtual MnsMatrixRep* divRep (MnsMatrixComplexArr& left, Bool rightTmp);

  virtual void negate();

  virtual void sin();
  virtual void cos();
  virtual void exp();
  virtual void conj();
  virtual MnsMatrixRep* min();
  virtual MnsMatrixRep* max();
  virtual MnsMatrixRep* mean();
  virtual MnsMatrixRep* sum();


  double itsValue;
};


#endif

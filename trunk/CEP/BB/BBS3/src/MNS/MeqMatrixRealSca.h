//# MeqMatrixRealSca.h: Temporary matrix for Mns
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

#if !defined(MNS_MEQMATRIXREALSCA_H)
#define MNS_MEQMATRIXREALSCA_H

// \file MNS/MeqMatrixRealSca.h
// Temporary matrix for Mns

//# Includes
#include <BBS3/MNS/MeqMatrixRep.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

class MeqMatrixRealSca : public MeqMatrixRep
{
friend class MeqMatrixRealArr;
friend class MeqMatrixComplexSca;
friend class MeqMatrixComplexArr;

public:
  MeqMatrixRealSca (double value)
    : MeqMatrixRep(1, 1, sizeof(double)), itsValue (value) {}

  virtual ~MeqMatrixRealSca();

  virtual MeqMatrixRep* clone() const;

  virtual void show (ostream& os) const;

  virtual bool isDouble() const;
  virtual const double* doubleStorage() const;
  virtual double getDouble (int x, int y) const;
  virtual dcomplex getDComplex (int x, int y) const;

  virtual MeqMatrixRep* add      (MeqMatrixRep& right, bool rightTmp);
  virtual MeqMatrixRep* subtract (MeqMatrixRep& right, bool rightTmp);
  virtual MeqMatrixRep* multiply (MeqMatrixRep& right, bool rightTmp);
  virtual MeqMatrixRep* divide   (MeqMatrixRep& right, bool rightTmp);
  virtual MeqMatrixRep* posdiff  (MeqMatrixRep& right);
  virtual MeqMatrixRep* tocomplex(MeqMatrixRep& right);


private:
  virtual MeqMatrixRep* addRep (MeqMatrixRealSca& left, bool rightTmp);
  virtual MeqMatrixRep* addRep (MeqMatrixComplexSca& left, bool rightTmp);
  virtual MeqMatrixRep* addRep (MeqMatrixRealArr& left, bool rightTmp);
  virtual MeqMatrixRep* addRep (MeqMatrixComplexArr& left, bool rightTmp);

  virtual MeqMatrixRep* subRep (MeqMatrixRealSca& left, bool rightTmp);
  virtual MeqMatrixRep* subRep (MeqMatrixRealArr& left, bool rightTmp);
  virtual MeqMatrixRep* subRep (MeqMatrixComplexSca& left, bool rightTmp);
  virtual MeqMatrixRep* subRep (MeqMatrixComplexArr& left, bool rightTmp);

  virtual MeqMatrixRep* mulRep (MeqMatrixRealSca& left, bool rightTmp);
  virtual MeqMatrixRep* mulRep (MeqMatrixRealArr& left, bool rightTmp);
  virtual MeqMatrixRep* mulRep (MeqMatrixComplexSca& left, bool rightTmp);
  virtual MeqMatrixRep* mulRep (MeqMatrixComplexArr& left, bool rightTmp);

  virtual MeqMatrixRep* divRep (MeqMatrixRealSca& left, bool rightTmp);
  virtual MeqMatrixRep* divRep (MeqMatrixRealArr& left, bool rightTmp);
  virtual MeqMatrixRep* divRep (MeqMatrixComplexSca& left, bool rightTmp);
  virtual MeqMatrixRep* divRep (MeqMatrixComplexArr& left, bool rightTmp);

  virtual MeqMatrixRep* posdiffRep (MeqMatrixRealSca& left);
  virtual MeqMatrixRep* posdiffRep (MeqMatrixRealArr& left);

  virtual MeqMatrixRep* tocomplexRep (MeqMatrixRealSca& left);
  virtual MeqMatrixRep* tocomplexRep (MeqMatrixRealArr& left);

  virtual MeqMatrixRep* negate();

  virtual MeqMatrixRep* sin();
  virtual MeqMatrixRep* cos();
  virtual MeqMatrixRep* exp();
  virtual MeqMatrixRep* sqr();
  virtual MeqMatrixRep* sqrt();
  virtual MeqMatrixRep* conj();
  virtual MeqMatrixRep* min();
  virtual MeqMatrixRep* max();
  virtual MeqMatrixRep* mean();
  virtual MeqMatrixRep* sum();


  double itsValue;
};

// @}

}

#endif

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


//# Includes
#include <PSS3/MNS/MeqMatrixRep.h>


namespace LOFAR {

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
  virtual complex<double> getDComplex (int x, int y) const;

  virtual MeqMatrixRep* add      (MeqMatrixRep& right, Bool rightTmp);
  virtual MeqMatrixRep* subtract (MeqMatrixRep& right, Bool rightTmp);
  virtual MeqMatrixRep* multiply (MeqMatrixRep& right, Bool rightTmp);
  virtual MeqMatrixRep* divide   (MeqMatrixRep& right, Bool rightTmp);
  virtual MeqMatrixRep* posdiff  (MeqMatrixRep& right);
  virtual MeqMatrixRep* tocomplex(MeqMatrixRep& right);


private:
  virtual MeqMatrixRep* addRep (MeqMatrixRealSca& left, Bool rightTmp);
  virtual MeqMatrixRep* addRep (MeqMatrixComplexSca& left, Bool rightTmp);
  virtual MeqMatrixRep* addRep (MeqMatrixRealArr& left, Bool rightTmp);
  virtual MeqMatrixRep* addRep (MeqMatrixComplexArr& left, Bool rightTmp);

  virtual MeqMatrixRep* subRep (MeqMatrixRealSca& left, Bool rightTmp);
  virtual MeqMatrixRep* subRep (MeqMatrixRealArr& left, Bool rightTmp);
  virtual MeqMatrixRep* subRep (MeqMatrixComplexSca& left, Bool rightTmp);
  virtual MeqMatrixRep* subRep (MeqMatrixComplexArr& left, Bool rightTmp);

  virtual MeqMatrixRep* mulRep (MeqMatrixRealSca& left, Bool rightTmp);
  virtual MeqMatrixRep* mulRep (MeqMatrixRealArr& left, Bool rightTmp);
  virtual MeqMatrixRep* mulRep (MeqMatrixComplexSca& left, Bool rightTmp);
  virtual MeqMatrixRep* mulRep (MeqMatrixComplexArr& left, Bool rightTmp);

  virtual MeqMatrixRep* divRep (MeqMatrixRealSca& left, Bool rightTmp);
  virtual MeqMatrixRep* divRep (MeqMatrixRealArr& left, Bool rightTmp);
  virtual MeqMatrixRep* divRep (MeqMatrixComplexSca& left, Bool rightTmp);
  virtual MeqMatrixRep* divRep (MeqMatrixComplexArr& left, Bool rightTmp);

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

}

#endif

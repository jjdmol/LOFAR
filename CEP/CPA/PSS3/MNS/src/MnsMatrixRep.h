//# MnsMatrixRep.h: Temporary matrix for Mns
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

#if !defined(MNS_MNSMATRIXREP_H)
#define MNS_MNSMATRIXREP_H


//# Includes
#include <Common/lofar_complex.h>

//# Forward Declarations
class MnsMatrixRealSca;
class MnsMatrixComplexSca;
class MnsMatrixRealArr;
class MnsMatrixComplexArr;
#include <aips/iosfwd.h>


class MnsMatrixRep
{
public:
  MnsMatrixRep (int nx, int ny, int elemLength)
    : itsCount(0), itsNx(nx), itsNy(ny), itsLength(nx*ny),
      itsElemLength(elemLength) {}

  virtual ~MnsMatrixRep();

  virtual MnsMatrixRep* clone() const = 0;

  MnsMatrixRep* link()
    { itsCount++; return this; }

  static void unlink (MnsMatrixRep*);

  int nx() const
    { return itsNx; }

  int ny() const
    { return itsNy; }

  int nelements() const
    { return itsLength; }

  int elemLength() const
    { return itsElemLength; }

  virtual void show (ostream& os) const = 0;

  virtual MnsMatrixRep* add      (MnsMatrixRep& right, Bool rightTmp) = 0;
  virtual MnsMatrixRep* subtract (MnsMatrixRep& right, Bool rightTmp) = 0;
  virtual MnsMatrixRep* multiply (MnsMatrixRep& right, Bool rightTmp) = 0;
  virtual MnsMatrixRep* divide   (MnsMatrixRep& right, Bool rightTmp) = 0;

  virtual const double* doubleStorage() const;
  virtual double getDouble (int x, int y) const = 0;
  virtual complex<double> getDComplex (int x, int y) const = 0;

  virtual MnsMatrixRep* addRep (MnsMatrixRealSca& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* addRep (MnsMatrixRealArr& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* addRep (MnsMatrixComplexSca& left,
				Bool rightTmp) = 0;
  virtual MnsMatrixRep* addRep (MnsMatrixComplexArr& left,
				Bool rightTmp) = 0;

  virtual MnsMatrixRep* subRep (MnsMatrixRealSca& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* subRep (MnsMatrixRealArr& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* subRep (MnsMatrixComplexSca& left,
				Bool rightTmp) = 0;
  virtual MnsMatrixRep* subRep (MnsMatrixComplexArr& left,
				Bool rightTmp) = 0;

  virtual MnsMatrixRep* mulRep (MnsMatrixRealSca& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* mulRep (MnsMatrixRealArr& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* mulRep (MnsMatrixComplexSca& left,
				Bool rightTmp) = 0;
  virtual MnsMatrixRep* mulRep (MnsMatrixComplexArr& left,
				Bool rightTmp) = 0;

  virtual MnsMatrixRep* divRep (MnsMatrixRealSca& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* divRep (MnsMatrixRealArr& left, Bool rightTmp) = 0;
  virtual MnsMatrixRep* divRep (MnsMatrixComplexSca& left,
				Bool rightTmp) = 0;
  virtual MnsMatrixRep* divRep (MnsMatrixComplexArr& left,
				Bool rightTmp) = 0;

  virtual void negate() = 0;

  virtual void sin() = 0;
  virtual void cos() = 0;
  virtual void exp() = 0;
  virtual void conj() = 0;
  virtual MnsMatrixRep* min() = 0;
  virtual MnsMatrixRep* max() = 0;
  virtual MnsMatrixRep* mean() = 0;
  virtual MnsMatrixRep* sum() = 0;

protected:
  int offset (int x, int y) const
    { return y*itsNx + x; }

private:
  int itsCount;
  int itsNx;
  int itsNy;
  int itsLength;
  int itsElemLength;
};


#endif

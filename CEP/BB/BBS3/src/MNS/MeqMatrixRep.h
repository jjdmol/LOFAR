//# MeqMatrixRep.h: Temporary matrix for Mns
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

#if !defined(MNS_MEQMATRIXREP_H)
#define MNS_MEQMATRIXREP_H

// \file MNS/MeqMatrixRep.h
// Temporary matrix for Mns

//# Includes
#include <Common/lofar_complex.h>
#include <Common/lofar_iostream.h>
#include <casa/iosfwd.h>

#if defined _OPENMP
#include <omp.h>
#endif

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward Declarations
class MeqMatrixRealSca;
class MeqMatrixComplexSca;
class MeqMatrixRealArr;
class MeqMatrixComplexArr;


class MeqMatrixRep
{
public:
  enum type {
    RealScalar     = 0,  // don't change; they may be interpreted as bitfields
    RealArray      = 1,
    ComplexScalar  = 2,
    ComplexArray   = 3
  };

  MeqMatrixRep (int nx, int ny, enum type type)
    : type(type),
      itsCount     (0),
      itsNx        (nx),
      itsNy        (ny),
      itsLength    (nx*ny)
    {
      nctor++;
    }

  virtual ~MeqMatrixRep();

  virtual MeqMatrixRep* clone() const = 0;

  MeqMatrixRep* link()
    {
      itsCount++;
      return this;
    }

  static void unlink (MeqMatrixRep* rep)
    { if (rep != 0 && -- rep->itsCount == 0) delete rep; }

  int nx() const
    { return itsNx; }

  int ny() const
    { return itsNy; }

  int nelements() const
    { return itsLength; }

  virtual void show (ostream& os) const = 0;

  bool isComplex() const
    { return (type & 2) != 0; }

  bool isArray() const
    { return (type & 1) != 0; }

  virtual const double* doubleStorage() const;
  virtual void dcomplexStorage(const double *&realPtr, const double *&imagPtr) const;
  virtual double getDouble (int x, int y) const = 0;
  virtual dcomplex getDComplex (int x, int y) const = 0;

  virtual MeqMatrixRep* add      (MeqMatrixRep& right, bool rightTmp) = 0;
  virtual MeqMatrixRep* subtract (MeqMatrixRep& right, bool rightTmp) = 0;
  virtual MeqMatrixRep* multiply (MeqMatrixRep& right, bool rightTmp) = 0;
  virtual MeqMatrixRep* divide   (MeqMatrixRep& right, bool rightTmp) = 0;
  virtual MeqMatrixRep* posdiff  (MeqMatrixRep& right);
  virtual MeqMatrixRep* tocomplex(MeqMatrixRep& right);

  virtual MeqMatrixRep* addRep (MeqMatrixRealSca& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* addRep (MeqMatrixRealArr& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* addRep (MeqMatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MeqMatrixRep* addRep (MeqMatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MeqMatrixRep* subRep (MeqMatrixRealSca& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* subRep (MeqMatrixRealArr& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* subRep (MeqMatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MeqMatrixRep* subRep (MeqMatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MeqMatrixRep* mulRep (MeqMatrixRealSca& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* mulRep (MeqMatrixRealArr& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* mulRep (MeqMatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MeqMatrixRep* mulRep (MeqMatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MeqMatrixRep* divRep (MeqMatrixRealSca& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* divRep (MeqMatrixRealArr& left, bool rightTmp) = 0;
  virtual MeqMatrixRep* divRep (MeqMatrixComplexSca& left,
				bool rightTmp) = 0;
  virtual MeqMatrixRep* divRep (MeqMatrixComplexArr& left,
				bool rightTmp) = 0;

  virtual MeqMatrixRep* posdiffRep (MeqMatrixRealSca& left);
  virtual MeqMatrixRep* posdiffRep (MeqMatrixRealArr& left);

  virtual MeqMatrixRep* tocomplexRep (MeqMatrixRealSca& left);
  virtual MeqMatrixRep* tocomplexRep (MeqMatrixRealArr& left);

  virtual MeqMatrixRep* negate() = 0;

  virtual MeqMatrixRep* sin() = 0;
  virtual MeqMatrixRep* cos() = 0;
  virtual MeqMatrixRep* exp() = 0;
  virtual MeqMatrixRep* sqr() = 0;
  virtual MeqMatrixRep* sqrt() = 0;
  virtual MeqMatrixRep* conj() = 0;
  virtual MeqMatrixRep* min() = 0;
  virtual MeqMatrixRep* max() = 0;
  virtual MeqMatrixRep* mean() = 0;
  virtual MeqMatrixRep* sum() = 0;

  virtual void fillWithProducts(dcomplex v0, dcomplex factor);

  static int nctor;
  static int ndtor;
  static int nreused;
  static int ndeleted;

  const enum type type;

protected:
  inline int offset (int x, int y) const
    { return y*itsNx + x; }

private:
  int  itsCount;
  int  itsNx;
  int  itsNy;
  int  itsLength;
  int  itsCreator;
};

// @}

}

#endif

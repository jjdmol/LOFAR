//# VellsRep.h: Temporary matrix for Mns
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

#ifndef MEQ_VELLSREP_H
#define MEQ_VELLSREP_H


//# Includes
#include <Common/lofar_complex.h>
#include <iostream>

namespace MEQ {

//# Forward Declarations
class VellsRealSca;
class VellsComplexSca;
class VellsRealArr;
class VellsComplexArr;


class VellsRep
{
public:
  VellsRep (int nx, int ny)
    : itsCount(0), itsNx(nx), itsNy(ny), itsLength(nx*ny)
    { nctor++; }

  virtual ~VellsRep();

  virtual VellsRep* clone() const = 0;

  VellsRep* link()
  { itsCount++; return this; }

  static void unlink (VellsRep* rep)
    {
      if (rep != 0  &&  --rep->itsCount == 0) {
	delete rep; 
      }
    }

  int nx() const
    { return itsNx; }

  int ny() const
    { return itsNy; }

  int nelements() const
    { return itsLength; }

  virtual void show (std::ostream& os) const = 0;

  virtual bool isReal() const;
  virtual double* realStorage();
  virtual const double* realStorage() const
    { return const_cast<VellsRep*>(this)->realStorage(); }
  virtual complex<double>* complexStorage();
  const complex<double>* complexStorage() const
    { return const_cast<VellsRep*>(this)->complexStorage(); }

  virtual VellsRep* add      (VellsRep& right, bool rightTmp) = 0;
  virtual VellsRep* subtract (VellsRep& right, bool rightTmp) = 0;
  virtual VellsRep* multiply (VellsRep& right, bool rightTmp) = 0;
  virtual VellsRep* divide   (VellsRep& right, bool rightTmp) = 0;
  virtual VellsRep* posdiff  (VellsRep& right);
  virtual VellsRep* tocomplex(VellsRep& right);

  virtual VellsRep* addRep (VellsRealSca& left, bool rightTmp) = 0;
  virtual VellsRep* addRep (VellsRealArr& left, bool rightTmp) = 0;
  virtual VellsRep* addRep (VellsComplexSca& left, bool rightTmp) = 0;
  virtual VellsRep* addRep (VellsComplexArr& left, bool rightTmp) = 0;

  virtual VellsRep* subRep (VellsRealSca& left, bool rightTmp) = 0;
  virtual VellsRep* subRep (VellsRealArr& left, bool rightTmp) = 0;
  virtual VellsRep* subRep (VellsComplexSca& left, bool rightTmp) = 0;
  virtual VellsRep* subRep (VellsComplexArr& left, bool rightTmp) = 0;

  virtual VellsRep* mulRep (VellsRealSca& left, bool rightTmp) = 0;
  virtual VellsRep* mulRep (VellsRealArr& left, bool rightTmp) = 0;
  virtual VellsRep* mulRep (VellsComplexSca& left, bool rightTmp) = 0;
  virtual VellsRep* mulRep (VellsComplexArr& left, bool rightTmp) = 0;

  virtual VellsRep* divRep (VellsRealSca& left, bool rightTmp) = 0;
  virtual VellsRep* divRep (VellsRealArr& left, bool rightTmp) = 0;
  virtual VellsRep* divRep (VellsComplexSca& left, bool rightTmp) = 0;
  virtual VellsRep* divRep (VellsComplexArr& left, bool rightTmp) = 0;

  virtual VellsRep* posdiffRep (VellsRealSca& left);
  virtual VellsRep* posdiffRep (VellsRealArr& left);

  virtual VellsRep* tocomplexRep (VellsRealSca& left);
  virtual VellsRep* tocomplexRep (VellsRealArr& left);

  virtual VellsRep* negate() = 0;

  virtual VellsRep* sin() = 0;
  virtual VellsRep* cos() = 0;
  virtual VellsRep* exp() = 0;
  virtual VellsRep* sqr() = 0;
  virtual VellsRep* sqrt() = 0;
  virtual VellsRep* conj() = 0;
  virtual VellsRep* min() = 0;
  virtual VellsRep* max() = 0;
  virtual VellsRep* mean() = 0;
  virtual VellsRep* sum() = 0;

  static int nctor;
  static int ndtor;

protected:
  inline int offset (int x, int y) const
    { return y*itsNx + x; }

private:
  int  itsCount;
  int  itsNx;
  int  itsNy;
  int  itsLength;
};


} // namespace MEQ

#endif

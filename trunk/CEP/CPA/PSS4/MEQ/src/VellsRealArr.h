//# VellsRealArr.h: Temporary vells for Mns
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

#ifndef MEQ_VELLSREALARR_H
#define MEQ_VELLSREALARR_H


//# Includes
#include <MEQ/VellsRep.h>

namespace MEQ {

class VellsRealArr : public VellsRep
{
friend class VellsRealSca;
friend class VellsComplexSca;
friend class VellsComplexArr;

public:
  VellsRealArr (int nx, int ny);

  VellsRealArr (double*, int nx, int ny);

  virtual ~VellsRealArr();

  virtual VellsRep* clone() const;

  void set (double value);

  virtual void show (std::ostream& os) const;

  virtual bool isReal() const;
  virtual double* realStorage();

  virtual VellsRep* add      (VellsRep& right, bool rightTmp);
  virtual VellsRep* subtract (VellsRep& right, bool rightTmp);
  virtual VellsRep* multiply (VellsRep& right, bool rightTmp);
  virtual VellsRep* divide   (VellsRep& right, bool rightTmp);
  virtual VellsRep* posdiff  (VellsRep& right);
  virtual VellsRep* tocomplex(VellsRep& right);


private:
  virtual VellsRep* addRep (VellsRealSca& left, bool rightTmp);
  virtual VellsRep* addRep (VellsComplexSca& left, bool rightTmp);
  virtual VellsRep* addRep (VellsRealArr& left, bool rightTmp);
  virtual VellsRep* addRep (VellsComplexArr& left, bool rightTmp);

  virtual VellsRep* subRep (VellsRealSca& left, bool rightTmp);
  virtual VellsRep* subRep (VellsRealArr& left, bool rightTmp);
  virtual VellsRep* subRep (VellsComplexSca& left, bool rightTmp);
  virtual VellsRep* subRep (VellsComplexArr& left, bool rightTmp);

  virtual VellsRep* mulRep (VellsRealSca& left, bool rightTmp);
  virtual VellsRep* mulRep (VellsRealArr& left, bool rightTmp);
  virtual VellsRep* mulRep (VellsComplexSca& left, bool rightTmp);
  virtual VellsRep* mulRep (VellsComplexArr& left, bool rightTmp);

  virtual VellsRep* divRep (VellsRealSca& left, bool rightTmp);
  virtual VellsRep* divRep (VellsRealArr& left, bool rightTmp);
  virtual VellsRep* divRep (VellsComplexSca& left, bool rightTmp);
  virtual VellsRep* divRep (VellsComplexArr& left, bool rightTmp);

  virtual VellsRep* posdiffRep (VellsRealSca& left);
  virtual VellsRep* posdiffRep (VellsRealArr& left);

  virtual VellsRep* tocomplexRep (VellsRealSca& left);
  virtual VellsRep* tocomplexRep (VellsRealArr& left);

  virtual VellsRep* negate();

  virtual VellsRep* sin();
  virtual VellsRep* cos();
  virtual VellsRep* exp();
  virtual VellsRep* sqr();
  virtual VellsRep* sqrt();
  virtual VellsRep* conj();
  virtual VellsRep* min();
  virtual VellsRep* max();
  virtual VellsRep* mean();
  virtual VellsRep* sum();


  double* itsValuePtr;
  bool    itsIsOwner;
};

} // namespace MEQ



#endif

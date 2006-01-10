//# ParmValue.cc: Value of a parm for a given domain
//#
//# Copyright (C) 2005
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

#include <lofar_config.h>
#include <ParmDB/ParmValue.h>

namespace LOFAR {
namespace ParmDB {

  ParmValueRep::ParmValueRep()
  : 
    itsType         ("polc"),
    itsPerturbation (1e-6),
    itsIsRelPert    (true),
    itsWeight       (1),
    itsID           (0),
    itsParentID     (0),
    itsDBTabRef     (-1),
    itsDBRowRef     (0),
    itsCount        (1)
  {}

  void ParmValueRep::setCoeff (const double* coeff,
			       const std::vector<int>& shape)
  {
    itsShape = shape;
    int nr = getLength(shape);
    itsCoeff.resize (nr);
    itsSolvMask.resize (nr);
    for (int i=0; i<nr; i++) {
      itsCoeff[i] = coeff[i];
      itsSolvMask[i] = true;
    }
  }

  void ParmValueRep::setCoeff (const double* coeff, const bool* solvableMask,
			       const std::vector<int>& shape)
  {
    itsShape = shape;
    int nr = getLength(shape);
    itsCoeff.resize (nr);
    itsSolvMask.resize (nr);
    for (int i=0; i<nr; i++) {
      itsCoeff[i] = coeff[i];
      itsSolvMask[i] = solvableMask[i];
    }
  }

  void ParmValueRep::setDomain (const ParmDomain& domain)
  {
    itsDomain = domain;
    const std::vector<double>& start = domain.getStart();
    const std::vector<double>& end = domain.getEnd();
    itsOffset.resize (start.size());
    itsScale.resize (start.size());
    for (uint i=0; i<start.size(); ++i) {
      itsOffset[i] = start[i];
      itsScale[i]  = end[i] - start[i];
    }
  }

  int ParmValueRep::getLength (const std::vector<int>& shape) const
  {
    int sz = 1;
    for (uint i=0; i<shape.size(); ++i) {
      sz *= shape[i];
    }
    return sz;
  }
  std::vector<int> ParmValueRep::makeShape (int nx, int ny)
  {
    std::vector<int> vec(2);
    vec[0] = nx;
    vec[1] = ny;
    return vec;
  }
  std::vector<double> ParmValueRep::makeVecDouble (double x, double y)
  {
    std::vector<double> vec(2);
    vec[0] = x;
    vec[1] = y;
    return vec;
  }



  ParmValue::ParmValue()
  : itsRep (new ParmValueRep())
  {}

  ParmValue::ParmValue (const ParmValue& that)
  : itsRep (that.itsRep->link())
  {}

  ParmValue& ParmValue::operator= (const ParmValue& that)
  {
    if (this != &that) {
      ParmValueRep::unlink (itsRep);
      itsRep = that.itsRep->link();
    }
    return *this;
  }

  ParmValue::~ParmValue()
  {
    ParmValueRep::unlink (itsRep);
  }



  ParmValueSet::ParmValueSet (const std::string& parmName)
  : itsRep (new ParmValueSetRep(parmName))
  {}

  ParmValueSet::ParmValueSet (const ParmValueSet& that)
  : itsRep (that.itsRep->link())
  {}

  ParmValueSet& ParmValueSet::operator= (const ParmValueSet& that)
  {
    if (this != &that) {
      ParmValueSetRep::unlink (itsRep);
      itsRep = that.itsRep->link();
    }
    return *this;
  }

  ParmValueSet::~ParmValueSet()
  {
    ParmValueSetRep::unlink (itsRep);
  }


} // namespace ParmDB
} // namespace LOFAR

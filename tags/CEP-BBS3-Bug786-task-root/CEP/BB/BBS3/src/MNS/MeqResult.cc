//# MeqResult.cc: The result of an expression for a domain.
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

#include <lofar_config.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <BBS3/MNS/Pool.h>

namespace LOFAR {

Pool<MeqResultRep> MeqResultRep::theirPool;
#pragma omp threadprivate(MeqResultRep::theirPool)

void *MeqResultRep::operator new(size_t size)
{
  return theirPool.allocate();
}

void MeqResultRep::operator delete(void *rep)
{
  theirPool.deallocate((MeqResultRep *) rep);
}


MeqResultRep::MeqResultRep (int nspid)
: itsCount           (0),
  itsDefPert         (0.),
  itsNspid	     (nspid),
  itsPerturbedValues (0),
  itsPerturbation    (0)
{
}

MeqResultRep::~MeqResultRep()
{
  clear();
}


void MeqResultRep::clear()
{
  if (itsPerturbedValues != 0) {
    delete itsPerturbedValues;
    itsPerturbedValues = 0;
  }

  if (itsPerturbation != 0) {
    delete itsPerturbation;
    itsPerturbation = 0;
  }
}

  
void MeqResultRep::setPerturbedValue (int i, const MeqMatrix& value)
{
  if (itsPerturbedValues == 0) itsPerturbedValues = new map<int, MeqMatrix>;
  (*itsPerturbedValues)[i] = value;
}

void MeqResultRep::show (ostream& os) // const
{
  os << "Value: " << itsValue << endl;
  for (int i=0; i<itsNspid; i++) {
    if (isDefined(i)) {
      os << "deriv parm " << i << " with " << (*itsPerturbation)[i] << endl;
      os << "   " << ((*itsPerturbedValues)[i] - itsValue) << endl;
      os << "   " << ((*itsPerturbedValues)[i] - itsValue) /
	 (*itsPerturbation)[i] << endl;
    }
  }
}




MeqResult::MeqResult (int nspid)
: itsRep (new MeqResultRep(nspid))
{
  itsRep->link();
}

MeqResult::MeqResult (const MeqResult& that)
: itsRep (that.itsRep)
{
  if (itsRep != 0) {
    itsRep->link();
  }
}
MeqResult& MeqResult::operator= (const MeqResult& that)
{
  if (this != &that) {
    MeqResultRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

}

//# MeqResultVec.cc: The result of a Jones expression for a domain.
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
#include <BBS3/MNS/MeqResultVec.h>

// This class represents the result in a Jones matrix of a domain for
// which the expressions have been evaluated.

namespace LOFAR {

  MeqResultVecRep::MeqResultVecRep (int size, int nspid)
  : itsCount (0)
  {
    itsResult.reserve (size);
    for (int i=0; i<size; ++i) {
      itsResult.push_back (MeqResult(nspid));
    }
  }

  void MeqResultVecRep::show (ostream&) const
  {
    for (uint i=0; i<itsResult.size(); ++i) {
      cout << "*** result" << i << " ***" << endl << itsResult[i];
    }
  }



  MeqResultVec::MeqResultVec (int size, int nspid)
    : itsRep (0)
  {
    itsRep = new MeqResultVecRep(size, nspid);
    itsRep->link();
  }

  MeqResultVec::MeqResultVec (const MeqResultVec& that)
    : itsRep (that.itsRep)
  {
    if (itsRep != 0) {
      itsRep->link();
    }
  }

  MeqResultVec& MeqResultVec::operator= (const MeqResultVec& that)
  {
    if (this != &that) {
      MeqResultVecRep::unlink (itsRep);
      itsRep = that.itsRep;
      if (itsRep != 0) {
	itsRep->link();
      }
    }
    return *this;
  }

}

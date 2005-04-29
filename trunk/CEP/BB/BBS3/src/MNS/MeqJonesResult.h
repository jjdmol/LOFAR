//# MeqJonesResult.h: The result of a Jones expression for a domain.
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

#ifndef MNS_MEQJONESRESULT_H
#define MNS_MEQJONESRESULT_H

// \file MNS/MeqJonesResult.h
// The result of the 4 expressions in a Jones matrix for a domain.

//# Includes
#include <BBS3/MNS/MeqResultVec.h>

// This class represents the result in a Jones matrix of a domain for
// which the expressions have been evaluated.

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

  class MeqJonesResult : public MeqResultVec
  {
  public:
    // Create a time,frequency result for the given number of parameters.
    explicit MeqJonesResult (int nspid = 0)
      : MeqResultVec(4, nspid) {}

    MeqJonesResult (const MeqJonesResult& that)
      : MeqResultVec(that) {}

    MeqJonesResult& operator= (const MeqJonesResult& that)
      { MeqResultVec::operator= (that); return *this; }

    // Get the individual results.
    // <group>
    const MeqResult& getResult11() const
      { return (*this)[0]; }
    MeqResult& result11()
      { return (*this)[0]; }
    const MeqResult& getResult12() const
      { return (*this)[1]; }
    MeqResult& result12()
      { return (*this)[1]; }
    const MeqResult& getResult21() const
      { return (*this)[2]; }
    MeqResult& result21()
      { return (*this)[2]; }
    const MeqResult& getResult22() const
      { return (*this)[3]; }
    MeqResult& result22()
      { return (*this)[3]; }
    // </group>
  };

// @}

}

#endif

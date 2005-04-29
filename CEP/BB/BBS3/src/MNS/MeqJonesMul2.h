//# MeqJonesMul2.h: Calculate left*right
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
#ifndef MNS_MEQJONESMUL2_H
#define MNS_MEQJONESMUL2_H

// \file MNS/MeqJonesMul2.h
// Calculate left*right

//# Includes
#include <BBS3/MNS/MeqJonesExpr.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{


// Calculate left*right.

class MeqJonesMul2: public MeqJonesExprRep
{
public:
  MeqJonesMul2 (const MeqJonesExpr& left, const MeqJonesExpr& right);

  ~MeqJonesMul2();

  // Get the result of the expression for the given domain.
  MeqJonesResult getResult (const MeqRequest&);

private:
  MeqJonesExpr itsLeft;
  MeqJonesExpr itsRight;
};

// @}

}

#endif

//# MeqStation.h: Class holding the ITRF position expressions of a station
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

#if !defined(MNS_MEQSTATION_H)
#define MNS_MEQSTATION_H

// \file MNS/MeqStation.h
// Class holding the ITRF position expressions of a station

//# Includes
#include <BBS3/MNS/MeqExpr.h>
#include <Common/lofar_string.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

// Class holding the ITRF position expressions of a station.

class MeqStation
{
public:
  // The default constructor.
  MeqStation();

  MeqStation (MeqExpr posX, MeqExpr posY, MeqExpr posZ, const string& name);

  MeqExpr& getPosX()
    { return itsX; }
  MeqExpr& getPosY()
    { return itsY; }
  MeqExpr& getPosZ()
    { return itsZ; }

  const string& getName() const
    { return itsName; }


private:
  MeqExpr itsX;
  MeqExpr itsY;
  MeqExpr itsZ;
  string   itsName;
};

// @}

}

#endif

//# SkyCoord.h: Class to hold a sky coordinate as 2 angles
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

#ifndef AMC_AMCBASE_SKYCOORD_H
#define AMC_AMCBASE_SKYCOORD_H

//# Forward Declarations.
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace AMC
  {

    class SkyCoord
    {
    public:
      // Default constructor uses 0 for the angles.
      SkyCoord()
        : itsAngle0(0), itsAngle1(0) {}

      // Create a sky coordinate by giving the longitude and latitude in
      // radians.  The context where the object is used defines the coordinate
      // system and frame, so the class can be used for any pair of sky
      // coordinates (like RA/DEC and AZ/ELEV).
      SkyCoord (double angle0, double angle1)
        : itsAngle0(angle0), itsAngle1(angle1) {}

      // Get the values out.
      double angle0() const
      { return itsAngle0; }
      double angle1() const
      { return itsAngle1; }

    private:
      double itsAngle0;
      double itsAngle1;
    };

    // Output in ASCII.
    ostream& operator<< (ostream&, const SkyCoord&);

  } // namespace AMC

} // namespace LOFAR


#endif

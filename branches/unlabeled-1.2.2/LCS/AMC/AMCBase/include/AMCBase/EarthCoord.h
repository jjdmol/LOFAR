//# EarthCoord.h: Class to hold an earth coordinate as lon,lat,height
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

#ifndef LOFAR_AMCBASE_EARTHCOORD_H
#define LOFAR_AMCBASE_EARTHCOORD_H

//# Forward Declarations.
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace AMC
  {

    class EarthCoord
    {
    public:
      // Default constructor uses 0 for the values.
      EarthCoord()
        : itsLong(0), itsLat(0), itsHeight(0) {}

      // Create a earth coordinate by giving the longitude and latitude in
      // radians and the height in meters.
      EarthCoord (double longitude, double latitude, double height=0)
        : itsLong(longitude), itsLat(latitude), itsHeight(height) {}

      // Get the values out.
      double longitude() const
      { return itsLong; }
      double latitude() const
      { return itsLat; }
      double height() const
      { return itsHeight; }

    private:
      double itsLong;
      double itsLat;
      double itsHeight;
    };

    // Output in ASCII.
    ostream& operator<< (ostream&, const EarthCoord&);

  } // namespace AMC

} // namespace LOFAR

#endif

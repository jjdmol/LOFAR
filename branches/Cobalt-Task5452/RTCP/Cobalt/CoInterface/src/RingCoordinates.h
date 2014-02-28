//# RingCoordinates.h
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$


#ifndef LOFAR_INTERFACE_RINGCOORDINATES_H
#define LOFAR_INTERFACE_RINGCOORDINATES_H

#include <utility>      // std::pair, std::make_pair
#include <vector>         
#include <cstddef>

namespace LOFAR
{
  namespace Cobalt
  {
    class RingCoordinates
    {
    public:

      enum COORDTYPES
      {
        J2000,
        B1950,
        OTHER
      };
      
      
      typedef std::pair<double, double> Coordinate;
      typedef std::vector<Coordinate > CoordinateVector;

      RingCoordinates(size_t nRings, double width, 
        Coordinate const &center, RingCoordinates::COORDTYPES type);

      const CoordinateVector&  coordinates() const;

    private:

      /*
      *  _
      * / \
      * \_/
      * |.|
      */
      double len_edge();

      /*
      *  _
      * / \
      * \_/
      *|...|
      */
      double len_width();

      /*
      *  _  _
      * / \ :
      * \_/ _
      * 
      */
      double len_height();

      /*
      *  _
      * / \_
      * \_/ \
      *   \_/
      *  |.|
      */
      double delta_width();

      /*
      *  _
      * / \_  -
      * \_/ \ -
      *   \_/
      *  
      */
      double delta_height();
     
      CoordinateVector createPrecompiledCoords();

      Coordinate cos_adjust(Coordinate const& offset);

      CoordinateVector itsCoordinates;
      size_t itsNRings;
      double itsWidth;
      Coordinate itsCenter;
      COORDTYPES itsType;
      

    };
  }
}

#endif

//# RingCoordinates.cc
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "RingCoordinates.h"
#include <math.h>       // sqrt
#include <algorithm>    // std::transform

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {
    RingCoordinates::RingCoordinates(size_t nRings, double width,
      RingCoordinates::Coordinate const  &center,
      RingCoordinates::COORDTYPES type)
    :
      itsNRings(nRings),
      itsWidth(width),
      itsCenter(center),
      itsType(type)
    {
      // If there are zero rings we do not create any tabs
      // itsCoordinates will be default constructed empty
      if (nRings == 0)
        return;

      // create the beams (tabs)     
      CoordinateVector preCompiledCoordDelta = createPrecompiledCoords();

      // start with central beam
      itsCoordinates.push_back(pair<double, double>(0., 0.));

      // ring 1-n: create the TAB Beams from the inner ring outwards
      for (size_t idx_ring = 1; idx_ring <= itsNRings; ++idx_ring)
      {
        double l = 0.0;
        double m = len_height() * idx_ring;

        // For each side in the hexagon
        for (size_t idx_side = 0; idx_side < 6; ++idx_side)
        {
          //each side has length of the tab ring we are in!
          for (size_t idx_in_side = 0; idx_in_side < idx_ring; ++idx_in_side)
          {
            itsCoordinates.push_back(Coordinate(l, m));

            l += preCompiledCoordDelta[idx_side].first;
            m += preCompiledCoordDelta[idx_side].second;
          }
        }
      }

      // adjust the coordinates depending on the reference frame
      for (CoordinateVector::iterator coord = itsCoordinates.begin();
        coord != itsCoordinates.end(); ++coord)
      {
        *coord = cos_adjust(*coord);
      }

    }

    /*
    *  _
    * / \
    * \_/
    * |.|
    */
    double RingCoordinates::len_edge()
    {
      return itsWidth / sqrt(3);
    }

    /*
    *  _
    * / \
    * \_/
    *|...|
    */
    double RingCoordinates::len_width()
    {
      return 2 * len_edge();
    } 

    /*
    *  _  _
    * / \ :
    * \_/ _
    *
    */
    double RingCoordinates::len_height()
    {
      return len_width(); 
    }

    /*
    *  _
    * / \_
    * \_/ \
    *   \_/
    *  |.|
    */
    double RingCoordinates::delta_width()
    {
      return 1.5 * len_edge();
    }

    /*
    *  _
    * / \_  -
    * \_/ \ -
    *   \_/
    *
    */
    double RingCoordinates::delta_height()
    {
      return 0.5 * len_width();
    }

    RingCoordinates::CoordinateVector RingCoordinates::createPrecompiledCoords()
    {
      //# stride for each side, starting left from the top, clock - wise
      CoordinateVector preCoords;

      /*
      #  _
      # / \_
      # \_/ \
      #   \_/
      */
      preCoords.push_back(Coordinate(delta_width(), -delta_height()));
      /*
      #  _
      # / \
      # \_/
      # / \
      # \_/
      */
      preCoords.push_back(Coordinate(0.0, -len_height()));

      /*
      #    _
      #  _/ \
      # / \_/
      # \_/
      */
      preCoords.push_back(Coordinate(-delta_width(), -delta_height()));

      /*
      #  _
      # / \_
      # \_/ \
      #   \_/
      */
      preCoords.push_back(Coordinate(-delta_width(), delta_height()));

      /*
      #  _
      # / \
      # \_/
      # / \
      # \_/
      */
      preCoords.push_back(Coordinate(0.0, len_height()));

      /*
      #    _
      #  _/ \
      # / \_/
      # \_/
      */
      preCoords.push_back(Coordinate(delta_width(), delta_height()));


      return preCoords;
    }

    RingCoordinates::Coordinate RingCoordinates::cos_adjust(
            RingCoordinates::Coordinate const &offset)
    {
      if (itsType == RingCoordinates::OTHER)
        return offset;

      double cos_dec = cos(itsCenter.second + offset.second);
      double epsilon = 0.0001;

      if (cos_dec > epsilon)
        return  Coordinate(offset.first / cos_dec, offset.second);
      else
        return offset;
    }

    const RingCoordinates::CoordinateVector&  RingCoordinates::coordinates() const
    {
      return itsCoordinates;
    }

    size_t  RingCoordinates::nCoordinates() const
    {
      return itsCoordinates.size();
    }

    std::string RingCoordinates::coordTypeAsString()const
    {
      if (itsType == RingCoordinates::J2000)
        return "J2000";
      else if (itsType == RingCoordinates::B1950)
        return "JB1950";
      else
        return "J2000";
    }
  }
}

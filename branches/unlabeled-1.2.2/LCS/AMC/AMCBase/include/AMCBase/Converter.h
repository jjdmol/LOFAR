//#  Converter.h: interface definition of the AMC converter classes.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_AMCBASE_CONVERTER_H
#define LOFAR_AMCBASE_CONVERTER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {
    // \addtogroup AMCBase
    // @{

    //# Forward Declarations
    class SkyCoord;
    class EarthCoord;
    class TimeCoord;

    // This class defines the interface of the diverse AMC Converter classes.
    class Converter
    {
    public:
      // Convert the given equatorial J2000 sky coordinate to azimuth/elevation
      // (in radians) for the given earth position and time.
      virtual SkyCoord j2000ToAzel(const SkyCoord& radec, 
                                   const EarthCoord& pos, 
                                   const TimeCoord& time) = 0;

      // Convert a series of sky coordinates.
      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
                                            const EarthCoord& pos,
                                            const TimeCoord& time) = 0;

      // Convert a sky coordinate for a series of earth positions.
      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
                                            const vector<EarthCoord>& pos,
                                            const TimeCoord& time) = 0;

      // Convert a sky coordinate for a series of times.
      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
                                            const EarthCoord& pos,
                                            const vector<TimeCoord>& time) = 0;

      // Convert a series of sky coordinates for a series of earth positions
      // and times.
      // The output vector contains <tt>radec.size() * pos.size() *
      // time.size()</tt> elements which can be seen as a cube with shape
      // <tt>[nsky,npos,ntime]</tt> in Fortran order (thus with sky as the
      // most rapidly varying axis).
      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time) = 0;

      // Convert an azimuth/elevation for the given earth position and time
      // to ra/dec.
      virtual SkyCoord azelToJ2000 (const SkyCoord& azel,
                                    const EarthCoord& pos,
                                    const TimeCoord& time) = 0;

      // Convert a series of azimuth/elevations for the given earth position
      // and time to ra/dec. The output vector has the same length as the
      // input \a azel vector.
      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
                                            const EarthCoord& pos,
                                            const TimeCoord& time) = 0;

      // Convert a series of azimuth/elevations for the given earth positions
      // and times to ra/dec. The output vector has the same length as the
      // input \a azel vector.
      // All input vectors must have the same length.
      // The difference with the function above is that here each \a azel has
      // its own earth position and time.
      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time) = 0;

    protected:
      // Destructor
      virtual ~Converter() {}

    };

  } // namespace AMC

} // namespace LOFAR

#endif

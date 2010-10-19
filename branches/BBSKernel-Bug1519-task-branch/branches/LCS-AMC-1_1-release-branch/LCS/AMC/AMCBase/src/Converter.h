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

// \file Converter.h
// Interface definition of the AMC converter classes

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
      // Destructor
      virtual ~Converter() {}

      // Convert the given equatorial J2000 sky coordinate to azimuth/elevation
      // (in radians) for the given earth position and time.
      virtual SkyCoord j2000ToAzel(const SkyCoord& dir, 
                                   const EarthCoord& pos, 
                                   const TimeCoord& time) = 0;

      // Convert a series of sky coordinates.
      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& dir,
                                            const EarthCoord& pos,
                                            const TimeCoord& time) = 0;

      // Convert a sky coordinate for a series of earth positions.
      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& dir,
                                            const vector<EarthCoord>& pos,
                                            const TimeCoord& time) = 0;

      // Convert a sky coordinate for a series of times.
      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& dir,
                                            const EarthCoord& pos,
                                            const vector<TimeCoord>& time) = 0;

      // Convert a series of sky coordinates for a series of earth positions
      // and times.
      // The output vector contains <tt>dir.size() * pos.size() *
      // time.size()</tt> elements which can be seen as a cube with shape
      // <tt>[nsky,npos,ntime]</tt> in Fortran order (thus with sky as the
      // most rapidly varying axis).
      // \pre \a dir must be given in \c J2000.
      // \return vector of SkyCoord in \c AZEL.
      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& dir,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time) = 0;

      // Convert an azimuth/elevation for the given earth position and time
      // to ra/dec.
      virtual SkyCoord azelToJ2000 (const SkyCoord& dir,
                                    const EarthCoord& pos,
                                    const TimeCoord& time) = 0;

      // Convert a series of azimuth/elevations for the given earth position
      // and time to ra/dec. The output vector has the same length as the
      // input \a azel vector.
      // \pre \a azel must be given in \c AZEL.
      // \post size of return vector is equal to size of vector \a azel.
      // \return vector of SkyCoord in \c J2000.
      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& dir,
                                            const EarthCoord& pos,
                                            const TimeCoord& time) = 0;

      // Convert a series of azimuth/elevations for the given earth positions
      // and times to ra/dec. The output vector has the same length as the
      // input \a azel vector.
      // \pre All input vectors must have the same length.
      //      \a azel must be given in \c AZEL.
      // \post size of return vector is equal to size of vector \a azel.
      // \return vector of SkyCoord in \c J2000.
      // \note The difference with the function above is that here each \a
      // azel has its own earth position and time.
      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& dir,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time) = 0;


      // Convert the given equatorial J2000 sky coordinate to ITRF 
      // (in radians) for the given earth position and time.
      virtual SkyCoord j2000ToItrf(const SkyCoord& dir, 
                                   const EarthCoord& pos, 
                                   const TimeCoord& time) = 0;

      // Convert a series of sky coordinates.
      virtual vector<SkyCoord> j2000ToItrf (const vector<SkyCoord>& dir,
                                            const EarthCoord& pos,
                                            const TimeCoord& time) = 0;

      // Convert a sky coordinate for a series of earth positions.
      virtual vector<SkyCoord> j2000ToItrf (const SkyCoord& dir,
                                            const vector<EarthCoord>& pos,
                                            const TimeCoord& time) = 0;

      // Convert a sky coordinate for a series of times.
      virtual vector<SkyCoord> j2000ToItrf (const SkyCoord& dir,
                                            const EarthCoord& pos,
                                            const vector<TimeCoord>& time) = 0;

      // Convert a series of sky coordinates for a series of earth positions
      // and times.
      // The output vector contains <tt>dir.size() * pos.size() *
      // time.size()</tt> elements which can be seen as a cube with shape
      // <tt>[nsky,npos,ntime]</tt> in Fortran order (thus with sky as the
      // most rapidly varying axis).
      // \pre \a dir must be given in \c J2000.
      // \return vector of SkyCoord given in \c ITRF.
      virtual vector<SkyCoord> j2000ToItrf (const vector<SkyCoord>& dir,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time) = 0;

      // Convert the given ITRF sky coordinate for the given earth position
      // and time to an equatorial J2000 sky coordinate
      virtual SkyCoord itrfToJ2000 (const SkyCoord& dir,
                                    const EarthCoord& pos,
                                    const TimeCoord& time) = 0;

      // Convert a series of ITRF sky coordinates for the given earth position
      // and time to J2000 ra/dec. The output vector has the same length as
      // the input \a dir vector.
      // \pre \a dir must be given in \c ITRF.
      // \post size of return vector is equal to size of vector \a dir.
      // \return vector of SkyCoord in \c J2000.
      virtual vector<SkyCoord> itrfToJ2000 (const vector<SkyCoord>& dir,
                                            const EarthCoord& pos,
                                            const TimeCoord& time) = 0;

      // Convert a series of ITRF sky coordinates for the given earth
      // positions and times to J2000 ra/dec. The output vector has the same
      // length as the input \a dir vector.
      // \pre All input vectors must have the same length.
      //      \a dir must be given in \c ITRF.
      // \post size of return vector is equal to size of vector \a dir.
      // \return vector of SkyCoord in \c J2000.
      // \note The difference with the function above is that here each \a
      // dir has its own earth position and time.
      virtual vector<SkyCoord> itrfToJ2000 (const vector<SkyCoord>& dir,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time) = 0;

    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif

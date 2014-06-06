//# Converter.h: interface definition of the AMC converter classes.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifndef LOFAR_AMCBASE_CONVERTER_H
#define LOFAR_AMCBASE_CONVERTER_H

// \file
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
    struct RequestData;
    struct ResultData;

    // This class defines the interface of the diverse AMC Converter classes.
    // All conversion routines operate on a struct RequestData, returning a
    // struct ResultData. RequestData contains three vectors: \a direction, a
    // series of sky coordinates; \a position, a series of earth positions;
    // and \a epoch, a series of points in time. ResultData contains one
    // vector: \a direction, a series of sky coordinates.
    //
    // Conversion from J2000 to another coordinate system always produces a
    // vector of sky coordinates that contains all possible combinations of
    // the elements in the input vectors \a direction, \a position, and \a
    // epoch. Hence, the output vector contains <tt>direction.size() *
    // position.size() * epoch.size()</tt> elements, which can be seen
    // as a cube with shape <tt>[nsky,npos,ntime]</tt> in Fortran order (i.e.
    // with sky as the most rapidly varying axis).
    //
    // Conversion from another coordinate system to J2000 always produces a
    // vector of sky coordinates that is equal in size to the input vector \a
    // direction. The input vectors \a position and \a epoch should
    // either both be equal in size to the input vector \a direction, or both
    // contain exactly one element. In the former case, the conversion routine
    // will calculate one output direction for each triplet of input sky
    // coordinates, earth positions, and points in time. In the latter case,
    // the conversion routine will convert the input sky coordinates for the
    // given earth position and point in time.
    //
    // \note None of the member functions below can be made \c const, because
    // the ConverterClient class that implements these functions has data
    // members that cannot be \c const (e.g., DH_Request, DH_Result,
    // TH_Socket, and Connection). Alas!
    class Converter
    {
    public:
      // Destructor
      virtual ~Converter() {}

      // Convert the given equatorial J2000 sky coordinates to
      // azimuth/elevation (in radians) for the given earth positions and
      // points in time.
      // \pre \a request.direction.type() == Direction::J2000
      // \post \a result.direction.type() == Direction::AZEL
      // \post \a result.direction.size() == request.direction.size() *
      // request.position.size() * request.epoch.size().
      virtual void j2000ToAzel(ResultData& result, 
                               const RequestData& request) = 0;
      
      // Convert a series of azimuth/elevations for the given earth
      // position(s) and point(s) in time to J2000 ra/dec.
      // \pre \a request.direction.type() == Direction::AZEL
      // \pre \a request.position.size() == 1 || 
      //         request.position.size() == request.direction.size()
      // \pre \a request.position.size() == request.epoch.size()
      // \post \a result.direction.type() == Direction::J2000
      // \post \a result.direction.size() == request.direction.size()
      virtual void azelToJ2000(ResultData& result, 
                               const RequestData& request) = 0;
      
      // Convert the given equatorial J2000 sky coordinate to ITRF (in
      // radians) for the given earth positions and points in time.
      // \pre \a request.direction.type() == Direction::J2000
      // \post \a result.direction.type() == Direction::ITRF
      // \post \a result.direction.size() == request.direction.size() *
      // request.position.size() * request.epoch.size().
      virtual void j2000ToItrf(ResultData& result, 
                               const RequestData& request) = 0;

      // Convert a series of ITRF sky coordinates for the given earth
      // position(s) and point(s) in time to J2000 ra/dec.
      // \pre \a request.direction.type() == Direction::ITRF
      // \pre \a request.position.size() == 1 || 
      //         request.position.size() == request.direction.size()
      // \pre \a request.position.size() == request.epoch.size()
      // \post \a result.direction.type() == Direction::J2000
      // \post \a result.direction.size() == request.direction.size()
      virtual void itrfToJ2000(ResultData& result, 
                               const RequestData& request) = 0;
    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif

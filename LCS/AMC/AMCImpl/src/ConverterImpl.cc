//#  ConverterImpl.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCImpl/ConverterImpl.h>
#include <AMCImpl/Exceptions.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Exceptions/Error.h>

using namespace casa;

namespace LOFAR
{
  namespace AMC
  {

    ConverterImpl::ConverterImpl()
    {
    }


    SkyCoord 
    ConverterImpl::j2000ToAzel (const SkyCoord& skyCoord, 
                                const EarthCoord& earthCoord, 
                                const TimeCoord& timeCoord)
    {
      return j2000ToAzel (vector<SkyCoord>  (1, skyCoord),
                          vector<EarthCoord>(1, earthCoord),
                          vector<TimeCoord> (1, timeCoord)) [0];
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToAzel (const vector<SkyCoord>& skyCoord,
                                const EarthCoord& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return j2000ToAzel(skyCoord, 
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToAzel (const SkyCoord& skyCoord,
                                const vector<EarthCoord>& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return j2000ToAzel(vector<SkyCoord> (1, skyCoord),
                         earthCoord, 
                         vector<TimeCoord>(1, timeCoord));
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToAzel (const SkyCoord& skyCoord,
                                const EarthCoord& earthCoord,
                                const vector<TimeCoord>& timeCoord)
    {
      return j2000ToAzel(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         timeCoord);
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToAzel (const vector<SkyCoord>& skyCoord,
                                const vector<EarthCoord>& earthCoord,
                                const vector<TimeCoord>& timeCoord)
    {
      // Check pre-conditions.
      // -# skyCoord type must be J2000
      // -# earthCoord types must be equal
      for (uint i = 0; i < skyCoord.size(); ++i) {
        ASSERT(skyCoord[i].type() == SkyCoord::J2000);
      }
      for (uint i = 1; i < earthCoord.size(); ++i) {
        ASSERT(earthCoord[i].type() == earthCoord[0].type());
      }

      // This vector will hold the result of the conversion operation.
      vector<SkyCoord> result;

      // If any of the input vectors has zero length, we can return
      // immediately.
      if (skyCoord.size()   == 0 || 
          earthCoord.size() == 0 || 
          timeCoord.size()  == 0)
        return result;

      // Reserve space for the result to avoid resizing of the vector.
      result.reserve(skyCoord.size() * earthCoord.size() * timeCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(earthCoord[i].height(), "m")),
                            (Quantity(earthCoord[i].longitude(), "rad")),
                            (Quantity(earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterError, "Invalid EarthCoord type: " 
              << earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(timeCoord[0].getDay(),
                                 timeCoord[0].getFraction())));
        
        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));
        
        // Set-up the conversion engine, using reference direction AZEL.
        MDirection::Convert conv (MDirection::J2000, 
                                  MDirection::Ref (MDirection::AZEL, frame));

        // For each given moment in time ...
        for (uint i = 0; i < timeCoord.size(); i++) {

          // Set the instant in time in the frame.
          frame.resetEpoch(MVEpoch(timeCoord[i].getDay(),
                                   timeCoord[i].getFraction()));
          
          // For each given position on earth ...
          for (uint j = 0; j < earthCoord.size(); j++) {

            // Set the position on earth in the frame.
            frame.resetPosition(pos[j]);
              
            // For each given direction in the sky ...
            for (uint k = 0; k < skyCoord.size(); k++) {

              // Define the astronomical direction as a J2000 direction.
              MVDirection sky(skyCoord[k].angle0(), skyCoord[k].angle1());

              // Convert this direction, using the conversion engine.
              MDirection dir = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = dir.getAngle().getBaseValue();

              // Convert to local sky coordinates and add to the return vector.
              result.push_back(SkyCoord(angles(0), angles(1), SkyCoord::AZEL));

            }
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterError, "AipsError: " << e.what());
      }

      // Check on post-condition that the return vector contains 
      // <tt>skyCoord.size() * earthCoord.size() * timeCoord.size()</tt>
      // elements.
      ASSERT (result.size() == 
              skyCoord.size() * earthCoord.size() * timeCoord.size());

      return result;
    }


    SkyCoord 
    ConverterImpl::azelToJ2000 (const SkyCoord& skyCoord,
                                const EarthCoord& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return azelToJ2000(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord)) [0];
    }


    vector<SkyCoord>
    ConverterImpl::azelToJ2000 (const vector<SkyCoord>& skyCoord,
                                const EarthCoord& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return azelToJ2000(skyCoord,
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord>
    ConverterImpl::azelToJ2000 (const vector<SkyCoord>& skyCoord,
                                const vector<EarthCoord>& earthCoord,
                                const vector<TimeCoord>& timeCoord)
    {
      // Check pre-conditions.
      // -# \a earthCoord and \a timeCoord must have equal sizes;
      // -# if \a earthCoord and \a timeCoord have sizes unequal to one, then
      //    their sizes must be equal to the size of \a skyCoord. Note that
      //    we only need to do one check, the other is implied by the first
      //    pre-condition.
      // -# skyCoord type must be AZEL
      // -# earthCoord types must be the equal
      ASSERT (earthCoord.size() == timeCoord.size());
      if (earthCoord.size() != 1) {
        ASSERT (earthCoord.size() == skyCoord.size());
      }
      for (uint i = 0; i < skyCoord.size(); ++i) {
        ASSERT (skyCoord[i].type() == SkyCoord::AZEL);
      }
      for (uint i = 1; i < earthCoord.size(); ++i) {
        ASSERT (earthCoord[i].type() == earthCoord[0].type());
      }

      // This vector will hold the result of the conversion operation.
      vector<SkyCoord> result;

      // Reserve space for the result to avoid resizing of the vector.
      result.reserve(skyCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(earthCoord[i].height(), "m")),
                            (Quantity(earthCoord[i].longitude(), "rad")),
                            (Quantity(earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterError, "Invalid EarthCoord type: " 
              << earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(timeCoord[0].getDay(), 
                                 timeCoord[0].getFraction())));

        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));

        // Set-up the conversion engine, using reference direction J2000..
        MDirection::Convert conv(MDirection::AZEL,
                                 MDirection::Ref(MDirection::J2000, frame));

        // If there's only one earthCoord and one timeCoord, we only need to
        // loop over all \a skyCoord values.
        if (timeCoord.size() == 1) {

          // For each given direction in the sky ...
          for (uint i = 0; i < skyCoord.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(skyCoord[i].angle0(), skyCoord[i].angle1()); 
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getAngle().getBaseValue();

            // Convert to local sky coordinates and add to the return vector.
            result.push_back(SkyCoord(angles(0), angles(1), SkyCoord::J2000));
          }
        }

        // We need to calculate the converted sky coordinate for each triplet
        // of \a skyCoord, \a earthCoord, and \a timeCoord.
        else {
          
          // For each triplet ...
          for (uint i = 0; i < skyCoord.size(); i++) {
            
            // Set the instant in time in the frame.
            frame.resetEpoch(MVEpoch(timeCoord[i].getDay(), 
                                     timeCoord[i].getFraction()));
            
            // Set the position on earth in the frame.
            frame.resetPosition(pos[i]);
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(skyCoord[i].angle0(), skyCoord[i].angle1());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getAngle().getBaseValue();

            // Convert to local sky coordinates and add to the return vector.
            result.push_back(SkyCoord(angles(0), angles(1), SkyCoord::J2000));
            
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterError, "AipsError: " << e.what());
      }

      // Check post-condition.
      ASSERT(result.size() == skyCoord.size());

      // Return the result vector.
      return result;
    }
    
    
    SkyCoord 
    ConverterImpl::j2000ToItrf(const SkyCoord& skyCoord, 
                               const EarthCoord& earthCoord, 
                               const TimeCoord& timeCoord)
    {
      return j2000ToItrf(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord)) [0];
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToItrf (const vector<SkyCoord>& skyCoord,
                                const EarthCoord& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return j2000ToItrf(skyCoord,
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord>
    ConverterImpl::j2000ToItrf (const SkyCoord& skyCoord,
                                const vector<EarthCoord>& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return j2000ToItrf(vector<SkyCoord>  (1, skyCoord),
                         earthCoord,
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToItrf (const SkyCoord& skyCoord,
                                const EarthCoord& earthCoord,
                                const vector<TimeCoord>& timeCoord)
    {
      return j2000ToItrf(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         timeCoord);
    }


    vector<SkyCoord> 
    ConverterImpl::j2000ToItrf (const vector<SkyCoord>& skyCoord,
                                const vector<EarthCoord>& earthCoord,
                                const vector<TimeCoord>& timeCoord)
    {
      // Check pre-conditions.
      // -# skyCoord type must be J2000
      // -# earthCoord types must be equal
      for (uint i=0; i<skyCoord.size(); ++i) {
        ASSERT(skyCoord[i].type() == SkyCoord::J2000);
      }
      for (uint i = 1; i < earthCoord.size(); ++i) {
        ASSERT(earthCoord[i].type() == earthCoord[0].type());
      }

      // This vector will hold the result of the conversion operation.
      vector<SkyCoord> result;

      // If any of the input vectors has zero length, we can return
      // immediately.
      if (skyCoord.size()   == 0 || 
          earthCoord.size() == 0 || 
          timeCoord.size()  == 0)
        return result;

      // Reserve space for the result to avoid resizing of the vector.
      result.reserve(skyCoord.size() * earthCoord.size() * timeCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(earthCoord[i].height(), "m")),
                            (Quantity(earthCoord[i].longitude(), "rad")),
                            (Quantity(earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterError, "Invalid EarthCoord type: " 
              << earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(timeCoord[0].getDay(),
                                 timeCoord[0].getFraction())));
        
        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));
        
        // Set-up the conversion engine, using reference direction ITRF.
        MDirection::Convert conv (MDirection::J2000, 
                                  MDirection::Ref (MDirection::ITRF, frame));

        // For each given moment in time ...
        for (uint i = 0; i < timeCoord.size(); i++) {

          // Set the instant in time in the frame.
          frame.resetEpoch(MVEpoch(timeCoord[i].getDay(), 
                                   timeCoord[i].getFraction()));
          
          // For each given position on earth ...
          for (uint j = 0; j < earthCoord.size(); j++) {

            // Set the position on earth in the frame.
            frame.resetPosition(pos[j]);
            
            // For each given direction in the sky ...
            for (uint k = 0; k < skyCoord.size(); k++) {
              
              // Define the astronomical direction as a J2000 direction.
              MVDirection sky(skyCoord[k].angle0(), skyCoord[k].angle1());

              // Convert this direction, using the conversion engine.
              MDirection dir = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = dir.getAngle().getBaseValue();

              // Convert to local sky coordinates and add to the return vector.
              result.push_back(SkyCoord(angles(0), angles(1), SkyCoord::ITRF));

            }
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterError, "AipsError: " << e.what());
      }

      // Check on post-condition that the return vector contains 
      // <tt>skyCoord.size() * earthCoord.size() * timeCoord.size()</tt>
      // elements.
      ASSERT (result.size() == 
              skyCoord.size() * earthCoord.size() * timeCoord.size());

      return result;
    }


    SkyCoord 
    ConverterImpl::itrfToJ2000 (const SkyCoord& skyCoord,
                                const EarthCoord& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return itrfToJ2000(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord)) [0];
    }


    vector<SkyCoord> 
    ConverterImpl::itrfToJ2000 (const vector<SkyCoord>& skyCoord,
                                const EarthCoord& earthCoord,
                                const TimeCoord& timeCoord)
    {
      return itrfToJ2000(skyCoord,
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord> 
    ConverterImpl::itrfToJ2000 (const vector<SkyCoord>& skyCoord,
                                const vector<EarthCoord>& earthCoord,
                                const vector<TimeCoord>& timeCoord)
    {
      // Check pre-conditions.
      // -# \a earthCoord and \a timeCoord must have equal sizes;
      // -# if \a earthCoord and \a timeCoord have sizes unequal to one, then
      //    their sizes must be equal to the size of \a skyCoord. Note that
      //    we only need to do one check, the other is implied by the first
      //    pre-condition.
      // -# skyCoord type must be ITRF
      // -# earthCoord types must be the equal
      ASSERT (earthCoord.size() == timeCoord.size());
      if (earthCoord.size() != 1) {
        ASSERT (earthCoord.size() == skyCoord.size());
      }
      for (uint i = 0; i < skyCoord.size(); ++i) {
        ASSERT (skyCoord[i].type() == SkyCoord::ITRF);
      }
      for (uint i = 1; i < earthCoord.size(); ++i) {
        ASSERT (earthCoord[i].type() == earthCoord[0].type());
      }

      // This vector will hold the result of the conversion operation.
      vector<SkyCoord> result;

      // Reserve space for the result to avoid resizing of the vector.
      result.reserve(skyCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(earthCoord[i].height(), "m")),
                            (Quantity(earthCoord[i].longitude(), "rad")),
                            (Quantity(earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterError, "Invalid EarthCoord type: " 
              << earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(timeCoord[0].getDay(), 
                                 timeCoord[0].getFraction())));

        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));

        // Set-up the conversion engine, using reference direction J2000..
        MDirection::Convert conv(MDirection::ITRF,
                                 MDirection::Ref(MDirection::J2000, frame));

        // If there's only one earthCoord and one timeCoord, we only need to
        // loop over all \a skyCoord values.
        if (timeCoord.size() == 1) {

          // For each given direction in the sky ...
          for (uint i = 0; i < skyCoord.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(skyCoord[i].angle0(), skyCoord[i].angle1());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getAngle().getBaseValue();

            // Convert to local sky coordinates and add to the return vector.
            result.push_back(SkyCoord(angles(0), angles(1), SkyCoord::J2000));
          }
        }

        // We need to calculate the converted sky coordinate for each triplet
        // of \a skyCoord, \a earthCoord, and \a timeCoord.
        else {

          // For each triplet ...
          for (uint i = 0; i < skyCoord.size(); i++) {

            // Set the instant in time in the frame.
            frame.resetEpoch(MVEpoch(timeCoord[i].getDay(), 
                                     timeCoord[i].getFraction()));
            
            // Set the position on earth in the frame.
            frame.resetPosition(pos[i]);
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(skyCoord[i].angle0(), skyCoord[i].angle1());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getAngle().getBaseValue();

            // Convert to local sky coordinates and add to the return vector.
            result.push_back(SkyCoord(angles(0), angles(1), SkyCoord::J2000));
            
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterError, "AipsError: " << e.what());
      }

      // Check post-condition.
      ASSERT(result.size() == skyCoord.size());

      // Return the result vector.
      return result;
    }


  } // namespace AMC
  
} // namespace LOFAR

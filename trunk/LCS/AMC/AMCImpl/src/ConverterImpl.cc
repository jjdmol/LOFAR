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
      // This vector will hold the result of the conversion operation.
      vector<SkyCoord> result;

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Create a dummy(?) astronomical direction. We (only) need it as
        // argument for the MDirection::Convert constructor.
        MDirection dir;

        // For each given moment in time ...
        for (uint i = 0; i < timeCoord.size(); i++) {

          // Set the instant in time in the frame.
          frame.set (MEpoch(MVEpoch(timeCoord[i].getDay(), 
                                    timeCoord[i].getFraction())));

          // For each given position on earth ...
          for (uint j = 0; j < earthCoord.size(); j++) {

            // Set the position on earth in the frame.
            MVPosition pos((Quantity(earthCoord[j].height(), "m")),
                           (Quantity(earthCoord[j].longitude(), "rad")),
                           (Quantity(earthCoord[j].latitude(), "rad")));
            frame.set (MPosition(pos));

            // Set-up the conversion engine, using reference direction AZEL.
            // \note Since we're only using \a dir as a dummy argument, we
            // might as well use a temporary MDirection object.
            MDirection::Convert 
              conv (dir, MDirection::Ref (MDirection::AZEL, frame));
//            conv (MDirection(), MDirection::Ref (MDirection::AZEL, frame));
            
            // For each given direction in the sky ...
            for (uint k = 0; k < skyCoord.size(); k++) {

              // Define the astronomical direction as a J2000 direction.
              MDirection sky((MVDirection(skyCoord[k].angle0(), 
                                          skyCoord[k].angle1())), 
                             MDirection::J2000);

              // Convert this direction, using the conversion engine.
              MDirection res = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = res.getAngle().getBaseValue();

              // Convert to local sky coordinates and add to the return vector.
              result.push_back(SkyCoord(angles(0), angles(1)));

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
      ASSERT (earthCoord.size() == timeCoord.size());
      if (earthCoord.size() != 1) {
        ASSERT (earthCoord.size() == skyCoord.size());
      }

      // This vector will hold the result of the conversion operation.
      vector<SkyCoord> result;

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Create a dummy(?) astronomical direction. We (only) need it as
        // argument for the MDirection::Convert constructor.
        MDirection dir;

        // If there's only one earthCoord and one timeCoord, we can preset
        // these data in the Measure frame; we only need to loop over all 
        // \a skyCoord values.
        if (timeCoord.size() == 1) {

          // Set the instant in time in the frame.
          frame.set (MEpoch(MVEpoch(timeCoord[0].getDay(), 
                                    timeCoord[0].getFraction())));

          // Set the position on earth in the frame.
          MVPosition pos((Quantity(earthCoord[0].height(), "m")),
                         (Quantity(earthCoord[0].longitude(), "rad")),
                         (Quantity(earthCoord[0].latitude(), "rad")));
          frame.set (MPosition(pos));

          // Set-up the reference frame, using reference direction AZEL.
          MDirection::Ref ref(MDirection::AZEL, frame);

          // Set-up the conversion engine, using reference direction J2000..
          MDirection::Convert conv (dir, MDirection::J2000);
//          conv (MDirection(), MDirection::Ref (MDirection::J2000, frame));
            
          // For each given direction in the sky ...
          for (uint i = 0; i < skyCoord.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MDirection sky((MVDirection(skyCoord[i].angle0(), 
                                        skyCoord[i].angle1())), 
                           ref);
            
            // Convert this direction, using the conversion engine.
            MDirection res = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = res.getAngle().getBaseValue();

            // Convert to local sky coordinates and add to the return vector.
            result.push_back(SkyCoord(angles(0), angles(1)));
            
          }

        }
        // We need to calculate the converted sky coordinate for each triplet
        // of \a skyCoord, \a earthCoord, and \a timeCoord.
        else {

          // For each triplet ...
          for (uint i = 0; i < skyCoord.size(); i++) {

            // Set the instant in time in the frame.
            frame.set (MEpoch(MVEpoch(timeCoord[i].getDay(), 
                                      timeCoord[i].getFraction())));
            
            // Set the position on earth in the frame.
            MVPosition pos((Quantity(earthCoord[i].height(), "m")),
                           (Quantity(earthCoord[i].longitude(), "rad")),
                           (Quantity(earthCoord[i].latitude(), "rad")));
            frame.set (MPosition(pos));

            // Set-up the reference frame, using reference direction AZEL.
            MDirection::Ref ref(MDirection::AZEL, frame);
            
            // Set-up the conversion engine, using reference direction J2000..
            MDirection::Convert conv (dir, MDirection::J2000);
//          conv (MDirection(), MDirection::Ref (MDirection::J2000, frame));
           
            // Define the astronomical direction w.r.t. the reference frame.
            MDirection sky((MVDirection(skyCoord[i].angle0(), 
                                        skyCoord[i].angle1())),
                           ref);
            
            // Convert this direction, using the conversion engine.
            MDirection res = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = res.getAngle().getBaseValue();

            // Convert to local sky coordinates and add to the return vector.
            result.push_back(SkyCoord(angles(0), angles(1)));
            
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

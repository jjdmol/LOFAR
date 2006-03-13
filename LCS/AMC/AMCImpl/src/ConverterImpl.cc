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
#include <AMCBase/Exceptions.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
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


    void 
    ConverterImpl::j2000ToAzel (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# skyCoord type must be J2000
      // -# earthCoord types must be equal
      for (uint i = 0; i < in.skyCoord.size(); ++i) {
        ASSERT(in.skyCoord[i].type() == SkyCoord::J2000);
      }
      for (uint i = 1; i < in.earthCoord.size(); ++i) {
        ASSERT(in.earthCoord[i].type() == in.earthCoord[0].type());
      }

      // If any of the input vectors has zero length, we can return
      // immediately.
      if (in.skyCoord.size()   == 0 || 
          in.earthCoord.size() == 0 || 
          in.timeCoord.size()  == 0)
        return;

      // Reserve space for the result to avoid resizing of the vector.
      out.skyCoord.reserve(in.skyCoord.size() * 
                           in.earthCoord.size() * 
                           in.timeCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(in.earthCoord[i].height(), "m")),
                            (Quantity(in.earthCoord[i].longitude(), "rad")),
                            (Quantity(in.earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(in.earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterException, "Invalid EarthCoord type: " 
              << in.earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.timeCoord[0].getDay(),
                                 in.timeCoord[0].getFraction())));
        
        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));
        
        // Set-up the conversion engine, using reference direction AZEL.
        MDirection::Convert conv (MDirection::J2000, 
                                  MDirection::Ref (MDirection::AZEL, frame));

        // For each given moment in time ...
        for (uint i = 0; i < in.timeCoord.size(); i++) {

          // Set the instant in time in the frame.
          frame.resetEpoch(MVEpoch(in.timeCoord[i].getDay(),
                                   in.timeCoord[i].getFraction()));
          
          // For each given position on earth ...
          for (uint j = 0; j < in.earthCoord.size(); j++) {

            // Set the position on earth in the frame.
            frame.resetPosition(pos[j]);
              
            // For each given direction in the sky ...
            for (uint k = 0; k < in.skyCoord.size(); k++) {

              // Define the astronomical direction as a J2000 direction.
              MVDirection sky(in.skyCoord[k].angle0(), 
                              in.skyCoord[k].angle1());

              // Convert this direction, using the conversion engine.
              MDirection dir = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = dir.getValue().get();

              // Convert to local sky coordinates and add to the return vector.
              out.skyCoord.push_back(SkyCoord(angles(0), angles(1), 
                                              SkyCoord::AZEL));

            }
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check on post-condition that the return vector contains 
      // <tt>skyCoord.size() * earthCoord.size() * timeCoord.size()</tt>
      // elements.
      ASSERT (out.skyCoord.size() == 
              in.skyCoord.size() * in.earthCoord.size() * in.timeCoord.size());

      return;
    }


    void
    ConverterImpl::azelToJ2000 (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# \a earthCoord and \a timeCoord must have equal sizes;
      // -# if \a earthCoord and \a timeCoord have sizes unequal to one, then
      //    their sizes must be equal to the size of \a skyCoord. Note that
      //    we only need to do one check, the other is implied by the first
      //    pre-condition.
      // -# skyCoord type must be AZEL
      // -# earthCoord types must be the equal
      ASSERT (in.earthCoord.size() == in.timeCoord.size());
      if (in.earthCoord.size() != 1) {
        ASSERT (in.earthCoord.size() == in.skyCoord.size());
      }
      for (uint i = 0; i < in.skyCoord.size(); ++i) {
        ASSERT (in.skyCoord[i].type() == SkyCoord::AZEL);
      }
      for (uint i = 1; i < in.earthCoord.size(); ++i) {
        ASSERT (in.earthCoord[i].type() == in.earthCoord[0].type());
      }

      // Reserve space for the result to avoid resizing of the vector.
      out.skyCoord.reserve(in.skyCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(in.earthCoord[i].height(), "m")),
                            (Quantity(in.earthCoord[i].longitude(), "rad")),
                            (Quantity(in.earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(in.earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterException, "Invalid EarthCoord type: " 
              << in.earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.timeCoord[0].getDay(), 
                                 in.timeCoord[0].getFraction())));

        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));

        // Set-up the conversion engine, using reference direction J2000..
        MDirection::Convert conv(MDirection::AZEL,
                                 MDirection::Ref(MDirection::J2000, frame));

        // If there's only one earthCoord and one timeCoord, we only need to
        // loop over all \a skyCoord values.
        if (in.timeCoord.size() == 1) {

          // For each given direction in the sky ...
          for (uint i = 0; i < in.skyCoord.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.skyCoord[i].angle0(), in.skyCoord[i].angle1()); 
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.skyCoord.push_back(SkyCoord(angles(0), angles(1), 
                                            SkyCoord::J2000));
          }
        }

        // We need to calculate the converted sky coordinate for each triplet
        // of \a skyCoord, \a earthCoord, and \a timeCoord.
        else {
          
          // For each triplet ...
          for (uint i = 0; i < in.skyCoord.size(); i++) {
            
            // Set the instant in time in the frame.
            frame.resetEpoch(MVEpoch(in.timeCoord[i].getDay(), 
                                     in.timeCoord[i].getFraction()));
            
            // Set the position on earth in the frame.
            frame.resetPosition(pos[i]);
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.skyCoord[i].angle0(), in.skyCoord[i].angle1());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.skyCoord.push_back(SkyCoord(angles(0), angles(1), 
                                            SkyCoord::J2000));
            
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check post-condition.
      ASSERT(out.skyCoord.size() == in.skyCoord.size());

      return;
    }
    
    
    void
    ConverterImpl::j2000ToItrf (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# skyCoord type must be J2000
      // -# earthCoord types must be equal
      for (uint i = 0; i < in.skyCoord.size(); ++i) {
        ASSERT(in.skyCoord[i].type() == SkyCoord::J2000);
      }
      for (uint i = 1; i < in.earthCoord.size(); ++i) {
        ASSERT(in.earthCoord[i].type() == in.earthCoord[0].type());
      }

      // If any of the input vectors has zero length, we can return
      // immediately.
      if (in.skyCoord.size()   == 0 || 
          in.earthCoord.size() == 0 || 
          in.timeCoord.size()  == 0)
        return;

      // Reserve space for the result to avoid resizing of the vector.
      out.skyCoord.reserve(in.skyCoord.size() * 
                              in.earthCoord.size() * 
                              in.timeCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(in.earthCoord[i].height(), "m")),
                            (Quantity(in.earthCoord[i].longitude(), "rad")),
                            (Quantity(in.earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(in.earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterException, "Invalid EarthCoord type: " 
              << in.earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.timeCoord[0].getDay(),
                                 in.timeCoord[0].getFraction())));
        
        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));
        
        // Set-up the conversion engine, using reference direction ITRF.
        MDirection::Convert conv (MDirection::J2000, 
                                  MDirection::Ref (MDirection::ITRF, frame));

        // For each given moment in time ...
        for (uint i = 0; i < in.timeCoord.size(); i++) {

          // Set the instant in time in the frame.
          frame.resetEpoch(MVEpoch(in.timeCoord[i].getDay(), 
                                   in.timeCoord[i].getFraction()));
          
          // For each given position on earth ...
          for (uint j = 0; j < in.earthCoord.size(); j++) {

            // Set the position on earth in the frame.
            frame.resetPosition(pos[j]);
            
            // For each given direction in the sky ...
            for (uint k = 0; k < in.skyCoord.size(); k++) {
              
              // Define the astronomical direction as a J2000 direction.
              MVDirection sky(in.skyCoord[k].angle0(), 
                              in.skyCoord[k].angle1());

              // Convert this direction, using the conversion engine.
              MDirection dir = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = dir.getValue().get();

              // Convert to local sky coordinates and add to the return vector.
              out.skyCoord.push_back(SkyCoord(angles(0), angles(1), 
                                              SkyCoord::ITRF));

            }
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check on post-condition that the return vector contains 
      // <tt>skyCoord.size() * earthCoord.size() * timeCoord.size()</tt>
      // elements.
      ASSERT (out.skyCoord.size() == 
              in.skyCoord.size() * in.earthCoord.size() * in.timeCoord.size());

      return;
    }


    void
    ConverterImpl::itrfToJ2000 (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# \a earthCoord and \a timeCoord must have equal sizes;
      // -# if \a earthCoord and \a timeCoord have sizes unequal to one, then
      //    their sizes must be equal to the size of \a skyCoord. Note that
      //    we only need to do one check, the other is implied by the first
      //    pre-condition.
      // -# skyCoord type must be ITRF
      // -# earthCoord types must be the equal
      ASSERT (in.earthCoord.size() == in.timeCoord.size());
      if (in.earthCoord.size() != 1) {
        ASSERT (in.earthCoord.size() == in.skyCoord.size());
      }
      for (uint i = 0; i < in.skyCoord.size(); ++i) {
        ASSERT (in.skyCoord[i].type() == SkyCoord::ITRF);
      }
      for (uint i = 1; i < in.earthCoord.size(); ++i) {
        ASSERT (in.earthCoord[i].type() == in.earthCoord[0].type());
      }

      // Reserve space for the result to avoid resizing of the vector.
      out.skyCoord.reserve(in.skyCoord.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.earthCoord.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition((Quantity(in.earthCoord[i].height(), "m")),
                            (Quantity(in.earthCoord[i].longitude(), "rad")),
                            (Quantity(in.earthCoord[i].latitude(), "rad")));
      }

      // Determine the position reference.
      MPosition::Types posref;
      switch(in.earthCoord[0].type()) {
      case EarthCoord::ITRF:  
        posref = MPosition::ITRF;  
        break;
      case EarthCoord::WGS84: 
        posref = MPosition::WGS84; 
        break;
      default: 
        THROW(ConverterException, "Invalid EarthCoord type: " 
              << in.earthCoord[0].type());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.timeCoord[0].getDay(), 
                                 in.timeCoord[0].getFraction())));

        // Set initial position for the frame, using the appropriate reference.
        frame.set(MPosition(pos[0], posref));

        // Set-up the conversion engine, using reference direction J2000..
        MDirection::Convert conv(MDirection::ITRF,
                                 MDirection::Ref(MDirection::J2000, frame));

        // If there's only one earthCoord and one timeCoord, we only need to
        // loop over all \a skyCoord values.
        if (in.timeCoord.size() == 1) {

          // For each given direction in the sky ...
          for (uint i = 0; i < in.skyCoord.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.skyCoord[i].angle0(), in.skyCoord[i].angle1());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.skyCoord.push_back(SkyCoord(angles(0), angles(1), 
                                            SkyCoord::J2000));
          }
        }

        // We need to calculate the converted sky coordinate for each triplet
        // of \a skyCoord, \a earthCoord, and \a timeCoord.
        else {

          // For each triplet ...
          for (uint i = 0; i < in.skyCoord.size(); i++) {

            // Set the instant in time in the frame.
            frame.resetEpoch(MVEpoch(in.timeCoord[i].getDay(), 
                                     in.timeCoord[i].getFraction()));
            
            // Set the position on earth in the frame.
            frame.resetPosition(pos[i]);
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.skyCoord[i].angle0(), in.skyCoord[i].angle1());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.skyCoord.push_back(SkyCoord(angles(0), angles(1), 
                                            SkyCoord::J2000));
            
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check post-condition.
      ASSERT(out.skyCoord.size() == in.skyCoord.size());

      return;
    }


  } // namespace AMC
  
} // namespace LOFAR

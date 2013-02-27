//# ConverterImpl.cc: one line description
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
#include <measures/Measures/MCDirection.h>
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
      // -# direction type must be J2000
      for (uint i = 0; i < in.direction.size(); ++i) {
        ASSERT(in.direction[i].type() == Direction::J2000);
      }

      // If any of the input vectors has zero length, we can return
      // immediately.
      if (in.direction.size()   == 0 || 
          in.position.size() == 0 || 
          in.epoch.size()  == 0)
        return;

      // Reserve space for the result to avoid resizing of the vector.
      out.direction.reserve(in.direction.size() * 
                            in.position.size() * 
                            in.epoch.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.position.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition(in.position[i].coord().get());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.epoch[0].getDay(),
                                 in.epoch[0].getFraction())));
        
        // Set initial position for the frame.
        frame.set(MPosition(pos[0], MPosition::ITRF));
        
        // Set-up the conversion engine, using reference direction AZEL.
        MDirection::Convert conv (MDirection::J2000, 
                                  MDirection::Ref (MDirection::AZEL, frame));

        // For each given moment in time ...
        for (uint i = 0; i < in.epoch.size(); i++) {

          // Set the instant in time in the frame.
          frame.resetEpoch(MVEpoch(in.epoch[i].getDay(),
                                   in.epoch[i].getFraction()));
          
          // For each given position on earth ...
          for (uint j = 0; j < in.position.size(); j++) {

            // Set the position on earth in the frame.
            frame.resetPosition(pos[j]);
              
            // For each given direction in the sky ...
            for (uint k = 0; k < in.direction.size(); k++) {

              // Define the astronomical direction as a J2000 direction.
              MVDirection sky(in.direction[k].coord().get());

              // Convert this direction, using the conversion engine.
              MDirection dir = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = dir.getValue().get();

              // Convert to local sky coordinates and add to the return vector.
              out.direction.push_back(Direction(angles(0), angles(1), 
                                                Direction::AZEL));
            }
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check on post-condition that the return vector contains 
      // <tt>direction.size() * position.size() * epoch.size()</tt>
      // elements.
      ASSERT (out.direction.size() == 
              in.direction.size() * in.position.size() * in.epoch.size());

      return;
    }


    void
    ConverterImpl::azelToJ2000 (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# \a position and \a epoch must have equal sizes;
      // -# if \a position and \a epoch have sizes unequal to one, then
      //    their sizes must be equal to the size of \a direction. Note that
      //    we only need to do one check, the other is implied by the first
      //    pre-condition.
      // -# direction type must be AZEL
      ASSERT (in.position.size() == in.epoch.size());
      if (in.position.size() != 1) {
        ASSERT (in.position.size() == in.direction.size());
      }
      for (uint i = 0; i < in.direction.size(); ++i) {
        ASSERT (in.direction[i].type() == Direction::AZEL);
      }

      // If the direction input vector has zero length, we can return
      // immediately.
      if (in.direction.size() == 0) return;

      // Reserve space for the result to avoid resizing of the vector.
      out.direction.reserve(in.direction.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.position.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition(in.position[i].coord().get());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.epoch[0].getDay(), 
                                 in.epoch[0].getFraction())));

        // Set initial position for the frame.
        frame.set(MPosition(pos[0], MPosition::ITRF));

        // Set-up the conversion engine, using reference direction J2000..
        MDirection::Convert conv(MDirection::AZEL,
                                 MDirection::Ref(MDirection::J2000, frame));

        // If there's only one position and one epoch, we only need to
        // loop over all \a direction values.
        if (in.epoch.size() == 1) {

          // For each given direction in the sky ...
          for (uint i = 0; i < in.direction.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.direction[i].coord().get());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.direction.push_back(Direction(angles(0), angles(1), 
                                              Direction::J2000));
          }
        }

        // We need to calculate the converted sky coordinate for each triplet
        // of \a direction, \a position, and \a epoch.
        else {
          
          // For each triplet ...
          for (uint i = 0; i < in.direction.size(); i++) {
            
            // Set the instant in time in the frame.
            frame.resetEpoch(MVEpoch(in.epoch[i].getDay(), 
                                     in.epoch[i].getFraction()));
            
            // Set the position on earth in the frame.
            frame.resetPosition(pos[i]);
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.direction[i].coord().get());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.direction.push_back(Direction(angles(0), angles(1), 
                                              Direction::J2000));
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check post-condition.
      ASSERT(out.direction.size() == in.direction.size());

      return;
    }
    
    
    void
    ConverterImpl::j2000ToItrf (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# direction type must be J2000
      for (uint i = 0; i < in.direction.size(); ++i) {
        ASSERT(in.direction[i].type() == Direction::J2000);
      }

      // If any of the input vectors has zero length, we can return
      // immediately.
      if (in.direction.size()   == 0 || 
          in.position.size() == 0 || 
          in.epoch.size()  == 0)
        return;

      // Reserve space for the result to avoid resizing of the vector.
      out.direction.reserve(in.direction.size() * 
                            in.position.size() * 
                            in.epoch.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.position.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition(in.position[i].coord().get());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.epoch[0].getDay(),
                                 in.epoch[0].getFraction())));
        
        // Set initial position for the frame.
        frame.set(MPosition(pos[0], MPosition::ITRF));
        
        // Set-up the conversion engine, using reference direction ITRF.
        MDirection::Convert conv (MDirection::J2000, 
                                  MDirection::Ref (MDirection::ITRF, frame));

        // For each given moment in time ...
        for (uint i = 0; i < in.epoch.size(); i++) {

          // Set the instant in time in the frame.
          frame.resetEpoch(MVEpoch(in.epoch[i].getDay(), 
                                   in.epoch[i].getFraction()));
          
          // For each given position on earth ...
          for (uint j = 0; j < in.position.size(); j++) {

            // Set the position on earth in the frame.
            frame.resetPosition(pos[j]);
            
            // For each given direction in the sky ...
            for (uint k = 0; k < in.direction.size(); k++) {
              
              // Define the astronomical direction as a J2000 direction.
              MVDirection sky(in.direction[k].coord().get());

              // Convert this direction, using the conversion engine.
              MDirection dir = conv(sky);

              // Retrieve the direction angles of the converted direction.
              Vector<Double> angles = dir.getValue().get();

              // Convert to local sky coordinates and add to the return vector.
              out.direction.push_back(Direction(angles(0), angles(1), 
                                                Direction::ITRF));
            }
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check on post-condition that the return vector contains 
      // <tt>direction.size() * position.size() * epoch.size()</tt>
      // elements.
      ASSERT (out.direction.size() == 
              in.direction.size() * in.position.size() * in.epoch.size());

      return;
    }


    void
    ConverterImpl::itrfToJ2000 (ResultData& out,
                                const RequestData& in)
    {
      // Initialize the result data
      out = ResultData();

      // Check pre-conditions.
      // -# \a position and \a epoch must have equal sizes;
      // -# if \a position and \a epoch have sizes unequal to one, then
      //    their sizes must be equal to the size of \a direction. Note that
      //    we only need to do one check, the other is implied by the first
      //    pre-condition.
      // -# direction type must be ITRF
      ASSERT (in.position.size() == in.epoch.size());
      if (in.position.size() != 1) {
        ASSERT (in.position.size() == in.direction.size());
      }
      for (uint i = 0; i < in.direction.size(); ++i) {
        ASSERT (in.direction[i].type() == Direction::ITRF);
      }

      // If the direction input vector has zero length, we can return
      // immediately.
      if (in.direction.size() == 0) return;

      // Reserve space for the result to avoid resizing of the vector.
      out.direction.reserve(in.direction.size());

      // Precalculate the positions on earth; they do not change with time.
      vector<MVPosition> pos(in.position.size());
      for (uint i = 0; i < pos.size(); i++) {
        pos[i] = MVPosition(in.position[i].coord().get());
      }

      try {

        // Create a container for the Measure frame.
        MeasFrame frame;

        // Set initial epoch for the frame, using UTC (default) as reference.
        frame.set(MEpoch(MVEpoch(in.epoch[0].getDay(), 
                                 in.epoch[0].getFraction())));

        // Set initial position for the frame.
        frame.set(MPosition(pos[0], MPosition::ITRF));

        // Set-up the conversion engine, using reference direction J2000..
        MDirection::Convert conv(MDirection::ITRF,
                                 MDirection::Ref(MDirection::J2000, frame));

        // If there's only one position and one epoch, we only need to
        // loop over all \a direction values.
        if (in.epoch.size() == 1) {

          // For each given direction in the sky ...
          for (uint i = 0; i < in.direction.size(); i++) {
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.direction[i].coord().get());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.direction.push_back(Direction(angles(0), angles(1), 
                                              Direction::J2000));
          }
        }

        // We need to calculate the converted sky coordinate for each triplet
        // of \a direction, \a position, and \a epoch.
        else {

          // For each triplet ...
          for (uint i = 0; i < in.direction.size(); i++) {

            // Set the instant in time in the frame.
            frame.resetEpoch(MVEpoch(in.epoch[i].getDay(), 
                                     in.epoch[i].getFraction()));
            
            // Set the position on earth in the frame.
            frame.resetPosition(pos[i]);
            
            // Define the astronomical direction w.r.t. the reference frame.
            MVDirection sky(in.direction[i].coord().get());
            
            // Convert this direction, using the conversion engine.
            MDirection dir = conv(sky);

            // Retrieve the direction angles of the converted direction.
            Vector<Double> angles = dir.getValue().get();

            // Convert to local sky coordinates and add to the return vector.
            out.direction.push_back(Direction(angles(0), angles(1), 
                                              Direction::J2000));
          }
        }
      }

      catch (AipsError& e) {
        THROW (ConverterException, "AipsError: " << e.what());
      }

      // Check post-condition.
      ASSERT(out.direction.size() == in.direction.size());

      return;
    }


  } // namespace AMC
  
} // namespace LOFAR

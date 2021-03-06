//# DPInput.h: Abstract base class for a DPStep generating input
//# Copyright (C) 2010
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
//#
//# @author Ger van Diepen

#ifndef DPPP_DPINPUT_H
#define DPPP_DPINPUT_H

// @file
// @brief Abstract base class for a DPStep generating input

#include <DPPP/DPStep.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/UVWCalculator.h>
#include <DPPP/FlagCounter.h>
#include <StationResponse/Station.h>
#include <Common/lofar_vector.h>

#include <tables/Tables/TableIter.h>
#include <tables/Tables/RefRows.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR {
  namespace DPPP {

    // @ingroup NDPPP

    // This class is the abstract base class for a DPStep object that
    // handles the input. A concrete example is MSReader that reads the
    // data from a MeasurementSet. However, it is also possible to have
    // input steps generating data on the fly as done in test programs
    // like tAverager.cc.
    //
    // A particular task of the class is to fetch the input for various
    // data items like weight, uvw, etc.. This is done by testing if the
    // item's data array is in the DPBuffer. If so, it will be returned.
    // Otherwise the appropriate 'get' function will be called to read the
    // data array from the input.
    // The derived classes should implement those 'get' functions, unless
    // they are sure the data arrays are always put in the buffer.

    class DPInput: public DPStep
    {
    public:
      virtual ~DPInput();

      // Read the UVW at the given row numbers into the buffer.
      // The default implementation throws an exception.
      virtual void getUVW (const casa::RefRows& rowNrs,
                           double time,
                           DPBuffer&);

      // Read the weights at the given row numbers into the buffer.
      // The default implementation throws an exception.
      virtual void getWeights (const casa::RefRows& rowNrs,
                               DPBuffer&);

      // Read the fullRes flags (LOFAR_FULL_RES_FLAG) at the given row numbers
      // into the buffer.
      // If undefined, false is returned.
      // The default implementation throws an exception.
      virtual bool getFullResFlags (const casa::RefRows& rowNrs,
                                    DPBuffer&);

      // Read the model data at the given row numbers into the array.
      // The default implementation throws an exception.
      virtual void getModelData (const casa::RefRows& rowNrs,
                                 casa::Cube<casa::Complex>&);

      // Get the MS name.
      // The default implementation returns an empty string.
      virtual casa::String msName() const;

      // Fill the vector with station beam info from the input source (MS).
      // Only fill it for the given station names.
      // The default implementation throws an exception.
      virtual void fillBeamInfo (vector<StationResponse::Station::Ptr>&,
                                 const casa::Vector<casa::String>& antNames);

      // Fetch the FullRes flags.
      // If defined in the buffer, they are taken from there.
      // Otherwise there are read from the input.
      // If not defined in the input, they are filled using the flags in the
      // buffer assuming that no averaging has been done so far.
      // <src>If desired, they can be merged with the buffer's FLAG which means
      // that if an averaged channel is flagged, the corresponding FullRes
      // flags are set.
      // <br>It does a stop/start of the timer when actually reading the data.
      const casa::Cube<bool>& fetchFullResFlags (const DPBuffer& bufin,
                                                 DPBuffer& bufout,
                                                 NSTimer& timer,
                                                 bool merge=false);

      // Fetch the weights.
      // If defined in the buffer, they are taken from there.
      // Otherwise there are read from the input.
      // If they have to be read and if autoweighting is in effect, the buffer
      // must contain DATA to calculate the weights.
      // <br>It does a stop/start of the timer when actually reading the data.
      const casa::Cube<float>& fetchWeights (const DPBuffer& bufin,
                                             DPBuffer& bufout,
                                             NSTimer& timer);

      // Fetch the UVW.
      // If defined in the buffer, they are taken from there.
      // Otherwise there are read from the input.
      // <br>It does a stop/start of the timer when actually reading the data.
      const casa::Matrix<double>& fetchUVW (const DPBuffer& bufin,
                                            DPBuffer& bufout,
                                            NSTimer& timer);
    };

  } //# end namespace
}

#endif

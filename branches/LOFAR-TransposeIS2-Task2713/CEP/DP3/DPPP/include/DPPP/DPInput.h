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
#include <tables/Tables/TableIter.h>
#include <tables/Tables/RefRows.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <Common/lofar_vector.h>

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

      // Read the UVW at the given row numbers.
      // The default implementation throws an exception.
      virtual casa::Matrix<double> getUVW (const casa::RefRows& rowNrs);

      // Read the weights at the given row numbers.
      // The default implementation throws an exception.
      virtual casa::Cube<float> getWeights (const casa::RefRows& rowNrs,
                                            const DPBuffer&);

      // Read the fullRes flags (LOFAR_FULL_RES_FLAG) at the given row numbers.
      // The default implementation throws an exception.
      virtual casa::Cube<bool> getFullResFlags (const casa::RefRows& rowNrs);

      // Read the given data column at the given row numbers.
      // The default implementation throws an exception.
      virtual casa::Cube<casa::Complex> getData
      (const casa::String& columnName, const casa::RefRows& rowNrs);

      // Get the MS name.
      // The default implementation returns an empty string.
      virtual casa::String msName() const;

      // Get info.
      double startTime() const
        { return itsStartTime; }
      uint ncorr() const
        { return itsNrCorr; }
      uint nchan() const
        { return itsNrChan; }
      uint nbaselines() const
        { return itsNrBl; }
      const casa::Vector<casa::Int>& getAnt1() const
        { return itsAnt1; }
      const casa::Vector<casa::Int>& getAnt2() const
        { return itsAnt2; }

      // Get the baseline table index of the autocorrelations.
      // A negative value means there are no autocorrelations for that antenna.
      const vector<int>& getAutoCorrIndex() const;

      // Get the lengths of the baselines (in meters).
      const vector<double>& getBaselineLengths() const;

      // Get the antenna names.
      const casa::Vector<casa::String>& antennaNames() const
        { return itsAntNames; }

      // Get the antenna positions.
      const vector<casa::MPosition>& antennaPos() const
        { return itsAntPos; }

      // Get the array position.
      const casa::MPosition& arrayPos() const
        { return itsArrayPos; }
      
      // Get the phase reference direction.
      const casa::MDirection& phaseCenter() const
        { return itsPhaseCenter; }

      // Get the delay center direction.
      const casa::MDirection& delayCenter() const
        { return itsDelayCenter; }

      // Get the tile beam direction.
      const casa::MDirection& tileBeamDir() const
        { return itsTileBeamDir; }

      // Get the channel frequencies.
      const casa::Vector<double>& chanFreqs() const
        { return itsChanFreqs; }

      // Get averaged channel frequencies.
      casa::Vector<double> chanFreqs (uint nchanAvg) const;

      // Fetch the FullRes flags.
      // If defined in the buffer, they are taken from there.
      // Otherwise there are read from the input.
      // If not defined in the input, they are filled using the flags in the
      // buffer assuming that no averaging has been done so far.
      // If defined, they can be merged with the buffer's flags which means
      // that if an averaged channel is flagged, the corresponding FullRes
      // flags are set.
      casa::Cube<bool> fetchFullResFlags (const DPBuffer& buf,
                                          const casa::RefRows& rowNrs,
                                          bool merge=false);

      // Fetch the weights.
      // If defined in the buffer, they are taken from there.
      // Otherwise there are read from the input.
      casa::Cube<float> fetchWeights (const DPBuffer& buf,
                                      const casa::RefRows& rowNrs);

      // Fetch the UVW.
      // If defined in the buffer, they are taken from there.
      // Otherwise there are read from the input.
      casa::Matrix<double> fetchUVW (const DPBuffer& buf,
                                     const casa::RefRows& rowNrs);

    protected:
      double itsStartTime;
      uint   itsNrChan;
      uint   itsNrCorr;
      uint   itsNrBl;
      casa::Vector<casa::Int>    itsAnt1;          //# ant1 of all baselines
      casa::Vector<casa::Int>    itsAnt2;          //# ant2 of all baselines
      mutable vector<double>     itsBLength;       //# baseline lengths
      mutable vector<int>        itsAutoCorrIndex; //# autocorr index per ant
      casa::Vector<casa::String> itsAntNames;
      vector<casa::MPosition>    itsAntPos;
      casa::MPosition            itsArrayPos;
      casa::MDirection           itsPhaseCenter;
      casa::MDirection           itsDelayCenter;
      casa::MDirection           itsTileBeamDir;
      casa::Vector<double>       itsChanFreqs;
      casa::Vector<double>       itsChanWidths;
    };

  } //# end namespace
}

#endif

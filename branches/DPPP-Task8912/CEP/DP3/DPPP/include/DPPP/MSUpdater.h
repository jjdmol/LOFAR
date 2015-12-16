//# MSUpdater.h: DPPP step writing to an MS
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

#ifndef DPPP_MSUPDATER_H
#define DPPP_MSUPDATER_H

// @file
// @brief DPPP step writing to an MS

#include <DPPP/DPStep.h>
#include <Common/LofarTypes.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/RefRows.h>
#include <tables/Tables/Table.h>

namespace LOFAR {

  class ParameterSet;

  namespace DPPP {
    //# Forward Declarations.
    class MSReader;

    // @ingroup NDPPP

    // This class updates the flags in an existing MeasurementSet.
    // Hardly anything is done in this class.
    // It uses function putFlags in MSReader to do the actual write.
    //
    // Like MSWriter it adds an entry to the HISTORY table of the MS
    // containing the parset values and DPPP version.

    class MSUpdater: public DPStep
    {
    public:
      MSUpdater (MSReader* reader, casa::String msName,
                const ParameterSet& parset, const std::string& prefix,
                bool writeHistory=true);

      virtual ~MSUpdater();

      // Process the next data chunk.
      // It returns false when at the end.
      virtual bool process (const DPBuffer&);

      // Finish the processing of this step and subsequent steps.
      virtual void finish();

      // Update the general info.
      virtual void updateInfo (const DPInfo&);

      // Add some data to the MeasurementSet written/updated.
      // Calls addToMS from the previous step, with the current output msname.
      virtual void addToMS (const string&);

      // Show the step parameters.
      virtual void show (std::ostream&) const;

      // Show the IO statistics.
      virtual void showIOStats (std::ostream&) const;

      // Show the timings.
      virtual void showTimings (std::ostream&, double duration) const;

      // Tests if an update of the buffer described in info to the MS msName
      // is possible. When throwError is true, it will throw an error with a
      // descriptive string before returning false
      static bool updateAllowed (const DPInfo& info, casa::String msName,
                                  bool throwError=true);

    private:
      // Write the flags at the given row numbers.
      void putFlags (const casa::RefRows& rowNrs,
                     const casa::Cube<bool>& flags);

      // Write the weights at the given row numbers
      void putWeights (const casa::RefRows& rowNrs,
                       const casa::Cube<float>& weights);

      // Write the data at the given row numbers.
      void putData (const casa::RefRows& rowNrs,
                    const casa::Cube<casa::Complex>& data);

      // If not existing yet, add the column specified by colname.
      // Column will containt arrays of type datatype.
      // If the column has been added, this function returns true
      bool addColumn(const string& colname, const casa::DataType dataType,
          const casa::ColumnDesc& cd);

      //# Data members
      MSReader*    itsReader;
      string       itsName;
      casa::String itsMSName;
      casa::Table  itsMS;
      const ParameterSet& itsParset;
      DPBuffer     itsBuffer;
      casa::String itsDataColName;
      casa::String itsWeightColName;
      uint         itsNrTimesFlush; //# flush every N time slots (0=no flush)
      bool         itsWriteData;
      bool         itsWriteWeights;
      bool         itsWriteFlags;
      uint         itsNrDone;       //# nr of time slots written
      bool         itsDataColAdded; //# has data column been added?
      bool         itsWeightColAdded; //# has weight column been added?
      bool         itsWriteHistory; //# Should history be written?
      NSTimer      itsTimer;
    };

  } //# end namespace
}

#endif

//# BufferSettings.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id: $

#ifndef LOFAR_INPUT_PROC_BUFFER_SETTINGS_H
#define LOFAR_INPUT_PROC_BUFFER_SETTINGS_H

#include <ostream>

#include <Common/LofarLogger.h>
#include "StationID.h"

namespace LOFAR
{
  namespace Cobalt
  {

    struct BufferSettings {
    private:
      static const unsigned currentVersion = 1;

      unsigned version;

      bool valid() const
      {
        return version == currentVersion;
      }

    public:
      struct StationID station;

      // true: sync reader and writer, useful in real-time mode
      bool sync;

      unsigned nrBeamletsPerBoard;

      size_t nrSamples;

      unsigned nrBoards;
      size_t nrAvailableRanges;

      key_t dataKey;

      BufferSettings();

      // if attach=true, read settings from shared memory, using the given stationID
      // if attach=false, set sane default values
      BufferSettings(const struct StationID &station, bool attach);

      // Shortcut to set nrSamples to represent `seconds' of buffer.
      void setBufferSize(double seconds);

      size_t boardIndex(unsigned beamlet) const
      {
        return beamlet / nrBeamletsPerBoard;
      }

      bool operator==(const struct BufferSettings &other) const
      {
        return station == other.station
               && sync == other.sync
               && nrBeamletsPerBoard == other.nrBeamletsPerBoard
               && nrSamples == other.nrSamples
               && nrBoards == other.nrBoards
               && nrAvailableRanges == other.nrAvailableRanges
               && dataKey == other.dataKey;
      }
    private:

      // Derive sane values from the station field.
      void deriveDefaultSettings();
    };

    std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s );

  }
}

#endif


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
//# $Id$

#ifndef LOFAR_INPUT_PROC_BUFFER_SETTINGS_H
#define LOFAR_INPUT_PROC_BUFFER_SETTINGS_H

#include <ostream>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <CoInterface/SparseSet.h>
#include <CoInterface/SlidingPointer.h>
#include "StationID.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class SyncLock;

    struct BufferSettings {
    private:
      static const unsigned currentVersion = 1;

      unsigned version;

      bool valid() const
      {
        return version == currentVersion;
      }

    public:
      typedef uint64 range_type;
      typedef SparseSet<range_type> flags_type;

      struct StationID station;

      // true: sync reader and writer, useful in real-time mode
      bool sync;
      SyncLock *syncLock;

      unsigned nrBeamletsPerBoard() const {
        // the number of beamlets scales with the bitmode:
        // 16-bit:  61
        //  8-bit: 122
        //  4-bit: 244
        switch (station.bitMode) {
          default:
          case 16:
            return nrBeamletsPerBoard_16bit;
          case 8:
            return nrBeamletsPerBoard_16bit << 1;
          case 4:
            return nrBeamletsPerBoard_16bit << 2;
        }
      }

      size_t nrSamples;

      unsigned nrBoards;
      size_t nrAvailableRanges;

      key_t dataKey;

      BufferSettings();

      // if attach=true, read settings from shared memory, using the given stationID
      // if attach=false, set sane default values
      BufferSettings(const struct StationID &station, bool attach, time_t timeout = 60);

      // Shortcut to set nrSamples to represent `seconds' of buffer.
      void setBufferSize(double seconds);

      size_t boardIndex(unsigned beamlet) const
      {
        return beamlet / nrBeamletsPerBoard();
      }

      bool operator==(const struct BufferSettings &other) const
      {
        return station == other.station
               && sync == other.sync
               && nrSamples == other.nrSamples
               && nrBoards == other.nrBoards
               && nrAvailableRanges == other.nrAvailableRanges
               && dataKey == other.dataKey;
      }
    private:

      // number of beamlets per RSP board in 16-bit mode.
      //
      // NOTE: this is actually the beamlet index increase between RSP boards.
      // Regardless of how many beamlets a packet actually carries, the second
      // RSP board starts sending from beamlet 61, leaving
      // a gap. For example, if each board sends 2 beamlets, then the beamlet
      // indices in the parset that can be used are:
      //
      // 0, 1, 61, 62, 122, 123, 183, 184.
      //
      // So it's best to leave nrBeamletsPerBoard_16bit at 61,
      // regardless of the number of beamlets contained in each packet.
      //
      // This value is hard-coded at the stations.
      static const unsigned nrBeamletsPerBoard_16bit = 61;

      // Derive sane values from the station field.
      void deriveDefaultSettings();
    };

    std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s );

    struct BoardLock {
      SlidingPointer<BufferSettings::range_type> readPtr;
      SlidingPointer<BufferSettings::range_type> writePtr;
    };

    class SyncLock: public std::vector<struct BoardLock> { // [board]
    public:
      SyncLock(const BufferSettings &settings)
      :
        std::vector<struct BoardLock>(settings.nrBoards)
      {
      }
    };
  }
}

#endif


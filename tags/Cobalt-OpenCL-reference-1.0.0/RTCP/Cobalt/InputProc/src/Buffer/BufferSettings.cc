//# BufferSettings.cc
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

#include <lofar_config.h>

#include "BufferSettings.h"

#include <Common/LofarLogger.h>
#include "SharedMemory.h"

namespace LOFAR
{
  namespace Cobalt
  {


    BufferSettings::BufferSettings()
      :
      version(currentVersion)
    {
    }

    BufferSettings::BufferSettings(const struct StationID &station, bool attach)
      :
      version(currentVersion),
      station(station)
    {
      if (attach) {
        do {
          SharedStruct<struct BufferSettings> shm(station.hash(), false);

          *this = shm.get();
        } while (!valid());

        ASSERT( valid() );
      } else {
        deriveDefaultSettings();
      }
    }


    void BufferSettings::deriveDefaultSettings()
    {
      sync = false;
      syncLock = 0;

      switch (station.bitMode) {
      default:
      case 16:
        nrBeamletsPerBoard = 61;
        break;

      case 8:
        nrBeamletsPerBoard = 122;
        break;

      case 4:
        nrBeamletsPerBoard = 244;
        break;
      }

      // 1 second buffer
      setBufferSize(1.0);

      nrBoards = 4;
      nrAvailableRanges = 64;

      dataKey = station.hash();
    }


    void BufferSettings::setBufferSize(double seconds)
    {
      // Make sure nrSamples is a multiple of 16, which
      // is the expected number of samples in a block.
      //
      // Doing so allows the writer to prevent split
      // writes of packets. (TODO: That's not implemented,
      // because the timestamps of the packets are not
      // necessarily a multiple of 16).
      nrSamples = static_cast<size_t>(seconds * (station.clockMHz * 1000000) / 1024) & ~0xFLL;
    }


    std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s )
    {
      str << s.station << " beamlets: " << (s.nrBoards * s.nrBeamletsPerBoard) << " buffer: " << (1.0 * s.nrSamples / s.station.clockMHz / 1000000 * 1024) << "s";

      return str;
    }


  }
}


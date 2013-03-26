/* SampleBuffer.h
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#ifndef LOFAR_INPUT_PROC_SAMPLEBUFFER_H
#define LOFAR_INPUT_PROC_SAMPLEBUFFER_H

#include <string>
#include <vector>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Allocator.h>
#include "BufferSettings.h"
#include "SharedMemory.h"
#include "Ranges.h"

namespace LOFAR
{
  namespace Cobalt
  {


    /*
     * Maintains a sample buffer in shared memory, which can be created
     * or attached to.
     *
     * The sample buffer contains the following information:
     *
     *   1. A copy of `settings', against which attaches are verified.
     *   2. A beamlets matrix [subband][sample]
     *   3. A flags vector [board]
     *
     * The IPC key used for the shared memory is settings.dataKey.
     */
    template<typename T>
    class SampleBuffer
    {
    public:
      // Create (create=true) or attach to (create=false) a sample buffer
      // in shared memory.
      SampleBuffer( const struct BufferSettings &settings, bool create );

    private:
      const std::string logPrefix;
      SharedMemoryArena data;
      SparseSetAllocator allocator;

      struct BufferSettings *initSettings( const struct BufferSettings &localSettings, bool create );

      static size_t dataSize( const struct BufferSettings &settings )
      {
        return sizeof settings
               + settings.nrBoards * (Ranges::size(settings.nrFlagRanges) + 8)
               + settings.nrBoards * settings.nrBeamletsPerBoard * (settings.nrSamples * sizeof(T) + 128);
      }

    public:
      struct BufferSettings *settings;

      const size_t nrBeamletsPerBoard;
      const size_t nrSamples;
      const size_t nrBoards;
      const size_t nrFlagRanges; // width of each flag range

      MultiDimArray<T,2>  beamlets; // [subband][sample]
      std::vector<Ranges> flags; // [board]
    };


    template<typename T>
    SampleBuffer<T>::SampleBuffer( const struct BufferSettings &_settings, bool create )
      :
      logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaField)),
      data(_settings.dataKey, dataSize(_settings), create ? SharedMemoryArena::CREATE : SharedMemoryArena::READ),
      allocator(data),
      settings(initSettings(_settings, create)),

      nrBeamletsPerBoard(settings->nrBeamletsPerBoard),
      nrSamples(settings->nrSamples),
      nrBoards(settings->nrBoards),
      nrFlagRanges(settings->nrFlagRanges),

      beamlets(boost::extents[nrBoards * nrBeamletsPerBoard][nrSamples], 128, allocator, false, false),
      flags(nrBoards)
    {
      for (size_t f = 0; f < flags.size(); f++) {
        size_t numBytes = Ranges::size(nrFlagRanges);

        flags[f] = Ranges(static_cast<int64*>(allocator.allocate(numBytes, 8)), numBytes, nrSamples, create);
      }

      LOG_INFO_STR( logPrefix << "Initialised" );
    }

    template<typename T>
    struct BufferSettings *SampleBuffer<T>::initSettings( const struct BufferSettings &localSettings, bool create )
    {
      struct BufferSettings *sharedSettings = allocator.allocateTyped();

      if (create) {
        // register settings
        LOG_INFO_STR( logPrefix << "Registering " << localSettings.station );
        *sharedSettings = localSettings;
      } else {
        // verify settings
        ASSERT( *sharedSettings == localSettings );
        LOG_INFO_STR( logPrefix << "Connected to " << localSettings.station );
      }

      return sharedSettings;
    }

  }
}

#endif


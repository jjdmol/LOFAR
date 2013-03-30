//# SampleBuffer.h
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

#ifndef LOFAR_INPUT_PROC_SAMPLEBUFFER_H
#define LOFAR_INPUT_PROC_SAMPLEBUFFER_H

#include <string>
#include <vector>

#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/RSPTimeStamp.h>
#include "BufferSettings.h"
#include "SharedMemory.h"
#include "SlidingPointer.h"
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
     *   2. A beamlets matrix. [subband][sample]
     *   3. A boards vector. [board]
     *
     * The IPC key used for the shared memory is settings.dataKey.
     *
     * The buffer can run in synchronised mode (settings.sync = true), in which case
     * we support exactly one writer and one reader to run in sync through
     * the Boards's functions. The writer synchronisation makes sure that
     * no data is being overwritten that is still in the reader's future. The
     * reader synchronisation makes sure that the reader will wait for the
     * writer to complete writing the requested range.
     */
    template<typename T>
    class SampleBuffer
    {
    public:
      // Create (create=true) or attach to (create=false) a sample buffer
      // in shared memory. If sync=true, readers and writers are kept in sync,
      // which is useful in the non-real-time mode, but only supports one
      // reader as well as in-order data arrival.
      SampleBuffer( const struct BufferSettings &settings, bool create );

    private:
      const std::string logPrefix;
      SharedMemoryArena data;
      SparseSetAllocator allocator;

      struct BufferSettings *initSettings( const struct BufferSettings &localSettings, bool create );

      static size_t dataSize( const struct BufferSettings &settings );

    public:
      struct BufferSettings *settings;

      // Keep readers/writers in sync
      const bool sync;

      const size_t nrBeamletsPerBoard;
      const size_t nrSamples;
      const size_t nrBoards;
      const size_t nrAvailableRanges; // width of each available range

      MultiDimArray<T,2>  beamlets; // [subband][sample]

      size_t offset( const TimeStamp &timestamp ) const { return (int64)timestamp % nrSamples; }

      class Board {
      public:
        Board( SampleBuffer<T> &buffer );

        Ranges available;

        // Signal start of write intent for data in [begin, end). The flags will be updated
        // for any data that will be overwritten, but not set for any data that is
        // written.
        void startWrite( const TimeStamp &begin, const TimeStamp &end );

        // Signal stop of write intent at end.
        void stopWrite( const TimeStamp &end );

        // Signal end-of-data (we're done writing).
        void noMoreWriting();

        // Signal start of read intent for data in [begin, end). Waits for data to arrive
        // until or after `end'.
        void startRead( const TimeStamp &begin, const TimeStamp &end );

        // Signal release of data before end, thus allowing it to be overwritten by newer data.
        void stopRead( const TimeStamp &end );

        // Signal that we're done reading.
        void noMoreReading();

      private:
        SampleBuffer<T> &buffer;

        // Read/write pointers to keep readers and writers in sync
        // if buffer.sync == true. The pointers assume that data will both be read
        // and written in-order.
        SlidingPointer<BufferSettings::range_type> readPtr;
        SlidingPointer<BufferSettings::range_type> writePtr;
      };

      std::vector<Board> boards;
    };
  }
}

#include "SampleBuffer.tcc"

#endif


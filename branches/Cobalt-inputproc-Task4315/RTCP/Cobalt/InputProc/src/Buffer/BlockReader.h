/* BlockReader.h
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

#ifndef LOFAR_INPUT_PROC_SAMPLE_BUFFER_READER_H
#define LOFAR_INPUT_PROC_SAMPLE_BUFFER_READER_H

#include <string>
#include <vector>

#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/SmartPtr.h>

#include <WallClockTime.h>

#include "BufferSettings.h"
#include "SampleBuffer.h"


namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * An abstract class for the implementation of a reader for SampleBuffers.
     */
    template<typename T>
    class BlockReader
    {
    public:
      // Initialise a block delivery system for the given buffer and beamlets.
      //
      // beamlets:
      //   the set of station beamlets to send, out of
      //   [0, settings.nrBeamletsPerBoard * settings.nrBoards)
      // maxDelay:
      //   the time (seconds) to wait for data to arrive (in real-time mode).
      BlockReader( const BufferSettings &settings, const std::vector<size_t> beamlets, double maxDelay = 0.0 );
      ~BlockReader();

      struct Block {
      private:
        BlockReader<T> &reader;

      public:
        TimeStamp from;
        TimeStamp to;

        struct Beamlet {
          // Actual beamlet number on station
          unsigned stationBeamlet;

          // Copy as one or two ranges of [from, to).
          struct Range {
            const T* from;
            const T* to;
          } ranges[2];

          unsigned nrRanges;

          // The offset at which the data is accessed.
          ssize_t offset;

          SparseSet<int64> flagsAtBegin;
        };

        std::vector<struct Beamlet> beamlets; // [beamlet]

        /*
         * Read the flags for a specific beamlet. Readers should read the flags
         * after reading the data. The valid data is then indicated by
         * the intersection of (beamlets[i].flagsAtBegin & flags(i))
        */
        SparseSet<int64> flags( size_t beamletIdx ) const;

        ~Block();

      private:
        Block(BlockReader<T> &reader, const TimeStamp &from, const TimeStamp &to);
        Block(const Block&);

        struct Beamlet getBeamlet( size_t beamletIdx );

        friend class BlockReader<T>;
      };

      SmartPtr<struct Block> block( const TimeStamp &from, const TimeStamp &to );

    protected:
      const BufferSettings settings;
      SampleBuffer<T> buffer;

      const std::vector<size_t> beamlets;

      /*
       * Provide the offset in samples for a certain beamlet, based on the
       * geometric delays for the respective subband.
       */
      virtual ssize_t beamletOffset( size_t beamletIdx, const TimeStamp &from, const TimeStamp &to )
      {
        (void)beamletIdx;
        (void)from;
        (void)to;
        return 0;
      }

      friend class Block;

    private:
      const TimeStamp maxDelay;

      WallClockTime waiter;
    };

  }
}

#include "BlockReader.tcc"

#endif


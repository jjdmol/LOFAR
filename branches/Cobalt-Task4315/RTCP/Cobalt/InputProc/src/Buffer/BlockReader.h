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

#ifndef LOFAR_INPUT_PROC_BLOCK_READER_H
#define LOFAR_INPUT_PROC_BLOCK_READER_H

#include <string>
#include <vector>

#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/SmartPtr.h>

#include <InputProc/WallClockTime.h>

#include <InputProc/Buffer/BufferSettings.h>
#include <InputProc/Buffer/SampleBuffer.h>
#include <InputProc/Buffer/Block.h>


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
      //   the set of station beamlets to read, out of
      //   [0, settings.nrBeamletsPerBoard * settings.nrBoards)
      // maxDelay:
      //   the time (seconds) to wait for data to arrive (in real-time mode).
      BlockReader( const BufferSettings &settings, const std::vector<size_t> beamlets, size_t nrHistorySamples = 0, double maxDelay = 0.0 );
      ~BlockReader();

      // The LockedBlock locks the SampleBuffer for reading until destruction,
      // allowing non-real-time operation.
      struct LockedBlock: public Block<T> {
        virtual ~LockedBlock();

        /*
         * Read the flags for a specific beamlet. Readers should read the flags
         * after reading the data. The valid data is then indicated by
         * the intersection of (beamlets[i].flagsAtBegin & flags(i))
        */
        virtual SparseSet<uint64> flags( size_t beamletIdx ) const;

      private:
        LockedBlock( BlockReader<T> &reader, const TimeStamp &from, const TimeStamp &to, const std::vector<ssize_t> &beamletOffsets );
        LockedBlock( const LockedBlock& );

        BlockReader<T> &reader;

        struct Block<T>::Beamlet getBeamlet( size_t beamletIdx, ssize_t offset );

        friend class BlockReader<T>;
      };

      // Returns information for copying the block [from - nrHistorySamples - offset, to - offset).
      // The Block's from and to fields do not take the offset into account.
      SmartPtr<struct LockedBlock> block( const TimeStamp &from, const TimeStamp &to, const std::vector<ssize_t> &beamletOffsets );

    protected:
      const BufferSettings settings;
      SampleBuffer<T> buffer;

      const std::vector<size_t> beamlets;
      const size_t nrHistorySamples;

    private:
      const TimeStamp maxDelay;

      WallClockTime waiter;

      friend class LockedBlock;
    };

  }
}

#include <InputProc/Buffer/BlockReader.tcc>

#endif


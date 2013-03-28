/* BlockReader.tcc
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

#include <mpi.h>

#include <Common/LofarLogger.h>

namespace LOFAR {

  namespace Cobalt {

    template<typename T>
    BlockReader<T>::BlockReader( const BufferSettings &settings, const std::vector<size_t> beamlets, size_t nrHistorySamples, double maxDelay )
    :
      settings(settings),
      buffer(settings, false),

      beamlets(beamlets),
      nrHistorySamples(nrHistorySamples),
      maxDelay(static_cast<int64>(maxDelay * settings.station.clockMHz * 1000000 / 1024), settings.station.clockMHz * 1000000)
    {
      // Check whether the selected beamlets exist
      size_t nrBeamlets = settings.nrBoards * settings.nrBeamletsPerBoard;

      for (size_t i = 0; i < beamlets.size(); ++i)
        ASSERT( beamlets[i] < nrBeamlets );
    }


    template<typename T>
    BlockReader<T>::~BlockReader()
    {
      // signal end of reading
      for( typename std::vector< typename SampleBuffer<T>::Board >::iterator board = buffer.boards.begin(); board != buffer.boards.end(); ++board ) {
        (*board).noMoreReading();
      }
    }


    template<typename T>
    SparseSet<int64> BlockReader<T>::LockedBlock::flags( size_t beamletIdx ) const
    {
      const struct Block<T>::Beamlet &ib = this->beamlets[beamletIdx];

      // Determine corresponding RSP board
      size_t boardIdx = reader.settings.boardIndex(ib.stationBeamlet);

      ssize_t beam_offset = ib.offset;

      // Translate available packets to missing packets.
      const TimeStamp from = this->from + beam_offset;
      const TimeStamp to   = this->to   + beam_offset;
      return reader.buffer.boards[boardIdx].available.sparseSet(from, to).invert(from, to);
    }


    template<typename T>
    BlockReader<T>::LockedBlock::LockedBlock( BlockReader<T> &reader, const TimeStamp &from, const TimeStamp &to, const std::vector<ssize_t> &beamletOffsets )
    :
      reader(reader)
    {
      this->from = from;
      this->to   = to;

      ASSERT(beamletOffsets.size() == reader.beamlets.size());

      // fill static beamlet info
      this->beamlets.resize(reader.beamlets.size());
      for (size_t i = 0; i < this->beamlets.size(); ++i) {
        this->beamlets[i] = getBeamlet(i, beamletOffsets[i]);
      }

      // signal read intent on all buffers
      for( typename std::vector< typename SampleBuffer<T>::Board >::iterator board = reader.buffer.boards.begin(); board != reader.buffer.boards.end(); ++board ) {
        (*board).startRead(this->from, this->to);
      }

      // record initial flags
      for (size_t i = 0; i < this->beamlets.size(); ++i) {
        this->beamlets[i].flagsAtBegin = flags(i);
      }
    }


    template<typename T>
    struct Block<T>::Beamlet BlockReader<T>::LockedBlock::getBeamlet( size_t beamletIdx, ssize_t offset )
    {
      // Create instructions for copying this beamlet
      typename Block<T>::Beamlet b;

      // Cache the actual beam number at the station
      b.stationBeamlet = reader.beamlets[beamletIdx];
      
      // Store the sample offset with which this beamlet is read
      b.offset = offset;

      // Determine the relevant offsets in the buffer, processing:
      //   offset: the shift applied to compensate geometric delays (etc)
      //   reader.nrHistorySamples: the number of past samples to include (for
      //                            PPF initialisation)
      size_t from_offset = reader.buffer.offset(this->from + offset - reader.nrHistorySamples);
      size_t to_offset   = reader.buffer.offset(this->to   + offset);

      if (to_offset == 0)
        // we need the other end, actually
        to_offset = reader.buffer.nrSamples;

      // Determine whether we need to wrap around the end of the buffer
      size_t wrap_offset = from_offset < to_offset ? 0 : reader.buffer.nrSamples - from_offset;

      const T* origin = &reader.buffer.beamlets[b.stationBeamlet][0];

      if (wrap_offset > 0) {
        // Copy as two parts
        b.nrRanges = 2;

        b.ranges[0].from = origin + from_offset;
        b.ranges[0].to   = origin + reader.buffer.nrSamples;

        b.ranges[1].from = origin;
        b.ranges[1].to   = origin + to_offset;
      } else {
        // Copy as one part
        b.nrRanges = 1;

        b.ranges[0].from = origin + from_offset;
        b.ranges[0].to   = origin + to_offset;
      }

      return b;
    }


    template<typename T>
    BlockReader<T>::LockedBlock::~LockedBlock()
    {
      // Signal end of read intent on all buffers
      for( typename std::vector< typename SampleBuffer<T>::Board >::iterator board = reader.buffer.boards.begin(); board != reader.buffer.boards.end(); ++board ) {
        // Unlock data, saving nrHistorySamples for the next block
        (*board).stopRead(this->to - reader.nrHistorySamples);
      }
    }


    template<typename T>
    SmartPtr<typename BlockReader<T>::LockedBlock> BlockReader<T>::block( const TimeStamp &from, const TimeStamp &to, const std::vector<ssize_t> &beamletOffsets )
    {
      ASSERT( to - from < (int64)buffer.nrSamples );

      // wait for block start (but only in real-time mode)
      if (!buffer.sync) {
        LOG_INFO_STR("Waiting until " << (to + maxDelay) << " for " << from << " to " << to);
        waiter.waitUntil(to + maxDelay);
      }

      return new LockedBlock(*this, from, to, beamletOffsets);
    }

  }
}


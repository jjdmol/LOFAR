/* SampleBufferReader.tcc
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


template<typename T> SampleBufferReader<T>::SampleBufferReader( const BufferSettings &settings, const std::vector<size_t> beamlets, const TimeStamp &from, const TimeStamp &to, size_t blockSize, size_t nrHistorySamples )
:
  settings(settings),
  buffer(settings, false),

  beamlets(beamlets),
  from(from),
  to(to),
  blockSize(blockSize),
  nrHistorySamples(nrHistorySamples)
{
  size_t nrBeamlets = settings.nrBoards * settings.nrBeamletsPerBoard;

  for (size_t i = 0; i < beamlets.size(); ++i)
    ASSERT( beamlets[i] < nrBeamlets );

  ASSERT( blockSize > 0 );
  ASSERT( blockSize < settings.nrSamples );
  ASSERT( blockSize > nrHistorySamples );
  ASSERT( from < to );
}


template<typename T> void SampleBufferReader<T>::process( double maxDelay )
{
  const TimeStamp maxDelay_ts(static_cast<int64>(maxDelay * settings.station.clockMHz * 1000000 / 1024) + blockSize, settings.station.clockMHz * 1000000);

  const TimeStamp current(from);
  const size_t increment = blockSize - nrHistorySamples;

#if 1
  for (TimeStamp current = from; current + blockSize < to; current += increment) {
    // wait
    LOG_INFO_STR("Waiting until " << (current + maxDelay_ts) << " for " << current);
    waiter.waitUntil( current + maxDelay_ts );

    // read
    LOG_INFO_STR("Reading from " << current << " to " << (current + blockSize));
    sendBlock(current - nrHistorySamples, current - nrHistorySamples + blockSize);
  }
#else
  double totalwait = 0.0;
  unsigned totalnr = 0;

  double lastreport = MPI_Wtime();

  for (TimeStamp current = from; current < to; current += increment) {
    // wait
    waiter.waitUntil( current + maxDelay_ts );

    // read
    double bs = MPI_Wtime();

    sendBlock(current - nrHistorySamples, current - nrHistorySamples + blockSize);

    totalwait += MPI_Wtime() - bs;
    totalnr++;

    if (bs - lastreport > 1.0) {
      double mbps = (sizeof(T) * blockSize * beamlets.size() * 8) / (totalwait/totalnr) / 1e6;
      lastreport = bs;
      totalwait = 0.0;
      totalnr = 0;

      LOG_INFO_STR("Reading speed: " << mbps << " Mbit/s");
    }
  }
#endif

  for( typename std::vector< typename SampleBuffer<T>::Board >::iterator board = buffer.boards.begin(); board != buffer.boards.end(); ++board ) {
    (*board).noMoreReading();
  }

  LOG_INFO("Done reading data");
}


template<typename T> void SampleBufferReader<T>::sendBlock( const TimeStamp &from, const TimeStamp &to )
{
  ASSERT( from < to );
  ASSERT( to - from < (int64)buffer.nrSamples );

  // Create instructions for copying this block
  struct CopyInstructions info;
  info.from = from;
  info.to   = to;
  info.beamlets.resize(beamlets.size());

  for (size_t i = 0; i < beamlets.size(); ++i) {
    unsigned b = beamlets[i];
    struct CopyInstructions::Beamlet &ib = info.beamlets[i];
    
    // Determine the offset with which this beamlet is read (likely based on
    // the relevant station beam).
    ssize_t offset = beamletOffset(i, from, to);

    ib.offset = offset;

    // Determine the relevant offsets in the buffer
    size_t from_offset = buffer.offset(info.from + offset);
    size_t to_offset   = buffer.offset(info.to   + offset);

    if (to_offset == 0)
      to_offset = buffer.nrSamples;

    // Determine whether we need to wrap around the end of the buffer
    size_t wrap_offset = from_offset < to_offset ? 0 : buffer.nrSamples - from_offset;

    const T* origin = &buffer.beamlets[b][0];

    if (wrap_offset > 0) {
      // Copy as two parts
      ib.nrRanges = 2;

      ib.ranges[0].from = origin + from_offset;
      ib.ranges[0].to   = origin + buffer.nrSamples;

      ib.ranges[1].from = origin;
      ib.ranges[1].to   = origin + to_offset;
    } else {
      // Copy as one part
      ib.nrRanges = 1;

      ib.ranges[0].from = origin + from_offset;
      ib.ranges[0].to   = origin + to_offset;
    }

    ib.flagsAtBegin = flags(info, b);
  }

  // Signal read intent on all buffers
  for( typename std::vector< typename SampleBuffer<T>::Board >::iterator board = buffer.boards.begin(); board != buffer.boards.end(); ++board ) {
    (*board).startRead(from, to);
  }

  sendBlock(info);

  // Signal end of read intent on all buffers, reserving nrHistorySamples for the
  // next read.
  for( typename std::vector< typename SampleBuffer<T>::Board >::iterator board = buffer.boards.begin(); board != buffer.boards.end(); ++board ) {
    (*board).stopRead(to - nrHistorySamples);
  }
}


template<typename T> SparseSet<int64> SampleBufferReader<T>::flags( const struct CopyInstructions &info, unsigned beamlet )
{
  // Determine corresponding RSP board
  size_t boardIdx = settings.boardIndex(beamlet);

  ssize_t beam_offset = info.beamlets[beamlet].offset;

  // Translate available packets to missing packets.
  return buffer.boards[boardIdx].available.sparseSet(info.from + beam_offset, info.to + beam_offset).invert(info.from + beam_offset, info.to + beam_offset);
}

}
}


#include <Common/LofarLogger.h>
#include "WallClockTime.h"
#include <Interface/RSPTimeStamp.h>
#include "BufferSettings.h"
#include "SampleBuffer.h"

#include <mpi.h>
#include <vector>
#include <string>

namespace LOFAR {
namespace RTCP {


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
  for (size_t i = 0; i < beamlets.size(); ++i)
    ASSERT( beamlets[i] < buffer.nrBeamlets );

  ASSERT( blockSize > 0 );
  ASSERT( blockSize < settings.nrSamples );
  ASSERT( from < to );
}


template<typename T> void SampleBufferReader<T>::process( double maxDelay )
{
  const TimeStamp maxDelay_ts(static_cast<int64>(maxDelay * settings.station.clock / 1024) + blockSize, settings.station.clock);

  const TimeStamp current(from);
  const size_t increment = blockSize - nrHistorySamples;

  /*
  for (TimeStamp current = from; current < to; current += increment) {
    // wait
    LOG_INFO_STR("Waiting until " << (current + maxDelay_ts) << " for " << current);
    waiter.waitUntil( current + maxDelay_ts );

    // read
    LOG_INFO_STR("Reading from " << current << " to " << (current + blockSize));
    copy(current - nrHistorySamples, current - nrHistorySamples + blockSize);
  }

  LOG_INFO("Done reading data");*/

  double totalwait = 0.0;
  unsigned totalnr = 0;

  double lastreport = MPI_Wtime();

  for (TimeStamp current = from; current < to; current += increment) {
    // wait
    waiter.waitUntil( current + maxDelay_ts );

    // read
    double bs = MPI_Wtime();

    copy(current - nrHistorySamples, current - nrHistorySamples + blockSize);

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

  LOG_INFO("Done reading data");
}


template<typename T> void SampleBufferReader<T>::copy( const TimeStamp &from, const TimeStamp &to )
{
  ASSERT( from < to );
  ASSERT( to - from < (int64)buffer.nrSamples );

  struct CopyInstructions info;
  info.from = from;
  info.to   = to;

  // Determine the buffer offsets for all beamlets. Because beamlets can belong
  // to different station beams, the offsets can differ per beamlet.
  std::vector<ssize_t> beam_offsets(beamlets.size());
  std::vector<size_t> from_offsets(beamlets.size());
  std::vector<size_t> to_offsets(beamlets.size());
  std::vector<size_t> wrap_offsets(beamlets.size());

  for (size_t i = 0; i < beamlets.size(); ++i) {
    ssize_t offset = beamletOffset(i, from, to);

    beam_offsets[i] = offset;

    // Determine the relevant offsets in the buffer
    from_offsets[i] = (info.from + offset) % buffer.nrSamples;
    to_offsets[i]   = (info.to   + offset) % buffer.nrSamples;

    if (to_offsets[i] == 0)
      to_offsets[i] = buffer.nrSamples;

    // Determine whether we need to wrap around the end of the buffer
    wrap_offsets[i] = from_offsets[i] < to_offsets[i] ? 0 : buffer.nrSamples - from_offsets[i];

  }

  // Signal start of block
  copyStart(from, to, wrap_offsets);

  // Copy all specified beamlets
  for (size_t i = 0; i < beamlets.size(); ++i) {
    unsigned nr = beamlets[i];
    const T* origin = &buffer.beamlets[nr][0];

    ssize_t beam_offset = beam_offsets[i];
    size_t from_offset = from_offsets[i];
    size_t wrap_offset = wrap_offsets[i];
    size_t to_offset   = to_offsets[i];

    info.beamlet        = i;

    if (wrap_offset > 0) {
      // Copy as two parts
      info.nrRanges = 2;

      info.ranges[0].from = origin + from_offset;
      info.ranges[0].to   = origin + wrap_offset;

      info.ranges[1].from = origin;
      info.ranges[1].to   = origin + to_offset;
    } else {
      // Copy as one part
      info.nrRanges = 1;

      info.ranges[0].from = origin + from_offset;
      info.ranges[0].to   = origin + to_offset;
    }

    // Add the flags (translate available packets to missing packets)
    size_t flagRange = settings.flagRange(i);

    info.flags = buffer.flags[flagRange].sparseSet(from + beam_offset, to + beam_offset).invert(from + beam_offset, to + beam_offset);

    // Copy the beamlet
    copy(info);
  }

  // Signal end of block
  copyEnd(from, to);
}

}
}


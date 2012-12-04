#ifndef __SAMPLEBUFFERREADER__
#define __SAMPLEBUFFERREADER__

#include <Common/LofarLogger.h>
#include <WallClockTime.h>
#include <Interface/RSPTimeStamp.h>
#include "StationSettings.h"
#include "SampleBuffer.h"

#include <mpi.h>
#include <vector>
#include <string>

using namespace LOFAR;
using namespace RTCP;

namespace LOFAR {
namespace RTCP {

template<typename T> class SampleBufferReader {
public:
  SampleBufferReader( const StationSettings &settings, const std::vector<size_t> beamlets, const TimeStamp &from, const TimeStamp &to, size_t blockSize );

  void process( double maxDelay );

protected:
  const StationSettings settings;
  SampleBuffer<T> buffer;

  const std::vector<size_t> beamlets;
  const TimeStamp from, to;
  const size_t blockSize;

  virtual void copyNothing( const TimeStamp &from, const TimeStamp &to ) { (void)from, (void)to; }

  virtual void copyBeamlet( unsigned beamlet, unsigned transfer, const TimeStamp &from_ts, const T* from, size_t nrSamples ) = 0;
  virtual void copyStart( const TimeStamp &from, const TimeStamp &to, size_t wrap ) { (void)from, (void)to, (void)wrap; }

  virtual void copyFlags  ( unsigned transfer, const SparseSet<int64> &flags ) = 0;
  virtual void copyEnd() {}

  void copy( const TimeStamp &from, const TimeStamp &to );

private:
  WallClockTime waiter;
};


template<typename T> SampleBufferReader<T>::SampleBufferReader( const StationSettings &settings, const std::vector<size_t> beamlets, const TimeStamp &from, const TimeStamp &to, size_t blockSize )
:
  settings(settings),
  buffer(settings, false),

  beamlets(beamlets),
  from(from),
  to(to),
  blockSize(blockSize)
{
  for (size_t i = 0; i < beamlets.size(); ++i)
    ASSERT( beamlets[i] < buffer.nrBeamlets );

  ASSERT( blockSize > 0 );
  ASSERT( blockSize < settings.nrSamples );
  ASSERT( from < to );
}


template<typename T> void SampleBufferReader<T>::process( double maxDelay )
{
  /*const TimeStamp maxDelay_ts(static_cast<int64>(maxDelay * settings.station.clock / 1024) + blockSize, settings.station.clock);

  const TimeStamp current(from);

  for (TimeStamp current = from; current < to; current += blockSize) {
    // wait
    LOG_INFO_STR("Waiting until " << (current + maxDelay_ts) << " for " << current);
    waiter.waitUntil( current + maxDelay_ts );

    // read
    LOG_INFO_STR("Reading from " << current << " to " << (current + blockSize));
    copy(current, current + blockSize);
  }

  LOG_INFO("Done reading data");*/
  const TimeStamp maxDelay_ts(static_cast<int64>(maxDelay * settings.station.clock / 1024) + blockSize, settings.station.clock);

  const TimeStamp current(from);

  double totalwait = 0.0;
  unsigned totalnr = 0;

  double lastreport = MPI_Wtime();

  for (TimeStamp current = from; current < to; current += blockSize) {
    // wait
    waiter.waitUntil( current + maxDelay_ts );

    // read
    double bs = MPI_Wtime();

    copy(current, current + blockSize);

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

  const unsigned nrBoards = buffer.flags.size();

#if 0
  // check whether there is any data at all
  bool data = false;

  for (unsigned b = 0; b < nrBoards; ++b)
    if (buffer.flags[b].anythingBetween(from, to)) {
      data = true;
      break;
    }

  if (!data) {
    copyNothing(from, to);
    return;
  }
#endif

  // copy the beamlets

  size_t from_offset = (int64)from % buffer.nrSamples;
  size_t to_offset   = (int64)to % buffer.nrSamples;

  if (to_offset == 0)
    to_offset = buffer.nrSamples;

  // wrap > 0 if we need to wrap around the end of the buffer
  size_t wrap         = from_offset < to_offset ? 0 : buffer.nrSamples - from_offset;

  copyStart(from, to, wrap);

  for (size_t i = 0; i < beamlets.size(); ++i) {
    unsigned nr = beamlets[i];
    const T* origin = &buffer.beamlets[nr][0];

    if (wrap > 0) {
      copyBeamlet( nr, 0, from, origin + from_offset, wrap );
      copyBeamlet( nr, 1, from, origin,               to_offset );
    } else {
      copyBeamlet( nr, 0, from, origin + from_offset, to_offset - from_offset );
    }
  }

  // copy the flags

  for (unsigned b = 0; b < nrBoards; ++b)
    copyFlags( b, buffer.flags[b].sparseSet(from, to).invert(from, to) );

  copyEnd();
}

}
}

#endif

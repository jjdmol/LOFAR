#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Stream.h>
#include <WallClockTime.h>
#include "SharedMemory.h"
#include "Ranges.h"
#include "OMPThread.h"
#include "StationID.h"
#include "StationSettings.h"
#include "SampleBuffer.h"
#include "StationData.h"
#include "Generator.h"
#include "mpi.h"

#include <vector>
#include <omp.h>
#include <string>
#include <boost/format.hpp>

#define DURATION 60 
#define BLOCKSIZE 0.005
#define NRSTATIONS 3

using namespace LOFAR;
using namespace RTCP;

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

Mutex MPIMutex;

//#define USE_RMA

#ifdef USE_RMA

#define MULTIPLE_WINDOWS

template<typename T> class MPISharedBuffer: public SampleBuffer<T> {
public:
  MPISharedBuffer( const struct StationSettings &settings );

  ~MPISharedBuffer();

private:
#ifdef MULTIPLE_WINDOWS
  std::vector<MPI_Win> beamlets_windows;
#else
  MPI_Win beamlets_window;
#endif
};

template<typename T> MPISharedBuffer<T>::MPISharedBuffer( const struct StationSettings &settings )
:
  SampleBuffer<T>(settings, false)
#ifdef MULTIPLE_WINDOWS
  , beamlets_windows(NRSTATIONS)
#endif
{
#ifdef MULTIPLE_WINDOWS
  for (int i = 0; i < NRSTATIONS; ++i) {
    int error = MPI_Win_create(this->beamlets.origin(), this->beamlets.num_elements() * sizeof(T), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &beamlets_windows[i]);

    ASSERT(error == MPI_SUCCESS);
  }
#else
  int error = MPI_Win_create(this->beamlets.origin(), this->beamlets.num_elements() * sizeof(T), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &beamlets_window);

  ASSERT(error == MPI_SUCCESS);
#endif
}

template<typename T> MPISharedBuffer<T>::~MPISharedBuffer()
{
#ifdef MULTIPLE_WINDOWS
  for (int i = 0; i < NRSTATIONS; ++i) {
    int error = MPI_Win_free(&beamlets_windows[i]);
      
    ASSERT(error == MPI_SUCCESS);
  }
#else
  int error = MPI_Win_free(&beamlets_window);
    
  ASSERT(error == MPI_SUCCESS);
#endif
}

template<typename T> class MPISharedBufferReader {
public:
  MPISharedBufferReader( const std::vector<struct StationSettings> &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets );

  ~MPISharedBufferReader();

  void process( double maxDelay );

private:
  const std::vector<struct StationSettings> settings;
  const TimeStamp from, to;
  const size_t blockSize;
  const std::vector<size_t> beamlets;

  MultiDimArray<T, 3> buffer; // [station][beamlet][sample]

#ifdef MULTIPLE_WINDOWS
  std::vector<MPI_Win> beamlets_windows;
#else
  MPI_Win beamlets_window;
#endif

  WallClockTime waiter;

  void copy( const TimeStamp &from, const TimeStamp &to );
};

template<typename T> MPISharedBufferReader<T>::MPISharedBufferReader( const std::vector<struct StationSettings> &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets )
:
  settings(settings),
  from(from),
  to(to),
  blockSize(blockSize),
  beamlets(beamlets),

  buffer(boost::extents[settings.size()][beamlets.size()][blockSize], 128, heapAllocator, false, false)
#ifdef MULTIPLE_WINDOWS
  , beamlets_windows(settings.size())
#endif
{
  ASSERT( settings.size() > 0 );
  ASSERT( from.getClock() == to.getClock() );
  ASSERT( settings[0].station.clock == from.getClock());

  for (size_t i = 0; i < settings.size(); ++i) {
    ASSERT(settings[i].station.clock   == settings[0].station.clock);
    ASSERT(settings[i].station.clock   == from.getClock());
    ASSERT(settings[i].station.bitmode == settings[0].station.bitmode);

    ASSERT(settings[i].nrSamples > blockSize);
  }

#ifdef MULTIPLE_WINDOWS
  for (int i = 0; i < settings.size(); ++i) {
    int error = MPI_Win_create(MPI_BOTTOM, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &beamlets_windows[i]);

    ASSERT(error == MPI_SUCCESS);
  }
#else
  int error = MPI_Win_create(MPI_BOTTOM, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &beamlets_window);

  ASSERT(error == MPI_SUCCESS);
#endif
}

template<typename T> MPISharedBufferReader<T>::~MPISharedBufferReader()
{
#ifdef MULTIPLE_WINDOWS
  for (int i = 0; i < settings.size(); ++i) {
    int error = MPI_Win_free(&beamlets_windows[i]);

    ASSERT(error == MPI_SUCCESS);
  }
#else
  int error = MPI_Win_free(&beamlets_window);

  ASSERT(error == MPI_SUCCESS);
#endif
}

template<typename T> void MPISharedBufferReader<T>::process( double maxDelay )
{
  const TimeStamp maxDelay_ts(static_cast<int64>(maxDelay * settings[0].station.clock / 1024) + blockSize, settings[0].station.clock);

  const TimeStamp current(from);

  double totalwait = 0.0;
  unsigned totalnr = 0;

  double lastreport = MPI_Wtime();

  for (TimeStamp current = from; current < to; current += blockSize) {
    // wait
    //LOG_INFO_STR("Waiting until " << (current + maxDelay_ts) << " for " << current);
    waiter.waitUntil( current + maxDelay_ts );

    // read
    //LOG_INFO_STR("Reading from " << current << " to " << (current + blockSize));
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

template<typename T> void MPISharedBufferReader<T>::copy( const TimeStamp &from, const TimeStamp &to )
{
  int error;

#ifdef MULTIPLE_WINDOWS
  for (int i = 0; i < settings.size(); ++i) {
    error = MPI_Win_lock( MPI_LOCK_SHARED, i, MPI_MODE_NOCHECK, beamlets_windows[i] );
    ASSERT(error == MPI_SUCCESS);
  }
#endif

  for (size_t s = 0; s < settings.size(); ++s) {
#ifndef MULTIPLE_WINDOWS
    error = MPI_Win_lock( MPI_LOCK_SHARED, s, MPI_MODE_NOCHECK, beamlets_window );
    ASSERT(error == MPI_SUCCESS);
#endif

    //LOG_INFO_STR("Copying from station " << s);
    const struct StationSettings settings = this->settings[s];

    size_t from_offset = (int64)from % settings.nrSamples;
    size_t to_offset   = (int64)to % settings.nrSamples;

    if (to_offset == 0)
      to_offset = settings.nrSamples;

    size_t wrap        = from_offset < to_offset ? 0 : settings.nrSamples - from_offset;

    for (size_t i = 0; i < beamlets.size(); ++i) {
      unsigned nr = beamlets[i];

      size_t origin = nr * settings.nrSamples;

      if (wrap > 0) {
        //if (i==0) LOG_INFO_STR("Reading wrapped data");
#ifdef MULTIPLE_WINDOWS
        error = MPI_Get( &buffer[s][i][0], wrap * sizeof(T), MPI_CHAR, s, (origin + from_offset) * sizeof(T), wrap * sizeof(T), MPI_CHAR, beamlets_windows[s] );
#else
        error = MPI_Get( &buffer[s][i][0], wrap * sizeof(T), MPI_CHAR, s, (origin + from_offset) * sizeof(T), wrap * sizeof(T), MPI_CHAR, beamlets_window );
#endif

        ASSERT(error == MPI_SUCCESS);

#ifdef MULTIPLE_WINDOWS
        error = MPI_Get( &buffer[s][i][wrap], to_offset * sizeof(T), MPI_CHAR, s, origin * sizeof(T), to_offset * sizeof(T), MPI_CHAR, beamlets_windows[s] );
#else
        error = MPI_Get( &buffer[s][i][wrap], to_offset * sizeof(T), MPI_CHAR, s, origin * sizeof(T), to_offset * sizeof(T), MPI_CHAR, beamlets_window );
#endif

        ASSERT(error == MPI_SUCCESS);
      } else {
        // higher performance by splitting into multiple requests if block size is large -- formula yet unknown
        //size_t partSize = (to_offset - from_offset) / 2 + 1;
        size_t partSize = to_offset - from_offset;

        for (size_t x = from_offset; x < to_offset; x += partSize) {
          size_t y = std::min(x + partSize, to_offset);

#ifdef MULTIPLE_WINDOWS
          error = MPI_Get( &buffer[s][i][x - from_offset], (y - x) * sizeof(T), MPI_CHAR, s, (origin + x) * sizeof(T), (y - x) * sizeof(T), MPI_CHAR, beamlets_windows[s] );
#else
          error = MPI_Get( &buffer[s][i][x - from_offset], (y - x) * sizeof(T), MPI_CHAR, s, (origin + x) * sizeof(T), (y - x) * sizeof(T), MPI_CHAR, beamlets_window );
#endif

          ASSERT(error == MPI_SUCCESS);
        }
      }
    }

#ifndef MULTIPLE_WINDOWS
    error = MPI_Win_unlock( s, beamlets_window );
    ASSERT(error == MPI_SUCCESS);
#endif
  }

#ifdef MULTIPLE_WINDOWS
  for (int i = 0; i < settings.size(); ++i) {
    error = MPI_Win_unlock( i, beamlets_windows[i] );
    ASSERT(error == MPI_SUCCESS);
  }
#endif
}
#else

template<typename T> class MPISendStation: public SampleBufferReader<T> {
public:
  MPISendStation( const struct StationSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets, unsigned destRank );

  struct Header {
    StationID station;

    bool data;
    size_t wrap;

    int64 from, to;

    size_t nrBeamlets;
    size_t nrFlags;

    size_t flagsSize;
  };

  union tag_t {
    struct {
      unsigned type:2;
      unsigned beamlet:10;
      unsigned transfer:3;
    } bits;

    int value;

    tag_t(): value(0) {}
  };

  enum tag_types { CONTROL = 0, BEAMLET = 1, FLAGS = 2 };

protected:
  const unsigned destRank;

  std::vector<MPI_Request> requests;
  size_t nrRequests;

  Matrix<char> flagsData;

  virtual void copyNothing( const TimeStamp &from, const TimeStamp &to );
  virtual void copyStart( const TimeStamp &from, const TimeStamp &to, size_t wrap );
  virtual void copyBeamlet( unsigned beamlet, unsigned transfer, const TimeStamp &from_ts, const T* from, size_t nrSamples );
  virtual void copyFlags  ( unsigned transfer, const SparseSet<int64> &flags );
  virtual void copyEnd();

  size_t flagsSize() const {
    return sizeof(uint32_t) + this->settings.nrFlagRanges * sizeof(int64) * 2;
  }
};


template<typename T> MPISendStation<T>::MPISendStation( const struct StationSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets, unsigned destRank )
:
  SampleBufferReader<T>(settings, beamlets, from, to, blockSize),
  destRank(destRank),
  requests(this->buffer.flags.size() + beamlets.size() * 2, 0),
  nrRequests(0),
  flagsData(this->buffer.flags.size(), flagsSize())
{
}


template<typename T> void MPISendStation<T>::copyNothing( const TimeStamp &from, const TimeStamp &to )
{
  LOG_INFO_STR( "No valid data!" );

  Header header;
  header.station = this->settings.station;
  header.data = false;
  header.wrap = 0;
  header.from = from;
  header.to   = to;
  header.nrBeamlets = 0;
  header.nrFlags    = 0;

  {
    ScopedLock sl(MPIMutex);

    union tag_t tag;

    tag.bits.type = CONTROL;

    int error = MPI_Isend(&header, sizeof header, MPI_CHAR, destRank, tag.value, MPI_COMM_WORLD, &requests[nrRequests++]);
    ASSERT(error == MPI_SUCCESS);
  }
}


template<typename T> void MPISendStation<T>::copyStart( const TimeStamp &from, const TimeStamp &to, size_t wrap )
{
  Header header;
  header.station = this->settings.station;
  header.data = true;
  header.wrap = wrap;
  header.from = from;
  header.to   = to;
  header.nrBeamlets = this->beamlets.size();
  header.nrFlags    = this->buffer.flags.size();
  header.flagsSize  = this->flagsSize();

  {
    ScopedLock sl(MPIMutex);

    int error = MPI_Isend(&header, sizeof header, MPI_CHAR, destRank, 0, MPI_COMM_WORLD, &requests[nrRequests++]);

    ASSERT(error == MPI_SUCCESS);
  }  

  //LOG_INFO( "Header sent" );
}


template<typename T> void MPISendStation<T>::copyBeamlet( unsigned beamlet, unsigned transfer, const TimeStamp &from_ts, const T* from, size_t nrSamples)
{
  (void)from_ts;

  ScopedLock sl(MPIMutex);

  union tag_t tag;

  tag.bits.type     = BEAMLET;
  tag.bits.beamlet  = beamlet;
  tag.bits.transfer = transfer;

  int error = MPI_Isend(
            (void*)from, nrSamples * sizeof(T), MPI_CHAR,
            destRank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

  ASSERT(error == MPI_SUCCESS);
}


template<typename T> void MPISendStation<T>::copyFlags( unsigned transfer, const SparseSet<int64> &flags )
{
  //LOG_INFO_STR( "Copy flags for beamlets [" << fromBeamlet << ", " << toBeamlet << "): " << (100.0 * flags.count() / this->blockSize) << "% " << flags );
  ssize_t numBytes = flags.marshall(&flagsData[transfer][0], flagsSize());

  ASSERT(numBytes >= 0);

  union tag_t tag;

  tag.bits.type     = FLAGS;
  tag.bits.transfer = transfer;

  {
    ScopedLock sl(MPIMutex);

    int error = MPI_Isend(
            (void*)&flagsData[transfer][0], flagsSize(), MPI_CHAR,
            destRank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

    ASSERT(error == MPI_SUCCESS);
  }  
}


template<typename T> void MPISendStation<T>::copyEnd()
{
  int flag = false;
  std::vector<MPI_Status> statusses(nrRequests);

  while (!flag) {
    {
      ScopedLock sl(MPIMutex);

      int error = MPI_Testall(nrRequests, &requests[0], &flag, &statusses[0]);

      ASSERT(error == MPI_SUCCESS);
    }

    // can't hold lock indefinitely
    pthread_yield();
  }

  //LOG_INFO( "Copy done");

  nrRequests = 0;
}


template<typename T> class MPIReceiveStation {
public:
  MPIReceiveStation( const struct StationSettings &settings, const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize );

  void receiveBlock();

private:
  const struct StationSettings settings;
  const std::vector<int> stationRanks;

public:
  const std::vector<size_t> beamlets;
  const size_t blockSize;
  MultiDimArray<T, 3> samples;       // [station][beamlet][sample]
  Matrix< SparseSet<int64> > flags;  // [station][board]
};

template<typename T> MPIReceiveStation<T>::MPIReceiveStation( const struct StationSettings &settings, const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize )
:
  settings(settings),
  stationRanks(stationRanks),
  beamlets(beamlets),
  blockSize(blockSize),
  samples(boost::extents[stationRanks.size()][beamlets.size()][blockSize], 128, heapAllocator, false, false),
  flags(stationRanks.size(), settings.nrBoards)
{
}


template<typename T> void MPIReceiveStation<T>::receiveBlock()
{
  struct MPISendStation<T>::Header header;

  int error;

  size_t nrRequests = 0;
  std::vector<MPI_Request> header_requests(stationRanks.size());
  std::vector<struct MPISendStation<T>::Header> headers(stationRanks.size());

  // post receives for all headers

  for (size_t nr = 0; nr < stationRanks.size(); ++nr) {
    typename MPISendStation<T>::tag_t tag;

    // receive the header

    tag.bits.type = MPISendStation<T>::CONTROL;

    error = MPI_Irecv(&headers[nr], sizeof header, MPI_CHAR, stationRanks[nr], tag.value, MPI_COMM_WORLD, &header_requests[nr]);
    ASSERT(error == MPI_SUCCESS);
  }

  // process stations in the order in which we receive the headers

  std::vector<MPI_Request> requests(beamlets.size() * 2 * stationRanks.size());
  std::vector<MPI_Status> statusses(beamlets.size() * 2 * stationRanks.size());
  Matrix< std::vector<char> > flagData(stationRanks.size(), settings.nrBoards); // [station][board][data]

  for (size_t i = 0; i < stationRanks.size(); ++i) {
    int nr;

    // wait for any header request to finish
    error = MPI_Waitany(header_requests.size(), &header_requests[0], &nr, MPI_STATUS_IGNORE);
    ASSERT(error == MPI_SUCCESS);

    typename MPISendStation<T>::tag_t tag;

    // check the header

    const struct MPISendStation<T>::Header header = headers[nr];

    ASSERT(header.to - header.from == (int64)blockSize);
    ASSERT(header.wrap < blockSize);

    if (!header.data)
      continue;

    // post receives for the beamlets

    ASSERT(header.nrBeamlets == beamlets.size());

    tag.value = 0; // reset
    tag.bits.type = MPISendStation<T>::BEAMLET;

    int rank = stationRanks[nr];

    for (size_t beamlet = 0; beamlet < header.nrBeamlets; ++beamlet) {
      tag.bits.beamlet  = beamlet;
      tag.bits.transfer = 0;

      error = MPI_Irecv(
          &samples[nr][beamlet][0], sizeof(T) * (header.wrap ? header.wrap : blockSize), MPI_CHAR,
          rank, tag.value,
          MPI_COMM_WORLD, &requests[nrRequests++]);

      ASSERT(error == MPI_SUCCESS);

      if (header.wrap > 0) {
        tag.bits.transfer = 1;

        error = MPI_Irecv(
            &samples[nr][beamlet][header.wrap], sizeof(T) * (blockSize - header.wrap), MPI_CHAR,
            rank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

        ASSERT(error == MPI_SUCCESS);
      }
    }

    // post receives for the flags

    ASSERT(header.nrFlags == settings.nrBoards);

    tag.value = 0; // reset
    tag.bits.type = MPISendStation<T>::FLAGS;

    for (size_t board = 0; board < header.nrFlags; ++board) {
      tag.bits.transfer = board;

      flagData[nr][board].resize(header.flagsSize);

      error = MPI_Irecv(
          &flagData[nr][0][0], header.flagsSize, MPI_CHAR,
          rank, tag.value,
          MPI_COMM_WORLD, &requests[nrRequests++]);

      ASSERT(error == MPI_SUCCESS);
    }  
  }

  // wait for all transfers to finish

  if (nrRequests > 0) {
    error = MPI_Waitall(nrRequests, &requests[0], &statusses[0]);
    ASSERT(error == MPI_SUCCESS);
  }

  // convert raw flagData to flags array

  for (size_t nr = 0; nr < stationRanks.size(); ++nr)
    for (size_t board = 0; board < settings.nrBoards; ++board)
      flags[nr][board].unmarshall(&flagData[nr][board][0]);

}
#endif

void sighandler(int)
{
  /* no-op */
}

int main( int argc, char **argv )
{
  size_t clock = 200*1000*1000;

  typedef SampleBuffer<int16>::SampleType SampleT;
  const TimeStamp from(time(0L) + 1, 0, clock);
  const TimeStamp to(time(0L) + 1 + DURATION, 0, clock);
  const size_t blockSize = BLOCKSIZE * clock / 1024;
  std::map<unsigned, std::vector<size_t> > beamlets;

  struct StationID stationID("RS106", "LBA", clock, 16);
  struct StationSettings settings;

  settings.station = stationID;
  settings.nrBeamlets = 244;
  settings.nrBoards = 4;

  settings.nrSamples = (5 * stationID.clock / 1024);// & ~0xFL;
  settings.nrFlagRanges = 64;

  settings.dataKey = stationID.hash();

  INIT_LOGGER(argv[0]);

  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  int nrHosts, rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

#ifdef USE_RMA
  int nrStations = NRSTATIONS;
#else
  int nrStations = NRSTATIONS;
#endif

  for (unsigned i = 0; i < 244; ++i)
    beamlets[nrStations + i % (nrHosts - nrStations)].push_back(i);

  if (rank > nrStations - 1) {
    // receiver
    LOG_INFO_STR("Receiver " << rank << " starts, handling " << beamlets[rank].size() << " subbands from " << nrStations << " stations." );

#ifdef USE_RMA
    std::vector<struct StationSettings> stations(nrStations, settings);

    {
      MPISharedBufferReader<SampleT> receiver(stations, from, to, blockSize, beamlets[rank]);

      receiver.process(0.0);
    }  
#else
    std::vector<int> stationRanks(nrStations);

    for (size_t i = 0; i < stationRanks.size(); i++)
      stationRanks[i] = i;

    {
      MPIReceiveStation<SampleT> receiver(settings, stationRanks, beamlets[rank], blockSize);

      for(size_t block = 0; block < (to-from)/blockSize + 1; ++block) {
        receiver.receiveBlock();

        //LOG_INFO_STR("Receiver " << rank << " received block " << block);
      }
    }  
#endif
    LOG_INFO_STR("Receiver " << rank << " done");

    MPI_Finalize();
    return 0;
  }

  omp_set_nested(true);
  omp_set_num_threads(32);

  signal(SIGHUP, sighandler);
  siginterrupt(SIGHUP, 1);

  std::vector<std::string> inputStreams(4);
  inputStreams[0] = "udp:127.0.0.1:4346";
  inputStreams[1] = "udp:127.0.0.1:4347";
  inputStreams[2] = "udp:127.0.0.1:4348";
  inputStreams[3] = "udp:127.0.0.1:4349";

  if(rank == 0) {
    Station< SampleT > station( settings, inputStreams );
    Generator< SampleT > generator( settings, inputStreams );

    #pragma omp parallel sections num_threads(4)
    {
      #pragma omp section
      { station.process(); }

      #pragma omp section
      { generator.process(); }

      #pragma omp section
      { sleep(DURATION + 1); station.stop(); sleep(1); generator.stop(); }

      #pragma omp section
      {
        struct StationID lookup("RS106", "HBA0");
        struct StationSettings s(stationID);

        LOG_INFO_STR("Detected " << s);
#ifdef USE_RMA
        MPISharedBuffer<SampleT> streamer(s);
#else
        #pragma omp parallel for num_threads(nrHosts - nrStations)
        for (int i = nrStations; i < nrHosts; ++i) {
          LOG_INFO_STR("Connecting to receiver " << i );
          MPISendStation< SampleT > streamer(s, from, to, blockSize, beamlets[i], i );

          LOG_INFO_STR("Sending to receiver " << i );
          streamer.process( 0.0 );
        }
#endif
      }
    }
  } else {
      struct StationID lookup("RS106", "HBA0");
      struct StationSettings s(stationID);

      LOG_INFO_STR("Detected " << s);
#ifdef USE_RMA
      MPISharedBuffer<SampleT> streamer(s);
#else
      #pragma omp parallel for num_threads(nrHosts - nrStations)
      for (int i = nrStations; i < nrHosts; ++i) {
        LOG_INFO_STR("Connecting to receiver " << i );
        MPISendStation< SampleT > streamer(s, from, to, blockSize, beamlets[i], i );

        LOG_INFO_STR("Sending to receiver " << i );
        streamer.process( 0.0 );
      }
#endif
  }

  MPI_Finalize();
}

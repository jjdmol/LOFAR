#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Stream.h>
#include "WallClockTime.h"
#include "SharedMemory.h"
#include "Ranges.h"
#include "OMPThread.h"
#include "StationID.h"
#include "BufferSettings.h"
#include "SampleType.h"
#include "SampleBuffer.h"
#include "SampleBufferReader.h"
#include "Generator.h"
#include "PacketsToBuffer.h"
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

Mutex MPIMutex;

//#define USE_RMA

#ifdef USE_RMA

#define MULTIPLE_WINDOWS

template<typename T> class MPISharedBuffer: public SampleBuffer<T> {
public:
  MPISharedBuffer( const struct BufferSettings &settings );

  ~MPISharedBuffer();

private:
#ifdef MULTIPLE_WINDOWS
  std::vector<MPI_Win> beamlets_windows;
#else
  MPI_Win beamlets_window;
#endif
};

template<typename T> MPISharedBuffer<T>::MPISharedBuffer( const struct BufferSettings &settings )
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
  MPISharedBufferReader( const std::vector<struct BufferSettings> &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets );

  ~MPISharedBufferReader();

  void process( double maxDelay );

private:
  const std::vector<struct BufferSettings> settings;
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

template<typename T> MPISharedBufferReader<T>::MPISharedBufferReader( const std::vector<struct BufferSettings> &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets )
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
    const struct BufferSettings settings = this->settings[s];

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
  MPISendStation( const struct BufferSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets, unsigned destRank );

  struct Header {
    StationID station;

    int64 from, to;
    size_t wrapOffsets[1024];

    size_t nrBeamlets;
    size_t flagsSize;
  };

  union tag_t {
    struct {
      unsigned type:2;
      unsigned beamlet:10;
      unsigned transfer:1;
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

  virtual void copyStart( const TimeStamp &from, const TimeStamp &to, const std::vector<size_t> &wrapOffsets );
  virtual void copy( const struct SampleBufferReader<T>::CopyInstructions &info );
  virtual void copyEnd( const TimeStamp &from, const TimeStamp &to );

  size_t flagsSize() const {
    return sizeof(uint32_t) + this->settings.nrFlagRanges * sizeof(int64) * 2;
  }
};


template<typename T> MPISendStation<T>::MPISendStation( const struct BufferSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, const std::vector<size_t> &beamlets, unsigned destRank )
:
  SampleBufferReader<T>(settings, beamlets, from, to, blockSize),
  destRank(destRank),
  requests(1 + beamlets.size() * 3, 0), // apart from the header, at most three transfers per beamlet: one or two for the samples, plus one for the flags
  nrRequests(0),
  flagsData(this->buffer.flags.size(), flagsSize())
{
}


template<typename T> void MPISendStation<T>::copyStart( const TimeStamp &from, const TimeStamp &to, const std::vector<size_t> &wrapOffsets )
{
  Header header;

  // Copy static information
  header.station     = this->settings.station;
  header.from        = from;
  header.to          = to;
  header.nrBeamlets  = this->beamlets.size();
  header.flagsSize   = this->flagsSize();

  // Copy the wrapOffsets
  ASSERT(wrapOffsets.size() * sizeof wrapOffsets[0] <= sizeof header.wrapOffsets);
  memcpy(&header.wrapOffsets[0], &wrapOffsets[0], wrapOffsets.size() * sizeof wrapOffsets[0]);

  {
    ScopedLock sl(MPIMutex);

    int error = MPI_Isend(&header, sizeof header, MPI_CHAR, destRank, 0, MPI_COMM_WORLD, &requests[nrRequests++]);

    ASSERT(error == MPI_SUCCESS);
  }  

  //LOG_INFO( "Header sent" );
}


template<typename T> void MPISendStation<T>::copy( const struct SampleBufferReader<T>::CopyInstructions &info )
{
  ScopedLock sl(MPIMutex);

  // Send beamlet
  for(unsigned transfer = 0; transfer < info.nrRanges; ++transfer) {
    union tag_t tag;

    tag.bits.type     = BEAMLET;
    tag.bits.beamlet  = info.beamlet;
    tag.bits.transfer = transfer;
    ASSERT(tag.value >= 0); // Silly MPI requirement

    const T *from = info.ranges[transfer].from;
    const T *to   = info.ranges[transfer].to;

    int error = MPI_Isend(
              (void*)from, (to - from) * sizeof(T), MPI_CHAR,
              destRank, tag.value,
              MPI_COMM_WORLD, &requests[nrRequests++]);

    ASSERT(error == MPI_SUCCESS);
  }

  // Send flags
  ssize_t numBytes = info.flags.marshall(&flagsData[info.beamlet][0], flagsSize());

  ASSERT(numBytes >= 0);

  union tag_t tag;

  tag.bits.type     = FLAGS;
  tag.bits.beamlet  = info.beamlet;
  ASSERT(tag.value >= 0); // Silly MPI requirement

  int error = MPI_Isend(
            (void*)&flagsData[info.beamlet][0], flagsSize(), MPI_CHAR,
            destRank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

  ASSERT(error == MPI_SUCCESS);
}


template<typename T> void MPISendStation<T>::copyEnd( const TimeStamp &from, const TimeStamp &to )
{
  (void)from; (void)to;

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


/*
 * Note: we need to receive all station data in one loop, because MPI wants to
 * have a single thread listening to all requests.
 */
template<typename T> class MPIReceiveStations {
public:
  MPIReceiveStations( const struct BufferSettings &settings, const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize );

  void receiveBlock();

private:
  const struct BufferSettings settings;
  const std::vector<int> stationRanks;

public:
  const std::vector<size_t> beamlets;
  const size_t blockSize;
  MultiDimArray<T, 3> samples;       // [station][beamlet][sample]
  Matrix< SparseSet<int64> > flags;  // [station][beamlet]
};


template<typename T> MPIReceiveStations<T>::MPIReceiveStations( const struct BufferSettings &settings, const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize )
:
  settings(settings),
  stationRanks(stationRanks),
  beamlets(beamlets),
  blockSize(blockSize), // TODO: nrHistorySamples support
  samples(boost::extents[stationRanks.size()][beamlets.size()][blockSize], 128, heapAllocator, false, false),
  flags(stationRanks.size(), beamlets.size())
{
}


template<typename T> void MPIReceiveStations<T>::receiveBlock()
{
  int error;

  // All requests except the headers
  std::vector<MPI_Request> requests(beamlets.size() * 3 * stationRanks.size());
  size_t nrRequests = 0;

  // Post receives for all headers
  std::vector<MPI_Request> header_requests(stationRanks.size());
  std::vector<struct MPISendStation<T>::Header> headers(stationRanks.size());

  for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
    typename MPISendStation<T>::tag_t tag;

    // receive the header
    tag.bits.type = MPISendStation<T>::CONTROL;
    ASSERT(tag.value >= 0); // Silly MPI requirement

    error = MPI_Irecv(&headers[stat], sizeof headers[stat], MPI_CHAR, stationRanks[stat], tag.value, MPI_COMM_WORLD, &header_requests[stat]);
    ASSERT(error == MPI_SUCCESS);
  }

  // Process stations in the order in which we receive the headers
  Matrix< std::vector<char> > flagData(stationRanks.size(), beamlets.size()); // [station][beamlet][data]

  for (size_t i = 0; i < stationRanks.size(); ++i) {
    int stat;

    /*
     * For each station, receive its header, and post the relevant sample and
     * flag Irecvs.
     */

    // Wait for any header request to finish
    error = MPI_Waitany(header_requests.size(), &header_requests[0], &stat, MPI_STATUS_IGNORE);
    ASSERT(error == MPI_SUCCESS);

    int rank = stationRanks[stat];

    // Check the header
    const struct MPISendStation<T>::Header &header = headers[stat];

    ASSERT(header.to - header.from == (int64)blockSize);
    ASSERT(header.nrBeamlets == beamlets.size());

    // Post receives for the samples
    for (size_t beamlet = 0; beamlet < header.nrBeamlets; ++beamlet) {
      const size_t wrapOffset = header.wrapOffsets[beamlet];

      typename MPISendStation<T>::tag_t tag;
      tag.bits.type    = MPISendStation<T>::BEAMLET;
      tag.bits.beamlet = beamlet;

      // First sample transfer
      tag.bits.transfer = 0;
      ASSERT(tag.value >= 0); // Silly MPI requirement

      error = MPI_Irecv(
          &samples[stat][beamlet][0], sizeof(T) * (wrapOffset ? wrapOffset : blockSize), MPI_CHAR,
          rank, tag.value,
          MPI_COMM_WORLD, &requests[nrRequests++]);

      ASSERT(error == MPI_SUCCESS);

      // Second sample transfer
      if (wrapOffset > 0) {
        tag.bits.transfer = 1;
        ASSERT(tag.value >= 0); // Silly MPI requirement

        error = MPI_Irecv(
            &samples[stat][beamlet][wrapOffset], sizeof(T) * (blockSize - wrapOffset), MPI_CHAR,
            rank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

        ASSERT(error == MPI_SUCCESS);
      }

      // Flags transfer
      tag.value = 0; // reset
      tag.bits.type    = MPISendStation<T>::FLAGS;
      tag.bits.beamlet = beamlet;
      ASSERT(tag.value >= 0); // Silly MPI requirement

      flagData[stat][beamlet].resize(header.flagsSize);

      error = MPI_Irecv(
            &flagData[stat][0][0], header.flagsSize, MPI_CHAR,
            rank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

      ASSERT(error == MPI_SUCCESS);
    }
  }

  // Wait for all transfers to finish
  if (nrRequests > 0) {
    std::vector<MPI_Status> statusses(requests.size());

    error = MPI_Waitall(nrRequests, &requests[0], &statusses[0]);
    ASSERT(error == MPI_SUCCESS);
  }

  // Convert raw flagData to flags array
  for (size_t stat = 0; stat < stationRanks.size(); ++stat)
    for (size_t beamlet = 0; beamlet < beamlets.size(); ++beamlet)
      flags[stat][beamlet].unmarshall(&flagData[stat][beamlet][0]);

}
#endif


int main( int argc, char **argv )
{
  size_t clock = 200*1000*1000;

  typedef SampleType<i16complex> SampleT;
  const TimeStamp from(time(0L) + 1, 0, clock);
  const TimeStamp to(time(0L) + 1 + DURATION, 0, clock);
  const size_t blockSize = BLOCKSIZE * clock / 1024;
  std::map<unsigned, std::vector<size_t> > beamlets;

  struct StationID stationID("RS106", "LBA", clock, 16);
  struct BufferSettings settings;

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
    std::vector<struct BufferSettings> stations(nrStations, settings);

    {
      MPISharedBufferReader<SampleT> receiver(stations, from, to, blockSize, beamlets[rank]);

      receiver.process(0.0);
    }  
#else
    std::vector<int> stationRanks(nrStations);

    for (size_t i = 0; i < stationRanks.size(); i++)
      stationRanks[i] = i;

    {
      MPIReceiveStations<SampleT> receiver(settings, stationRanks, beamlets[rank], blockSize);

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
  OMPThread::init();

  std::vector<std::string> inputStreams(4);
  inputStreams[0] = "udp:127.0.0.1:4346";
  inputStreams[1] = "udp:127.0.0.1:4347";
  inputStreams[2] = "udp:127.0.0.1:4348";
  inputStreams[3] = "udp:127.0.0.1:4349";

  if(rank == 0) {
    PacketsToBuffer< SampleT > station( settings, inputStreams );
    Generator generator( settings, inputStreams );

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
        struct BufferSettings s(stationID);

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
      struct BufferSettings s(stationID);

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

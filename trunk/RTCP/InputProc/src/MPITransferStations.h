#ifndef __MPI_TRANSFERSTATIONS__
#define __MPI_TRANSFERSTATIONS__

#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include "SampleBufferReader.h"
#include "BufferSettings.h"
#include "mpi.h"
#include <pthread.h>
#include <vector>

namespace LOFAR {
  namespace RTCP {

Mutex MPIMutex;

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

  int alldone = false;
  std::vector<MPI_Status> statusses(nrRequests);

  while (!alldone) {
    {
      ScopedLock sl(MPIMutex);

      int error = MPI_Testall(nrRequests, &requests[0], &alldone, &statusses[0]);

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

}
}

#endif


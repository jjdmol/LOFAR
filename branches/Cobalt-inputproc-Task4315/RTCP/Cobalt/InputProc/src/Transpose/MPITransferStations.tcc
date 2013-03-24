/* MPITransferStations.tcc
 * Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <pthread.h>

#include <Common/LofarLogger.h>
#include <Common/Singleton.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Semaphore.h>

#include <boost/format.hpp>

// Send headers asynchroneously (disable for debugging)
#define SEND_HEADERS_ASYNC

using namespace std;

namespace LOFAR {
  namespace Cobalt {

Mutex MPIMutex;

/*
 *
 * The MPISendStation object sends all beamlets from
 * one station to all receiver nodes.
 *
 * The following data flow is observed. Blocks are
 * sent sequentially.
 *
 *          |-- BEAMLET 0 -- FLAGS 0 --|
 *          |
 * HEADER --|-- BEAMLET 1 -- FLAGS 1 --|
 *          |
 *          |-- BEAMLET 2 -- FLAGS 2 --|
 *
 * One header is sent per receiver node per block.
 */

namespace ThreadSafeMPI {
/*
 * MPIRequestManager keeps track of a set of outstanding
 * MPI requests across threads. Any thread can call
 *    wait(request)
 * which will return once `request' has completed.
 *
 * The process() function must be running for the request
 * manager to work, and can be stopped through the stop()
 * function.
 */

class MPIRequestManager {
public:
  MPIRequestManager(): done(false)  {}
  ~MPIRequestManager() { stop(); }

  void stop() {
    done = true;

    // unlock waits for request
    requestAdded.signal();
  }

  bool wait( const MPI_Request &request ) {
    Semaphore doneWaiting;

    addRequest(request, &doneWaiting);

    // Wait for request to finish
    return doneWaiting.down();
  }

  // Note: if a request is put in the background, its
  // data STILL needs to stay in scope until the transfer
  // finishes.
  void waitInBackground( const MPI_Request &request ) {
    addRequest(request, NULL);
  }

  void process() {
    while(!done) {
      // don't occupy CPU indefinitely
      pthread_yield();

      // wait for at least one pending request
      {
        ScopedLock sl(requestsMutex);

        while(!done && empty()) {
          requestAdded.wait(requestsMutex);
        }
      }

      // handle any finished request
      if (!empty()) {
        handleAny();
      }
    }
  }

private:
  volatile bool done;

  Mutex requestsMutex;
  Condition requestAdded;

  map<MPI_Request, Semaphore*> requests;

  bool empty() const {
    return requests.empty();
  }

  void addRequest( const MPI_Request &request, Semaphore *semaphore ) {
    ScopedLock sl(requestsMutex);

    // MPI_REQUEST_NULL is used to signal a completed request.
    ASSERT(request != MPI_REQUEST_NULL);

    // Request may not already be present.
    ASSERT(requests.find(request) == requests.end());

    requests[request] = semaphore;

    // Signal that a request has been added
    requestAdded.signal();
  }

  void handleAny() {
    ScopedLock sl(MPIMutex);

    // Convert the requests map to a vector of request identifiers
    std::vector<MPI_Request> ids;

    {
      ScopedLock sl(requestsMutex);

      ids.reserve(requests.size());
      for(map<MPI_Request, Semaphore*>::const_iterator i = requests.begin(); i != requests.end(); ++i) {
        ids.push_back(i->first);
      }
    }

    // MPI_Testany wants something to test
    ASSERT(ids.size() > 0);

    // Test if any request has finished
    std::vector<int> readyIds(ids.size());
    int readyCount;

    // NOTE: MPI_Testsome sets a completed request to MPI_REQUEST_NULL in the
    // ids array! So we need to create a copy in order to lookup the original
    // MPI_Request.
    std::vector<MPI_Request> ids_copy(ids);
    int error = ::MPI_Testsome(ids_copy.size(), &ids_copy[0], &readyCount, &readyIds[0], MPI_STATUS_IGNORE);
    ASSERT(error == MPI_SUCCESS);

    ASSERT(readyCount != MPI_UNDEFINED);

    for (int i = 0; i < readyCount; ++i) {
      int readyIdx = readyIds[i];

      ASSERT(readyIdx != MPI_UNDEFINED);

      // A request is finished. Remove it from the map and raise the
      // associated Semaphore.
      ScopedLock sl(requestsMutex);

      Semaphore *result = requests.at(ids[readyIdx]);
      requests.erase(ids[readyIdx]);

      if (result) {
        // Wake up client
        result->up();
      }
    }
  }
};

/*
 * Send [ptr, ptr + numBytes) to destRank.
 */
MPI_Request MPI_Isend(void *ptr, size_t numBytes, int destRank, int tag) {
  ASSERT(tag >= 0); // Silly MPI requirement

  MPI_Request request;

  {
    ScopedLock sl(MPIMutex);

    int error;

    error = ::MPI_Isend(ptr, numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
    ASSERT(error == MPI_SUCCESS);
  }

  return request;
}

/*
 * Receive [ptr, ptr + numBytes) from destRank.
 */
MPI_Request MPI_Irecv(void *ptr, size_t numBytes, int destRank, int tag) {
  ASSERT(tag >= 0); // Silly MPI requirement

  MPI_Request request;

  {
    ScopedLock sl(MPIMutex);

    int error;

    error = ::MPI_Irecv(ptr, numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
    ASSERT(error == MPI_SUCCESS);
  }

  return request;
}

void MPI_Wait(const MPI_Request &request) {
  Singleton<MPIRequestManager>::instance().wait(request);
}

void MPI_Send(void *ptr, size_t numBytes, int destRank, int tag) {
  MPI_Request request = MPI_Isend(ptr, numBytes, destRank, tag);

  MPI_Wait(request);
}

void MPI_Recv(void *ptr, size_t numBytes, int destRank, int tag) {
  MPI_Request request = MPI_Irecv(ptr, numBytes, destRank, tag);

  MPI_Wait(request);
}

};


// Returns the keys of an std::map.
template<typename K, typename V> std::vector<K> keys( const std::map<K, V> &m )
{
  std::vector<K> keys;

  keys.reserve(m.size());
  for (typename std::map<K,V>::const_iterator i = m.begin(); i != m.end(); ++i) {
    keys.push_back(i->first);
  }

  return keys;
}


// Returns the set of unique values of an std::map.
template<typename K, typename V> std::set<V> values( const std::map<K, V> &m )
{
  std::set<V> values;

  for (typename std::map<K,V>::const_iterator i = m.begin(); i != m.end(); ++i) {
    values.insert(i->second);
  }

  return values;
}


// Returns the inverse of an std::map.
template<typename K, typename V> std::map<V, std::vector<K> > inverse( const std::map<K, V> &m )
{
  std::map<V, std::vector<K> > inverse;

  for (typename std::map<K,V>::const_iterator i = m.begin(); i != m.end(); ++i) {
    inverse[i->second].push_back(i->first);
  }

  return inverse;
}


template<typename T> MPISendStation<T>::MPISendStation( const struct BufferSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, size_t nrHistorySamples, const std::map<size_t, int> &beamletDistribution )
:
  SampleBufferReader<T>(settings, keys(beamletDistribution), from, to, blockSize, nrHistorySamples),
  logPrefix(str(boost::format("[station %s] [MPISendStation] ") % settings.station.stationName)),
  beamletDistribution(beamletDistribution),
  targetRanks(values(beamletDistribution)),
  beamletsOfTarget(inverse(beamletDistribution))
{
  LOG_INFO_STR(logPrefix << "Initialised");
}

template<typename T> MPI_Request MPISendStation<T>::sendHeader( int rank, Header &header, const struct SampleBufferReader<T>::CopyInstructions &info )
{
  LOG_DEBUG_STR(logPrefix << "Sending header to rank " << rank);

  const std::vector<size_t> &beamlets = beamletsOfTarget.at(rank);

  // Copy static information
  header.station      = this->settings.station;
  header.from         = info.from;
  header.to           = info.to;
  header.nrBeamlets   = beamlets.size();
  header.metaDataSize = this->metaDataSize();

  // Copy the wrapOffsets
  ASSERT(beamlets.size() <= sizeof header.wrapOffsets / sizeof header.wrapOffsets[0]);

  for(unsigned beamlet = 0; beamlet < beamlets.size(); ++beamlet) {
    const struct SampleBufferReader<T>::CopyInstructions::Beamlet &ib = info.beamlets[beamlets[beamlet]];

    header.wrapOffsets[beamlet] = ib.nrRanges == 1 ? 0 : ib.ranges[0].to - ib.ranges[0].from;
  }

  // Send the actual header
  union tag_t tag;
  tag.bits.type     = CONTROL;

#ifdef SEND_HEADERS_ASYNC
  return ThreadSafeMPI::MPI_Isend(&header, sizeof header, rank, tag.value);
#else
  ThreadSafeMPI::MPI_Send(&header, sizeof header, rank, tag.value);
  return 0;
#endif
}

template<typename T> void MPISendStation<T>::sendData( int rank, unsigned beamlet, const struct SampleBufferReader<T>::CopyInstructions::Beamlet &ib )
{
  LOG_DEBUG_STR(logPrefix << "Sending beamlet " << beamlet << " to rank " << rank << " using " << ib.nrRanges << " transfers");

  // Send beamlet using 1 or 2 transfers
# pragma omp parallel for num_threads(ib.nrRanges)
  for(unsigned transfer = 0; transfer < ib.nrRanges; ++transfer) {
    union tag_t tag;

    tag.bits.type     = BEAMLET;
    tag.bits.beamlet  = beamlet;
    tag.bits.transfer = transfer;

    const T *from = ib.ranges[transfer].from;
    const T *to   = ib.ranges[transfer].to;

    ASSERT( from < to ); // There must be data to send, or MPI will error

    ThreadSafeMPI::MPI_Send((void*)from, (to - from) * sizeof(T), rank, tag.value);
  }
}

template<typename T> void MPISendStation<T>::sendFlags( int rank, unsigned beamlet, const SparseSet<int64> &flags )
{
  //LOG_DEBUG_STR("Sending flags to rank " << rank);

  std::vector<char> metaData(metaDataSize());

  ssize_t numBytes = flags.marshall(&metaData[0], metaData.size());
  ASSERT(numBytes >= 0);

  union tag_t tag;
  tag.bits.type     = FLAGS;
  tag.bits.beamlet  = beamlet;

  ThreadSafeMPI::MPI_Send(&metaData[0], metaData.size(), rank, tag.value);
}


template<typename T> void MPISendStation<T>::sendBlock( const struct SampleBufferReader<T>::CopyInstructions &info )
{
  /*
   * SEND HEADER (ASYNC)
   */

  std::map<int, Header> headers;
  std::map<int, MPI_Request> headerRequests;

  // No need for parallellisation since we only post the sends
  for(std::set<int>::const_iterator i = targetRanks.begin(); i != targetRanks.end(); ++i) {
    int rank = *i;

    headerRequests[rank] = sendHeader(rank, headers[rank], info);
  }
  
  /*
   * SEND PAYLOAD
   */

  // Send beamlets to all nodes in parallel, so for each beamlet the flags
  // can be send immediately after.
# pragma omp parallel for num_threads(info.beamlets.size())
  for(size_t beamlet = 0; beamlet < info.beamlets.size(); ++beamlet) {
    const struct SampleBufferReader<T>::CopyInstructions::Beamlet &ib = info.beamlets[beamlet];
    const int rank = beamletDistribution.at(beamlet);

    /*
     * OBTAIN FLAGS BEFORE DATA IS SENT
     */

    SparseSet<int64> flagsBefore = flags(info, beamlet);

    /*
     * SEND BEAMLETS
     */

    sendData(rank, beamlet, ib);

    /*
     * OBTAIN FLAGS AFTER DATA IS SENT
     */

    SparseSet<int64> flagsAfter = flags(info, beamlet);

    /*
     * SEND FLAGS
     */

    // The only valid samples are those that existed both
    // before and after the transfer.
    sendFlags(rank, beamlet, flagsBefore & flagsAfter);
  }

  /*
   * WRAP UP ASYNC HEADER SEND
   */

  // No need for parallellisation, because all sends should be done by now. We do
  // need to make sure though, because the Headers will soon go out of scope.
#ifdef SEND_HEADERS_ASYNC
  for(std::map<int, MPI_Request>::const_iterator i = headerRequests.begin(); i != headerRequests.end(); ++i) {
    ThreadSafeMPI::MPI_Wait(i->second);
  }
#endif
}


template<typename T> MPIReceiveStations<T>::MPIReceiveStations( const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize )
:
  lastBlock(stationRanks.size()),
  logPrefix(str(boost::format("[beamlets %u..%u (%u)] [MPIReceiveStations] ") % beamlets[0] % beamlets[beamlets.size()-1] % beamlets.size())),
  stationRanks(stationRanks),
  beamlets(beamlets),
  blockSize(blockSize)
{
  for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
    lastBlock[stat].samples.resize(boost::extents[beamlets.size()][blockSize], 128, heapAllocator);
    lastBlock[stat].flags.resize(boost::extents[beamlets.size()], 128, heapAllocator);
  }
}


template<typename T> MPI_Request MPIReceiveStations<T>::receiveHeader( int rank, struct MPISendStation<T>::Header &header )
{
  MPI_Request request;

  typename MPISendStation<T>::tag_t tag;

  // receive the header
  tag.bits.type = MPISendStation<T>::CONTROL;
  ASSERT(tag.value >= 0); // Silly MPI requirement

#ifdef SEND_HEADERS_ASYNC
  int error = MPI_Irecv(&header, sizeof header, MPI_CHAR, rank, tag.value, MPI_COMM_WORLD, &request);
  ASSERT(error == MPI_SUCCESS);

  return request;
#else
  (void)request;
  int error = MPI_Recv(&header, sizeof header, MPI_CHAR, rank, tag.value, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  ASSERT(error == MPI_SUCCESS);

  return 0;
#endif
}


template<typename T> MPI_Request MPIReceiveStations<T>::receiveBeamlet( int rank, size_t beamlet, int transfer, T *from, size_t nrSamples )
{
  MPI_Request request;

  typename MPISendStation<T>::tag_t tag;
  tag.bits.type    = MPISendStation<T>::BEAMLET;
  tag.bits.beamlet = beamlet;
  tag.bits.transfer = transfer;
  ASSERT(tag.value >= 0); // Silly MPI requirement

  int error = MPI_Irecv(
      from, nrSamples * sizeof(T), MPI_CHAR,
      rank, tag.value,
      MPI_COMM_WORLD, &request);

  ASSERT(error == MPI_SUCCESS);

  return request;
}


template<typename T> MPI_Request MPIReceiveStations<T>::receiveFlags( int rank, size_t beamlet, std::vector<char> &buffer )
{
  MPI_Request request;

  typename MPISendStation<T>::tag_t tag;
  tag.bits.type    = MPISendStation<T>::FLAGS;
  tag.bits.beamlet = beamlet;
  ASSERT(tag.value >= 0); // Silly MPI requirement

  int error = MPI_Irecv(
        &buffer[0], buffer.size(), MPI_CHAR,
        rank, tag.value,
        MPI_COMM_WORLD, &request);

  ASSERT(error == MPI_SUCCESS);

  return request;
}


template<typename T> int MPIReceiveStations<T>::waitAny( std::vector<MPI_Request> &requests )
{
  int idx;

  // Wait for any header request to finish
  // NOTE: MPI_Waitany will overwrite completed entries with
  // MPI_REQUEST_NULL.
  int error = MPI_Waitany(requests.size(), &requests[0], &idx, MPI_STATUS_IGNORE);
  ASSERT(error == MPI_SUCCESS);

  ASSERT(idx != MPI_UNDEFINED);
  ASSERT(requests[idx] == MPI_REQUEST_NULL);

  return idx;
}


template<typename T> void MPIReceiveStations<T>::waitAll( std::vector<MPI_Request> &requests )
{
  if (requests.size() > 0) {
    // NOTE: MPI_Waitall will write MPI_REQUEST_NULL into requests array.
    int error = MPI_Waitall(requests.size(), &requests[0], MPI_STATUS_IGNORE);
    ASSERT(error == MPI_SUCCESS);
  }
}


template<typename T> void MPIReceiveStations<T>::receiveBlock()
{
  // All requests except the headers
  std::vector<MPI_Request> requests(beamlets.size() * 3 * stationRanks.size(), MPI_REQUEST_NULL);
  size_t nrRequests = 0;

  // Post receives for all headers
  std::vector<MPI_Request> header_requests(stationRanks.size(), MPI_REQUEST_NULL);
  std::vector<struct MPISendStation<T>::Header> headers(stationRanks.size());

  for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
    LOG_DEBUG_STR(logPrefix << "Posting receive for header from rank " << stationRanks[stat]);

    // receive the header
    header_requests[stat] = receiveHeader(stationRanks[stat], headers[stat]);
  }

  // Process stations in the order in which we receive the headers
  Matrix< std::vector<char> > metaData(stationRanks.size(), beamlets.size()); // [station][beamlet][data]

  for (size_t i = 0; i < stationRanks.size(); ++i) {
    /*
     * For each station, receive its header, and post the relevant sample and
     * flag Irecvs.
     */

    LOG_DEBUG_STR(logPrefix << "Waiting for headers");

    // Wait for any header request to finish
#ifdef SEND_HEADERS_ASYNC
    int stat = waitAny(header_requests);
#else
    int stat = i;
#endif
    int rank = stationRanks[stat];

    // Check the header
    const struct MPISendStation<T>::Header &header = headers[stat];

    LOG_DEBUG_STR(logPrefix << "Received header from rank " << rank);

    ASSERT(header.to - header.from == (int64)blockSize);
    ASSERTSTR(header.nrBeamlets == beamlets.size(), "Got " << header.nrBeamlets << " beamlets, but expected " << beamlets.size());

    // Post receives for the samples
    for (size_t beamletIdx = 0; beamletIdx < header.nrBeamlets; ++beamletIdx) {
      const size_t beamlet = beamlets[beamletIdx];
      const size_t wrapOffset = header.wrapOffsets[beamletIdx];

      LOG_DEBUG_STR(logPrefix << "Receiving beamlet " << beamlet << " from rank " << rank << " using " << (wrapOffset > 0 ? 2 : 1) << " transfers");

      // First sample transfer
      requests[nrRequests++] = receiveBeamlet(rank, beamlet, 0, &lastBlock[stat].samples[beamletIdx][0], wrapOffset ? wrapOffset : blockSize);

      // Second sample transfer
      if (wrapOffset > 0) {
        requests[nrRequests++] = receiveBeamlet(rank, beamlet, 1, &lastBlock[stat].samples[beamletIdx][wrapOffset], blockSize - wrapOffset);
      }

      // Flags transfer
      metaData[stat][beamletIdx].resize(header.metaDataSize);
      requests[nrRequests++] = receiveFlags(rank, beamlet, metaData[stat][beamletIdx]);
    }
  }

  // Wait for all transfers to finish
  requests.resize(nrRequests);
  waitAll(requests);

  // Convert raw metaData to flags array
  for (size_t stat = 0; stat < stationRanks.size(); ++stat)
    for (size_t beamlet = 0; beamlet < beamlets.size(); ++beamlet)
      lastBlock[stat].flags[beamlet].unmarshall(&metaData[stat][beamlet][0]);

}

}
}


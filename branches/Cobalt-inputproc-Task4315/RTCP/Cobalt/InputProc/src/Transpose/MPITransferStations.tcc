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

#include <boost/format.hpp>

using namespace std;

namespace LOFAR {
  namespace Cobalt {


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


// Wait for any request to finish. Returns the index of the request that
// finished. Finished requests are set to MPI_REQUEST_NULL and ignored in
// subsequent calls.
static int waitAny( std::vector<MPI_Request> &requests )
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


// Wait for all given requests to finish. Finished requests are set to
// MPI_REQUEST_NULL and ignored in subsequent calls.
static void waitAll( std::vector<MPI_Request> &requests )
{
  if (requests.size() > 0) {
    // NOTE: MPI_Waitall will write MPI_REQUEST_NULL into requests array.
    int error = MPI_Waitall(requests.size(), &requests[0], MPI_STATUS_IGNORE);
    ASSERT(error == MPI_SUCCESS);
  }
}

/*
 * A guarded version of MPI_Isend with fewer parameters.
 */
static MPI_Request Guarded_MPI_Isend(void *ptr, size_t numBytes, int destRank, int tag) {
  ASSERT(tag >= 0); // Silly MPI requirement

  MPI_Request request;

  int error;

  error = ::MPI_Isend(ptr, numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
  ASSERT(error == MPI_SUCCESS);

  return request;
}

/*
 * A guarded version of MPI_Irecv with fewer parameters.
 */
static MPI_Request Guarded_MPI_Irecv(void *ptr, size_t numBytes, int srcRank, int tag) {
  ASSERT(tag >= 0); // Silly MPI requirement

  MPI_Request request;

  int error;

  error = ::MPI_Irecv(ptr, numBytes, MPI_CHAR, srcRank, tag, MPI_COMM_WORLD, &request);
  ASSERT(error == MPI_SUCCESS);

  return request;
}


template<typename T> MPISendStation<T>::MPISendStation( const struct BufferSettings &settings, size_t stationIdx, const TimeStamp &from, const TimeStamp &to, size_t blockSize, size_t nrHistorySamples, const std::map<size_t, int> &beamletDistribution )
:
  SampleBufferReader<T>(settings, keys(beamletDistribution), from, to, blockSize, nrHistorySamples),
  logPrefix(str(boost::format("[station %s] [MPISendStation] ") % settings.station.stationName)),
  stationIdx(stationIdx),
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
  tag.bits.station  = stationIdx;

  return Guarded_MPI_Isend(&header, sizeof header, rank, tag.value);
}


template<typename T> unsigned MPISendStation<T>::sendData( int rank, unsigned beamlet, const struct SampleBufferReader<T>::CopyInstructions::Beamlet &ib, MPI_Request requests[2] )
{
  LOG_DEBUG_STR(logPrefix << "Sending beamlet " << beamlet << " to rank " << rank << " using " << ib.nrRanges << " transfers");

  // Send beamlet using 1 or 2 transfers
  for(unsigned transfer = 0; transfer < ib.nrRanges; ++transfer) {
    union tag_t tag;

    tag.bits.type     = BEAMLET;
    tag.bits.station  = stationIdx;
    tag.bits.beamlet  = beamlet;
    tag.bits.transfer = transfer;

    const T *from = ib.ranges[transfer].from;
    const T *to   = ib.ranges[transfer].to;

    ASSERT( from < to ); // There must be data to send, or MPI will error

    requests[transfer] = Guarded_MPI_Isend((void*)from, (to - from) * sizeof(T), rank, tag.value);
  }

  return ib.nrRanges;
}


template<typename T> MPI_Request MPISendStation<T>::sendFlags( int rank, unsigned beamlet, const SparseSet<int64> &flags )
{
  //LOG_DEBUG_STR("Sending flags to rank " << rank);

  std::vector<char> metaData(metaDataSize());

  ssize_t numBytes = flags.marshall(&metaData[0], metaData.size());
  ASSERT(numBytes >= 0);

  union tag_t tag;
  tag.bits.type     = FLAGS;
  tag.bits.station  = stationIdx;
  tag.bits.beamlet  = beamlet;

  return Guarded_MPI_Isend(&metaData[0], metaData.size(), rank, tag.value);
}


template<typename T> void MPISendStation<T>::sendBlock( const struct SampleBufferReader<T>::CopyInstructions &info )
{
  /*
   * SEND HEADERS (ASYNC)
   */

  std::map<int, Header> headers;
  std::vector<MPI_Request> headerRequests;

  for(std::set<int>::const_iterator i = targetRanks.begin(); i != targetRanks.end(); ++i) {
    int rank = *i;

    headerRequests.push_back(sendHeader(rank, headers[rank], info));
  }
  
  /*
   * SEND PAYLOADS
   */
  std::vector<MPI_Request> beamletRequests(info.beamlets.size() * 2, MPI_REQUEST_NULL); // [beamlet][transfer]
  size_t nrBeamletRequests = 0;

  for(size_t beamlet = 0; beamlet < info.beamlets.size(); ++beamlet) {
    const struct SampleBufferReader<T>::CopyInstructions::Beamlet &ib = info.beamlets[beamlet];
    const int rank = beamletDistribution.at(beamlet);

    /*
     * SEND BEAMLETS
     */

    nrBeamletRequests += sendData(rank, beamlet, ib, &beamletRequests[beamlet * 2]);
  }

  /*
   * SEND FLAGS
   */

  std::vector<MPI_Request> flagRequests;

  for(size_t b = 0; b < nrBeamletRequests; ++b) {
    const size_t sendIdx = waitAny(beamletRequests);
    const size_t beamlet  = sendIdx / 2;
    const size_t transfer = sendIdx % 2;
    const size_t nrTransfers = info.beamlets[beamlet].nrRanges;

    // waitAny sets finished requests to MPI_REQUEST_NULL in our array.
    if (nrTransfers == 1 || beamletRequests[beamlet * 2 + (1 - transfer)] == MPI_REQUEST_NULL) {
      /*
       * SEND FLAGS FOR BEAMLET
       */

      const int rank = beamletDistribution.at(beamlet);

      /*
       * OBTAIN FLAGS AFTER DATA IS SENT
       */

      // The only valid samples are those that existed both
      // before and after the transfer.
      SparseSet<int64> finalFlags = info.beamlets[beamlet].flagsAtBegin & flags(info, beamlet);

      /*
       * SEND FLAGS
       */
      flagRequests.push_back(sendFlags(rank, beamlet, finalFlags));
    }
  }

  /*
   * WRAP UP ASYNC SENDS
   */

  // Collect all requests
  std::vector<MPI_Request> allRequests(headerRequests.size() + flagRequests.size());
  std::vector<MPI_Request>::iterator curPos = allRequests.begin();
  curPos = std::copy(headerRequests.begin(), headerRequests.end(), curPos);
  curPos = std::copy(flagRequests.begin(), flagRequests.end(), curPos);

  // Wait on them all
  waitAll(allRequests);
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


template<typename T> MPI_Request MPIReceiveStations<T>::receiveHeader( size_t station, struct MPISendStation<T>::Header &header )
{
  typename MPISendStation<T>::tag_t tag;

  // receive the header
  tag.bits.type    = MPISendStation<T>::CONTROL;
  tag.bits.station = station;

  return Guarded_MPI_Irecv(&header, sizeof header, stationRanks[station], tag.value);
}


template<typename T> MPI_Request MPIReceiveStations<T>::receiveBeamlet( size_t station, size_t beamlet, int transfer, T *from, size_t nrSamples )
{
  typename MPISendStation<T>::tag_t tag;
  tag.bits.type    = MPISendStation<T>::BEAMLET;
  tag.bits.station = station;
  tag.bits.beamlet = beamlet;
  tag.bits.transfer = transfer;

  return Guarded_MPI_Irecv(from, nrSamples * sizeof(T), stationRanks[station], tag.value);
}


template<typename T> MPI_Request MPIReceiveStations<T>::receiveFlags( size_t station, size_t beamlet, std::vector<char> &buffer )
{
  typename MPISendStation<T>::tag_t tag;
  tag.bits.type    = MPISendStation<T>::FLAGS;
  tag.bits.station = station;
  tag.bits.beamlet = beamlet;

  return Guarded_MPI_Irecv(&buffer[0], buffer.size(), stationRanks[station], tag.value);
}


template<typename T> void MPIReceiveStations<T>::receiveBlock()
{
  // All requests except the headers
  std::vector<MPI_Request> requests(beamlets.size() * 3 * stationRanks.size(), MPI_REQUEST_NULL);
  size_t nrRequests = 0;

  /*
   * RECEIVE HEADERS (ASYNC)
   */

  // Post receives for all headers
  std::vector<MPI_Request> header_requests(stationRanks.size(), MPI_REQUEST_NULL);
  std::vector<struct MPISendStation<T>::Header> headers(stationRanks.size());

  for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
    LOG_DEBUG_STR(logPrefix << "Posting receive for header from rank " << stationRanks[stat]);

    // receive the header
    header_requests[stat] = receiveHeader(stat, headers[stat]);
  }

  // Process stations in the order in which we receive the headers
  Matrix< std::vector<char> > metaData(stationRanks.size(), beamlets.size()); // [station][beamlet][data]

  for (size_t i = 0; i < stationRanks.size(); ++i) {
    /*
     * WAIT FOR ANY HEADER
     */

    LOG_DEBUG_STR(logPrefix << "Waiting for headers");

    // Wait for any header request to finish
    int stat = waitAny(header_requests);
    int rank = stationRanks[stat];

    /*
     * CHECK HEADER
     */

    const struct MPISendStation<T>::Header &header = headers[stat];

    LOG_DEBUG_STR(logPrefix << "Received header from rank " << rank);

    ASSERT(header.to - header.from == (int64)blockSize);
    ASSERTSTR(header.nrBeamlets == beamlets.size(), "Got " << header.nrBeamlets << " beamlets, but expected " << beamlets.size());

    // Post receives for all beamlets from this station
    for (size_t beamletIdx = 0; beamletIdx < header.nrBeamlets; ++beamletIdx) {
      const size_t beamlet = beamlets[beamletIdx];
      const size_t wrapOffset = header.wrapOffsets[beamletIdx];

      /*
       * RECEIVE BEAMLET (ASYNC)
       */

      LOG_DEBUG_STR(logPrefix << "Receiving beamlet " << beamlet << " from rank " << rank << " using " << (wrapOffset > 0 ? 2 : 1) << " transfers");

      // First sample transfer
      requests[nrRequests++] = receiveBeamlet(stat, beamlet, 0, &lastBlock[stat].samples[beamletIdx][0], wrapOffset ? wrapOffset : blockSize);

      // Second sample transfer
      if (wrapOffset > 0) {
        requests[nrRequests++] = receiveBeamlet(stat, beamlet, 1, &lastBlock[stat].samples[beamletIdx][wrapOffset], blockSize - wrapOffset);
      }

      /*
       * RECEIVE FLAGS (ASYNC)
       */

      metaData[stat][beamletIdx].resize(header.metaDataSize);
      requests[nrRequests++] = receiveFlags(stat, beamlet, metaData[stat][beamletIdx]);
    }
  }

  /*
   * WAIT FOR ALL DATA TO ARRIVE
   */

  requests.resize(nrRequests);
  waitAll(requests);

  /*
   * PROCESS DATA
   */

  // Convert raw metaData to flags array
  for (size_t stat = 0; stat < stationRanks.size(); ++stat)
    for (size_t beamlet = 0; beamlet < beamlets.size(); ++beamlet)
      lastBlock[stat].flags[beamlet].unmarshall(&metaData[stat][beamlet][0]);
}

}
}


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
#include <Common/Thread/Mutex.h>

namespace LOFAR {
  namespace Cobalt {

Mutex MPIMutex;


template<typename T> MPISendStation<T>::MPISendStation( const struct BufferSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, size_t nrHistorySamples, const std::vector<size_t> &beamlets, unsigned destRank )
:
  SampleBufferReader<T>(settings, beamlets, from, to, blockSize, nrHistorySamples),
  destRank(destRank),
  requests(1 + beamlets.size() * 3, 0), // apart from the header, at most three transfers per beamlet: one or two for the samples, plus one for the flags
  nrRequests(0),
  metaData(this->buffer.nrBoards, metaDataSize())
{
}


template<typename T> void MPISendStation<T>::copyStart( const TimeStamp &from, const TimeStamp &to, const std::vector<size_t> &wrapOffsets )
{
  Header header;

  // Copy static information
  header.station      = this->settings.station;
  header.from         = from;
  header.to           = to;
  header.nrBeamlets   = this->beamlets.size();
  header.metaDataSize = this->metaDataSize();

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
  ssize_t numBytes = info.flags.marshall(&metaData[info.beamlet][0], metaDataSize());

  ASSERT(numBytes >= 0);

  union tag_t tag;

  tag.bits.type     = FLAGS;
  tag.bits.beamlet  = info.beamlet;
  ASSERT(tag.value >= 0); // Silly MPI requirement

  int error = MPI_Isend(
            (void*)&metaData[info.beamlet][0], metaDataSize(), MPI_CHAR,
            destRank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

  ASSERT(error == MPI_SUCCESS);
}


template<typename T> void MPISendStation<T>::copyEnd( const TimeStamp &from, const TimeStamp &to )
{
  (void)from; (void)to;

  int alldone = false;
  std::vector<MPI_Status> statusses(nrRequests);

  // Poll until all transfers are finished. Note that we can't hold the
  // MPIMutex lock, because multiple MPISendStation objects might be active.
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


template<typename T> MPIReceiveStations<T>::MPIReceiveStations( const struct BufferSettings &settings, const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize )
:
  lastBlock(stationRanks.size()),
  settings(settings),
  stationRanks(stationRanks),
  beamlets(beamlets),
  blockSize(blockSize)
{
  for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
    lastBlock[stat].samples.resize(boost::extents[beamlets.size()][blockSize], 128, heapAllocator);
    lastBlock[stat].flags.resize(boost::extents[beamlets.size()], 128, heapAllocator);
  }
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
  Matrix< std::vector<char> > metaData(stationRanks.size(), beamlets.size()); // [station][beamlet][data]

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
          &lastBlock[stat].samples[beamlet][0], sizeof(T) * (wrapOffset ? wrapOffset : blockSize), MPI_CHAR,
          rank, tag.value,
          MPI_COMM_WORLD, &requests[nrRequests++]);

      ASSERT(error == MPI_SUCCESS);

      // Second sample transfer
      if (wrapOffset > 0) {
        tag.bits.transfer = 1;
        ASSERT(tag.value >= 0); // Silly MPI requirement

        error = MPI_Irecv(
            &lastBlock[stat].samples[beamlet][wrapOffset], sizeof(T) * (blockSize - wrapOffset), MPI_CHAR,
            rank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

        ASSERT(error == MPI_SUCCESS);
      }

      // Flags transfer
      tag.value = 0; // reset
      tag.bits.type    = MPISendStation<T>::FLAGS;
      tag.bits.beamlet = beamlet;
      ASSERT(tag.value >= 0); // Silly MPI requirement

      metaData[stat][beamlet].resize(header.metaDataSize);

      error = MPI_Irecv(
            &metaData[stat][0][0], header.metaDataSize, MPI_CHAR,
            rank, tag.value,
            MPI_COMM_WORLD, &requests[nrRequests++]);

      ASSERT(error == MPI_SUCCESS);
    }
  }

  // Wait for all transfers to finish
  if (nrRequests > 0) {
    std::vector<MPI_Status> statusses(nrRequests);

    error = MPI_Waitall(nrRequests, &requests[0], &statusses[0]);
    ASSERT(error == MPI_SUCCESS);
  }

  // Convert raw metaData to flags array
  for (size_t stat = 0; stat < stationRanks.size(); ++stat)
    for (size_t beamlet = 0; beamlet < beamlets.size(); ++beamlet)
      lastBlock[stat].flags[beamlet].unmarshall(&metaData[stat][beamlet][0]);

}

}
}


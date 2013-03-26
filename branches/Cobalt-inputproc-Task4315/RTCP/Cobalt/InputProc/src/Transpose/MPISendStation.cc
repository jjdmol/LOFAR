/* MPISendStation.cc
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

#include <lofar_config.h>
#include "MPISendStation.h"
#include "MapUtil.h"
#include "MPIUtil.h"

#include <SampleType.h>

#include <Common/LofarLogger.h>

#include <boost/format.hpp>

using namespace std;

namespace LOFAR {

  namespace Cobalt {

    MPISendStation::MPISendStation( const struct BufferSettings &settings, size_t stationIdx, const std::map<size_t, int> &beamletDistribution )
    :
      logPrefix(str(boost::format("[station %s] [MPISendStation] ") % settings.station.stationName)),
      settings(settings),
      stationIdx(stationIdx),
      beamletDistribution(beamletDistribution),
      targetRanks(values(beamletDistribution)),
      beamletsOfTarget(inverse(beamletDistribution))
    {
      LOG_INFO_STR(logPrefix << "Initialised");
    }

    template<typename T>
    MPI_Request MPISendStation::sendHeader( int rank, Header &header, const struct SampleBufferReader<T>::Block &block )
    {
      LOG_DEBUG_STR(logPrefix << "Sending header to rank " << rank);

      const std::vector<size_t> &beamlets = beamletsOfTarget.at(rank);

      // Copy static blockrmation
      header.station      = this->settings.station;
      header.from         = block.from;
      header.to           = block.to;
      header.nrBeamlets   = beamlets.size();
      header.metaDataSize = this->metaDataSize();

      // Copy the wrapOffsets
      ASSERT(beamlets.size() <= sizeof header.wrapOffsets / sizeof header.wrapOffsets[0]);

      for(unsigned beamletIdx = 0; beamletIdx < beamlets.size(); ++beamletIdx) {
        const struct SampleBufferReader<T>::Block::Beamlet &ib = block.beamlets[beamlets[beamletIdx]];

        header.wrapOffsets[beamletIdx] = ib.nrRanges == 1 ? 0 : ib.ranges[0].to - ib.ranges[0].from;
      }

      // Send the actual header
      union tag_t tag;
      tag.bits.type     = CONTROL;
      tag.bits.station  = stationIdx;

      return Guarded_MPI_Isend(&header, sizeof header, rank, tag.value);
    }


    template<typename T>
    unsigned MPISendStation::sendData( int rank, unsigned beamlet, const struct SampleBufferReader<T>::Block::Beamlet &ib, MPI_Request requests[2] )
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


    MPI_Request MPISendStation::sendFlags( int rank, unsigned beamlet, const SparseSet<int64> &flags )
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


    template<typename T>
    void MPISendStation::sendBlock( const struct SampleBufferReader<T>::Block &block )
    {
      /*
       * SEND HEADERS (ASYNC)
       */

      std::map<int, Header> headers;
      std::vector<MPI_Request> headerRequests;

      for(std::set<int>::const_iterator i = targetRanks.begin(); i != targetRanks.end(); ++i) {
        int rank = *i;

        headerRequests.push_back(sendHeader<T>(rank, headers[rank], block));
      }
      
      /*
       * SEND PAYLOADS
       */
      std::vector<MPI_Request> beamletRequests(block.beamlets.size() * 2, MPI_REQUEST_NULL); // [beamlet][transfer]
      size_t nrBeamletRequests = 0;

      for(size_t beamletIdx = 0; beamletIdx < block.beamlets.size(); ++beamletIdx) {
        const struct SampleBufferReader<T>::Block::Beamlet &ib = block.beamlets[beamletIdx];
        const int rank = beamletDistribution.at(beamletIdx);

        /*
         * SEND BEAMLETS
         */

        nrBeamletRequests += sendData<T>(rank, beamletIdx, ib, &beamletRequests[beamletIdx * 2]);
      }

      /*
       * SEND FLAGS
       */

      std::vector<MPI_Request> flagRequests;

      for(size_t b = 0; b < nrBeamletRequests; ++b) {
        const size_t sendIdx = waitAny(beamletRequests);
        const size_t beamletIdx  = sendIdx / 2;
        const size_t transfer    = sendIdx % 2;

        const struct SampleBufferReader<T>::Block::Beamlet &ib = block.beamlets[beamletIdx];

        // waitAny sets finished requests to MPI_REQUEST_NULL in our array.
        if (ib.nrRanges == 1 || beamletRequests[beamletIdx * 2 + (1 - transfer)] == MPI_REQUEST_NULL) {
          /*
           * SEND FLAGS FOR BEAMLET
           */

          const int rank = beamletDistribution.at(beamletIdx);

          /*
           * OBTAIN FLAGS AFTER DATA IS SENT
           */

          // The only valid samples are those that existed both
          // before and after the transfer.
          SparseSet<int64> finalFlags = ib.flagsAtBegin & block.flags(beamletIdx);

          /*
           * SEND FLAGS
           */
          flagRequests.push_back(sendFlags(rank, beamletIdx, finalFlags));
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

    // Create all necessary instantiations
#define INSTANTIATE(T) \
      template MPI_Request MPISendStation::sendHeader<T>( int rank, Header &header, const struct SampleBufferReader<T>::Block &block ); \
      template unsigned MPISendStation::sendData<T>( int rank, unsigned beamlet, const struct SampleBufferReader<T>::Block::Beamlet &ib, MPI_Request requests[2] ); \
      template void MPISendStation::sendBlock<T>( const struct SampleBufferReader<T>::Block &block );

    INSTANTIATE(SampleType<i4complex>);
    INSTANTIATE(SampleType<i8complex>);
    INSTANTIATE(SampleType<i16complex>);

  }
}


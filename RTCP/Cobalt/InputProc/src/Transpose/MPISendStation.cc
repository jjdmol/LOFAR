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
 * $Id$
 */

#include <lofar_config.h>
#include "MPISendStation.h"
#include "MapUtil.h"
#include "MPIUtil.h"

#include <InputProc/SampleType.h>

#include <Common/LofarLogger.h>

#include <boost/format.hpp>

using namespace std;
using namespace LOFAR::Cobalt::MPIProtocol;

//#define DEBUG_MPI

#ifdef DEBUG_MPI
#define DEBUG(str)    LOG_DEBUG_STR(__PRETTY_FUNCTION__ << ": " << str)
#else
#define DEBUG(str)
#endif

namespace LOFAR {

  namespace Cobalt {

    MPISendStation::MPISendStation( const struct BufferSettings &settings, size_t stationIdx, const std::map<int, std::vector<size_t> > &beamletDistribution )
    :
      logPrefix(str(boost::format("[station %s] [MPISendStation] ") % settings.station.stationName)),
      settings(settings),
      stationIdx(stationIdx),
      beamletDistribution(beamletDistribution),
      targetRanks(keys(beamletDistribution)),
      beamletTargets(inverse(beamletDistribution))
    {
      LOG_INFO_STR(logPrefix << "Initialised");

      // Check whether we send each subband to at most one node
      ASSERT(beamletTargets.size() == values(beamletDistribution).size());

      // Set static header info
      for(std::vector<int>::const_iterator rank = targetRanks.begin(); rank != targetRanks.end(); ++rank) {
        headers[*rank].station      = settings.station;
      }

      // Set beamlet info
      for(std::map<int, std::vector<size_t> >::const_iterator dest = beamletDistribution.begin(); dest != beamletDistribution.end(); ++dest) {
        const int rank = dest->first;
        const std::vector<size_t> &beamlets = dest->second;

        Header &header = headers[rank];

        header.nrBeamlets = beamlets.size();
        ASSERT(header.nrBeamlets < sizeof header.beamlets / sizeof header.beamlets[0]);

        std::copy(beamlets.begin(), beamlets.end(), &header.beamlets[0]);
      }
    }


    template<typename T>
    MPI_Request MPISendStation::sendHeader( int rank, struct MPIProtocol::Header &header, const struct Block<T> &block )
    {
      DEBUG(logPrefix << "Sending header to rank " << rank);

      // Copy dynamic header info
      header.from             = block.from;
      header.to               = block.to;

      // Copy the beam-specific data
      ASSERT(header.nrBeamlets <= sizeof header.wrapOffsets / sizeof header.wrapOffsets[0]);

      for(size_t i = 0; i < header.nrBeamlets; ++i) {
        size_t beamletIdx = header.beamlets[i];
        const struct Block<T>::Beamlet &ib = block.beamlets[beamletIdx];

        header.wrapOffsets[i] = ib.nrRanges == 1 ? 0 : ib.ranges[0].to - ib.ranges[0].from;
      }

      // Send the actual header
      union tag_t tag;
      tag.bits.type     = CONTROL;
      tag.bits.station  = stationIdx;

      return Guarded_MPI_Isend(&header, sizeof header, rank, tag.value);
    }


    template<typename T>
    unsigned MPISendStation::sendData( int rank, unsigned beamlet, const struct Block<T>::Beamlet &ib, MPI_Request requests[2] )
    {
      DEBUG(logPrefix << "Sending beamlet " << beamlet << " to rank " << rank << " using " << ib.nrRanges << " transfers");

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


    MPI_Request MPISendStation::sendMetaData( int rank, unsigned beamlet, const struct MPIProtocol::MetaData &metaData )
    {
      DEBUG("Sending flags to rank " << rank);

      union tag_t tag;
      tag.bits.type     = METADATA;
      tag.bits.station  = stationIdx;
      tag.bits.beamlet  = beamlet;

      return Guarded_MPI_Isend(&metaData, sizeof metaData, rank, tag.value);
    }


    template<typename T>
    void MPISendStation::sendBlock( const struct Block<T> &block, std::vector<SubbandMetaData> &metaData )
    {
      DEBUG("entry");

      ASSERT(metaData.size() == block.beamlets.size());

      /*
       * SEND HEADERS
       */

      std::vector<MPI_Request> headerRequests;

      for(std::vector<int>::const_iterator rank = targetRanks.begin(); rank != targetRanks.end(); ++rank) {
        headerRequests.push_back(sendHeader<T>(*rank, headers[*rank], block));
      }

      /*
       * SEND BEAMLETS
       */
      std::vector<MPI_Request> beamletRequests(block.beamlets.size() * 2, MPI_REQUEST_NULL); // [beamlet][transfer]
      size_t nrBeamletRequests = 0;
      for(std::map<size_t, int>::const_iterator dest = beamletTargets.begin(); dest != beamletTargets.end(); ++dest) {
        const size_t beamletIdx = dest->first;
        const int rank = dest->second;

        ASSERTSTR(beamletIdx < block.beamlets.size(), "Want to send beamlet #" << beamletIdx << " but block only contains " << block.beamlets.size() << " beamlets");

        // Send beamlet
        const struct Block<T>::Beamlet &ib = block.beamlets[beamletIdx];

        nrBeamletRequests += sendData<T>(rank, beamletIdx, ib, &beamletRequests[beamletIdx * 2]);
      }

      /*
       * SEND METADATA
       */

      std::vector<MPI_Request> metaDataRequests;
      std::vector<struct MetaData> metaDataBuffers(block.beamlets.size());

      for(size_t b = 0; b < nrBeamletRequests; ++b) {
        const size_t sendIdx           = waitAny(beamletRequests);
        const size_t globalBeamletIdx  = sendIdx / 2;
        const size_t transfer          = sendIdx % 2;

        const struct Block<T>::Beamlet &ib = block.beamlets[globalBeamletIdx];

        // waitAny sets finished requests to MPI_REQUEST_NULL in our array.
        if (ib.nrRanges == 1 || beamletRequests[globalBeamletIdx * 2 + (1 - transfer)] == MPI_REQUEST_NULL) {
          /*
           * SEND FLAGS FOR BEAMLET
           */

          const int rank = beamletTargets.at(globalBeamletIdx);

          /*
           * OBTAIN FLAGS AFTER DATA IS SENT
           */

          // The only valid samples are those that existed both
          // before and after the transfer.

          SubbandMetaData &md = metaData[globalBeamletIdx];
          md.flags = block.beamlets[globalBeamletIdx].flagsAtBegin | block.flags(globalBeamletIdx);

          struct MetaData &buffer = metaDataBuffers[globalBeamletIdx];
          buffer = md;

          /*
           * SEND FLAGS
           */
          metaDataRequests.push_back(sendMetaData(rank, globalBeamletIdx, buffer));
        }
      }

      /*
       * WRAP UP ASYNC SENDS
       */

      // Collect all requests
      std::vector<MPI_Request> allRequests(headerRequests.size() + metaDataRequests.size());
      std::vector<MPI_Request>::iterator curPos = allRequests.begin();
      curPos = std::copy(headerRequests.begin(), headerRequests.end(), curPos);
      curPos = std::copy(metaDataRequests.begin(), metaDataRequests.end(), curPos);

      // Wait on them all
      waitAll(allRequests);

      DEBUG("exit");
    }

    // Create all necessary instantiations
#define INSTANTIATE(T) \
      template MPI_Request MPISendStation::sendHeader<T>( int rank, struct MPIProtocol::Header &header, const struct Block<T> &block ); \
      template unsigned MPISendStation::sendData<T>( int rank, unsigned beamlet, const struct Block<T>::Beamlet &ib, MPI_Request requests[2] ); \
      template void MPISendStation::sendBlock<T>( const struct Block<T> &block, std::vector<SubbandMetaData> &metaData );

    INSTANTIATE(SampleType<i4complex>);
    INSTANTIATE(SampleType<i8complex>);
    INSTANTIATE(SampleType<i16complex>);

  }
}


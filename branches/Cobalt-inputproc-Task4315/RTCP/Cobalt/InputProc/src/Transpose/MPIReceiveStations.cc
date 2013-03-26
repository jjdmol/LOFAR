/* MPIReceiveStations.cc
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
#include "MPIReceiveStations.h"
#include "MPIUtil.h"

#include <SampleType.h>

#include <Common/LofarLogger.h>

#include <boost/format.hpp>

using namespace std;
using namespace LOFAR::Cobalt::MPIProtocol;

namespace LOFAR {
  namespace Cobalt {

    MPIReceiveStations::MPIReceiveStations( const std::vector<int> &stationRanks, const std::vector<size_t> &beamlets, size_t blockSize )
    :
      logPrefix(str(boost::format("[beamlets %u..%u (%u)] [MPIReceiveStations] ") % beamlets[0] % beamlets[beamlets.size()-1] % beamlets.size())),
      stationRanks(stationRanks),
      beamlets(beamlets),
      blockSize(blockSize)
    {
    }


    MPI_Request MPIReceiveStations::receiveHeader( size_t station, struct MPIProtocol::Header &header )
    {
      tag_t tag;
      tag.bits.type    = CONTROL;
      tag.bits.station = station;

      return Guarded_MPI_Irecv(&header, sizeof header, stationRanks[station], tag.value);
    }


    template<typename T>
    MPI_Request MPIReceiveStations::receiveBeamlet( size_t station, size_t beamlet, int transfer, T *from, size_t nrSamples )
    {
      tag_t tag;
      tag.bits.type    = BEAMLET;
      tag.bits.station = station;
      tag.bits.beamlet = beamlet;
      tag.bits.transfer = transfer;

      return Guarded_MPI_Irecv(from, nrSamples * sizeof(T), stationRanks[station], tag.value);
    }


    MPI_Request MPIReceiveStations::receiveFlags( size_t station, size_t beamlet, std::vector<char> &buffer )
    {
      tag_t tag;
      tag.bits.type    = FLAGS;
      tag.bits.station = station;
      tag.bits.beamlet = beamlet;

      return Guarded_MPI_Irecv(&buffer[0], buffer.size(), stationRanks[station], tag.value);
    }


    template<typename T>
    void MPIReceiveStations::receiveBlock( std::vector< struct MPIReceiveStations::Block<T> > &blocks )
    {
      ASSERT(blocks.size() == stationRanks.size());

      // All requests except the headers
      std::vector<MPI_Request> requests;

      /*
       * RECEIVE HEADERS (ASYNC)
       */

      // Post receives for all headers
      std::vector<MPI_Request> header_requests(stationRanks.size(), MPI_REQUEST_NULL);
      std::vector<struct Header> headers(stationRanks.size());

      for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
        LOG_DEBUG_STR(logPrefix << "Posting receive for header from rank " << stationRanks[stat]);

        // receive the header
        header_requests[stat] = receiveHeader(stat, headers[stat]);
      }

      // Process stations in the order in which we receive the headers
      Matrix< std::vector<char> > flagsBlobs(stationRanks.size(), beamlets.size()); // [station][beamlet][data]

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

        const struct Header &header = headers[stat];

        LOG_DEBUG_STR(logPrefix << "Received header from rank " << rank);

        ASSERT(header.to - header.from == (int64)blockSize);
        ASSERTSTR(header.nrBeamlets == beamlets.size(), "Got " << header.nrBeamlets << " beamlets, but expected " << beamlets.size());

        // Post receives for all beamlets from this station
        for (size_t beamletIdx = 0; beamletIdx < header.nrBeamlets; ++beamletIdx) {
          const size_t beamlet = beamlets[beamletIdx];
          const size_t wrapOffset = header.wrapOffsets[beamletIdx];

          ASSERTSTR(header.beamlets[beamletIdx] == beamlet, "Got beamlet " << header.beamlets[beamletIdx] << ", but expected beamlet " << beamlet);
          ASSERT(wrapOffset < blockSize);

          /*
           * RECEIVE BEAMLET (ASYNC)
           */

          LOG_DEBUG_STR(logPrefix << "Receiving beamlet " << beamlet << " from rank " << rank << " using " << (wrapOffset > 0 ? 2 : 1) << " transfers");

          // First sample transfer
          requests.push_back(receiveBeamlet<T>(stat, beamlet, 0, &blocks[stat].beamlets[beamletIdx].samples[0], wrapOffset ? wrapOffset : blockSize));

          // Second sample transfer
          if (wrapOffset > 0) {
            requests.push_back(receiveBeamlet<T>(stat, beamlet, 1, &blocks[stat].beamlets[beamletIdx].samples[wrapOffset], blockSize - wrapOffset));
          }

          /*
           * RECEIVE FLAGS (ASYNC)
           */

          flagsBlobs[stat][beamletIdx].resize(header.flagsSize);
          requests.push_back(receiveFlags(stat, beamlet, flagsBlobs[stat][beamletIdx]));
        }
      }

      /*
       * WAIT FOR ALL DATA TO ARRIVE
       */

      waitAll(requests);

      /*
       * PROCESS DATA
       */

      for (size_t stat = 0; stat < stationRanks.size(); ++stat) {
        const Header &header = headers[stat];

        // Convert the flags array
        for (size_t beamletIdx = 0; beamletIdx < beamlets.size(); ++beamletIdx) {
          const std::vector<char> &flagsBlob = flagsBlobs[stat][beamletIdx];

          blocks[stat].beamlets[beamletIdx].flags.unmarshall(&flagsBlob[0]);
        }

        // Copy the metaData blob
        blocks[stat].metaDataBlob = std::vector<char>(&header.metaDataBlob[0], &header.metaDataBlob[header.metaDataBlobSize]);
      }
    }

    // Create all necessary instantiations
#define INSTANTIATE(T) \
    template MPI_Request MPIReceiveStations::receiveBeamlet<T>( size_t station, size_t beamlet, int transfer, T *from, size_t nrSamples ); \
    template void MPIReceiveStations::receiveBlock<T>( std::vector< struct MPIReceiveStations::Block<T> > &blocks );

    INSTANTIATE(SampleType<i4complex>);
    INSTANTIATE(SampleType<i8complex>);
    INSTANTIATE(SampleType<i16complex>);

  }
}


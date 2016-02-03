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
 * $Id$
 */

#include <lofar_config.h>
#include "MPIReceiveStations.h"
#include "MPIUtil.h"

#include <InputProc/SampleType.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <boost/format.hpp>

using namespace std;
using namespace LOFAR::Cobalt::MPIProtocol;

namespace LOFAR {
  namespace Cobalt {

    namespace {
      ssize_t first(const std::vector<size_t> &v) {
        return v.empty() ? -1 : v.at(0);
      }
      ssize_t last(const std::vector<size_t> &v) {
        return v.empty() ? -1 : v.at(v.size()-1);
      }
    };

    MPIReceiveStations::MPIReceiveStations( size_t nrStations, const std::vector<size_t> &beamlets, size_t blockSize )
    :
      logPrefix(str(boost::format("[beamlets %d..%d (%u)] [MPIReceiveStations] ") % first(beamlets) % last(beamlets) % beamlets.size())),
      nrStations(nrStations),
      beamlets(beamlets),
      blockSize(blockSize),
      stationSourceRanks(nrStations, MPI_ANY_SOURCE),
      stationDone(nrStations, false)
    {
    }


    template<typename T>
    MPI_Request MPIReceiveStations::receiveData( size_t station, T *buffer )
    {
      tag_t tag;
      tag.bits.type    = BEAMLET;
      tag.bits.station = station;
      tag.bits.beamlet = beamlets[0];

      return Guarded_MPI_Irecv(buffer, beamlets.size() * blockSize * sizeof(T), stationSourceRanks[station], tag.value);
    }


    MPI_Request MPIReceiveStations::receiveMetaData( size_t station, struct MPIProtocol::MetaData *buffer )
    {
      tag_t tag;
      tag.bits.type    = METADATA;
      tag.bits.station = station;
      tag.bits.beamlet = beamlets[0];

      return Guarded_MPI_Irecv(buffer, beamlets.size() * sizeof(struct MPIProtocol::MetaData), stationSourceRanks[station], tag.value);
    }


    template<typename T>
    bool MPIReceiveStations::receiveBlock( MultiDimArray<T, 3> &data, MultiDimArray<struct MPIProtocol::MetaData, 2> &metaData )
    {
      if (beamlets.empty())
        return true;

      ASSERT(data.num_elements() == nrStations * beamlets.size() * blockSize);
      ASSERT(metaData.num_elements() == nrStations * beamlets.size());

      // All requests except the headers
      std::vector<MPI_Request> requests;

      {
        ScopedLock sl(MPIMutex);

        for (size_t stat = 0; stat < nrStations; ++stat) {
          if (stationDone[stat]) {
            // Set EOS bit for this station
            for (size_t b = 0; b < beamlets.size(); ++b )
              metaData[stat][b].EOS = true;

            continue;
          }

          requests.push_back(receiveData<T>(stat, &data[stat][0][0]));
          requests.push_back(receiveMetaData(stat, &metaData[stat][0]));
        }
      }

      /*
       * WAIT FOR ALL DATA TO ARRIVE
       */

      RequestSet payload_rs(requests, true, str(boost::format("%s data & metadata") % logPrefix));
      payload_rs.waitAll();

      // Sync stationDone and metaData[stat][0].EOS, and count the number of finished stations
      size_t nrStationsDone = 0;

      for (size_t stat = 0; stat < nrStations; ++stat) {
        if (metaData[stat][0].EOS)
          stationDone[stat] = true;

        if (stationDone[stat])
          nrStationsDone++;
      }

      return nrStationsDone == nrStations;
    }

    // Create all necessary instantiations
#define INSTANTIATE(T) \
    template MPI_Request MPIReceiveStations::receiveData<T>( size_t station, T *buffer ); \
    template bool MPIReceiveStations::receiveBlock<T>( MultiDimArray<T, 3> &data, MultiDimArray<struct MPIProtocol::MetaData, 2> &metaData );

    INSTANTIATE(SampleType<i4complex>);
    INSTANTIATE(SampleType<i8complex>);
    INSTANTIATE(SampleType<i16complex>);

  }
}


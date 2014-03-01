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
#include "MPIUtil2.h"

#include <InputProc/SampleType.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <CoInterface/PrintVector.h>

#include <boost/format.hpp>

using namespace std;
using namespace LOFAR::Cobalt::MPIProtocol;

//#define DEBUG_MPI

#ifdef DEBUG_MPI
//#define DEBUG(str)    LOG_DEBUG_STR(__PRETTY_FUNCTION__ << ": " << str)
#define DEBUG(str)    LOG_DEBUG_STR(str)
#else
#define DEBUG(str)
#endif

namespace LOFAR {

  namespace Cobalt {

    MPISendStation::MPISendStation( size_t stationIdx, int targetRank, const std::vector<size_t> &beamlets, size_t nrSamples )
    :
      logPrefix(str(boost::format("[station %s to rank %s] [MPISendStation] ") % stationIdx % targetRank)),
      stationIdx(stationIdx),
      targetRank(targetRank),
      beamlets(beamlets),
      nrSamples(nrSamples)
    {
      LOG_DEBUG_STR(logPrefix << "Initialised");

      // Check whether we send each subband to at most one node
      ASSERT(!beamlets.empty());
    }


    MPISendStation::~MPISendStation()
    {
    }

    template<typename T>
    MPI_Request MPISendStation::sendData( const T *data )
    {
      DEBUG(logPrefix << "Sending beamlets to rank " << targetRank);

      union tag_t tag;
      tag.bits.type     = BEAMLET;
      tag.bits.station  = stationIdx;
      tag.bits.beamlet  = beamlets[0];

      return Guarded_MPI_Issend(data, nrSamples * sizeof(T), targetRank, tag.value);
    }


    MPI_Request MPISendStation::sendMetaData( const MPIProtocol::MetaData *metaData )
    {
      DEBUG("Sending flags to rank " << targetRank);

      union tag_t tag;
      tag.bits.type     = METADATA;
      tag.bits.station  = stationIdx;
      tag.bits.beamlet  = beamlets[0];

      // Flags are sent if the data have been transferred,
      // and the flags Irecv is posted before the data Irecvs,
      // so we are sure that the flag Irecv is posted.
      return Guarded_MPI_Isend(metaData, beamlets.size() * sizeof *metaData, targetRank, tag.value);
    }


    template<typename T>
    void MPISendStation::sendBlock( const T *data, MPIProtocol::MetaData *metaData )
    {
      DEBUG("entry");

      /*
       * SEND BEAMLETS AND METADATA
       */

      std::vector<MPI_Request> requests(2, MPI_REQUEST_NULL);

      {
        ScopedLock sl(MPIMutex);

        requests[0] = sendData(data);
        requests[1] = sendMetaData(metaData);
      }

      RequestSet rs(requests, true, str(boost::format("%s data") % logPrefix));
      rs.waitAll();

      DEBUG("exit");
    }

    // Create all necessary instantiations
#define INSTANTIATE(T) \
      template MPI_Request MPISendStation::sendData<T>( const T* data ); \
      template void MPISendStation::sendBlock<T>( const T* data, MPIProtocol::MetaData *metaData );

    INSTANTIATE(SampleType<i4complex>);
    INSTANTIATE(SampleType<i8complex>);
    INSTANTIATE(SampleType<i16complex>);

  }
}


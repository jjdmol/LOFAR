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

    MPISendStation::MPISendStation( const struct BufferSettings &settings, size_t stationIdx, int targetRank, const std::vector<size_t> &beamlets )
    :
      logPrefix(str(boost::format("[station %s to rank %s] [MPISendStation] ") % settings.station.name() % targetRank)),
      settings(settings),
      stationIdx(stationIdx),
      targetRank(targetRank),
      beamlets(beamlets),
      metaData(beamlets.size(), 1, mpiAllocator)
    {
      LOG_DEBUG_STR(logPrefix << "Initialised");

      // Check whether we send each subband to at most one node
      ASSERT(!beamlets.empty());
    }


    MPISendStation::~MPISendStation()
    {
    }


    MPI_Request MPISendStation::sendData()
    {
      DEBUG(logPrefix << "Sending beamlets to rank " << targetRank);

      union tag_t tag;
      tag.bits.type     = BEAMLET;
      tag.bits.station  = stationIdx;
      tag.bits.beamlet  = beamlets[0];

      return Guarded_MPI_Issend(data.get(), blockSize, targetRank, tag.value);
    }


    MPI_Request MPISendStation::sendMetaData()
    {
      DEBUG("Sending flags to rank " << targetRank);

      union tag_t tag;
      tag.bits.type     = METADATA;
      tag.bits.station  = stationIdx;
      tag.bits.beamlet  = beamlets[0];

      // Flags are sent if the data have been transferred,
      // and the flags Irecv is posted before the data Irecvs,
      // so we are sure that the flag Irecv is posted.
      return Guarded_MPI_Isend(&metaData[0], beamlets.size() * sizeof metaData, targetRank, tag.value);
    }


    template<typename T>
    void MPISendStation::sendBlock( const struct Block<T> &block, std::vector<SubbandMetaData> &metaData )
    {
      DEBUG("entry");

      ASSERT(metaData.size() == block.beamlets.size());

      const size_t nrBeamlets = block.beamlets.size();
      const size_t nrSamples  = block.beamlets[0].size();

      if (!data) {
        blockSize = nrSamples * sizeof(T);
        data = static_cast<char*>(mpiAllocator.allocate(nrBeamlets * nrSamples * sizeof(T)));
      }

      MultiDimArray<T,2> dataMatrix(boost::extents[nrBeamlets][nrSamples], (T*)(data.get()), false);

      /*
       * STAGE BEAMLETS
       */

      NSTimer timer(str(boost::format("data processing station %s") % stationIdx), true, false);
      timer.start();
      for(size_t b = 0; b < beamlets.size(); ++b) {
        block.beamlets[b].copy(&dataMatrix[b][0]);
      }
      timer.stop();

      /*
       * COMPUTE METADATA
       */

      for(size_t beamletIdx = 0; beamletIdx < beamlets.size(); beamletIdx++) {
        /*
         * OBTAIN FLAGS AFTER DATA IS SENT
         */

        // The only valid samples are those that existed both
        // before and after the transfer.

        SubbandMetaData &md = metaData[beamletIdx];
        md.flags = block.beamlets[beamletIdx].flagsAtBegin | block.flags(beamletIdx);

        this->metaData[beamletIdx] = md;
      }

      /*
       * SEND BEAMLETS AND METADATA
       */

      std::vector<MPI_Request> requests(2, MPI_REQUEST_NULL);

      {
        ScopedLock sl(MPIMutex);

        requests[0] = sendData();
        requests[1] = sendMetaData();
      }

      RequestSet rs(requests, true, str(boost::format("%s data") % logPrefix));
      rs.waitAll();

      DEBUG("exit");
    }

    // Create all necessary instantiations
#define INSTANTIATE(T) \
      template void MPISendStation::sendBlock<T>( const struct Block<T> &block, std::vector<SubbandMetaData> &metaData );

    INSTANTIATE(SampleType<i4complex>);
    INSTANTIATE(SampleType<i8complex>);
    INSTANTIATE(SampleType<i16complex>);

  }
}


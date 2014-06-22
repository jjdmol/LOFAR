//# StationTranspose.cc: Manages the transpose of station data, which exchanges
//#                      [station][subband] to [subband][station] over MPI.
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "StationTranspose.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <boost/format.hpp>

#ifdef HAVE_MPI
#include <mpi.h>
#include <InputProc/Transpose/MPISendStation.h>
#include <InputProc/Transpose/MapUtil.h>
#include <InputProc/Transpose/MPIUtil.h>
#endif

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Stream/FileStream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/OMPThread.h>
#include <CoInterface/TimeFuncs.h>
#include <CoInterface/Stream.h>
#include <CoInterface/PrintVector.h>

#include <InputProc/SampleType.h>
#include <InputProc/Station/PacketReader.h>
#include <InputProc/Station/PacketFactory.h>
#include <InputProc/Station/PacketStream.h>
#include <InputProc/Buffer/BoardMode.h>
#include <InputProc/Delays/Delays.h>
#include <InputProc/RSPTimeStamp.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

namespace LOFAR {
  namespace Cobalt {

    template<typename SampleT>
    bool MPIData<SampleT>::write(const struct RSP &packet, const ssize_t *beamletIndices, size_t nrBeamletIndices) {
      /* An optimisation as we'll never encounter anything else */
      ASSERTSTR(packet.header.nrBlocks == 16, "Packet has " << (int)packet.header.nrBlocks << " samples/beamlet, expected 16.");
      const size_t nrSamples = 16;

      /* Prevent accesses beyond beamletIndices */
      ASSERTSTR(packet.header.nrBeamlets <= nrBeamletIndices,"Packet has " << (int)packet.header.nrBeamlets << " beamlets, expected at most " << nrBeamletIndices << ". Packet bitmode is " << packet.bitMode());

      const uint64_t packetBegin = packet.timeStamp();
      const uint64_t packetEnd   = packetBegin + nrSamples;

      bool consider_next = false;

      const SampleT *srcPtr = reinterpret_cast<const SampleT*>(&packet.payload.data[0]);

      for (size_t b = 0; b < packet.header.nrBeamlets; ++b) {
        const ssize_t absBeamlet = beamletIndices[b];

        /* Discard beamlets that are not used in the observation */
        if (absBeamlet == -1)
          continue;

        /* Reading with offset X is the same as writing that data with offset -X */
        const ssize_t offset = read_offsets[absBeamlet];
        const uint64_t beamletBegin = packetBegin - offset;
        const uint64_t beamletEnd   = packetEnd   - offset;

        /* XXXX    = packet data
         * [.....] = this->data
         */

        /* XXXX [......] -> discard */
        if (beamletEnd < from)
          continue;

        /* [......] XXXX -> discard */
        if (beamletBegin >= to) {
          consider_next = true;
          continue;
        }

        // number of samples to transfer
        size_t nrSamplesToCopy  = nrSamples;

        // first sample to write
        size_t firstSample = 0;

        /* XX [XX....] -> cut off start */
        if (beamletBegin < from) {
          firstSample      = from - beamletBegin;
          nrSamplesToCopy -= firstSample;
        }

        /* [...XX] XX -> cut off end */
        if (beamletEnd > to) {
          nrSamplesToCopy -= beamletEnd - to;
          consider_next = true;
        }

        if (beamletEnd == to) {
          // The next packet has at most 1 sample overlap with this packet,
          // due to the speed of light and the earth's rotation speed.
          consider_next = true;
        }

        /* Absolute offset within our block */
        const size_t absSample = beamletBegin - from + firstSample;

        /* Write the remaining data */
        memcpy(&mpi_samples[absBeamlet][absSample],
         srcPtr + b * nrSamples + firstSample,
         nrSamplesToCopy * sizeof(SampleT));

        /* Update the flags */
        metaData[absBeamlet].flags.include(absSample, absSample + nrSamplesToCopy);
      }

      return consider_next;
    }

    template struct MPIData< SampleType<i16complex> >;
    template struct MPIData< SampleType<i8complex> >;
    template struct MPIData< SampleType<i4complex> >;

    MPISender::MPISender( const std::string &logPrefix, size_t stationIdx, const SubbandDistribution &subbandDistribution )
    :
      logPrefix(logPrefix),
      stationIdx(stationIdx),
      subbandDistribution(subbandDistribution),
      targetRanks(keys(subbandDistribution)),
      subbandOffsets(targetRanks.size(), 0),
      nrSubbands(values(subbandDistribution).size())
    {
      // Determine the offset of the set of subbands for each rank within
      // the members in MPIData<SampleT>.
      for (size_t rank = 0; rank < targetRanks.size(); ++rank)
        for(size_t i = 0; i < rank; ++i)
          subbandOffsets[rank] += subbandDistribution.at(i).size();
    }


    template <typename SampleT>
    void MPISender::sendBlocks( Queue< SmartPtr< MPIData<SampleT> > > &inputQueue, Queue< SmartPtr< MPIData<SampleT> > > &outputQueue )
    {
      SmartPtr< MPIData<SampleT> > mpiData;

      NSTimer mpiSendTimer(str(format("%s MPI send data") % logPrefix), true, true);

      size_t nrProcessedSamples = 0;
      size_t nrFlaggedSamples = 0;

      while((mpiData = inputQueue.remove()) != NULL) {
        const ssize_t block = mpiData->block;
        const size_t  nrSamples = mpiData->nrSamples;

        nrProcessedSamples += nrSamples * nrSubbands;

        LOG_DEBUG_STR(logPrefix << str(format("[block %d] Finalising metaData") % block));

        // Convert the metaData -> mpi_metaData for transfer over MPI
        for(size_t sb = 0; sb < mpiData->metaData.size(); ++sb) {
          SubbandMetaData &md = mpiData->metaData[sb];

          // MPIData::write adds flags for what IS present, but the receiver
          // needs flags for what IS NOT present. So invert the flags here.
          mpiData->metaData[sb].flags = md.flags.invert(0, nrSamples);

          nrFlaggedSamples += md.flags.count();

          // Write the meta data into the fixed buffer.
          mpiData->mpi_metaData[sb] = md;
        }

        LOG_DEBUG_STR(logPrefix << str(format("[block %d] Sending data") % block));

        mpiSendTimer.start();

        std::vector<MPI_Request> requests;

        {
          ScopedLock sl(MPIMutex);

          for(size_t i = 0; i < targetRanks.size(); ++i) {
            const int rank = targetRanks.at(i);

            if (subbandDistribution.at(rank).empty())
              continue;

            MPISendStation sender(stationIdx, rank, subbandDistribution.at(rank), nrSamples);

            const size_t offset = subbandOffsets[rank];

            requests.push_back(sender.sendData<SampleT>(&mpiData->mpi_samples[offset][0]));
            requests.push_back(sender.sendMetaData(&mpiData->mpi_metaData[offset]));
          }
        }

        RequestSet rs(requests, true, str(format("station %d block %d") % stationIdx % block));
        rs.waitAll();

        mpiSendTimer.stop();

        LOG_DEBUG_STR(logPrefix << str(format("[block %d] Data sent") % block));

        outputQueue.append(mpiData);
        ASSERT(!mpiData);
      }

      // report average loss
      const double avgloss = nrProcessedSamples == 0 ? 0.0 : 100.0 * nrFlaggedSamples / nrProcessedSamples;

      LOG_INFO_STR(logPrefix << str(format("Average data loss/flagged: %.4f%%") % avgloss));
    }

    template void MPISender::sendBlocks( Queue< SmartPtr< MPIData< SampleType<i16complex> > > > &inputQueue, Queue< SmartPtr< MPIData< SampleType<i16complex> > > > &outputQueue );
    template void MPISender::sendBlocks( Queue< SmartPtr< MPIData< SampleType<i8complex> > > > &inputQueue, Queue< SmartPtr< MPIData< SampleType<i8complex> > > > &outputQueue );
    template void MPISender::sendBlocks( Queue< SmartPtr< MPIData< SampleType<i4complex> > > > &inputQueue, Queue< SmartPtr< MPIData< SampleType<i4complex> > > > &outputQueue );
  }
}


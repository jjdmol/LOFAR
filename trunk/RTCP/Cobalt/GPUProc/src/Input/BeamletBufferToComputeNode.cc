//# BeamletBufferToComputeNode.cc: Catch RSP ethernet frames and synchronize RSP inputs
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "BeamletBufferToComputeNode.h"

#include <sys/time.h>
#include <cstdio>
#include <stdexcept>
#include <iomanip>
#include <boost/format.hpp>

#include <Common/LofarTypes.h>
#include <Common/Timer.h>
#include <Common/PrettyUnits.h>
#include <Common/LofarLogger.h>

#include <GPUProc/Scheduling.h>
#include <GPUProc/SubbandMetaData.h>

namespace LOFAR
{
  namespace Cobalt
  {


    template<typename SAMPLE_TYPE>
    const unsigned BeamletBufferToComputeNode<SAMPLE_TYPE>::itsMaximumDelay;


    template<typename SAMPLE_TYPE>
    BeamletBufferToComputeNode<SAMPLE_TYPE>::BeamletBufferToComputeNode(const Parset &ps, const std::string &stationName, const std::vector<SmartPtr<BeamletBuffer<SAMPLE_TYPE> > > &beamletBuffers, unsigned firstBlockNumber)
      :
      itsPS(ps),
      itsNrRSPboards(beamletBuffers.size()),
      itsBeamletBuffers(beamletBuffers),
      itsDelayTimer("delay consumer", true, true)
    {
      bool haveStationInput = itsNrRSPboards > 0;
      ASSERTSTR(itsNrRSPboards > 0, "BeamletBufferToComputeNode requires at least one BeamletBuffer");

      itsLogPrefix = str(boost::format("[obs %u station %s] ") % ps.observationID() % stationName);

      itsSubbandBandwidth = ps.subbandBandwidth();
      itsNrSubbands = ps.nrSubbands();
      itsNrSamplesPerSubband = ps.nrSamplesPerSubband();
      itsNrBeams = ps.nrBeams();
      itsMaxNrTABs = ps.maxNrTABs();
      itsNrTABs = ps.nrTABs();

      itsSampleDuration = ps.sampleDuration();
      itsDelayCompensation = ps.delayCompensation();
      itsCorrectClocks = ps.correctClocks();
      itsNeedDelays = itsDelayCompensation || itsMaxNrTABs > 1 || itsCorrectClocks;
      itsSubbandToSAPmapping = ps.subbandToSAPmapping();
      itsSubbandToRSPboardMapping = ps.subbandToRSPboardMapping(stationName);
      itsSubbandToRSPslotMapping = ps.subbandToRSPslotMapping(stationName);

      ASSERT( itsSubbandToSAPmapping.size() == itsNrSubbands );
      ASSERT( itsSubbandToRSPboardMapping.size() == itsNrSubbands );
      ASSERT( itsSubbandToRSPslotMapping.size() == itsNrSubbands );

      itsCurrentTimeStamp = TimeStamp(static_cast<int64>(ps.startTime() * itsSubbandBandwidth + firstBlockNumber * itsNrSamplesPerSubband), ps.clockSpeed());

      itsIsRealTime = ps.realTime();
      itsMaxNetworkDelay = ps.maxNetworkDelay();
      itsNrHistorySamples = ps.nrHistorySamples();

      LOG_DEBUG_STR(itsLogPrefix << "nrSubbands = " << itsNrSubbands);
      LOG_DEBUG_STR(itsLogPrefix << "nrChannelsPerSubband = " << ps.nrChannelsPerSubband());
      LOG_DEBUG_STR(itsLogPrefix << "nrStations = " << ps.nrStations());
      LOG_DEBUG_STR(itsLogPrefix << "nrBitsPerSample = " << ps.nrBitsPerSample());
      LOG_DEBUG_STR(itsLogPrefix << "maxNetworkDelay = " << itsMaxNetworkDelay << " samples");

      if (haveStationInput && itsNeedDelays) {
        itsDelaysAtBegin.resize(itsNrBeams, itsMaxNrTABs + 1);
        itsDelaysAfterEnd.resize(itsNrBeams, itsMaxNrTABs + 1);
        itsBeamDirectionsAtBegin.resize(itsNrBeams, itsMaxNrTABs + 1);
        itsBeamDirectionsAfterEnd.resize(itsNrBeams, itsMaxNrTABs + 1);

        if (itsDelayCompensation || itsMaxNrTABs > 1) {
          itsDelays = new Delays(ps, stationName, itsCurrentTimeStamp);
          itsDelays->start();
        }

        if (itsCorrectClocks)
          itsClockCorrectionTime = ps.clockCorrectionTime(stationName);

        computeNextDelays(); // initialize itsDelaysAfterEnd before we really start
      }

      itsDelayedStamps.resize(itsNrBeams);
      itsSamplesDelay.resize(itsNrBeams);
      itsFineDelaysAtBegin.resize(itsNrBeams, itsMaxNrTABs + 1);
      itsFineDelaysAfterEnd.resize(itsNrBeams, itsMaxNrTABs + 1);
      itsFlags.resize(boost::extents[itsNrRSPboards][itsNrBeams]);

#if defined HAVE_BGP_ION // FIXME: not in preprocess
      doNotRunOnCore0();
      setPriority(3);
#endif
    }


    template<typename SAMPLE_TYPE>
    BeamletBufferToComputeNode<SAMPLE_TYPE>::~BeamletBufferToComputeNode()
    {
      LOG_DEBUG_STR(itsLogPrefix << "BeamletBufferToComputeNode::~BeamletBufferToComputeNode");

      for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp++)
        itsBeamletBuffers[rsp]->noMoreReading();
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::computeNextDelays()
    {
      // track source

#ifdef USE_VALGRIND
      for (unsigned beam = 0; beam < itsNrBeams; beam++)
        for (unsigned pencil = 0; pencil < itsMaxNrTABs + 1; pencil++)
          itsDelaysAfterEnd[beam][pencil] = 0;
#endif

      if (itsDelays != 0)
        itsDelays->getNextDelays(itsBeamDirectionsAfterEnd, itsDelaysAfterEnd);
      else
        for (unsigned beam = 0; beam < itsNrBeams; beam++)
          for (unsigned pencil = 0; pencil < itsMaxNrTABs + 1; pencil++)
            itsDelaysAfterEnd[beam][pencil] = 0;

      // apply clock correction due to cable differences

      if (itsCorrectClocks)
        for (unsigned beam = 0; beam < itsNrBeams; beam++)
          for (unsigned pencil = 0; pencil < itsMaxNrTABs + 1; pencil++)
            itsDelaysAfterEnd[beam][pencil] += itsClockCorrectionTime;
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::limitFlagsLength(SparseSet<unsigned> &flags)
    {
      const SparseSet<unsigned>::Ranges &ranges = flags.getRanges();

      if (ranges.size() > 16)
        flags.include(ranges[15].begin, ranges.back().end);
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::computeDelays()
    {
      itsDelayTimer.start();

      // begin of this integration is end of previous integration
      itsDelaysAtBegin = itsDelaysAfterEnd;
      itsBeamDirectionsAtBegin = itsBeamDirectionsAfterEnd;

      computeNextDelays();

      for (unsigned beam = 0; beam < itsNrBeams; beam++) {
        // The coarse delay is determined for the center of the current
        // time interval and is expressed in an entire amount of samples.
        //
        // We use the central pencil beam (#0) for the coarse delay compensation.
        signed int coarseDelay = static_cast<signed int>(floor(0.5 * (itsDelaysAtBegin[beam][0] + itsDelaysAfterEnd[beam][0]) * itsSubbandBandwidth + 0.5));

        // The fine delay is determined for the boundaries of the current
        // time interval and is expressed in seconds.
        double d = coarseDelay * itsSampleDuration;

        itsDelayedStamps[beam] -= coarseDelay;
        itsSamplesDelay[beam] = -coarseDelay;

        for (unsigned pencil = 0; pencil < itsNrTABs[beam] + 1; pencil++) {
          // we don't do coarse delay compensation for the individual pencil beams to avoid complexity and overhead
          itsFineDelaysAtBegin[beam][pencil] = static_cast<float>(itsDelaysAtBegin[beam][pencil] - d);
          itsFineDelaysAfterEnd[beam][pencil] = static_cast<float>(itsDelaysAfterEnd[beam][pencil] - d);
        }
      }

      itsDelayTimer.stop();
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::startTransaction()
    {
      for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp++) {
        itsBeamletBuffers[rsp]->startReadTransaction(itsDelayedStamps, itsNrSamplesPerSubband + itsNrHistorySamples);

        for (unsigned beam = 0; beam < itsNrBeams; beam++)
        /*if (itsMustComputeFlags[rsp][beam])*/ { // TODO
          itsFlags[rsp][beam] = itsBeamletBuffers[rsp]->readFlags(beam);
          limitFlagsLength(itsFlags[rsp][beam]);
        }
      }
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::writeLogMessage() const
    {
      std::stringstream logStr;

      logStr << itsLogPrefix << itsCurrentTimeStamp;

      if (itsIsRealTime) {
        struct timeval tv;

        gettimeofday(&tv, 0);

        double currentTime = tv.tv_sec + tv.tv_usec / 1e6;
        double expectedTime = itsCorrelationStartTime * itsSampleDuration;
        double recordingTime = itsCurrentTimeStamp * itsSampleDuration;

        logStr << ", age: " << PrettyTime(currentTime - recordingTime) << ", late: " << PrettyTime(currentTime - expectedTime);
      }

      if (itsNeedDelays) {
        for (unsigned beam = 0; beam < itsNrBeams; beam++)
          logStr << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam][0], 7);
        //logStr << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam], 7) << " = " << itsSamplesDelay[beam] << " samples + " << PrettyTime(itsFineDelaysAtBegin[beam], 7);

        logStr << "]";
      }

      for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp++)
        logStr << ", flags " << rsp << ": " << itsFlags[rsp][0] << '(' << std::setprecision(3) << (100.0 * itsFlags[rsp][0].count() / (itsNrSamplesPerSubband + itsNrHistorySamples)) << "%)";  // not really correct; beam(0) may be shifted

      LOG_INFO(logStr.str());
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::sendSubband( Stream *stream, unsigned subband )
    {
      ASSERT( subband < itsSubbandToSAPmapping.size() );

      unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
      unsigned rspSlot = itsSubbandToRSPslotMapping[subband];
      unsigned beam = itsSubbandToSAPmapping[subband];

      struct header header;

      header.subband = subband;
      header.nrSamples = itsNrSamplesPerSubband + itsNrHistorySamples;
      header.sampleSize = NR_POLARIZATIONS * sizeof(SAMPLE_TYPE);
      header.nrTABs = itsNrTABs[beam];

      // send header
      stream->write(&header, sizeof header);

      // send subband
      itsBeamletBuffers[rspBoard]->sendUnalignedSubband(stream, rspSlot, beam);

      // send meta data
      SubbandMetaData metaData(itsNrTABs[beam]);

      if (itsNeedDelays) {
        for (unsigned p = 0; p < itsNrTABs[beam] + 1; p++) {
          struct SubbandMetaData::beamInfo &beamInfo = (p == 0 ? metaData.stationBeam : metaData.TABs[p - 1]);

          beamInfo.delayAtBegin = itsFineDelaysAtBegin[beam][p];
          beamInfo.delayAfterEnd = itsFineDelaysAfterEnd[beam][p];

          // extract the carthesian coordinates
          const casa::Vector<double> &beamDirBegin = itsBeamDirectionsAtBegin[beam][p].getValue();
          const casa::Vector<double> &beamDirEnd = itsBeamDirectionsAfterEnd[beam][p].getValue();

          for (unsigned i = 0; i < 3; i++) {
            beamInfo.beamDirectionAtBegin[i] = beamDirBegin[i];
            beamInfo.beamDirectionAfterEnd[i] = beamDirEnd[i];
          }
        }
      }

      metaData.flags = itsFlags[rspBoard][beam];

      metaData.write(stream);
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::toStream( Stream *stream )
    {
      // send all subband data
      for (unsigned subband = 0; subband < itsNrSubbands; subband++)
        sendSubband(stream, subband);
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::stopTransaction()
    {
      for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp++)
        itsBeamletBuffers[rsp]->stopReadTransaction();
    }


    template<typename SAMPLE_TYPE>
    void BeamletBufferToComputeNode<SAMPLE_TYPE>::process( Stream *stream )
    {
      // stay in sync with other psets even if there are no inputs to allow a synchronised early abort

      for (unsigned beam = 0; beam < itsNrBeams; beam++)
        itsDelayedStamps[beam] = itsCurrentTimeStamp - itsNrHistorySamples;

      if (itsNeedDelays)
        computeDelays();

      if (itsIsRealTime) {
        // wait for the deadline for these data
        itsCorrelationStartTime = itsCurrentTimeStamp + itsNrSamplesPerSubband + itsMaxNetworkDelay + itsMaximumDelay;

        itsWallClock.waitUntil(itsCorrelationStartTime);
      }

      startTransaction();
      writeLogMessage();

      NSTimer timer;
      timer.start();

      /* write data to buffer */
      toStream(stream);

      stopTransaction();

      itsCurrentTimeStamp += itsNrSamplesPerSubband;
      timer.stop();

      if (itsNrRSPboards > 0)
        LOG_DEBUG_STR(itsLogPrefix << " ION->CN: " << PrettyTime(timer.getElapsed()));
    }

    template class BeamletBufferToComputeNode<i4complex>;
    template class BeamletBufferToComputeNode<i8complex>;
    template class BeamletBufferToComputeNode<i16complex>;

  } // namespace Cobalt
} // namespace LOFAR


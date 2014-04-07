//#  BeamletBufferToComputeNode.cc: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: BeamletBufferToComputeNode.cc 18226 2011-06-09 12:56:47Z romein $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/Timer.h>
#include <Common/PrettyUnits.h>
#include <BeamletBufferToComputeNode.h>
#include <BeamletBuffer.h>
#include <ION_Allocator.h>
#include <Scheduling.h>
#include <Interface/AlignedStdAllocator.h>
#include <Interface/BFRawFormat.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Mapping.h>
#include <Interface/Stream.h>
#include <Interface/SubbandMetaData.h>
#include <Interface/Exceptions.h>

#include <sys/time.h>

#include <cstdio>
#include <stdexcept>

#include <boost/format.hpp>


namespace LOFAR {
namespace RTCP {


template<typename SAMPLE_TYPE> const unsigned BeamletBufferToComputeNode<SAMPLE_TYPE>::itsMaximumDelay;


template<typename SAMPLE_TYPE> BeamletBufferToComputeNode<SAMPLE_TYPE>::BeamletBufferToComputeNode(const Parset &ps, const std::vector<Stream *> &phaseOneTwoStreams, const std::vector<SmartPtr<BeamletBuffer<SAMPLE_TYPE> > > &beamletBuffers, unsigned psetNumber)
:
  itsFileHeaderWritten(false),
  itsPhaseOneTwoStreams(phaseOneTwoStreams),
  itsPS(ps),
  itsNrInputs(beamletBuffers.size()),
  itsPsetNumber(psetNumber),
  itsBeamletBuffers(beamletBuffers),
  itsBlockNumber(0),
  itsDelayTimer("delay consumer", true, true)
{
  bool haveStationInput = itsNrInputs > 0;
  string stationName = haveStationInput ? ps.getStationNamesAndRSPboardNumbers(psetNumber)[0].station : "none"; // TODO: support more than one station

  itsLogPrefix = str(boost::format("[obs %u station %s] ") % ps.observationID() % stationName);

  itsSubbandBandwidth	      = ps.subbandBandwidth();
  itsNrSubbands		      = ps.nrSubbands();
  itsNrSubbandsPerPset	      = ps.nrSubbandsPerPset();
  itsNrSamplesPerSubband      = ps.nrSamplesPerSubband();
  itsNrBeams		      = ps.nrBeams();
  itsNrTABs		      = ps.nrTABs();
  itsNrPhaseTwoPsets	      = ps.phaseTwoPsets().size();
  itsCurrentPhaseOneTwoComputeCore = 0;
  itsSampleDuration	      = ps.sampleDuration();
  itsDelayCompensation	      = ps.delayCompensation();
  itsCorrectClocks	      = ps.correctClocks();
  itsNeedDelays               = (itsDelayCompensation || itsNrTABs > 1 || itsCorrectClocks) && itsNrInputs > 0;
  itsSubbandToSAPmapping      = ps.subbandToSAPmapping();

  if (haveStationInput) {
    itsSubbandToRSPboardMapping = ps.subbandToRSPboardMapping(stationName);
    itsSubbandToRSPslotMapping  = ps.subbandToRSPslotMapping(stationName);
  }

  itsCurrentTimeStamp	      = TimeStamp(static_cast<int64>(ps.startTime() * itsSubbandBandwidth), ps.clockSpeed());
  itsIsRealTime		      = ps.realTime();
  itsMaxNetworkDelay	      = ps.maxNetworkDelay();
  itsDumpRawData	      = ps.dumpRawData();
  itsNrHistorySamples	      = itsDumpRawData ? 0 : ps.nrHistorySamples();
  itsObservationID	      = ps.observationID();

  LOG_DEBUG_STR(itsLogPrefix << "nrSubbands = " << itsNrSubbands);
  LOG_DEBUG_STR(itsLogPrefix << "nrChannelsPerSubband = " << ps.nrChannelsPerSubband());
  LOG_DEBUG_STR(itsLogPrefix << "nrStations = " << ps.nrStations());
  LOG_DEBUG_STR(itsLogPrefix << "nrBitsPerSample = " << ps.nrBitsPerSample());
  LOG_DEBUG_STR(itsLogPrefix << "maxNetworkDelay = " << itsMaxNetworkDelay << " samples");

  if (haveStationInput && itsNeedDelays) {
    itsDelaysAtBegin.resize(itsNrBeams, itsNrTABs + 1);
    itsDelaysAfterEnd.resize(itsNrBeams, itsNrTABs + 1);
    itsBeamDirectionsAtBegin.resize(itsNrBeams, itsNrTABs + 1);
    itsBeamDirectionsAfterEnd.resize(itsNrBeams, itsNrTABs + 1);

    if (itsDelayCompensation || itsNrTABs > 1)
      itsDelays = new Delays(ps, stationName, itsCurrentTimeStamp);

    if (itsCorrectClocks)
      itsClockCorrectionTime = ps.clockCorrectionTime(stationName);

    computeNextDelays(); // initialize itsDelaysAfterEnd before we really start
  }
   
  itsDelayedStamps.resize(itsNrBeams);
  itsSamplesDelay.resize(itsNrBeams);
  itsFineDelaysAtBegin.resize(itsNrBeams, itsNrTABs + 1);
  itsFineDelaysAfterEnd.resize(itsNrBeams, itsNrTABs + 1);
  itsFlags.resize(boost::extents[itsNrInputs][itsNrBeams]);

  if (itsDumpRawData && itsNrInputs > 0) {
    LOG_INFO_STR(itsLogPrefix << "Dumping raw beamformed data only, no further processing done");

    vector<string> rawDataOutputs = ps.getStringVector("OLAP.OLAP_Conn.rawDataOutputs",true);
    unsigned	   psetIndex	  = ps.phaseOnePsetIndex(itsPsetNumber);

    if (psetIndex >= rawDataOutputs.size())
      THROW(IONProcException, "there are more input section nodes than entries in OLAP.OLAP_Conn.rawDataOutputs");

    string rawDataOutput = rawDataOutputs[psetIndex];
    LOG_INFO_STR(itsLogPrefix << "Writing raw data to " << rawDataOutput);
    itsRawDataStream = createStream(rawDataOutput, false);
  }

#if defined HAVE_BGP_ION // FIXME: not in preprocess
  doNotRunOnCore0();
  setPriority(3);
#endif
}


template<typename SAMPLE_TYPE> BeamletBufferToComputeNode<SAMPLE_TYPE>::~BeamletBufferToComputeNode() 
{
  LOG_DEBUG_STR(itsLogPrefix << "BeamletBufferToComputeNode::~BeamletBufferToComputeNode");

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    itsBeamletBuffers[rsp]->noMoreReading();
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::computeNextDelays()
{
  // track source

  if (itsDelays != 0)
    itsDelays->getNextDelays(itsBeamDirectionsAfterEnd, itsDelaysAfterEnd);
  else
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      for (unsigned pencil = 0; pencil < itsNrTABs + 1; pencil ++)
	itsDelaysAfterEnd[beam][pencil] = 0;
   
  // apply clock correction due to cable differences

  if (itsCorrectClocks)
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      for (unsigned pencil = 0; pencil < itsNrTABs + 1; pencil ++)
	itsDelaysAfterEnd[beam][pencil] += itsClockCorrectionTime;
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::limitFlagsLength(SparseSet<unsigned> &flags)
{
  const SparseSet<unsigned>::Ranges &ranges = flags.getRanges();

  if (ranges.size() > 16)
    flags.include(ranges[15].begin, ranges.back().end);
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::computeDelays()
{
  itsDelayTimer.start();

  // begin of this integration is end of previous integration
  itsDelaysAtBegin	   = itsDelaysAfterEnd;
  itsBeamDirectionsAtBegin = itsBeamDirectionsAfterEnd;
    
  computeNextDelays();

  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    // The coarse delay is determined for the center of the current
    // time interval and is expressed in an entire amount of samples.
    //
    // We use the central pencil beam (#0) for the coarse delay compensation.
    signed int coarseDelay = static_cast<signed int>(floor(0.5 * (itsDelaysAtBegin[beam][0] + itsDelaysAfterEnd[beam][0]) * itsSubbandBandwidth + 0.5));

    // The fine delay is determined for the boundaries of the current
    // time interval and is expressed in seconds.
    double d = coarseDelay * itsSampleDuration;

    itsDelayedStamps[beam] -= coarseDelay;
    itsSamplesDelay[beam]  = -coarseDelay;

    for (unsigned pencil = 0; pencil < itsNrTABs + 1; pencil ++) {
      // we don't do coarse delay compensation for the individual pencil beams to avoid complexity and overhead
      itsFineDelaysAtBegin[beam][pencil]  = static_cast<float>(itsDelaysAtBegin[beam][pencil] - d);
      itsFineDelaysAfterEnd[beam][pencil] = static_cast<float>(itsDelaysAfterEnd[beam][pencil] - d);
    }
  }

  itsDelayTimer.stop();
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::startTransaction()
{
  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++) {
    itsBeamletBuffers[rsp]->startReadTransaction(itsDelayedStamps, itsNrSamplesPerSubband + itsNrHistorySamples);

    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      /*if (itsMustComputeFlags[rsp][beam])*/ { // TODO
	itsFlags[rsp][beam] = itsBeamletBuffers[rsp]->readFlags(beam);
	limitFlagsLength(itsFlags[rsp][beam]);
      }
  }
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::writeLogMessage() const
{
  std::stringstream logStr;

  logStr << itsLogPrefix << itsCurrentTimeStamp;

  if (itsIsRealTime) {
    struct timeval tv;

    gettimeofday(&tv, 0);

    double currentTime  = tv.tv_sec + tv.tv_usec / 1e6;
    double expectedTime = itsCorrelationStartTime * itsSampleDuration;

    logStr << ", late: " << PrettyTime(currentTime - expectedTime);
  }

  if (itsNeedDelays) {
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      logStr << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam][0], 7);
      //logStr << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam], 7) << " = " << itsSamplesDelay[beam] << " samples + " << PrettyTime(itsFineDelaysAtBegin[beam], 7);

    logStr << "]";
  }

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    logStr << ", flags " << rsp << ": " << itsFlags[rsp][0] << '(' << std::setprecision(3) << (100.0 * itsFlags[rsp][0].count() / (itsNrSamplesPerSubband + itsNrHistorySamples)) << "%)"; // not really correct; beam(0) may be shifted
  
  LOG_INFO(logStr.str());
}

template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::toComputeNodes()
{
  CN_Command command(CN_Command::PROCESS, itsBlockNumber ++);

  if (!itsPhaseOneTwoStreams.empty()) {
    // If the total number of subbands is not dividable by the nrSubbandsPerPset,
    // we may have to send dummy process commands, without sending subband data.

    for (unsigned subbandBase = 0; subbandBase < itsNrSubbandsPerPset; subbandBase ++) {
      Stream *stream = itsPhaseOneTwoStreams[itsCurrentPhaseOneTwoComputeCore];

      // tell CN to process data
#if defined CLUSTER_SCHEDULING
      if (itsPsetNumber == 0)
#endif
	command.write(stream);

      if (itsNrInputs > 0) {
        // create and send all metadata in one "large" message, since initiating a message
        // has significant overhead in FCNP.
        SubbandMetaData metaData(itsNrPhaseTwoPsets, itsNrTABs + 1);

        for (unsigned psetIndex = 0; psetIndex < itsNrPhaseTwoPsets; psetIndex ++) {
          unsigned subband = itsNrSubbandsPerPset * psetIndex + subbandBase;

          if (subband < itsNrSubbands) {
            unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
            unsigned beam     = itsSubbandToSAPmapping[subband];

            if (itsNeedDelays) {
              for (unsigned p = 0; p < itsNrTABs + 1; p ++) {
                struct SubbandMetaData::beamInfo &beamInfo = metaData.beams(psetIndex)[p];

                beamInfo.delayAtBegin   = itsFineDelaysAtBegin[beam][p];
                beamInfo.delayAfterEnd  = itsFineDelaysAfterEnd[beam][p];

                // extract the carthesian coordinates
                const casa::Vector<double> &beamDirBegin = itsBeamDirectionsAtBegin[beam][p].getValue();
                const casa::Vector<double> &beamDirEnd   = itsBeamDirectionsAfterEnd[beam][p].getValue();

                for (unsigned i = 0; i < 3; i ++) {
                  beamInfo.beamDirectionAtBegin[i]  = beamDirBegin[i];
                  beamInfo.beamDirectionAfterEnd[i] = beamDirEnd[i];
                }
              }  
            }  

            metaData.alignmentShift(psetIndex) = itsBeamletBuffers[rspBoard]->alignmentShift(beam);
            metaData.setFlags(psetIndex, itsFlags[rspBoard][beam]);
          }
        }

        metaData.write(stream);

        // now send all subband data
        for (unsigned psetIndex = 0; psetIndex < itsNrPhaseTwoPsets; psetIndex ++) {
          unsigned subband = itsNrSubbandsPerPset * psetIndex + subbandBase;

          if (subband < itsNrSubbands) {
            unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
            unsigned rspSlot  = itsSubbandToRSPslotMapping[subband];
            unsigned beam     = itsSubbandToSAPmapping[subband];

            itsBeamletBuffers[rspBoard]->sendSubband(stream, rspSlot, beam);
          }
        }
      }

      if (++ itsCurrentPhaseOneTwoComputeCore == itsPhaseOneTwoStreams.size())
        itsCurrentPhaseOneTwoComputeCore = 0;
    }
  }
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::dumpRawData()
{
  // NOTE: we always have station input at this point

  std::string stationName = itsPS.getStationNamesAndRSPboardNumbers(itsPsetNumber)[0].station; // TODO: support more than one station

  vector<unsigned> subbandToSAPmapping      = itsPS.subbandToSAPmapping();
  vector<unsigned> subbandToRSPboardMapping = itsPS.subbandToRSPboardMapping(stationName);
  vector<unsigned> subbandToRSPslotMapping  = itsPS.subbandToRSPslotMapping(stationName);
  unsigned	   nrSubbands		    = itsPS.nrSubbands();
  BFRawFormat	   bfraw_data;

  if (!itsFileHeaderWritten) {
    if (nrSubbands > 62)
      THROW(IONProcException, "too many subbands for raw data format");

    memset(&bfraw_data.header, 0, sizeof bfraw_data.header);

    bfraw_data.header.magic		  = 0x3F8304EC;
    bfraw_data.header.bitsPerSample	  = 16;
    bfraw_data.header.nrPolarizations	  = 2;
    bfraw_data.header.nrSubbands	  = nrSubbands;
    bfraw_data.header.nrSamplesPerSubband = itsNrSamplesPerSubband;
    bfraw_data.header.subbandBandwidth	  = itsSubbandBandwidth;

    strncpy(bfraw_data.header.station, itsPS.getStationNamesAndRSPboardNumbers(itsPsetNumber)[0].station.c_str(), sizeof bfraw_data.header.station);
    memcpy(bfraw_data.header.subbandFrequencies, &itsPS.subbandToFrequencyMapping()[0], nrSubbands * sizeof(double));

    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      memcpy(bfraw_data.header.beamDirections[beam], &itsPS.getBeamDirection(beam)[0], sizeof bfraw_data.header.beamDirections[beam]);

    for (unsigned subband = 0; subband < nrSubbands; subband ++)
      bfraw_data.header.subbandToSAPmapping[subband] = subbandToSAPmapping[subband];

    itsRawDataStream->write(&bfraw_data.header, sizeof bfraw_data.header);
    itsFileHeaderWritten = true;
  }

  memset(&bfraw_data.block_header, 0, sizeof bfraw_data.block_header);

  bfraw_data.block_header.magic = 0x2913D852;

  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    bfraw_data.block_header.coarseDelayApplied[beam]	 = itsSamplesDelay[beam];
    bfraw_data.block_header.fineDelayRemainingAtBegin[beam]	 = itsFineDelaysAtBegin[beam][0];
    bfraw_data.block_header.fineDelayRemainingAfterEnd[beam] = itsFineDelaysAfterEnd[beam][0];
    bfraw_data.block_header.time[beam]			 = itsDelayedStamps[beam];

    // FIXME: the current BlockHeader format does not provide space for
    // the flags from multiple RSP boards --- use the flags of RSP board 0
    itsFlags[0][beam].marshall(reinterpret_cast<char *>(&bfraw_data.block_header.flags[beam]), sizeof(BFRawFormat::BlockHeader::marshalledFlags));
  }
  
  itsRawDataStream->write(&bfraw_data.block_header, sizeof bfraw_data.block_header);

  for (unsigned subband = 0; subband < nrSubbands; subband ++)
    itsBeamletBuffers[subbandToRSPboardMapping[subband]]->sendUnalignedSubband(itsRawDataStream, subbandToRSPslotMapping[subband], subbandToSAPmapping[subband]);
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::stopTransaction()
{
  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    itsBeamletBuffers[rsp]->stopReadTransaction();
}


template<typename SAMPLE_TYPE> void BeamletBufferToComputeNode<SAMPLE_TYPE>::process()
{
  // stay in sync with other psets even if there are no inputs to allow a synchronised early abort

  if (itsNrInputs > 0)
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      itsDelayedStamps[beam] = itsCurrentTimeStamp - itsNrHistorySamples;

  if (itsNeedDelays)
    computeDelays();

  if (itsIsRealTime) {
    itsCorrelationStartTime = itsCurrentTimeStamp + itsNrSamplesPerSubband + itsMaxNetworkDelay + itsMaximumDelay;

    itsWallClock.waitUntil(itsCorrelationStartTime);
  }

  if (itsNrInputs > 0) {
    startTransaction();
    writeLogMessage();
  }

  NSTimer timer;
  timer.start();
  
  if (!itsDumpRawData)
    toComputeNodes();
  else if (itsNrInputs > 0)
    dumpRawData();

  if (itsNrInputs > 0) {
    stopTransaction();
  }

  itsCurrentTimeStamp += itsNrSamplesPerSubband;
  timer.stop();

  if (itsNrInputs > 0)
    LOG_DEBUG_STR(itsLogPrefix << " ION->CN: " << PrettyTime(timer.getElapsed()));
}

template class BeamletBufferToComputeNode<i4complex>;
template class BeamletBufferToComputeNode<i8complex>;
template class BeamletBufferToComputeNode<i16complex>;

} // namespace RTCP
} // namespace LOFAR

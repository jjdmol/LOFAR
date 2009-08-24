//#  InputSection.cc: Catch RSP ethernet frames and synchronize RSP inputs 
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/Timer.h>
#include <Common/PrettyUnits.h>
#include <InputSection.h>
#include <BeamletBuffer.h>
#include <WH_DelayCompensation.h>
#include <InputThread.h>
#include <ION_Allocator.h>
#include <Scheduling.h>
#include <Interface/AlignedStdAllocator.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Mapping.h>
#include <Interface/SubbandMetaData.h>
#include <Interface/Exceptions.h>
#include <Stream/SocketStream.h>

#include <pthread.h>
#include <sys/time.h>

#include <cstdio>
#include <stdexcept>

//#define DUMP_RAW_DATA

//#if defined DUMP_RAW_DATA
#include <Stream/SocketStream.h>
//static uint32 test;
//static Float64 testttt;
//#include <LOFAR_Datatypes/BFRawFormat.h>
#include "/cephome/jong/projects/usg/trunk/src/DAL/implement/BFRawFormat.h" // FIXME needed for dumping raw data
//#endif

namespace LOFAR {
namespace RTCP {

//#if defined DUMP_RAW_DATA
static Stream *rawDataStream;
//#endif


template<typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE>::InputSection(const std::vector<Stream *> &clientStreams, unsigned psetNumber)
:
  itsClientStreams(clientStreams),
  itsPsetNumber(psetNumber),
  itsDelayComp(0),
  itsLogThread(0),
  itsDelayTimer("delay consumer",true,true)
{
}


template<typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE>::~InputSection() 
{
  LOG_DEBUG("InputSection::~InputSection");
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::startThreads()
{
  itsLogThread = new LogThread(itsNrInputs);

  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */

  typename InputThread<SAMPLE_TYPE>::ThreadArgs args;

  args.nrTimesPerPacket    = itsPS->getInt32("OLAP.nrTimesInFrame");
  args.nrSlotsPerPacket    = itsPS->nrSlotsInFrame();
  args.isRealTime	   = itsPS->realTime();
  args.startTime	   = itsSyncedStamp;

  itsInputThreads.resize(itsNrInputs);

  for (unsigned thread = 0; thread < itsNrInputs; thread ++) {
    args.threadID	    = thread;
    args.stream		    = itsInputStreams[thread];
    args.BBuffer            = itsBBuffers[thread];
    args.packetCounters     = &itsLogThread->itsCounters[thread];

    itsInputThreads[thread] = new InputThread<SAMPLE_TYPE>(args);
  }
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::preprocess(const Parset *ps)
{
	itsPS = ps;
  itsSampleRate = ps->sampleRate();
  TimeStamp::setStationClockSpeed(static_cast<unsigned>(1024 * itsSampleRate));
  std::vector<Parset::StationRSPpair> inputs = ps->getStationNamesAndRSPboardNumbers(itsPsetNumber);
  itsNrInputs = inputs.size();

  itsInputStreams.resize(itsNrInputs);

  LOG_DEBUG("input list:");
  

  for (unsigned i = 0; i < itsNrInputs; i ++) {
    string    &station	 = inputs[i].station;
    unsigned  rsp	 = inputs[i].rsp;
    string    streamName = ps->getInputStreamName(station, rsp);

    LOG_DEBUG_STR("  " << i << ": station \"" << station << "\", RSP board " << rsp << ", reads from \"" << streamName << '"');

    if (station != inputs[0].station)
      THROW(IONProcException, "inputs from multiple stations on one I/O node not supported (yet)");

    itsInputStreams[i] = Parset::createStream(streamName, true);

    SocketStream *sstr = dynamic_cast<SocketStream *>(itsInputStreams[i]);

    if (sstr != 0)
      sstr->setReadBufferSize(24 * 1024 * 1024); // stupid kernel multiplies this by 2
  }

  itsNSubbands          = ps->nrSubbands();
  itsNSubbandsPerPset	= ps->nrSubbandsPerPset();
  itsNSamplesPerSec	= ps->nrSubbandSamples();
  itsNrBeams		= ps->nrBeams();
  itsNrPencilBeams	= ps->nrPencilBeams();
  itsNrOutputPsets	= ps->outputPsets().size();
	/*
#if defined DUMP_RAW_DATA
  itsNHistorySamples	= 0;
#else
  itsNHistorySamples	= ps->nrHistorySamples();
#endif
	*/
	if (ps->dumpRawData()) {
  	itsNHistorySamples	= 0;
	}
	else {
  	itsNHistorySamples	= ps->nrHistorySamples();
	}

  itsCurrentComputeCore = 0;
  itsNrCoresPerPset	= ps->nrCoresPerPset();
  itsSampleDuration     = ps->sampleDuration();

  LOG_DEBUG_STR("nrSubbands = " << ps->nrSubbands());
  LOG_DEBUG_STR("nrChannelsPerSubband = " << ps->nrChannelsPerSubband());
  LOG_DEBUG_STR("nrStations = " << ps->nrStations());
  LOG_DEBUG_STR("nrBitsPerSample = " << ps->nrBitsPerSample());

  itsDelayCompensation	      = ps->delayCompensation();
  itsNeedDelays               = ps->delayCompensation() || ps->nrPencilBeams() > 0;
  itsSubbandToBeamMapping     = ps->subbandToBeamMapping();
  itsSubbandToRSPboardMapping = ps->subbandToRSPboardMapping();
  itsSubbandToRSPslotMapping  = ps->subbandToRSPslotMapping();

  const double startTime = ps->startTime();

  const double sampleFreq = ps->sampleRate();
  const unsigned seconds    = static_cast<unsigned>(floor(startTime));
  const unsigned samples    = static_cast<unsigned>((startTime - floor(startTime)) * sampleFreq);

  itsSyncedStamp = TimeStamp(seconds, samples);
 
  if (itsNeedDelays) {
    itsDelaysAtBegin.resize(itsNrBeams,itsNrPencilBeams);
    itsDelaysAfterEnd.resize(itsNrBeams,itsNrPencilBeams);
    itsBeamDirectionsAtBegin.resize(itsNrBeams,itsNrPencilBeams);
    itsBeamDirectionsAfterEnd.resize(itsNrBeams,itsNrPencilBeams);
    
    itsDelayComp = new WH_DelayCompensation(ps, inputs[0].station, itsSyncedStamp); // TODO: support more than one station

    itsDelayComp->getNextDelays( itsBeamDirectionsAfterEnd, itsDelaysAfterEnd );

    itsDelaysAtBegin = itsDelaysAfterEnd;
    itsBeamDirectionsAtBegin = itsBeamDirectionsAfterEnd;
  }
   
  itsIsRealTime       = ps->realTime();
  itsMaxNetworkDelay  = ps->maxNetworkDelay();
  LOG_DEBUG_STR("maxNetworkDelay = " << itsMaxNetworkDelay << " samples");

  itsBBuffers.resize(itsNrInputs);
  itsDelayedStamps.resize(itsNrBeams);
  itsSamplesDelay.resize(itsNrBeams);
  itsFineDelaysAtBegin.resize(itsNrBeams,itsNrPencilBeams);
  itsFineDelaysAfterEnd.resize(itsNrBeams,itsNrPencilBeams);
  itsFlags.resize(boost::extents[itsNrInputs][itsNrBeams]);

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    itsBBuffers[rsp] = new BeamletBuffer<SAMPLE_TYPE>(ps->inputBufferSize(), ps->getUint32("OLAP.nrTimesInFrame"), ps->nrSlotsInFrame(), itsNrBeams, itsNHistorySamples, !itsIsRealTime, itsMaxNetworkDelay);

//#if defined DUMP_RAW_DATA
	vector<string> rawDataOutputs;
	unsigned	 psetIndex;
	string rawDataOutput;
	
	if (itsPS->dumpRawData()) {
		rawDataOutputs = ps->getStringVector("OLAP.OLAP_Conn.rawDataOutputs");
  	psetIndex	= ps->inputPsetIndex(itsPsetNumber);

  if (psetIndex >= rawDataOutputs.size())
    THROW(IONProcException, "there are more input section nodes than entries in OLAP.OLAP_Conn.rawDataOutputs");

  	rawDataOutput = rawDataOutputs[psetIndex];
  	LOG_DEBUG_STR("writing raw data to " << rawDataOutput);
		rawDataStream = Parset::createStream(rawDataOutput, false); // client
	}
//#endif

  startThreads();

#if defined HAVE_BGP_ION // FIXME: not in preprocess
  doNotRunOnCore0();
  setPriority(3);
#endif
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::limitFlagsLength(SparseSet<unsigned> &flags)
{
  const SparseSet<unsigned>::Ranges &ranges = flags.getRanges();

  if (ranges.size() > 16)
    flags.include(ranges[15].begin, ranges.back().end);
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::computeDelays()
{
  if (itsNeedDelays) {
    itsDelayTimer.start();

    itsDelaysAtBegin = itsDelaysAfterEnd; // from previous integration
    itsBeamDirectionsAtBegin = itsBeamDirectionsAfterEnd; // from previous integration
    
    itsDelayComp->getNextDelays( itsBeamDirectionsAfterEnd, itsDelaysAfterEnd );
   
    for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
      // The coarse delay is determined for the center of the current
      // time interval and is expressed in an entire amount of samples.
      //
      // We use the central pencil beam (#0) for the coarse delay compensation.
      const signed int coarseDelay = static_cast<signed int>(floor(0.5 * (itsDelaysAtBegin[beam][0] + itsDelaysAfterEnd[beam][0]) * itsSampleRate + 0.5));

      // The fine delay is determined for the boundaries of the current
      // time interval and is expressed in seconds.
      const double d = coarseDelay * itsSampleDuration;

      itsDelayedStamps[beam]      -= coarseDelay;
      itsSamplesDelay[beam]       = -coarseDelay;

      for (unsigned pencil = 0; pencil < itsNrPencilBeams; pencil++ ) {
        // we don't do coarse delay compensation for the individual pencil beams to avoid complexity and overhead
        itsFineDelaysAtBegin[beam][pencil]  = static_cast<float>(itsDelaysAtBegin[beam][pencil] - d);
        itsFineDelaysAfterEnd[beam][pencil] = static_cast<float>(itsDelaysAfterEnd[beam][pencil] - d);
      }
    }

    itsDelayTimer.stop();
  } else {
    for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
      itsSamplesDelay[beam]       = 0;

      for (unsigned pencil = 0; pencil < itsNrPencilBeams; pencil++ ) {
        itsFineDelaysAtBegin[beam][pencil]  = 0;
        itsFineDelaysAfterEnd[beam][pencil] = 0;
      }
    }  
  }
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::startTransaction()
{
  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++) {
    itsBBuffers[rsp]->startReadTransaction(itsDelayedStamps, itsNSamplesPerSec + itsNHistorySamples);

    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      /*if (itsMustComputeFlags[rsp][beam])*/ { // TODO
	itsFlags[rsp][beam] = itsBBuffers[rsp]->readFlags(beam);
	limitFlagsLength(itsFlags[rsp][beam]);
      }
  }
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::writeLogMessage() const
{
  std::stringstream logStr;
  logStr << itsSyncedStamp;

  if (itsIsRealTime) {
    struct timeval tv;

    gettimeofday(&tv, 0);

    double currentTime  = tv.tv_sec + tv.tv_usec / 1e6;
    double expectedTime = (itsSyncedStamp + itsNSamplesPerSec + itsMaxNetworkDelay) * itsSampleDuration;

    logStr << " late: " << PrettyTime(currentTime - expectedTime);
  }

  if (itsDelayCompensation) {
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      logStr << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam][0], 7);
      //logStr << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam], 7) << " = " << itsSamplesDelay[beam] << " samples + " << PrettyTime(itsFineDelaysAtBegin[beam], 7);

    logStr << "]";
  }

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    logStr << ", flags " << rsp << ": " << itsFlags[rsp][0] << '(' << std::setprecision(3) << (100.0 * itsFlags[rsp][0].count() / (itsNSamplesPerSec + itsNHistorySamples)) << "%)"; // not really correct; beam(0) may be shifted
  
  LOG_INFO(logStr.str());
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::toComputeNodes()
{
  // If the total number of subbands is not dividable by the nrSubbandsPerPset,
  // we may have to send dummy process commands, without sending subband data.

  CN_Command command(CN_Command::PROCESS);
  
  for (unsigned subbandBase = 0; subbandBase < itsNSubbandsPerPset; subbandBase ++) {
    Stream *stream = itsClientStreams[itsCurrentComputeCore];

    // tell CN to process data
    command.write(stream);

    // create and send all metadata in one "large" message, since initiating a message
    // has significant overhead in FCNP.
    SubbandMetaData metaData(itsNrOutputPsets, itsNrPencilBeams, 16);
    
    for (unsigned psetIndex = 0; psetIndex < itsNrOutputPsets; psetIndex ++) {
      unsigned subband = itsNSubbandsPerPset * psetIndex + subbandBase;

      if (subband < itsNSubbands) {
        unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
        unsigned beam     = itsSubbandToBeamMapping[subband];

        if (itsNeedDelays) {
          for (unsigned p = 0; p < itsNrPencilBeams; p ++) {
            struct SubbandMetaData::beamInfo &beamInfo = metaData.beams(psetIndex)[p];

	    beamInfo.delayAtBegin   = itsFineDelaysAtBegin[beam][p];
	    beamInfo.delayAfterEnd  = itsFineDelaysAfterEnd[beam][p];

	    const vector<double> &beamDirBegin = itsBeamDirectionsAtBegin[beam][p].coord().get();
	    const vector<double> &beamDirEnd   = itsBeamDirectionsAfterEnd[beam][p].coord().get();

	    for (unsigned i = 0; i < 3; i ++) {
	      beamInfo.beamDirectionAtBegin[i]  = beamDirBegin[i];
	      beamInfo.beamDirectionAfterEnd[i] = beamDirEnd[i];
	    }
	  }  
        }  

        metaData.alignmentShift(psetIndex) = itsBBuffers[rspBoard]->alignmentShift(beam);
        metaData.setFlags(psetIndex, itsFlags[rspBoard][beam]);
      }
    }

    metaData.write(stream);

    // now send all subband data
    for (unsigned psetIndex = 0; psetIndex < itsNrOutputPsets; psetIndex ++) {
      unsigned subband = itsNSubbandsPerPset * psetIndex + subbandBase;

      if (subband < itsNSubbands) {
	unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
	unsigned rspSlot  = itsSubbandToRSPslotMapping[subband];
	unsigned beam     = itsSubbandToBeamMapping[subband];

	itsBBuffers[rspBoard]->sendSubband(stream, rspSlot, beam);
      }
    }

    if (++ itsCurrentComputeCore == itsNrCoresPerPset)
      itsCurrentComputeCore = 0;
  }
}


//#if defined DUMP_RAW_DATA

template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::dumpRawData()
{
  static bool fileHeaderWritten = false;

  vector<unsigned> subbandToBeamMapping     = itsPS->subbandToBeamMapping();
  vector<unsigned> subbandToRSPboardMapping = itsPS->subbandToRSPboardMapping();
  vector<unsigned> subbandToRSPslotMapping  = itsPS->subbandToRSPslotMapping();
  unsigned	   nrSubbands		    = itsPS->nrSubbands();

	DAL::BFRawFormat bfraw_data;
	
  if (!fileHeaderWritten) {
    if (nrSubbands > 62)
      THROW(IONProcException, "too many subbands for raw data format");
		/*
    struct FileHeader {
      uint32	magic;		// 0x3F8304EC, also determines endianness
      uint8	bitsPerSample;
      uint8	nrPolarizations;
			uint16	nrSubbands; 
      uint32	nrSamplesPerBeamlet;
      char	station[20];
      double	sampleRate;	// typically 156250 or 195312.5
      double	subbandFrequencies[62];
      double	beamDirections[8][2];
      int16     subbandToBeamMapping[62];
			int32     pad; // wordt padding
      
    } bfraw_data.header;  
		*/
		
    memset(&bfraw_data.header, 0, sizeof bfraw_data.header);

    bfraw_data.header.magic = 0x3F8304EC;
    bfraw_data.header.bitsPerSample = 16;
    bfraw_data.header.nrPolarizations = 2;
    bfraw_data.header.nrSubbands = nrSubbands;
		bfraw_data.header.nrSamplesPerSubband = itsNSamplesPerSec;
    strncpy(bfraw_data.header.station, itsPS->getStationNamesAndRSPboardNumbers(itsPsetNumber)[0].station.c_str(), sizeof bfraw_data.header.station);
    bfraw_data.header.sampleRate = itsSampleRate;
    memcpy(bfraw_data.header.subbandFrequencies, &itsPS->subbandToFrequencyMapping()[0], nrSubbands * sizeof(double));

    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      memcpy(bfraw_data.header.beamDirections[beam], &itsPS->getBeamDirection(beam)[0], sizeof bfraw_data.header.beamDirections[beam]);
    for (unsigned subband = 0; subband < nrSubbands; subband ++)
      bfraw_data.header.subbandToBeamMapping[subband] = subbandToBeamMapping[subband];

    rawDataStream->write(&bfraw_data.header, sizeof bfraw_data.header);
    fileHeaderWritten = true;

	}
	/*
  struct BlockHeader {
    uint32	magic; // 0x2913D852
    int32	coarseDelayApplied[8];
    int32       pad;
    double	fineDelayRemainingAtBegin[8], fineDelayRemainingAfterEnd[8];
    int64	time[8]; // compatible with TimeStamp class.
    struct marshalledFlags {
      uint32	nrRanges;
      struct range {
	uint32  begin; // inclusive
	uint32  end;   // exclusive
      } flagsRanges[16]; 
    } flags[8];
  } bfraw_data.block_header;  
	*/
	
  memset(&bfraw_data.block_header, 0, sizeof bfraw_data.block_header);

  bfraw_data.block_header.magic = 0x2913D852;

  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    bfraw_data.block_header.coarseDelayApplied[beam] = itsSamplesDelay[beam];
    bfraw_data.block_header.fineDelayRemainingAtBegin[beam]	 = itsFineDelaysAtBegin[beam][0];
    bfraw_data.block_header.fineDelayRemainingAfterEnd[beam] = itsFineDelaysAfterEnd[beam][0];
    bfraw_data.block_header.time[beam] = itsDelayedStamps[beam];

    // FIXME: the current BlockHeader format does not provide space for
    // the flags from multiple RSP boards --- use the flags of RSP board 0
		itsFlags[0][beam].marshall(reinterpret_cast<char *>(&bfraw_data.block_header.flags[beam]), sizeof(struct DAL::BFRawFormat::BlockHeader::marshalledFlags));
	}
  
  rawDataStream->write(&bfraw_data.block_header, sizeof bfraw_data.block_header);

  for (unsigned subband = 0; subband < nrSubbands; subband ++)
    itsBBuffers[subbandToRSPboardMapping[subband]]->sendUnalignedSubband(rawDataStream, subbandToRSPslotMapping[subband], subbandToBeamMapping[subband]);
}
//#endif


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::stopTransaction()
{
  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    itsBBuffers[rsp]->stopReadTransaction();
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::process()
{
  for (unsigned beam = 0; beam < itsNrBeams; beam ++)
    itsDelayedStamps[beam] = itsSyncedStamp - itsNHistorySamples;

  computeDelays();
  startTransaction();
  writeLogMessage();

  NSTimer timer;
  timer.start();
	/*
#if defined DUMP_RAW_DATA
  dumpRawData();
#else
  toComputeNodes();
#endif
	*/
	
	if (itsPS->dumpRawData()) {
  	dumpRawData();
	}
	else {
  	toComputeNodes();
	}

  stopTransaction();
  itsSyncedStamp += itsNSamplesPerSec;

  timer.stop();
  LOG_INFO_STR("ION->CN: " << PrettyTime(timer.getElapsed()));
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::postprocess()
{
  LOG_DEBUG("InputSection::postprocess");

  for (unsigned i = 0; i < itsInputThreads.size(); i ++)
    delete itsInputThreads[i];

  itsInputThreads.resize(0);

  for (unsigned i = 0; i < itsInputStreams.size(); i ++)
    delete itsInputStreams[i];

  itsInputStreams.resize(0);

  for (unsigned i = 0; i < itsBBuffers.size(); i ++)
    delete itsBBuffers[i];

  itsBBuffers.resize(0);

  delete itsLogThread;	itsLogThread	= 0;
  delete itsDelayComp;	itsDelayComp	= 0;

  itsSubbandToBeamMapping.resize(0);
  itsSubbandToRSPboardMapping.resize(0);
  itsSubbandToRSPslotMapping.resize(0);
}


template class InputSection<i4complex>;
template class InputSection<i8complex>;
template class InputSection<i16complex>;

} // namespace RTCP
} // namespace LOFAR

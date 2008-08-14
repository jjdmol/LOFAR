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
//#include <TH_ZoidServer.h>
#include <CS1_Interface/AlignedStdAllocator.h>
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/SubbandMetaData.h>

#include <pthread.h>
#include <sys/time.h>

#include <cstdio>
#include <stdexcept>

#undef DUMP_RAW_DATA
//#define DUMP_RAW_DATA

#if defined DUMP_RAW_DATA
#include <Stream/SocketStream.h>
#endif

namespace LOFAR {
namespace CS1 {

#if defined DUMP_RAW_DATA
static Stream *rawDataStream;
#endif


InputSection::InputSection(const std::vector<Stream *> &clientStreams, unsigned psetNumber)
:
  itsClientStreams(clientStreams),
  itsPsetNumber(psetNumber),
  itsDelayComp(0),
  itsLogThread(0),
  itsDelayTimer("delay")
{
  raisePriority();
}


InputSection::~InputSection() 
{
  std::clog << "InputSection::~InputSection" << std::endl;
}


void InputSection::raisePriority()
{
  struct sched_param sched_param;

  sched_param.sched_priority = 99;

  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched_param) < 0)
    perror("pthread_setschedparam");
}


void InputSection::startThreads()
{
  itsLogThread = new LogThread(itsNrInputs);

  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */

  InputThread::ThreadArgs args;

  args.ipHeaderSize       = itsCS1PS->getInt32("OLAP.IPHeaderSize");
  args.frameHeaderSize    = itsCS1PS->getInt32("OLAP.EPAHeaderSize");
  args.nTimesPerFrame     = itsCS1PS->getInt32("OLAP.nrTimesInFrame");
  args.nSubbandsPerFrame  = itsCS1PS->nrSubbandsPerFrame();
  args.frameSize          = args.frameHeaderSize + args.nSubbandsPerFrame * args.nTimesPerFrame * sizeof(Beamlet);
  args.isRealTime	  = itsCS1PS->realTime();
  args.startTime	  = itsSyncedStamp;

  itsInputThreads.resize(itsNrInputs);

  for (unsigned thread = 0; thread < itsNrInputs; thread ++) {
    args.threadID	    = thread;
    args.stream		    = itsInputStreams[thread];
    args.BBuffer            = itsBBuffers[thread];
    args.packetCounters     = &itsLogThread->itsCounters[thread];

    itsInputThreads[thread] = new InputThread(args);
  }
}


void InputSection::preprocess(const CS1_Parset *ps)
{
  itsCS1PS = ps;
  itsSampleRate = ps->sampleRate();
  TimeStamp::setMaxBlockId(itsSampleRate);
  std::vector<CS1_Parset::StationRSPpair> inputs = ps->getStationNamesAndRSPboardNumbers(itsPsetNumber);
  itsNrInputs = inputs.size();

  itsInputStreams.resize(itsNrInputs);

  std::clog << "input list:" << std::endl;

  for (unsigned i = 0; i < itsNrInputs; i ++) {
    string    &station	 = inputs[i].station;
    unsigned  rsp	 = inputs[i].rsp;
    string    streamName = ps->getInputStreamName(station, rsp);

    std::cout << "  " << i << ": station \"" << station << "\", RSP board " << rsp << ", reads from \"" << streamName << '"' << std::endl;

    if (station != inputs[0].station)
      throw std::runtime_error("inputs from multiple stations on one I/O node not supported (yet)");

    itsInputStreams[i] = CS1_Parset::createStream(streamName, true);
  }

  itsNSubbandsPerPset	= ps->nrSubbandsPerPset();
  itsNSamplesPerSec	= ps->nrSubbandSamples();
  itsNrBeams		= ps->nrBeams();
  itsNrPsets		= ps->nrPsets();

#if defined DUMP_RAW_DATA
  itsNHistorySamples	= 0;
#else
  itsNHistorySamples	= ps->nrHistorySamples();
#endif

  itsCurrentComputeCore = 0;
  itsNrCoresPerPset	= ps->nrCoresPerPset();
  itsSampleDuration     = ps->sampleDuration();

  std::clog << "nrSubbands = "<< ps->nrSubbands() << std::endl;
  std::clog << "nrStations = "<< ps->nrStations() << std::endl;

  itsDelayCompensation	      = ps->getBool("OLAP.delayCompensation");
  itsSubbandToBeamMapping     = ps->subbandToBeamMapping();
  itsSubbandToRSPboardMapping = ps->subbandToRSPboardMapping();
  itsSubbandToRSPslotMapping  = ps->subbandToRSPslotMapping();
  itsNrCalcDelays	      = ps->getUint32("OLAP.DelayComp.nrCalcDelays");

  double startTime = ps->startTime();

  int sampleFreq = (int) ps->sampleRate();
  int seconds	 = (int) floor(startTime);
  int samples	 = (int) ((startTime - floor(startTime)) * sampleFreq);

  itsSyncedStamp = TimeStamp(seconds, samples);
 
  if (itsDelayCompensation) {
    itsDelaysAtBegin.resize(itsNrBeams);
    itsDelaysAfterEnd.resize(itsNrBeams);
    itsNrCalcDelaysForEachTimeNrDirections.resize(itsNrCalcDelays * itsNrBeams);
    
    itsDelayComp = new WH_DelayCompensation(ps, inputs[0].station); // TODO: support more than one station

    std::vector<double> startTimes(itsNrCalcDelays);

    for (uint i = 0; i < itsNrCalcDelays; i ++)
      startTimes[i] = static_cast<int64>(itsSyncedStamp + i * itsNSamplesPerSec) * itsSampleDuration;

    itsNrCalcDelaysForEachTimeNrDirections = itsDelayComp->calcDelaysForEachTimeNrDirections(startTimes);
    
    itsCounter = 0;
    
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[beam];
    
    itsDelaysAtBegin = itsDelaysAfterEnd;
  }
   
  itsIsRealTime       = ps->realTime();
  itsMaxNetworkDelay  = ps->maxNetworkDelay();
  std::clog << "maxNetworkDelay = " << itsMaxNetworkDelay << " samples" << std::endl;

  itsBBuffers.resize(itsNrInputs);

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    itsBBuffers[rsp] = new BeamletBuffer(ps->inputBufferSize(), ps->nrSubbandsPerFrame(), itsNHistorySamples, !itsIsRealTime, itsMaxNetworkDelay);

#if defined DUMP_RAW_DATA
  vector<string> rawDataServers = ps->getStringVector("OLAP.OLAP_Conn.rawDataServers");
  vector<string> rawDataPorts = ps->getStringVector("OLAP.OLAP_Conn.rawDataPorts");

  if (itsStationNr >= rawDataServers.size() || itsStationNr >= rawDataPorts.size()) {
    std::cerr << "too many stations for too few raw data servers/ports" << std::endl;
    exit(1);
  }

  std::clog << "trying to connect raw data stream to " << rawDataServers[itsStationNr] << ':' << rawDataPorts[itsStationNr] << std::endl;
  rawDataStream = new SocketStream(rawDataServers[itsStationNr], rawDataPorts[itsStationNr], SocketStream::TCP, SocketStream::Client);
  std::clog << "raw data stream connected" << std::endl;
#endif

  startThreads();
}


void InputSection::limitFlagsLength(SparseSet<unsigned> &flags)
{
  const SparseSet<unsigned>::Ranges &ranges = flags.getRanges();

  if (ranges.size() > 16)
    flags.include(ranges[15].begin, ranges.back().end);
}


void InputSection::process() 
{
  std::vector<TimeStamp>	delayedStamps(itsNrBeams, itsSyncedStamp - itsNHistorySamples);
  std::vector<signed int>	samplesDelay(itsNrBeams);
  std::vector<float>		fineDelaysAtBegin(itsNrBeams), fineDelaysAfterEnd(itsNrBeams);
  boost::multi_array<SparseSet<unsigned>, 2> flags(boost::extents[itsNrInputs][itsNrBeams]);
  
  // set delay
  if (itsDelayCompensation) {    
    itsCounter ++;
    itsDelaysAtBegin = itsDelaysAfterEnd; // from previous integration
    
    if (itsCounter > itsNrCalcDelays - 1) {
      std::vector<double> stopTimes(itsNrCalcDelays);

      for (uint i = 0; i < itsNrCalcDelays; i ++)
        stopTimes[i] = static_cast<int64>(itsSyncedStamp + (i + 1) * itsNSamplesPerSec) * itsSampleDuration;

      itsDelayTimer.start();
      itsNrCalcDelaysForEachTimeNrDirections = itsDelayComp->calcDelaysForEachTimeNrDirections(stopTimes);
      itsDelayTimer.stop();

      itsCounter = 0;
      
      for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
	itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[beam];
      }	
    } else {
      unsigned firstBeam = itsCounter * itsNrBeams;
      for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
	itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[firstBeam++];
      }	
    }
   
    for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
      // The coarse delay is determined for the center of the current
      // time interval and is expressed in an entire amount of samples.
      signed int coarseDelay = (signed int) floor(0.5 * (itsDelaysAtBegin[beam] + itsDelaysAfterEnd[beam]) * itsSampleRate + 0.5);
    
      // The fine delay is determined for the boundaries of the current
      // time interval and is expressed in seconds.
      double d = coarseDelay * itsSampleDuration;
    
      delayedStamps[beam]     += coarseDelay;
      samplesDelay[beam]       = +coarseDelay; // FIXME: or - ?
      fineDelaysAtBegin[beam]  = (float) (itsDelaysAtBegin[beam] - d);
      fineDelaysAfterEnd[beam] = (float) (itsDelaysAfterEnd[beam] - d);
    }
  } else {
    for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
      samplesDelay[beam]       = 0;
      fineDelaysAtBegin[beam]  = 0;
      fineDelaysAfterEnd[beam] = 0;
    }  
  }

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++) {
    itsBBuffers[rsp]->startReadTransaction(delayedStamps, itsNSamplesPerSec + itsNHistorySamples);

    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      /*if (itsMustComputeFlags[rsp][beam])*/ { // TODO
	flags[rsp][beam] = itsBBuffers[rsp]->readFlags(beam);
	limitFlagsLength(flags[rsp][beam]);
      }
  }

  std::clog << itsSyncedStamp;

  if (itsIsRealTime) {
    struct timeval tv;

    gettimeofday(&tv, 0);

    double currentTime  = tv.tv_sec + tv.tv_usec / 1e6;
    double expectedTime = (itsSyncedStamp + itsNSamplesPerSec + itsMaxNetworkDelay) * itsSampleDuration;

    std::clog << " late: " << PrettyTime(currentTime - expectedTime);
  }

  if (itsDelayCompensation) {
    for (unsigned beam = 0; beam < itsNrBeams; beam ++)
      std::clog << (beam == 0 ? ", delays: [" : ", ") << PrettyTime(itsDelaysAtBegin[beam], 7);

    std::clog << "]";
  }

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    std::clog << ", flags " << rsp << ": " << flags[rsp][0] << '(' << std::setprecision(3) << (100.0 * flags[rsp][0].count() / (itsNSamplesPerSec + itsNHistorySamples)) << "%)"; // not really correct; beam(0) may be shifted
  
  std::clog << std::endl;

  NSTimer timer;
  timer.start();

#if defined DUMP_RAW_DATA
  static bool fileHeaderWritten = false;

  if (!fileHeaderWritten) {
    struct FileHeader {
      uint32	magic;		// 0x3F8304EC, also determines endianness
      uint8	bitsPerSample;
      uint8	nrPolarizations;
      uint16	nrBeamlets;
      uint32	nrSamplesPerBeamlet;
      char	station[20];
      double	sampleRate;	// typically 156250 or 195312.5
      double	subbandFrequencies[54];
      double	beamDirections[8][2];
      int16     beamlet2beams[54];
      int32     pad;
      
    } fileHeader;  

    fileHeader.magic = 0x3F8304EC;
    fileHeader.bitsPerSample = 16;
    fileHeader.nrPolarizations = 2;
    fileHeader.nrBeamlets = itsCS1PS->nrSubbands();
    fileHeader.nrSamplesPerBeamlet = itsNSamplesPerSec;
    strncpy(fileHeader.station, itsCS1PS->stationName(itsStationNr).c_str(), 16);
    fileHeader.sampleRate = itsSampleRate;
    memcpy(fileHeader.subbandFrequencies, &itsCS1PS->subbandToFrequencyMapping()[0], 54 * sizeof(double));
 
    for (unsigned beam = 1; beam < itsNrBeams+1; beam ++){
      vector<double> beamDir = itsCS1PS->getBeamDirection(beam);
   
      fileHeader.beamDirections[beam][0] = beamDir[0];
      fileHeader.beamDirections[beam][1] = beamDir[1];
    }
     
    for (unsigned beam = 0; beam < itsBeamlet2beams.size(); beam ++)
      fileHeader.beamlet2beams[beam] = itsBeamlet2beams[beam];

    rawDataStream->write(&fileHeader, sizeof fileHeader);
    fileHeaderWritten = true;
  }

  struct BlockHeader {
    uint32	magic; // 0x2913D852
    int32	coarseDelayApplied[8];
    int32       pad;
    double	fineDelayRemainingAtBegin[8], fineDelayRemainingAfterEnd[8];
    int64	time[8]; // compatible with TimeStamp class.
    uint32      nrFlagsRanges[8];
    struct range {
      uint32    begin; // inclusive
      uint32    end;   // exclusive
    } flagsRanges[8][16]; 
  } blockHeader;  

  blockHeader.magic = 0x2913D852;
  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    blockHeader.time[beam] = delayedStamps[beam];
    blockHeader.coarseDelayApplied[beam] = samplesDelay[beam];
    blockHeader.fineDelayRemainingAtBegin[beam] = itsIONtoCNdata.delayAtBegin(beam);
    blockHeader.fineDelayRemainingAfterEnd[beam] = itsIONtoCNdata.delayAfterEnd(beam);
    itsIONtoCNdata.flags(beam).marshall(reinterpret_cast<char *>(&blockHeader.nrFlagsRanges[beam]), sizeof blockHeader.nrFlagsRanges[beam] + sizeof blockHeader.flagsRanges[beam][0]);
  }
  
  rawDataStream->write(&blockHeader, sizeof blockHeader);

  for (unsigned subband = 0; subband < itsCS1PS->nrSubbands(); subband ++)
    itsBBuffers[0]->sendUnalignedSubband(rawDataStream, subband, itsBeamlet2beams[itsSubband2Index[subband]] - 1); // FIXME!!!

#else

  BGL_Command command(BGL_Command::PROCESS);
  
  for (unsigned subbandBase = 0; subbandBase < itsNSubbandsPerPset; subbandBase ++) {
    unsigned core    = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);
    Stream   *stream = itsClientStreams[core];

    // tell CN to process data
    command.write(stream);

    // create and send all metadata in one "large" message
    std::vector<SubbandMetaData, AlignedStdAllocator<SubbandMetaData, 16> > metaData(itsNrPsets);

    for (unsigned pset = 0; pset < itsNrPsets; pset ++) {
      unsigned subband  = itsNSubbandsPerPset * pset + subbandBase;
      unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
      unsigned beam     = itsSubbandToBeamMapping[subband];

      metaData[pset].delayAtBegin   = fineDelaysAtBegin[beam];
      metaData[pset].delayAfterEnd  = fineDelaysAfterEnd[beam];
      metaData[pset].alignmentShift = itsBBuffers[rspBoard]->alignmentShift(beam);
      metaData[pset].setFlags(flags[rspBoard][beam]);
    }

    stream->write(&metaData[0], metaData.size() * sizeof(SubbandMetaData));

    // now send all subband data
    for (unsigned pset = 0; pset < itsNrPsets; pset ++) {
      unsigned subband  = itsNSubbandsPerPset * pset + subbandBase;
      unsigned rspBoard = itsSubbandToRSPboardMapping[subband];
      unsigned rspSlot	= itsSubbandToRSPslotMapping[subband];
      unsigned beam     = itsSubbandToBeamMapping[subband];

      itsBBuffers[rspBoard]->sendSubband(stream, rspSlot, beam);
    }

    if (++ itsCurrentComputeCore == itsNrCoresPerPset)
      itsCurrentComputeCore = 0;
  }
#endif

  for (unsigned rsp = 0; rsp < itsNrInputs; rsp ++)
    itsBBuffers[rsp]->stopReadTransaction();

  itsSyncedStamp += itsNSamplesPerSec;

  timer.stop();
  std::clog << "ION->CN: " << PrettyTime(timer.getElapsed()) << std::endl;
}


void InputSection::postprocess()
{
  std::clog << "InputSection::postprocess" << std::endl;

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

  itsDelayTimer.print(std::clog);
}

} // namespace CS1
} // namespace LOFAR

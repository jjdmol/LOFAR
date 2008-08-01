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
#include <Connector.h>
#include <WH_DelayCompensation.h>
#include <InputThread.h>
#include <ION_Allocator.h>
//#include <TH_ZoidServer.h>
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/SubbandMetaData.h>

#include <sys/time.h>

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
  itsBBuffer(0),
  itsDelayComp(0),
  itsDelayTimer("delay")
{
}


InputSection::~InputSection() 
{
  std::clog << "InputSection::~InputSection" << std::endl;
}


void InputSection::startThreads()
{
  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */

  ThreadArgs args;

  args.BBuffer            = itsBBuffer;
  args.ipHeaderSize       = itsCS1PS->getInt32("OLAP.IPHeaderSize");
  args.frameHeaderSize    = itsCS1PS->getInt32("OLAP.EPAHeaderSize");
  args.nTimesPerFrame     = itsCS1PS->getInt32("OLAP.nrTimesInFrame");
  args.nSubbandsPerFrame  = itsCS1PS->nrSubbandsPerFrame();
  args.frameSize          = args.frameHeaderSize + args.nSubbandsPerFrame * args.nTimesPerFrame * sizeof(Beamlet);
  args.isRealTime	  = itsCS1PS->realTime();
  args.startTime	  = itsSyncedStamp;

  itsInputThreads.resize(itsStationsAndRSPboardNumbers.size());

  for (unsigned thread = 0; thread < 1; thread ++) { // FIXME
    args.stream		  = itsInputStreams[thread];

    itsInputThreads[thread] = new InputThread(args);
  }
}


void InputSection::preprocess(const CS1_Parset *ps)
{
  itsCS1PS = ps;
  itsSampleRate = ps->sampleRate();
  TimeStamp::setMaxBlockId(itsSampleRate);
  itsStationsAndRSPboardNumbers = ps->getStationNamesAndRSPboardNumbers(itsPsetNumber);

  itsInputStreams.resize(itsStationsAndRSPboardNumbers.size());

  std::clog << "input list:" << std::endl;

  for (unsigned i = 0; i < itsStationsAndRSPboardNumbers.size(); i ++) {
    string    &station = itsStationsAndRSPboardNumbers[i].first;
    unsigned  rspBoard = itsStationsAndRSPboardNumbers[i].second;
    string    input    = ps->getInputDescription(station, rspBoard);

    std::cout << "  " << i << ": station \"" << station << "\", RSP board " << rspBoard << ", reads from \"" << input << '"' << std::endl;

    itsInputStreams[i] = CS1_Parset::createStream(input, true);
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

  // create the buffer controller.
  int subbandsToReadFromFrame = ps->subbandsToReadFromFrame();
  std::clog << "nrSubbands = "<< ps->nrSubbands() << std::endl;
  std::clog << "nrStations = "<< ps->nrStations() << std::endl;
  std::clog << "nrRSPboards = "<< ps->nrRSPboards() << std::endl;
  ASSERTSTR(subbandsToReadFromFrame <= ps->nrSubbandsPerFrame(), subbandsToReadFromFrame << " < " << ps->nrSubbandsPerFrame());

  itsDelayCompensation = ps->getBool("OLAP.delayCompensation");
  itsBeamlet2beams = ps->beamlet2beams();
  itsSubband2Index = ps->subband2Index();
  
  itsNrCalcDelays = ps->getUint32("OLAP.DelayComp.nrCalcDelays");

  double startTime = ps->startTime();

  int sampleFreq = (int) ps->sampleRate();
  int seconds	 = (int) floor(startTime);
  int samples	 = (int) ((startTime - floor(startTime)) * sampleFreq);

  itsSyncedStamp = TimeStamp(seconds, samples);
 
  if (itsDelayCompensation) {
    itsDelaysAtBegin.resize(ps->nrBeams());
    itsDelaysAfterEnd.resize(ps->nrBeams());
    itsNrCalcDelaysForEachTimeNrDirections.resize(itsNrCalcDelays * ps->nrBeams());
    
    itsDelayComp = new WH_DelayCompensation(ps, itsStationsAndRSPboardNumbers[0].first); // FIXME

    std::vector<double> startTimes(itsNrCalcDelays);

    for (uint i = 0; i < itsNrCalcDelays; i ++)
      startTimes[i] = static_cast<int64>(itsSyncedStamp + i * itsNSamplesPerSec) * itsSampleDuration;

    itsNrCalcDelaysForEachTimeNrDirections = itsDelayComp->calcDelaysForEachTimeNrDirections(startTimes);
    
    itsCounter = 0;
    
    for (unsigned beam = 0; beam < ps->nrBeams(); beam ++)
      itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[beam];
    
    itsDelaysAtBegin = itsDelaysAfterEnd;
  }
   
  unsigned cyclicBufferSize = ps->nrSamplesToBuffer();
  itsIsRealTime       = ps->realTime();
  itsMaxNetworkDelay  = ps->maxNetworkDelay();
  std::clog << "maxNetworkDelay = " << itsMaxNetworkDelay << " samples" << std::endl;
  itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, itsNHistorySamples, !itsIsRealTime, itsMaxNetworkDelay);

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
  std::vector<int>		samplesDelay(itsNrBeams);
  std::vector<SubbandMetaData>	metaDataPerBeam(itsNrBeams);
  
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
      // The coarse delay will be determined for the center of the current
      // time interval and will be expressed in \e samples.
      int coarseDelay = (int) floor(0.5 * (itsDelaysAtBegin[beam] + itsDelaysAfterEnd[beam]) * itsSampleRate + 0.5);
    
      // The fine delay will be determined for the boundaries of the current
      // time interval and will be expressed in seconds.
      double d = coarseDelay * itsSampleDuration;
    
      delayedStamps[beam] += coarseDelay;
      samplesDelay[beam]  = +coarseDelay;
      metaDataPerBeam[beam].delayAtBegin  = (float)(itsDelaysAtBegin[beam] - d);
      metaDataPerBeam[beam].delayAfterEnd = (float)(itsDelaysAfterEnd[beam] - d);
    }
  } else {
    for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
      metaDataPerBeam[beam].delayAtBegin  = 0;
      metaDataPerBeam[beam].delayAfterEnd = 0;
      samplesDelay[beam] = 0;
    }  
  }

  itsBBuffer->startReadTransaction(delayedStamps, itsNSamplesPerSec + itsNHistorySamples);

  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    metaDataPerBeam[beam].alignmentShift = itsBBuffer->alignmentShift(beam);
 
    // set flags
    SparseSet<unsigned> flags = itsBBuffer->readFlags(beam);
    limitFlagsLength(flags);
    metaDataPerBeam[beam].setFlags(flags);
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

  std::clog << ", flags: " << metaDataPerBeam[0].getFlags() << " (" << std::setprecision(3) << (100.0 * metaDataPerBeam[0].getFlags().count() / (itsNSamplesPerSec + itsNHistorySamples)) << "%)" << std::endl; // not really correct; beam(0) may be shifted

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
    memcpy(fileHeader.subbandFrequencies, &itsCS1PS->refFreqs()[0], 54 * sizeof(double));
 
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
    itsBBuffer->sendUnalignedSubband(rawDataStream, subband, itsBeamlet2beams[itsSubband2Index[subband]] - 1);

#else

  BGL_Command command(BGL_Command::PROCESS);
  
  for (unsigned subbandBase = 0; subbandBase < itsNSubbandsPerPset; subbandBase ++) {
    unsigned core = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);
    Stream   *str = itsClientStreams[core];

    command.write(str);

    std::vector<SubbandMetaData> metaDataPerComputeNode(itsNrPsets);

    for (unsigned pset = 0; pset < itsNrPsets; pset ++) {
      unsigned subband = itsNSubbandsPerPset * pset + subbandBase;
      unsigned beam    = itsBeamlet2beams[itsSubband2Index[subband]] - 1;

      metaDataPerComputeNode[pset] = metaDataPerBeam[beam];
    }

    // send all metadata in one "large" message
    str->write(&metaDataPerComputeNode[0], metaDataPerComputeNode.size() * sizeof(SubbandMetaData));

    for (unsigned pset = 0; pset < itsNrPsets; pset ++) {
      unsigned subband = itsNSubbandsPerPset * pset + subbandBase;
      unsigned beam    = itsBeamlet2beams[itsSubband2Index[subband]] - 1;

      itsBBuffer->sendSubband(str, subband, beam);
    }

    if (++ itsCurrentComputeCore == itsNrCoresPerPset)
      itsCurrentComputeCore = 0;
  }
#endif

  itsBBuffer->stopReadTransaction();
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

  delete itsBBuffer;	itsBBuffer	= 0;
  delete itsDelayComp;	itsDelayComp	= 0;

  itsDelayTimer.print(std::clog);
}

} // namespace CS1
} // namespace LOFAR

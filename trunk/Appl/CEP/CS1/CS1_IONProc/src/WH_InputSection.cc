//#  WH_InputSection.cc: Catch RSP ethernet frames and synchronize RSP inputs 
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
#include <BGL_Personality.h>
#include <WH_InputSection.h>
#include <BeamletBuffer.h>
#include <WH_DelayCompensation.h>
#include <InputThread.h>
#include <ION_Allocator.h>
#include <TH_ZoidServer.h>
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <Transport/TransportHolder.h>

#include <sys/time.h>

#undef DUMP_RAW_DATA
//#define DUMP_RAW_DATA

#if defined DUMP_RAW_DATA
#include <Transport/TH_Socket.h>
#endif

namespace LOFAR {
namespace CS1 {

#if defined DUMP_RAW_DATA
static TransportHolder *rawDataTH;
#endif


WH_InputSection::WH_InputSection(const string &name, 
				 unsigned stationNumber,
				 CS1_Parset *ps,
				 TransportHolder *inputTH)
:
  WorkHolder(0, 0, name, "WH_InputSection"),
  itsInputThread(0),
  itsInputTH(inputTH),
  itsStationNr(stationNumber),
  itsCS1PS(ps),
  itsBBuffer(0),
  itsDelayComp(0),
  itsSampleRate(itsCS1PS->sampleRate()),
  itsDelayTimer("delay")
{
  // get parameters
  itsNSubbandsPerPset = itsCS1PS->nrSubbandsPerPset();
  itsNSamplesPerSec   = itsCS1PS->nrSubbandSamples();
#if defined DUMP_RAW_DATA
  itsNHistorySamples  = 0;
#else
  itsNHistorySamples  = itsCS1PS->nrHistorySamples();
#endif
}


WH_InputSection::~WH_InputSection() 
{
  std::clog << "WH_InputSection::~WH_InputSection" << std::endl;
}


WH_InputSection *WH_InputSection::make(const string& name)
{
  return new WH_InputSection(name, itsStationNr, itsCS1PS, itsInputTH);
}


void WH_InputSection::startThread()
{
  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */

  ThreadArgs args;

  args.BBuffer            = itsBBuffer;
  args.th                 = itsInputTH;
  args.ipHeaderSize       = itsCS1PS->getInt32("OLAP.IPHeaderSize");
  args.frameHeaderSize    = itsCS1PS->getInt32("OLAP.EPAHeaderSize");
  args.nTimesPerFrame     = itsCS1PS->getInt32("OLAP.nrTimesInFrame");
  args.nSubbandsPerFrame  = itsCS1PS->nrSubbandsPerFrame();
  args.frameSize          = args.frameHeaderSize + args.nSubbandsPerFrame * args.nTimesPerFrame * sizeof(Beamlet);

#if 0
  if (itsInputTH->getType() == "TH_File" || itsInputTH->getType() == "TH_Null") {
    // if we are reading from file, overwriting the buffer should not be allowed
    // this way we can work with smaller files
    itsBBuffer->setAllowOverwrite(false);
  }
#endif

  itsInputThread = new InputThread(args);
}


void WH_InputSection::preprocess()
{
  itsCurrentComputeCore = 0;
  itsNrCoresPerPset	= itsCS1PS->nrCoresPerPset();
  itsPsetNumber		= getBGLpersonality()->getPsetNum();
  itsSampleDuration     = itsCS1PS->sampleDuration();

  // create the buffer controller.
  int subbandsToReadFromFrame = itsCS1PS->subbandsToReadFromFrame();
  ASSERTSTR(subbandsToReadFromFrame <= itsCS1PS->nrSubbandsPerFrame(), subbandsToReadFromFrame << " < " << itsCS1PS->nrSubbandsPerFrame());

  itsDelayCompensation = itsCS1PS->getBool("OLAP.delayCompensation");
  itsBeamlet2beams = itsCS1PS->beamlet2beams();
  itsSubband2Index = itsCS1PS->subband2Index();
  
  itsNrCalcDelays = itsCS1PS->getUint32("OLAP.DelayComp.nrCalcDelays");

  double startTime = itsCS1PS->startTime(); // UTC

  int sampleFreq = (int) itsCS1PS->sampleRate();
  int seconds	 = (int) floor(startTime);
  int samples	 = (int) ((startTime - floor(startTime)) * sampleFreq);

  itsSyncedStamp = TimeStamp(seconds, samples);
  itsDelaySyncedStamp = itsSyncedStamp;
 
  if (itsDelayCompensation) {
    itsDelaysAtBegin.resize(itsCS1PS->nrBeams());
    itsDelaysAfterEnd.resize(itsCS1PS->nrBeams());
    itsNrCalcDelaysForEachTimeNrDirections.resize(itsNrCalcDelays*itsCS1PS->nrBeams());
    itsNrCalcIntTimes.resize(itsNrCalcDelays);
    
    itsDelayComp = new WH_DelayCompensation(itsCS1PS, itsCS1PS->stationName(itsStationNr));
    
    double startIntegrationTime = static_cast <int64>(itsDelaySyncedStamp)*itsSampleDuration;
    
    for (uint i = 0; i < itsNrCalcDelays; i++) {
      itsNrCalcIntTimes[i] = startIntegrationTime;
      itsDelaySyncedStamp += itsNSamplesPerSec;
      startIntegrationTime = static_cast<int64>(itsDelaySyncedStamp) * itsSampleDuration;   
    }

    itsNrCalcDelaysForEachTimeNrDirections = itsDelayComp->calcDelaysForEachTimeNrDirections(itsNrCalcIntTimes);
    
    itsCounter = 0;
    
    for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
      itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[beam];
    }
    
    itsDelaysAtBegin = itsDelaysAfterEnd;
  }
   
  unsigned cyclicBufferSize = itsCS1PS->nrSamplesToBuffer();
  bool	   synchronous	    = itsCS1PS->getString("OLAP.OLAP_Conn.station_Input_Transport") != "UDP";
  itsMaxNetworkDelay  = itsCS1PS->maxNetworkDelay();
  std::clog << "maxNetworkDelay = " << itsMaxNetworkDelay << std::endl;
  itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, itsNHistorySamples, synchronous, itsMaxNetworkDelay);

#if defined DUMP_RAW_DATA
  vector<string> rawDataServers = itsCS1PS->getStringVector("OLAP.OLAP_Conn.rawDataServers");
  vector<string> rawDataPorts = itsCS1PS->getStringVector("OLAP.OLAP_Conn.rawDataPorts");

  if (itsStationNr >= rawDataServers.size() || itsStationNr >= rawDataPorts.size()) {
    std::cerr << "too many stations for too few raw data servers/ports" << std::endl;
    exit(1);
  }

  std::clog << "trying to connect raw data stream to " << rawDataServers[itsStationNr] << ':' << rawDataPorts[itsStationNr] << std::endl;
  rawDataTH = new TH_Socket(rawDataServers[itsStationNr], std::string(rawDataPorts[itsStationNr]));
  std::clog << "raw data stream connected" << std::endl;
#endif

  startThread();
}


void WH_InputSection::limitFlagsLength(SparseSet<unsigned> &flags)
{
  const SparseSet<unsigned>::Ranges &ranges = flags.getRanges();

  if (ranges.size() > 16)
    flags.include(ranges[15].begin, ranges.back().end);
}


void WH_InputSection::process() 
{
  BGL_Command command(BGL_Command::PROCESS);
  
  std::vector<TimeStamp> delayedStamp(itsCS1PS->nrBeams());
  
  for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
    delayedStamp[beam] = itsSyncedStamp - itsNHistorySamples;
  }
  
  itsSyncedStamp += itsNSamplesPerSec;
  std::vector<int> samplesDelay(itsCS1PS->nrBeams());

  // set delay
  if (itsDelayCompensation) {    
    
    itsCounter++;
    itsDelaysAtBegin = itsDelaysAfterEnd; // from previous integration
    
    if (itsCounter > itsNrCalcDelays-1) {
      itsDelaySyncedStamp = itsSyncedStamp;
      double stopIntegrationTime = static_cast<int64>(itsDelaySyncedStamp) * itsSampleDuration;

      for (uint i = 0; i < itsNrCalcDelays; i++) {
        itsNrCalcIntTimes[i] = stopIntegrationTime;
        itsDelaySyncedStamp += itsNSamplesPerSec;
        stopIntegrationTime = static_cast<int64>(itsDelaySyncedStamp) * itsSampleDuration;   
      }
      
      itsDelayTimer.start();
      itsNrCalcDelaysForEachTimeNrDirections = itsDelayComp->calcDelaysForEachTimeNrDirections(itsNrCalcIntTimes);
      itsDelayTimer.stop();
     
      itsCounter = 0;
      
      for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
	itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[beam];
      }	
    }
    else 
    {
      unsigned firstBeam = itsCounter * itsCS1PS->nrBeams();
      for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
	itsDelaysAfterEnd[beam] = itsNrCalcDelaysForEachTimeNrDirections[firstBeam++];
      }	
    }
   
    std::vector<int32> coarseDelay(itsCS1PS->nrBeams());
    std::vector<double> d(itsCS1PS->nrBeams());
    std::vector<float> fineDelayAtBegin(itsCS1PS->nrBeams()), fineDelayAfterEnd(itsCS1PS->nrBeams());
    
    for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
      // The coarse delay will be determined for the center of the current
      // time interval and will be expressed in \e samples.
      coarseDelay[beam] = (int32)floor(0.5 * (itsDelaysAtBegin[beam] + itsDelaysAfterEnd[beam]) * itsSampleRate + 0.5);
    
      // The fine delay will be determined for the boundaries of the current
      // time interval and will be expressed in seconds.
      d[beam] = coarseDelay[beam] * itsSampleDuration;
    
      fineDelayAtBegin[beam] = (float)(itsDelaysAtBegin[beam] - d[beam]);
      fineDelayAfterEnd[beam]= (float)(itsDelaysAfterEnd[beam] - d[beam]);
    
      delayedStamp[beam] -= coarseDelay[beam];
      samplesDelay[beam]  = -coarseDelay[beam];
      itsIONtoCNdata.delayAtBegin(beam)  = fineDelayAtBegin[beam];
      itsIONtoCNdata.delayAfterEnd(beam) = fineDelayAfterEnd[beam];
    }
  } else {
    for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
      itsIONtoCNdata.delayAtBegin(beam)  = 0;
      itsIONtoCNdata.delayAfterEnd(beam) = 0;
      samplesDelay[beam] = 0;
    }  
  }
  itsBBuffer->startReadTransaction(delayedStamp, itsNSamplesPerSec + itsNHistorySamples);
  for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
    itsIONtoCNdata.alignmentShift(beam) = itsBBuffer->alignmentShift(beam);
 
    // set flags
    itsBBuffer->readFlags(itsIONtoCNdata.flags(beam), beam);
    limitFlagsLength(itsIONtoCNdata.flags(beam));
  }  
  struct timeval tv;
  gettimeofday(&tv, 0);
  double currentTime  = tv.tv_sec + tv.tv_usec / 1e6;
  double expectedTime;
  
  for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
    expectedTime = (delayedStamp[beam] + (itsNSamplesPerSec + itsNHistorySamples + itsMaxNetworkDelay)) * itsSampleDuration;
  
    std::clog << delayedStamp[beam]
	      << " late: " << PrettyTime(currentTime - expectedTime)
	      << ", delay: " << PrettyTime(samplesDelay[beam] * itsSampleDuration)
	      << ", flags: " << itsIONtoCNdata.flags(beam) << " (" << std::setprecision(3) << (100.0 * itsIONtoCNdata.flags(beam).count() / (itsNSamplesPerSec + itsNHistorySamples)) << "%)" << std::endl;
  }

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
      double	sampleRate;	// 155648 or 196608
      double	subbandFrequencies[54];
      double	beamDirections[8][2];
      int16     beamlet2beams[54];
      
    } fileHeader;  

    fileHeader.magic = 0x3F8304EC;
    fileHeader.bitsPerSample = 16;
    fileHeader.nrPolarizations = 2;
    fileHeader.nrBeamlets = itsCS1PS->nrSubbands();
    fileHeader.nrSamplesPerBeamlet = itsNSamplesPerSec;
    strncpy(fileHeader.station, itsCS1PS->stationName(itsStationNr).c_str(), 16);
    fileHeader.sampleRate = itsSampleRate;
    memcpy(fileHeader.subbandFrequencies, &itsCS1PS->refFreqs()[0], 54 * sizeof(double));
 
    for (unsigned beam = 1; beam < itsCS1PS->nrBeams()+1; beam ++){
      vector<double> beamDir = itsCS1PS->getBeamDirection(beam);
   
      fileHeader.beamDirections[beam][0] = beamDir[0];
      fileHeader.beamDirections[beam][1] = beamDir[1];
    }
     
    for (unsigned beam = 0; beam < itsBeamlet2beams.size(); beam ++)
      fileHeader.beamlet2beams[beam] = itsBeamlet2beams[beam];

    rawDataTH->sendBlocking(&fileHeader, sizeof fileHeader, 0, 0);
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
  for (unsigned beam = 0; beam < itsCS1PS->nrBeams(); beam ++) {
    blockHeader.time[beam] = delayedStamp[beam];
    blockHeader.coarseDelayApplied[beam] = samplesDelay[beam];
    blockHeader.fineDelayRemainingAtBegin[beam] = itsIONtoCNdata.delayAtBegin(beam);
    blockHeader.fineDelayRemainingAfterEnd[beam] = itsIONtoCNdata.delayAfterEnd(beam);
    itsIONtoCNdata.flags(beam).marshall(reinterpret_cast<char *>(&blockHeader.nrFlagsRanges[beam]), sizeof blockHeader.nrFlagsRanges[beam] + sizeof blockHeader.flagsRanges[beam][0]);
  }
  
  rawDataTH->sendBlocking(&blockHeader, sizeof blockHeader, 0, 0);

  for (unsigned subband = 0; subband < itsCS1PS->nrSubbands(); subband ++)
    itsBBuffer->sendUnalignedSubband(rawDataTH, subband, itsBeamlet2beams[itsSubband2Index[subband]] - 1);

#else

  for (unsigned subbandBase = 0; subbandBase < itsNSubbandsPerPset; subbandBase ++) {
    unsigned	    core = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);
    TransportHolder *th  = TH_ZoidServer::theirTHs[core];

    command.write(th);
    itsIONtoCNdata.write(th, itsCS1PS->nrBeams());

    for (unsigned pset = 0; pset < itsCS1PS->nrPsets(); pset ++) {
      unsigned subband = itsNSubbandsPerPset * pset + subbandBase;

      itsBBuffer->sendSubband(th, subband, itsBeamlet2beams[itsSubband2Index[subband]] - 1);
    }

    if (++ itsCurrentComputeCore == itsNrCoresPerPset)
      itsCurrentComputeCore = 0;
  }
#endif

  itsBBuffer->stopReadTransaction();

  timer.stop();
  std::clog << "ION->CN: " << PrettyTime(timer.getElapsed()) << std::endl;
}


void WH_InputSection::postprocess()
{
  std::clog << "WH_InputSection::postprocess" << std::endl;

  delete itsInputThread;	itsInputThread	= 0;
  delete itsBBuffer;		itsBBuffer	= 0;
  delete itsDelayComp;		itsDelayComp	= 0;

  itsDelayTimer.print(clog);
}

} // namespace CS1
} // namespace LOFAR

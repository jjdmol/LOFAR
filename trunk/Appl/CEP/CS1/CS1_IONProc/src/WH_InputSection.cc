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
#include <InputThread.h>
#include <ION_Allocator.h>
#include <TH_ZoidServer.h>
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/DH_Delay.h>
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <Transport/TransportHolder.h>

#include <sys/time.h>

#undef DUMP_RAW_DATA

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
  WorkHolder(1, 0, name, "WH_InputSection"),
  itsInputThread(0),
  itsInputTH(inputTH),
  itsStationNr(stationNumber),
  itsCS1PS(ps),
  itsBBuffer(0)
{
  // get parameters
  itsNSubbandsPerPset = itsCS1PS->nrSubbandsPerPset();
  itsNSamplesPerSec   = itsCS1PS->nrSubbandSamples();
#if defined DUMP_RAW_DATA
  itsNHistorySamples  = 0;
#else
  itsNHistorySamples  = itsCS1PS->nrHistorySamples();
#endif

  // create incoming dataholder holding the delay information 
  getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay", itsCS1PS->nrStations()));
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
  args.nSubbandsPerFrame  = itsCS1PS->getInt32("OLAP.nrSubbandsPerFrame");
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

  // create the buffer controller.
  int subbandsToReadFromFrame = itsCS1PS->subbandsToReadFromFrame();
  ASSERTSTR(subbandsToReadFromFrame <= itsCS1PS->getInt32("OLAP.nrSubbandsPerFrame"), subbandsToReadFromFrame << " < " << itsCS1PS->getInt32("OLAP.nrSubbandsPerFrame"));

  itsDelayCompensation = itsCS1PS->getBool("OLAP.delayCompensation");

  double startTime = itsCS1PS->startTime(); // UTC

  int sampleFreq = (int) itsCS1PS->sampleRate();
  int seconds	 = (int) floor(startTime);
  int samples	 = (int) ((startTime - floor(startTime)) * sampleFreq);

  itsSyncedStamp = TimeStamp(seconds, samples);

  unsigned cyclicBufferSize = itsCS1PS->nrSamplesToBuffer();
  bool	   synchronous	    = itsCS1PS->getString("OLAP.OLAP_Conn.station_Input_Transport") != "UDP";
  itsMaxNetworkDelay  = itsCS1PS->maxNetworkDelay();
  std::clog << "maxNetworkDelay = " << itsMaxNetworkDelay << std::endl;
  itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, itsNHistorySamples, synchronous, itsMaxNetworkDelay);

  itsSampleDuration = itsCS1PS->sampleDuration();

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

  TimeStamp delayedStamp = itsSyncedStamp - itsNHistorySamples;
  itsSyncedStamp += itsNSamplesPerSec;
  int samplesDelay;

  // set delay
  if (itsDelayCompensation) {
    DH_Delay *dh = static_cast<DH_Delay *>(getDataManager().getInHolder(0));
    struct DH_Delay::DelayInfo &delay = (*dh)[itsStationNr];
    delayedStamp -= delay.coarseDelay;
    samplesDelay		   = -delay.coarseDelay;
    itsIONtoCNdata.delayAtBegin()  = delay.fineDelayAtBegin;
    itsIONtoCNdata.delayAfterEnd() = delay.fineDelayAfterEnd;
  } else {
    samplesDelay		   = 0;
    itsIONtoCNdata.delayAtBegin()  = 0;
    itsIONtoCNdata.delayAfterEnd() = 0;
  }

  itsBBuffer->startReadTransaction(delayedStamp, itsNSamplesPerSec + itsNHistorySamples);

  itsIONtoCNdata.alignmentShift() = itsBBuffer->alignmentShift();

  // set flags
  itsBBuffer->readFlags(itsIONtoCNdata.flags());
  limitFlagsLength(itsIONtoCNdata.flags());

  struct timeval tv;
  gettimeofday(&tv, 0);
  double currentTime  = tv.tv_sec + tv.tv_usec / 1e6;
  double expectedTime = (delayedStamp + (itsNSamplesPerSec + itsNHistorySamples + itsMaxNetworkDelay)) * itsSampleDuration;
  
  std::clog << delayedStamp
	    << " late: " << PrettyTime(currentTime - expectedTime)
	    << ", delay: " << samplesDelay
	    << ", flags: " << itsIONtoCNdata.flags() << " (" << std::setprecision(3) << (100.0 * itsIONtoCNdata.flags().count() / (itsNSamplesPerSec + itsNHistorySamples)) << "%)" << std::endl;

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
      char	station[16];
      double	sampleRate;	// 156250.0 or 195312.5
      double	subbandFrequencies[54];
      double	beamDirections[54][2];
    } fileHeader;  

    fileHeader.magic = 0x3F8304EC;
    fileHeader.bitsPerSample = 16;
    fileHeader.nrPolarizations = 2;
    fileHeader.nrBeamlets = itsCS1PS->nrSubbands();
    fileHeader.nrSamplesPerBeamlet = itsNSamplesPerSec;
    strncpy(fileHeader.station, itsCS1PS->stationName(itsStationNr).c_str(), 16);
    fileHeader.sampleRate = itsCS1PS->sampleRate();
    memcpy(fileHeader.subbandFrequencies, &itsCS1PS->refFreqs()[0], itsCS1PS->nrSubbands() * sizeof(double));
    
    /* TODO: fill in beams/
    for (unsigned subband = 0; subband < itsCS1PS->nrSubbands(); subband ++)
      fileHeader.beamDirections[subband][0] = 
      */

    rawDataTH->sendBlocking(&fileHeader, sizeof fileHeader, 0, 0);
    fileHeaderWritten = true;
  }

  struct BlockHeader {
    uint32	magic; // 0x2913D852
    int32	coarseDelayApplied;
    double	fineDelayRemainingAtBegin, fineDelayRemainingAfterEnd;
    int64	time; // compatible with TimeStamp class.
    uint32      nrFlagsRanges;
    struct range {
      uint32    begin; // inclusive
      uint32    end;   // exclusive
    } flagsRanges[16];
  } blockHeader;  

  blockHeader.magic = 0x2913D852;
  blockHeader.time = delayedStamp;
  blockHeader.coarseDelayApplied = samplesDelay;
  blockHeader.fineDelayRemainingAtBegin = itsIONtoCNdata.delayAtBegin();
  blockHeader.fineDelayRemainingAfterEnd = itsIONtoCNdata.delayAfterEnd();
  itsIONtoCNdata.flags().marshall(reinterpret_cast<char *>(&blockHeader.nrFlagsRanges), sizeof blockHeader.nrFlagsRanges + sizeof blockHeader.flagsRanges);

  rawDataTH->sendBlocking(&blockHeader, sizeof blockHeader, 0, 0);

  for (unsigned subband = 0; subband < itsCS1PS->nrSubbands(); subband ++)
    itsBBuffer->sendUnalignedSubband(rawDataTH, subband);
#else
  for (unsigned subbandBase = 0; subbandBase < itsNSubbandsPerPset; subbandBase ++) {
    unsigned	    core = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);
    TransportHolder *th  = TH_ZoidServer::theirTHs[core];

    command.write(th);
    itsIONtoCNdata.write(th);

    for (unsigned pset = 0; pset < itsCS1PS->nrPsets(); pset ++) {
      unsigned subband = itsNSubbandsPerPset * pset + subbandBase;

      itsBBuffer->sendSubband(th, subband);
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
}

} // namespace CS1
} // namespace LOFAR

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
#include <Common/LofarLogger.h>
//#include <AMCBase/Epoch.h>
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
//#include <tinyCEP/Sel_RoundRobin.h>


namespace LOFAR {
namespace CS1 {

WH_InputSection::WH_InputSection(const string &name, 
				 unsigned stationNumber,
				 CS1_Parset *ps,
				 TransportHolder *inputTH)
:
  WorkHolder(1, 0, name, "WH_InputSection"),
  itsInputThread(0),
  itsInputTH(inputTH),
  itsStationNr(stationNumber),
  itsStationName(ps->stationName(stationNumber)),
  itsCS1PS(ps),
  itsBBuffer(0),
  itsPrePostTimer("pre/post"),
  itsProcessTimer("process"),
  itsGetElemTimer("getElem")
{
  LOG_TRACE_FLOW_STR("WH_InputSection constructor");    

  // get parameters
  itsNSubbandsPerPset = itsCS1PS->nrSubbandsPerPset();
  itsNSamplesPerSec   = itsCS1PS->nrSubbandSamples();
  itsNHistorySamples  = itsCS1PS->nrHistorySamples();

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
  LOG_TRACE_FLOW_STR("WH_InputSection starting thread");   

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
  itsPrePostTimer.start();

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
  unsigned maxNetworkDelay  = itsCS1PS->maxNetworkDelay();
  std::clog << "maxNetworkDelay = " << maxNetworkDelay << std::endl;
  itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, itsNHistorySamples, synchronous, maxNetworkDelay);

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

  itsProcessTimer.start();

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
  std::clog << itsStationName << ' ' << delayedStamp << " delay: " << samplesDelay << " flags: " << itsIONtoCNdata.flags() << " (" << (100.0 * itsIONtoCNdata.flags().count() / (itsNSamplesPerSec + itsNHistorySamples)) << "%)" << std::endl;

  for (unsigned subbandBase = 0; subbandBase < itsNSubbandsPerPset; subbandBase ++) {
    unsigned	    core = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);
    TransportHolder *th  = TH_ZoidServer::theirTHs[core];

    command.write(th);
    itsIONtoCNdata.write(th);

    itsGetElemTimer.start();

    for (unsigned pset = 0; pset < itsCS1PS->nrPsets(); pset ++) {
      unsigned subband = itsNSubbandsPerPset * pset + subbandBase;

      itsBBuffer->sendSubband(th, subband);
    }

    itsGetElemTimer.stop();

    if (++ itsCurrentComputeCore == itsNrCoresPerPset)
      itsCurrentComputeCore = 0;
  }

  itsBBuffer->stopReadTransaction();
  itsProcessTimer.stop();
}


void WH_InputSection::postprocess()
{
  std::clog << "WH_InputSection::postprocess" << std::endl;

  delete itsInputThread;	itsInputThread	= 0;
  delete itsBBuffer;		itsBBuffer	= 0;

  itsPrePostTimer.stop();

  itsPrePostTimer.print(clog);
  itsProcessTimer.print(clog);
  itsGetElemTimer.print(clog);
}

} // namespace CS1
} // namespace LOFAR

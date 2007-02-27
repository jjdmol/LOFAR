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
#include <AMCBase/Epoch.h>
#include <CS1_InputSection/WH_InputSection.h>
#include <CS1_Interface/DH_Delay.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_InputSection/BeamletBuffer.h>
#include <CS1_InputSection/InputThread.h>
#include <tinyCEP/Sel_RoundRobin.h>

#include <algorithm>

#if defined HAVE_MPI
#include <Transport/TH_MPI.h>
#include <mpi.h>
#endif

#include <signal.h>
#include <sys/time.h>


#undef USE_TIMER

namespace LOFAR {
namespace CS1 {

bool WH_InputSection::signalReceived;


WH_InputSection::WH_InputSection(const string &name, 
				 ACC::APS::ParameterSet &ps,
				 TransportHolder *inputTH,
				 unsigned stationNr,
				 unsigned nrInputChannels,
				 unsigned nrOutputChannels,
				 const std::vector<unsigned> &inputNodes,
				 const std::vector<unsigned> &outputNodes)
:
  WorkHolder(nrInputChannels, nrOutputChannels, name, "WH_InputSection"),
  itsInputNodes(inputNodes),
  itsOutputNodes(outputNodes),
  itsInputTH(inputTH),
  itsStationNr(stationNr),
  itsPS(ps),
  itsBBuffer(0),
  itsPrePostTimer("pre/post"),
  itsProcessTimer("process"),
  itsGetElemTimer("getElem")
{
  LOG_TRACE_FLOW_STR("WH_InputSection constructor");    

  // get parameters
  itsNSubbandsPerCell = ps.getUint32("General.SubbandsPerPset") * ps.getUint32("BGLProc.PsetsPerCell");
  itsNSamplesPerSec   = ps.getUint32("Observation.NSubbandSamples");
  itsNHistorySamples  = (ps.getUint32("BGLProc.NPPFTaps") - 1) * ps.getUint32("Observation.NChannels");

  // create incoming dataholder holding the delay information 
  if (nrInputChannels > 0)
    getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay", ps.getInt32("Input.NRSPBoards")));

  // create a outgoing dataholder for each subband
  if (nrOutputChannels > 0) {
    vector<int> channels;

    for (int i = 0; i < itsNoutputs; i ++) {
      getDataManager().addOutDataHolder(i, new DH_Subband("DH_Subband", ps));
      getDataManager().setAutoTriggerOut(i, false);
      channels.push_back(i);
    }

    getDataManager().setOutputSelector(new Sel_RoundRobin(channels));
  }
}


WH_InputSection::~WH_InputSection() 
{
}


WH_InputSection *WH_InputSection::make(const string& name)
{
  return new WH_InputSection(name, itsPS, itsInputTH, itsStationNr, itsNinputs, itsNoutputs, itsInputNodes, itsOutputNodes);
}


void WH_InputSection::startThread()
{
  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */
  LOG_TRACE_FLOW_STR("WH_InputSection starting thread");   

  ThreadArgs args;
  args.BBuffer            = itsBBuffer;
  args.th                 = itsInputTH;
  args.ipHeaderSize       = itsPS.getInt32("Input.IPHeaderSize");
  args.frameHeaderSize    = itsPS.getInt32("Input.SzEPAheader");
  args.nTimesPerFrame     = itsPS.getInt32("Input.NTimesInFrame");
  args.nSubbandsPerFrame  = itsPS.getInt32("Input.NSubbandsPerFrame");

  args.frameSize          = args.frameHeaderSize + args.nSubbandsPerFrame * args.nTimesPerFrame * sizeof(Beamlet);
  args.ID                 = itsStationNr;


  if (itsInputTH->getType() == "TH_File" || itsInputTH->getType() == "TH_Null") {
    // if we are reading from file, overwriting the buffer should not be allowed
    // this way we can work with smaller files
    itsBBuffer->setAllowOverwrite(false);
  }

  itsInputThreadObject = new InputThread(args);
  itsInputThread       = new boost::thread(*itsInputThreadObject);
}

void WH_InputSection::preprocess()
{
  itsPrePostTimer.start();

  itsIsInput  = std::find(itsInputNodes.begin(),  itsInputNodes.end(),  (unsigned) getNode()) != itsInputNodes.end();
  itsIsOutput = std::find(itsOutputNodes.begin(), itsOutputNodes.end(), (unsigned) getNode()) != itsOutputNodes.end();

  if (itsIsInput) {
    // create the buffer controller.
    int cyclicBufferSize = itsPS.getInt32("Input.NSamplesToBuffer");
    int subbandsToReadFromFrame = itsPS.getInt32("Observation.NSubbands") * itsPS.getInt32("Observation.NStations") / itsPS.getInt32("Input.NRSPBoards");
    ASSERTSTR(subbandsToReadFromFrame <= itsPS.getInt32("Input.NSubbandsPerFrame"), subbandsToReadFromFrame << " < " << itsPS.getInt32("Input.NSubbandsPerFrame"));

    itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, cyclicBufferSize/6, cyclicBufferSize/6);
    startThread();

    itsDelayCompensation = itsPS.getBool("Observation.DelayCompensation");

    // determine starttime
    double startTime = itsPS.getDouble("Observation.StartTime");

#if 1
    // interpret the time as utc
    double utc = startTime;
#else
    double utc = AMC::Epoch(startTime).utc();
#endif
    int sampleFreq = itsPS.getInt32("Observation.SampleRate");
    int seconds = (int)floor(utc);
    int samples = (int)((utc - floor(utc)) * sampleFreq);

    itsSyncedStamp = TimeStamp(seconds, samples);

    cout<<"Starting buffer at "<<itsSyncedStamp<<endl;cout.flush();
    itsBBuffer->startBufferRead(itsSyncedStamp);

    unsigned nrCells = itsPS.getUint32("Observation.NSubbands") / itsNSubbandsPerCell;

    itsInputData     = new boost::multi_array<SampleType, 4>(boost::extents[nrCells][itsNSubbandsPerCell][itsNSamplesPerSec + itsNHistorySamples][NR_POLARIZATIONS]);
    itsInputMetaData = new struct metaData[nrCells];
  }

  if (itsIsOutput) {
    unsigned nrStations = itsInputNodes.size();

    itsOutputData     = new boost::multi_array<SampleType, 4>(boost::extents[nrStations][itsNSubbandsPerCell][itsNSamplesPerSec + itsNHistorySamples][NR_POLARIZATIONS]);
    itsOutputMetaData = new struct metaData[nrStations];

#if defined USE_TIMER
    sighandler_t ret = signal(SIGALRM, *WH_InputSection::timerSignal);
    ASSERTSTR(ret != SIG_ERR, "WH_InputSection couldn't set signal handler for timer");    
    struct itimerval value;

    double interval = itsNSamplesPerSec / itsPS.getDouble("Observation.SampleRate") / itsNSubbandsPerCell;
    __time_t secs  = static_cast<__time_t>(floor(interval));
    __time_t usecs = static_cast<__time_t>(1e6 * (interval - secs));

    value.it_interval.tv_sec  = value.it_value.tv_sec  = secs;
    value.it_interval.tv_usec = value.it_value.tv_usec = usecs;
    cout << "Setting timer interval to " << secs << "secs and " << usecs << "ms" << endl;

    setitimer(ITIMER_REAL, &value, 0);
#endif
  }
}

void WH_InputSection::doInput(SparseSet &flags)
{
  TimeStamp delayedStamp = itsSyncedStamp - itsNHistorySamples;
  itsSyncedStamp += itsNSamplesPerSec;

  if (itsDelayCompensation) {
    DH_Delay *dh = static_cast<DH_Delay *>(getDataManager().getInHolder(0));
    delayedStamp += (*dh)[itsStationNr].coarseDelay;
  }

  // get the data from the cyclic buffer
  itsGetElemTimer.start();
  boost::multi_array_ref<SampleType, 3> inputData(itsInputData->origin(), boost::extents[itsOutputNodes.size() * itsNSubbandsPerCell][itsNSamplesPerSec + itsNHistorySamples][NR_POLARIZATIONS]);
  itsBBuffer->getElements(inputData, flags, delayedStamp, itsNSamplesPerSec + itsNHistorySamples);
  itsGetElemTimer.stop();

  std::clog << "WH_InputSection out " << itsStationNr << " " << delayedStamp << " flags: " << flags << std::endl;
}


void WH_InputSection::limitFlagsLength(SparseSet &flags)
{
  const std::vector<struct SparseSet::range> &ranges = flags.getRanges();

  if (ranges.size() > 16)
    flags.include(ranges[15].begin, ranges[ranges.size() - 1].end);
}


void WH_InputSection::transposeData() 
{
#if defined HAVE_MPI
  int nrNodes = TH_MPI::getNumberOfNodes();
#else
  int nrNodes = 1;
#endif

  int sendCounts[nrNodes], sendDisplacements[nrNodes];
  int receiveCounts[nrNodes], receiveDisplacements[nrNodes];

  memset(sendCounts, 0, sizeof sendCounts);
  memset(receiveCounts, 0, sizeof receiveCounts);

  if (itsIsInput)
    for (unsigned output = 0; output < itsOutputNodes.size(); output ++) {
      sendCounts[itsOutputNodes[output]] = (*itsInputData)[output].num_elements() * sizeof(SampleType);
      sendDisplacements[itsOutputNodes[output]] = reinterpret_cast<char *>((*itsInputData)[output].origin()) - reinterpret_cast<char *>(itsInputData->origin());
    }

  if (itsIsOutput)
    for (unsigned input = 0; input < itsInputNodes.size(); input ++) {
      receiveCounts[itsInputNodes[input]] = (*itsOutputData)[input].num_elements() * sizeof(SampleType);
      receiveDisplacements[itsInputNodes[input]] = reinterpret_cast<char *>((*itsOutputData)[input].origin()) - reinterpret_cast<char *>(itsOutputData->origin());
    }

#if defined HAVE_MPI
  if (MPI_Alltoallv(itsIsInput ? itsInputData->origin() : 0,
		    sendCounts, sendDisplacements, MPI_BYTE,
		    itsIsOutput ? itsOutputData->origin() : 0,
		    receiveCounts, receiveDisplacements, MPI_BYTE,
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }
#endif
}


void WH_InputSection::transposeMetaData(const SparseSet &flags)
{
#if defined HAVE_MPI
  int nrNodes = TH_MPI::getNumberOfNodes();
#else
  int nrNodes = 1;
#endif

  int sendCounts[nrNodes], sendDisplacements[nrNodes];
  int receiveCounts[nrNodes], receiveDisplacements[nrNodes];

  memset(sendCounts, 0, sizeof sendCounts);
  memset(receiveCounts, 0, sizeof receiveCounts);

  if (itsIsInput) {
    DH_Delay *delayDHp = static_cast<DH_Delay *>(getDataManager().getInHolder(0));

    for (unsigned output = 0; output < itsOutputNodes.size(); output++) { 
      itsInputMetaData[output].fineDelayAtBegin  = (*delayDHp)[itsStationNr].fineDelayAtBegin;
      itsInputMetaData[output].fineDelayAfterEnd = (*delayDHp)[itsStationNr].fineDelayAfterEnd;

      if (flags.marshall(itsInputMetaData[output].flagsBuffer, sizeof itsInputMetaData[output].flagsBuffer) < 0) {
	std::cerr << "Too many flags!" << std::endl;
	std::exit(1);
      }

      sendCounts[itsOutputNodes[output]] = sizeof(struct metaData);
      sendDisplacements[itsOutputNodes[output]] = reinterpret_cast<char *>(&itsInputMetaData[output]) - reinterpret_cast<char *>(itsInputMetaData);
    }
  }

  if (itsIsOutput)
    for (unsigned input = 0; input < itsInputNodes.size(); input ++) {
      receiveCounts[itsInputNodes[input]] = sizeof(struct metaData);
      receiveDisplacements[itsInputNodes[input]] = reinterpret_cast<char *>(&itsOutputMetaData[input]) - reinterpret_cast<char *>(itsOutputMetaData);
    }

#if defined HAVE_MPI
  if (MPI_Alltoallv(itsIsInput ? itsInputMetaData : 0,
		    sendCounts, sendDisplacements, MPI_BYTE,
		    itsIsOutput ? itsOutputMetaData : 0,
		    receiveCounts, receiveDisplacements, MPI_BYTE,
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }
#endif
}


void WH_InputSection::doOutput() 
{
  // Copy every subband to one BG/L core
  Selector *selector = getDataManager().getOutputSelector();

  for (unsigned subband = 0; subband < itsNSubbandsPerCell; subband ++) {
    // ask the round robin selector for the next output
    DH_Subband *outHolder = static_cast<DH_Subband *>(getDataManager().getOutHolder(selector->getCurrentSelection()));

    // Copy one subband from every input
    for (unsigned station = 0; station < itsInputNodes.size(); station ++) {
      ASSERT(outHolder->getSamples3D()[station].num_elements() == (*itsOutputData)[station][subband].num_elements());

      memcpy(outHolder->getSamples3D()[station].origin(),
	     (*itsOutputData)[station][subband].origin(),
	     outHolder->getSamples3D()[station].num_elements() * sizeof(DH_Subband::SampleType));

      // copy other information (delayInfo, flags etc)
      outHolder->getDelays()[station].delayAtBegin  = itsOutputMetaData[station].fineDelayAtBegin;
      outHolder->getDelays()[station].delayAfterEnd = itsOutputMetaData[station].fineDelayAfterEnd;
      outHolder->getFlags()[station].unmarshall(itsOutputMetaData[station].flagsBuffer);
    }

    outHolder->fillExtraData();

    getDataManager().readyWithOutHolder(selector->getCurrentSelection());
    selector->selectNext();
  }
}


void WH_InputSection::process() 
{ 
  itsProcessTimer.start();
  SparseSet flags;

  if (itsIsInput) {
    doInput(flags);
    limitFlagsLength(flags);
  }

  transposeData();
  transposeMetaData(flags);

  if (itsIsOutput)
    doOutput();

  itsProcessTimer.stop();

#if defined USE_TIMER
  while (!signalReceived)
    pause();

  signalReceived = false;
#endif
}

void WH_InputSection::postprocess()
{
  if (itsIsInput) {
    InputThread::stopThreads();
    itsBBuffer->clear();
    itsInputThread->join();
    delete itsInputThread;
    delete itsInputThreadObject;
    delete itsBBuffer;
    delete itsInputData;
    delete [] itsInputMetaData;
  }

  if (itsIsOutput) {
    delete itsOutputData;
    delete [] itsOutputMetaData;
  }

#if defined USE_TIMER
  // unset timer
  struct itimerval value;
  memset(&value, 0, sizeof value);
  setitimer(ITIMER_REAL, &value, 0);
  // remove sig handler
  sighandler_t ret = signal(SIGALRM, SIG_DFL);
  ASSERTSTR(ret != SIG_ERR, "WH_InputSection couldn't unset signal handler for timer");    
#endif

  itsPrePostTimer.stop();

  itsPrePostTimer.print(clog);
  itsProcessTimer.print(clog);
  itsGetElemTimer.print(clog);
}

void WH_InputSection::timerSignal(int)
{
  signalReceived = true;
}

} // namespace CS1
} // namespace LOFAR

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
#include <CS1_Interface/CS1_Parset.h>
#include <Transport/TransportHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_InputSection/BeamletBuffer.h>
#include <CS1_InputSection/InputThread.h>
#include <tinyCEP/Sel_RoundRobin.h>

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
				 bool doInput,
				 bool doTranspose,
				 bool doOutput,
				 CS1_Parset *ps,
				 TransportHolder *inputTH,
				 unsigned stationNr,
				 unsigned nrInputChannels,
				 unsigned nrOutputChannels,
				 const std::vector<unsigned> &inputNodes,
				 const std::vector<unsigned> &outputNodes)
:
  WorkHolder(nrInputChannels, nrOutputChannels, name, "WH_InputSection"),
  itsDoInput(doInput),
  itsDoTranspose(doTranspose),
  itsDoOutput(doOutput),
  itsInputNodes(inputNodes),
  itsOutputNodes(outputNodes),
  itsInputTH(inputTH),
  itsStationNr(stationNr),
  itsCS1PS(ps),
  itsBBuffer(0),
  itsPrePostTimer("pre/post"),
  itsProcessTimer("process"),
  itsGetElemTimer("getElem")
{
  LOG_TRACE_FLOW_STR("WH_InputSection constructor");    

  // get parameters
  itsNSubbandsPerCell = itsCS1PS->nrSubbandsPerCell();
  itsNSamplesPerSec   = itsCS1PS->nrSubbandSamples();
  itsNHistorySamples  = itsCS1PS->nrHistorySamples();

  // create incoming dataholder holding the delay information 
  if (doInput)
    getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay", itsCS1PS->getInt32("OLAP.nrRSPboards")));

  // create a outgoing dataholder for each subband
  if (doOutput) {
    vector<int> channels;

    for (int i = 0; i < itsNoutputs; i ++) {
      getDataManager().addOutDataHolder(i, new DH_Subband("DH_Subband", itsCS1PS));
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
  return new WH_InputSection(name, itsDoInput, itsDoTranspose, itsDoOutput, itsCS1PS, itsInputTH, itsStationNr, itsNinputs, itsNoutputs, itsInputNodes, itsOutputNodes);
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

  if (itsDoInput) {
    // create the buffer controller.
    int cyclicBufferSize = itsCS1PS->nrSamplesToBuffer();
    int subbandsToReadFromFrame = itsCS1PS->subbandsToReadFromFrame();
    ASSERTSTR(subbandsToReadFromFrame <= itsCS1PS->getInt32("OLAP.nrSubbandsPerFrame"), subbandsToReadFromFrame << " < " << itsCS1PS->getInt32("OLAP.nrSubbandsPerFrame"));

    itsBBuffer = new BeamletBuffer(cyclicBufferSize, subbandsToReadFromFrame, cyclicBufferSize/6, cyclicBufferSize/6);
    startThread();

    itsDelayCompensation = itsCS1PS->getBool("OLAP.delayCompensation");

    // determine starttime
    double startTime = itsCS1PS->startTime();

#if 1
    // interpret the time as utc
    double utc = startTime;
#else
    double utc = AMC::Epoch(startTime).utc();
#endif
    int sampleFreq = (int)itsCS1PS->sampleRate();
    int seconds = (int)floor(utc);
    int samples = (int)((utc - floor(utc)) * sampleFreq);

    itsSyncedStamp = TimeStamp(seconds, samples);

    std::clog << "Starting buffer at " << itsSyncedStamp << std::endl;
    itsBBuffer->startBufferRead(itsSyncedStamp);
    unsigned nrCells = itsCS1PS->nrCells();

    itsInputData     = new boost::multi_array<SampleType, 4>(boost::extents[nrCells][itsNSubbandsPerCell][itsNSamplesPerSec + itsNHistorySamples][NR_POLARIZATIONS]);
    itsInputMetaData = new struct metaData[nrCells];
  }

  if (itsDoOutput) {
    unsigned nrStations = itsCS1PS->nrStations();

    itsOutputData     = new boost::multi_array<SampleType, 4>(boost::extents[nrStations][itsNSubbandsPerCell][itsNSamplesPerSec + itsNHistorySamples][NR_POLARIZATIONS]);
    itsOutputMetaData = new struct metaData[nrStations];

#if defined USE_TIMER
    sighandler_t ret = signal(SIGALRM, *WH_InputSection::timerSignal);
    ASSERTSTR(ret != SIG_ERR, "WH_InputSection couldn't set signal handler for timer");    
    struct itimerval value;

    double interval = itsCS1PS->timeInterval();
    __time_t secs  = static_cast<__time_t>(floor(interval));
    __time_t usecs = static_cast<__time_t>(1e6 * (interval - secs));

    value.it_interval.tv_sec  = value.it_value.tv_sec  = secs;
    value.it_interval.tv_usec = value.it_value.tv_usec = usecs;
    cout << "Setting timer interval to " << secs << "secs and " << usecs << "ms" << endl;

    setitimer(ITIMER_REAL, &value, 0);
#endif
  }
}

void WH_InputSection::doInput(SparseSet<unsigned> &flags)
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


void WH_InputSection::limitFlagsLength(SparseSet<unsigned> &flags)
{
  const std::vector<struct SparseSet<unsigned>::range> &ranges = flags.getRanges();

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

  if (itsDoInput)
    for (unsigned output = 0; output < itsOutputNodes.size(); output ++) {
      sendCounts[itsOutputNodes[output]] = (*itsInputData)[output].num_elements() * sizeof(SampleType);
      sendDisplacements[itsOutputNodes[output]] = reinterpret_cast<char *>((*itsInputData)[output].origin()) - reinterpret_cast<char *>(itsInputData->origin());
    }

  if (itsDoOutput)
    for (unsigned input = 0; input < itsInputNodes.size(); input ++) {
      receiveCounts[itsInputNodes[input]] = (*itsOutputData)[input].num_elements() * sizeof(SampleType);
      receiveDisplacements[itsInputNodes[input]] = reinterpret_cast<char *>((*itsOutputData)[input].origin()) - reinterpret_cast<char *>(itsOutputData->origin());
    }

#if 1 && defined HAVE_MPI
  if (MPI_Alltoallv(itsDoInput ? itsInputData->origin() : 0,
		    sendCounts, sendDisplacements, MPI_BYTE,
		    itsDoOutput ? itsOutputData->origin() : 0,
		    receiveCounts, receiveDisplacements, MPI_BYTE,
		    MPI_COMM_WORLD) != MPI_SUCCESS) {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }
#else
  TH_MPI::synchroniseAllProcesses();
#endif
}


void WH_InputSection::transposeMetaData(const SparseSet<unsigned> &flags)
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

  if (itsDoInput) {
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

  if (itsDoOutput)
    for (unsigned input = 0; input < itsInputNodes.size(); input ++) {
      receiveCounts[itsInputNodes[input]] = sizeof(struct metaData);
      receiveDisplacements[itsInputNodes[input]] = reinterpret_cast<char *>(&itsOutputMetaData[input]) - reinterpret_cast<char *>(itsOutputMetaData);
    }

#if defined HAVE_MPI
  if (MPI_Alltoallv(itsDoInput ? itsInputMetaData : 0,
		    sendCounts, sendDisplacements, MPI_BYTE,
		    itsDoOutput ? itsOutputMetaData : 0,
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
  SparseSet<unsigned> flags;

  if (itsDoInput) {
    doInput(flags);
    limitFlagsLength(flags);
  }

  if (itsDoTranspose) {
    NSTimer transposeTimer("transpose", TH_MPI::getCurrentRank() == 0);
    transposeTimer.start();

    transposeData();
    transposeMetaData(flags);

    transposeTimer.stop();
  }

  if (itsDoOutput)
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
  if (itsDoInput) {
    InputThread::stopThreads();
    itsBBuffer->clear();
    itsInputThread->join();
    delete itsInputThread;
    delete itsInputThreadObject;
    delete itsBBuffer;
    delete itsInputData;
    delete [] itsInputMetaData;
  }

  if (itsDoOutput) {
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

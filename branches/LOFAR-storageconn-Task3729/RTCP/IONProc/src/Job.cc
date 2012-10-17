//#
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
//#  $Id: ION_main.cc 15296 2010-03-24 10:19:41Z romein $

#include <lofar_config.h>

#include <BeamletBufferToComputeNode.h>
#include <ControlPhase3Cores.h>
#include <Common/LofarLogger.h>
#include <Stream/PortBroker.h>
#include <Interface/Stream.h>
#include <Interface/CN_Command.h>
#include <Interface/Exceptions.h>
#include <Interface/PrintVector.h>
#include <Interface/RSPTimeStamp.h>
#include <InputSection.h>
#include <ION_Allocator.h>
#include <Scheduling.h>
#include <GlobalVars.h>
#include <Job.h>
#include <Scheduling.h>
#include <OutputSection.h>
#include <StreamMultiplexer.h>
#include <Stream/SocketStream.h>
#include <Stream/PortBroker.h>

#include <unistd.h>
#include <time.h>

#include <boost/format.hpp>


#define LOG_CONDITION (myPsetNumber == 0)

namespace LOFAR {
namespace RTCP {

unsigned Job::nextJobID = 1;
void	 *Job::theInputSection;
Mutex	 Job::theInputSectionMutex;
unsigned Job::theInputSectionRefCount = 0;

Queue<Job *> finishedJobs;


Job::Job(const char *parsetName)
:
  itsParset(parsetName),
  itsJobID(nextJobID ++), // no need to make thread safe
  itsObservationID(itsParset.observationID()),
  itsIsRunning(false),
  itsDoCancel(false),
  itsBlockNumber(0),
  itsRequestedStopTime(0.0),
  itsStopTime(0.0)
{
  itsLogPrefix = str(boost::format("[obs %d] ") % itsParset.observationID());

  // fill the cache to avoid regenerating it many times over
  itsParset.write(NULL);

  if (LOG_CONDITION) {
    // Handle PVSS (CEPlogProcessor) communication -- report PVSS name in the first log line to allow CEPlogProcessor to resolve obsIDs
    if (itsParset.PVSS_TempObsName() != "")
      LOG_INFO_STR(itsLogPrefix << "PVSS name: " << itsParset.PVSS_TempObsName());

    LOG_INFO_STR(itsLogPrefix << "----- Creating new job");
    LOG_DEBUG_STR(itsLogPrefix << "usedCoresInPset = " << itsParset.usedCoresInPset());
  }

  // Handle PLC communication
  if (myPsetNumber == 0) {
    if (itsParset.PLC_controlled()) {
      // let the ApplController decide what we should do
      try {
        // Do _not_ wait for the stop time to communicate with ApplController,
        // or the whole observation could be wasted.
        itsPLCStream = new SocketStream(itsParset.PLC_Host(), itsParset.PLC_Port(), SocketStream::TCP, SocketStream::Client, time(0) + 30);

        itsPLCClient = new PLCClient(*itsPLCStream, *this, itsParset.PLC_ProcID(), itsObservationID);
        itsPLCClient->start();
      } catch (Exception &ex) {
        LOG_WARN_STR(itsLogPrefix << "Could not connect to ApplController on " << itsParset.PLC_Host() << ":" << itsParset.PLC_Port() << " as " << itsParset.PLC_ProcID() << " -- continuing on autopilot: " << ex);
      }
    }

    if (!itsPLCClient) {
      // we are either not PLC controlled, or we're supposed to be but can't connect to
      // the ApplController
      LOG_INFO_STR(itsLogPrefix << "Not controlled by ApplController");
    }  

  }

  // check enough parset settings just to get to the coordinated check in jobThread safely
  if (itsParset.CNintegrationTime() <= 0)
    THROW(IONProcException,"CNintegrationTime must be bigger than 0");

  // synchronize roughly every 5 seconds to see if the job is cancelled
  itsNrBlockTokensPerBroadcast = static_cast<unsigned>(ceil(5.0 / itsParset.CNintegrationTime()));
  itsNrBlockTokens	       = 1; // trigger a rendez-vous immediately to sync latest stoptime info

  itsHasPhaseOne   = itsParset.phaseOnePsetIndex(myPsetNumber) >= 0;
  itsHasPhaseTwo   = itsParset.phaseTwoPsetIndex(myPsetNumber) >= 0;
  itsHasPhaseThree = itsParset.phaseThreePsetIndex(myPsetNumber) >= 0;

  itsJobThread = new Thread(this, &Job::jobThread, itsLogPrefix + "[JobThread] ", 65536);
}


Job::~Job()
{
  // explicitly free PLCClient first, because it refers to us and needs             
  // a valid Job object to work on                                                  
  delete itsPLCClient.release();                                                    

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Job " << (itsIsRunning ? "finished" : "cancelled") << " successfully");
}


void Job::createIONstreams()
{
  if (myPsetNumber == 0) {
    std::vector<unsigned> involvedPsets = itsParset.usedPsets();

    for (unsigned i = 0; i < involvedPsets.size(); i ++) {
      ASSERT(involvedPsets[i] < allIONstreamMultiplexers.size());

      if (involvedPsets[i] != 0) // do not send to itself
	itsIONstreams.push_back(new MultiplexedStream(*allIONstreamMultiplexers[involvedPsets[i]], itsJobID));
    }    
  } else {
    itsIONstreams.push_back(new MultiplexedStream(*allIONstreamMultiplexers[0], itsJobID));
  }
}


void Job::barrier()
{
  char byte = 0;

  if (myPsetNumber == 0) {
    for (unsigned i = 0; i < itsIONstreams.size(); i ++) {
      itsIONstreams[i]->read(&byte, sizeof byte);
      itsIONstreams[i]->write(&byte, sizeof byte);
    }
  } else {
    itsIONstreams[0]->write(&byte, sizeof byte);
    itsIONstreams[0]->read(&byte, sizeof byte);
  }
}


// returns true iff all psets supply true
bool Job::agree(bool iAgree)
{
  bool allAgree = iAgree; // pset 0 needs to start with its own decision, for other psets this value is ignored

  if (myPsetNumber == 0)
    for (unsigned i = 0; i < itsIONstreams.size(); i ++) {
      bool youAgree;
      itsIONstreams[i]->read(&youAgree, sizeof youAgree);

      allAgree = allAgree && youAgree;
    }
  else
    itsIONstreams[0]->write(&iAgree, sizeof iAgree);

  broadcast(allAgree);  

  return allAgree;
}


template <typename T> void Job::broadcast(T &value)
{
  if (myPsetNumber == 0)
    for (unsigned i = 0; i < itsIONstreams.size(); i ++)
      itsIONstreams[i]->write(&value, sizeof value);
  else
    itsIONstreams[0]->read(&value, sizeof value);
}


Job::StorageProcess::StorageProcess( const Parset &parset, const string &logPrefix, int rank, const string &hostname )
:
  itsParset(parset),
  itsLogPrefix(str(boost::format("%s [StorageWriter rank %2d host %s] ") % logPrefix % rank % hostname)),
  itsRank(rank),
  itsHostname(hostname)
{
}

Job::StorageProcess::~StorageProcess()
{
  // cancel the control thread in case it is still active
  itsThread->cancel();
}


void Job::StorageProcess::start()
{
  // fork (child process will exec)
  std::string userName   = itsParset.getString("OLAP.Storage.userName");
  std::string sshKey     = itsParset.getString("OLAP.Storage.sshIdentityFile");
  std::string executable = itsParset.getString("OLAP.Storage.msWriter");

  char cwd[1024];

  if (getcwd(cwd, sizeof cwd) == 0)
    throw SystemCallException("getcwd", errno, THROW_ARGS);

#ifdef HAVE_LIBSSH2
  std::string commandLine = str(boost::format("cd %s && %s%s %u %d %u 2>&1")
    % cwd
#if defined USE_VALGRIND
    % "valgrind --leak-check=full "
#else
    % ""
#endif
    % executable
    % itsParset.observationID()
    % itsRank
#if defined WORDS_BIGENDIAN
    % 1
#else
    % 0
#endif
  );

  itsSSHconnection = new SSHconnection(itsLogPrefix, itsHostname, commandLine, userName, sshKey, itsParset.stopTime());
  itsSSHconnection->start();
#else

#warning Using fork/exec for SSH processes to Storage
  const std::string obsID = boost::lexical_cast<std::string>(itsParset.observationID());
  const std::string rank = boost::lexical_cast<std::string>(itsRank);

  const char * const commandLine[] = {
    "cd ", cwd, " && ",
#if defined USE_VALGRIND
    "valgrind " "--leak-check=full "
#endif
    executable.c_str(),
    obsID.c_str(),
    rank.c_str(),
#if defined WORDS_BIGENDIAN
    "1",
#else
    "0",
#endif
    0
  };
  itsPID = forkSSH(itsLogPrefix, itsHostname.c_str(), commandLine, userName.c_str(), sshKey.c_str());

  // client process won't reach this point
#endif

  itsThread = new Thread(this, &Job::StorageProcess::controlThread, itsLogPrefix + "[ControlThread] ", 65535);
}


void Job::StorageProcess::stop(struct timespec deadline)
{
#ifdef HAVE_LIBSSH2
  itsSSHconnection->stop(deadline);
#else
  joinSSH(itsLogPrefix, itsPID, (deadline.tv_sec ? deadline.tv_sec : time(0)) + 1);
#endif

  itsThread->cancel();
}


void Job::StorageProcess::controlThread()
{
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] connecting...");
  std::string resource = getStorageControlDescription(itsParset.observationID(), itsRank);
  PortBroker::ClientStream stream(itsHostname, storageBrokerPort(itsParset.observationID()), resource, itsParset.stopTime());

  // for now, we just send the parset and call it a day
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] connected -- sending parset");
  itsParset.write(&stream);
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sent parset");
}


void Job::startStorageProcesses()
{
  vector<string> hostnames = itsParset.getStringVector("OLAP.Storage.hosts");

  itsStorageProcesses.resize(hostnames.size());

  for (unsigned rank = 0; rank < itsStorageProcesses.size(); rank ++) {
    itsStorageProcesses[rank] = new StorageProcess(itsParset, itsLogPrefix, rank, hostnames[rank]);
    itsStorageProcesses[rank]->start();
  }  
}


void Job::stopStorageProcesses()
{
  struct timespec deadline;

  deadline.tv_sec  = time(0) + 300;
  deadline.tv_nsec = 0;

  for (unsigned rank = 0; rank < itsStorageProcesses.size(); rank ++)
    itsStorageProcesses[rank]->stop(deadline);
}


void Job::waitUntilCloseToStartOfObservation(time_t secondsPriorToStart)
{
  time_t closeToStart = static_cast<time_t>(itsParset.startTime()) - secondsPriorToStart;
  char   buf[26];

  ctime_r(&closeToStart, buf);
  buf[24] = '\0';
  
  LOG_INFO_STR(itsLogPrefix << "Waiting for job to start: sleeping until " << buf);

  itsWallClockTime.waitUntil(closeToStart);
}


void Job::cancel()
{
  // note that JobQueue holds lock, so that this function executes atomically

  if (itsDoCancel) {
    LOG_WARN_STR(itsLogPrefix << "Observation already cancelled");
  } else {
    itsDoCancel = true;
    //jobQueue.itsReevaluate.broadcast();

    if (itsParset.realTime())
      itsWallClockTime.cancelWait();
  }
}


void Job::claimResources()
{
  ScopedLock scopedLock(jobQueue.itsMutex);

  while (!itsDoCancel) {
    bool conflict = false;

    for (std::vector<Job *>::iterator job = jobQueue.itsJobs.begin(); job != jobQueue.itsJobs.end(); job ++) {
      std::stringstream error;

      if ((*job)->itsIsRunning && (*job)->itsParset.conflictingResources(itsParset, error)) {
	conflict = true;
	LOG_WARN_STR(itsLogPrefix << "Postponed due to resource conflict with job " << (*job)->itsObservationID << ": " << error.str());
      }
    }

    if (!conflict) {
      itsIsRunning = true;
      return;
    }

    jobQueue.itsReevaluate.wait(jobQueue.itsMutex);
  }
}


void Job::jobThread()
{
#if defined HAVE_BGP_ION
  doNotRunOnCore0();
#endif

  if (myPsetNumber == 0 || itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
    createCNstreams();
    createIONstreams();

    if (myPsetNumber == 0) {
      // PLC: DEFINE phase
      bool canStart = true;

      if (!checkParset()) {
        canStart = false;
      }

      // obey the stop time in the parset -- the first anotherRun() will broadcast it
      if (!pause(itsParset.stopTime())) {
        LOG_ERROR_STR(itsLogPrefix << "Could not set observation stop time");
        canStart = false;
      }

      if (canStart) {
        // PLC: INIT phase
        if (itsParset.realTime())
          waitUntilCloseToStartOfObservation(20);

        // PLC: in practice, RUN must start here, because resources
        // can become available just before the observation starts.
        // This means we will miss the beginning of the observation
        // for now, because we need to calculate the delays still,
        // which can only be done if we know the start time.
        // That means we forgo full PLC control for now and ignore
        // the init/run commands. In practice, the define command
        // won't be useful either since we'll likely disconnect
        // due to an invalid parset before PLC can ask.

        claimResources();

        // we could start Storage before claiming resources
        if (itsIsRunning && itsParset.hasStorage())
          startStorageProcesses();
      } 
    }

    broadcast(itsIsRunning);

    if (itsIsRunning) {
      // PLC: RUN phase

      if (itsParset.realTime()) {
        // if we started after itsParset.startTime(), we want to skip ahead to
        // avoid data loss caused by having to catch up.
        if (myPsetNumber == 0) {
          time_t earliest_start = time(0L) + 5;

          if (earliest_start > itsParset.startTime()) {
            itsBlockNumber = static_cast<unsigned>((earliest_start - itsParset.startTime()) / itsParset.CNintegrationTime());

            LOG_WARN_STR(itsLogPrefix << "Skipping the first " << itsBlockNumber << " blocks to catch up"); 
          } else {
            itsBlockNumber = 0;
          }  
        }

        broadcast(itsBlockNumber);
      }

      // each node is expected to:
      // 1. agree() on starting, to allow the compute nodes to complain in preprocess()
      // 2. call anotherRun() until the end of the observation to synchronise the
      //    stop time.

      if (itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
	switch (itsParset.nrBitsPerSample()) {
	  case  4 : doObservation<i4complex>();
		    break;

	  case  8 : doObservation<i8complex>();
		    break;

	  case 16 : doObservation<i16complex>();
		    break;
	}
      } else {
        if (agree(true)) { // we always agree on the fact that we can start
          // force pset 0 to broadcast itsIsRunning periodically
	  while (anotherRun())
	    ;
        }    
      }    

      // PLC: PAUSE phase
      barrier();

      // PLC: RELEASE phase

      // all InputSections and OutputSections have finished their processing, so
      // Storage should be done any second now.

      stopStorageProcesses();
    }
  }

  finishedJobs.append(this);
}


void Job::createCNstreams()
{
  std::vector<unsigned> usedCoresInPset = itsParset.usedCoresInPset();

  itsCNstreams.resize(usedCoresInPset.size());

  for (unsigned core = 0; core < usedCoresInPset.size(); core ++) {
    ASSERT(usedCoresInPset[core] < nrCNcoresInPset);
    itsCNstreams[core] = allCNstreams[myPsetNumber][usedCoresInPset[core]];
  }

  if (itsHasPhaseOne || itsHasPhaseTwo) {
    std::vector<unsigned> phaseOneTwoCores = itsParset.phaseOneTwoCores();

    itsPhaseOneTwoCNstreams.resize(nrPsets, phaseOneTwoCores.size());

#ifdef CLUSTER_SCHEDULING
    for (unsigned pset = 0; pset < nrPsets; pset ++)
#else
    unsigned pset = myPsetNumber;
#endif
    {
      for (unsigned core = 0; core < phaseOneTwoCores.size(); core ++) {
        ASSERT(phaseOneTwoCores[core] < nrCNcoresInPset);
        itsPhaseOneTwoCNstreams[pset][core] = allCNstreams[pset][phaseOneTwoCores[core]];
      }
    }
  }

  if (itsHasPhaseThree) {
    std::vector<unsigned> phaseThreeCores = itsParset.phaseThreeCores();

    itsPhaseThreeCNstreams.resize(phaseThreeCores.size());

    for (unsigned core = 0; core < phaseThreeCores.size(); core ++) {
      ASSERT(phaseThreeCores[core] < nrCNcoresInPset);
      itsPhaseThreeCNstreams[core] = allCNstreams[myPsetNumber][phaseThreeCores[core]];
    }
  }
}


template <typename SAMPLE_TYPE> void Job::attachToInputSection()
{
  ScopedLock scopedLock(theInputSectionMutex);

  if (theInputSectionRefCount == 0) {
    theInputSection = new InputSection<SAMPLE_TYPE>(itsParset, myPsetNumber);
    ++ theInputSectionRefCount;
  }
}


template <typename SAMPLE_TYPE> void Job::detachFromInputSection()
{
  ScopedLock scopedLock(theInputSectionMutex);

  if (-- theInputSectionRefCount == 0)
    delete static_cast<InputSection<SAMPLE_TYPE> *>(theInputSection);
}


bool Job::configureCNs()
{
  bool success = true;

  CN_Command command(CN_Command::PREPROCESS, itsBlockNumber);
  
  LOG_DEBUG_STR(itsLogPrefix << "Configuring cores " << itsParset.usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++) {
    command.write(itsCNstreams[core]);
    itsParset.write(itsCNstreams[core]);
  }

#if 0 // FIXME: leads to deadlock when using TCP
  for (unsigned core = 0; core < itsCNstreams.size(); core ++) {
    char failed;
    itsCNstreams[core]->read(&failed, sizeof failed);

    if (failed) {
      LOG_ERROR_STR(itsLogPrefix << "Core " << core << " failed to initialise");
      success = false;
    }
  }
#endif
  
  LOG_DEBUG_STR(itsLogPrefix << "Configuring cores " << itsParset.usedCoresInPset() << " done");

  return success;
}


void Job::unconfigureCNs()
{
  CN_Command command(CN_Command::POSTPROCESS);
 
  LOG_DEBUG_STR(itsLogPrefix << "Unconfiguring cores " << itsParset.usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++)
    command.write(itsCNstreams[core]);

  LOG_DEBUG_STR(itsLogPrefix << "Unconfiguring cores " << itsParset.usedCoresInPset() << " done");
}


bool Job::anotherRun()
{
  if (-- itsNrBlockTokens == 0) {
    itsNrBlockTokens = itsNrBlockTokensPerBroadcast;

    // only consider cancelling at itsNrBlockTokensPerBroadcast boundaries
    itsIsRunning = !itsDoCancel;

    // only allow pset 0 to actually decide whether or not to stop
    broadcast(itsIsRunning);

    // sync updated stop times -- abuse atomicity of copying itsRequestedStopTime
    itsStopTime = itsRequestedStopTime;
    broadcast(itsStopTime);
  }

  // move on to the next block
  itsBlockNumber ++;

  bool done = !itsIsRunning;

  if (itsStopTime > 0.0) {
    // the end time of this block must still be within the observation
    double currentTime = itsParset.startTime() + (itsBlockNumber + 1) * itsParset.CNintegrationTime();

    done = done || currentTime >= itsStopTime;
  }

  return !done;
}


template <typename SAMPLE_TYPE> void Job::doObservation()
{
  std::vector<OutputSection *> outputSections;

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation start");

  // first: send configuration to compute nodes so they know what to expect
  if (!agree(configureCNs())) {
    unconfigureCNs();

    if (LOG_CONDITION)
      LOG_INFO_STR(itsLogPrefix << "----- Observation finished");

    return;
  }

  if (itsHasPhaseOne)
    attachToInputSection<SAMPLE_TYPE>();

  if (itsHasPhaseTwo) {
    if (itsParset.outputCorrelatedData())
      outputSections.push_back(new CorrelatedDataOutputSection(itsParset, itsBlockNumber));
  }

  if (itsHasPhaseThree) {
    if (itsParset.outputBeamFormedData())
      outputSections.push_back(new BeamFormedDataOutputSection(itsParset, itsBlockNumber));

    if (itsParset.outputTrigger())
      outputSections.push_back(new TriggerDataOutputSection(itsParset, itsBlockNumber));
  }

  // start the threads
  for (unsigned i = 0; i < outputSections.size(); i ++)
    outputSections[i]->start();

  LOG_DEBUG_STR(itsLogPrefix << "doObservation processing input start");

  { // separate scope to ensure that the beamletbuffertocomputenode objects
    // only exist if the beamletbuffers exist in the inputsection
    std::vector<SmartPtr<BeamletBuffer<SAMPLE_TYPE> > > noInputs;
    BeamletBufferToComputeNode<SAMPLE_TYPE>   beamletBufferToComputeNode(itsParset, itsPhaseOneTwoCNstreams, itsHasPhaseOne ? static_cast<InputSection<SAMPLE_TYPE> *>(theInputSection)->itsBeamletBuffers : noInputs, myPsetNumber, itsBlockNumber);

    ControlPhase3Cores controlPhase3Cores(itsParset, itsPhaseThreeCNstreams, itsBlockNumber);
    controlPhase3Cores.start(); // start the thread

    while (anotherRun()) {
      for (unsigned i = 0; i < outputSections.size(); i ++)
	outputSections[i]->addIterations(1);

      controlPhase3Cores.addIterations(1);

      beamletBufferToComputeNode.process();
    }

    LOG_DEBUG_STR(itsLogPrefix << "doObservation processing input done");
  }

  for (unsigned i = 0; i < outputSections.size(); i ++)
    outputSections[i]->noMoreIterations();

  for (unsigned i = 0; i < outputSections.size(); i ++)
    delete outputSections[i];

  if (itsHasPhaseOne)
    detachFromInputSection<SAMPLE_TYPE>();

  unconfigureCNs();
 
  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation finished");
}


bool Job::checkParset() const
{
  // any error detected by the python environment, invalidating this parset
  string pythonParsetError = itsParset.getString("OLAP.IONProc.parsetError","");

  if (pythonParsetError != "" ) {
    LOG_ERROR_STR(itsLogPrefix << "Early detected parset error: " << pythonParsetError );
    return false;
  }

  try {
    itsParset.check();
  } catch( InterfaceException &ex ) {
    LOG_ERROR_STR(itsLogPrefix << "Parset check failed on " << ex.what() );
    return false;
  }

  if (itsParset.nrCoresPerPset() > nrCNcoresInPset) {
    LOG_ERROR_STR(itsLogPrefix << "nrCoresPerPset (" << itsParset.nrCoresPerPset() << ") cannot exceed " << nrCNcoresInPset);
    return false;
  }

  return true;
}


void Job::printInfo() const
{
  LOG_INFO_STR(itsLogPrefix << "JobID = " << itsJobID << ", " << (itsIsRunning ? "running" : "not running"));
}


// expected sequence: define -> init -> run -> pause -> release -> quit

bool Job::define()
{
  LOG_DEBUG_STR(itsLogPrefix << "Job: define(): check parset");

  return checkParset();
}


bool Job::init()
{
  LOG_DEBUG_STR(itsLogPrefix << "Job: init(): allocate buffers / make connections");

  return true;
}


bool Job::run()
{
  LOG_DEBUG_STR(itsLogPrefix << "Job: run(): run observation");

  // we ignore this, since 'now' is both ill-defined and we need time
  // to communicate changes to other psets

  return true;
}


bool Job::pause(const double &when)
{
  char   buf[26];
  time_t whenRounded = static_cast<time_t>(when);

  ctime_r(&whenRounded, buf);
  buf[24] = '\0';
  
  LOG_DEBUG_STR(itsLogPrefix << "Job: pause(): pause observation at " << buf);

  // make sure we don't interfere with queue dynamics
  ScopedLock scopedLock(jobQueue.itsMutex);

  if (itsParset.realTime() && (when == 0 || when <= itsParset.startTime())) { // yes we can compare a double to 0
    // make sure we also stop waiting for the job to start

    if (!itsDoCancel)
      cancel();
  } else {
    itsRequestedStopTime = when;
  }

  return true;
}


bool Job::quit()
{
  LOG_DEBUG_STR(itsLogPrefix << "Job: quit(): end observation");
  // stop now

  if (!itsDoCancel) {
    ScopedLock scopedLock(jobQueue.itsMutex);

    cancel();
  }

  return true;
}


bool Job::observationRunning()
{
  LOG_DEBUG_STR(itsLogPrefix << "Job: observationRunning()");
  return itsIsRunning;
}

} // namespace RTCP
} // namespace LOFAR

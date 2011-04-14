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
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Interface/Exceptions.h>
#include <Interface/PrintVector.h>
#include <Interface/RSPTimeStamp.h>
#include <InputSection.h>
#include <ION_main.h>
#include <Job.h>
#include <OutputSection.h>
#include <StreamMultiplexer.h>
#include <Stream/SocketStream.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include <boost/format.hpp>
using boost::format;

#define LOG_CONDITION (myPsetNumber == 0)

namespace LOFAR {
namespace RTCP {

unsigned Job::nextJobID = 1;
void	 *Job::theInputSection;
Mutex	 Job::theInputSectionMutex;
unsigned Job::theInputSectionRefCount = 0;


Job::Job(const char *parsetName)
:
  itsParset(parsetName),
  itsJobID(nextJobID ++), // no need to make thread safe
  itsObservationID(itsParset.observationID()),
  itsPLCStream(0),
  itsPLCClient(0),
  itsIsRunning(false),
  itsDoCancel(false),
  itsBlockNumber(0),
  itsRequestedStopTime(0.0),
  itsStopTime(0.0)
{
  itsLogPrefix = str(format("[obs %d] ") % itsParset.observationID());

  if (LOG_CONDITION) {
    LOG_INFO_STR(itsLogPrefix << "----- Creating new job");
    LOG_DEBUG_STR(itsLogPrefix << "usedCoresInPset = " << itsParset.usedCoresInPset());
  }

  if (myPsetNumber == 0) {
    if (itsParset.PLC_controlled()) {
      // let the ApplController decide what we should do
      try {
        itsPLCStream = new SocketStream( itsParset.PLC_Host().c_str(), itsParset.PLC_Port(), SocketStream::TCP, SocketStream::Client, 60 );

        itsPLCClient = new PLCClient( *itsPLCStream, *this, itsParset.PLC_ProcID(), itsObservationID );
      } catch( Exception &ex ) {
        LOG_WARN_STR( itsLogPrefix << "Could not connect to ApplController on " << itsParset.PLC_Host() << ":" << itsParset.PLC_Port() << " as " << itsParset.PLC_ProcID() << " -- continuing on autopilot: " << ex );
      }
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
  delete itsJobThread;
  jobQueue.remove(this);

  delete itsPLCClient;
  delete itsPLCStream;

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Job " << (itsIsRunning ? "finished" : "cancelled") << " successfully");
}


void Job::createIONstreams()
{
  if (myPsetNumber == 0) {
    std::vector<unsigned> involvedPsets = itsParset.usedPsets();

    for (unsigned i = 0; i < involvedPsets.size(); i ++) {
      ASSERT( involvedPsets[i] < allIONstreamMultiplexers.size() );

      if (involvedPsets[i] != 0) // do not send to itself
	itsIONstreams.push_back(new MultiplexedStream(*allIONstreamMultiplexers[involvedPsets[i]], itsJobID));
    }    
  } else {
    itsIONstreams.push_back(new MultiplexedStream(*allIONstreamMultiplexers[0], itsJobID));
  }
}


void Job::deleteIONstreams()
{
  if (myPsetNumber == 0)
    for (unsigned i = 0; i < itsIONstreams.size(); i ++)
      delete itsIONstreams[i];
  else
    delete itsIONstreams[0];

  itsIONstreams.clear();
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


void Job::execSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *rank, const char *parset, const char *cwd, const char *isBigEndian)
{
  // DO NOT DO ANY CALL THAT GRABS A LOCK, since the lock may be held by a
  // thread that is no longer part of our address space

  // create a blocking stdin pipe
  // rationale: this forked process inherits stdin from the parent process, which is unusable because IONProc is started in the background
  // and routed through mpirun as well. Also, it is shared by all forked processes. Nevertheless, we want Storage to be able to determine
  // when to shut down based on whether stdin is open. So we create a new stdin. Even though the pipe we create below will block since there
  // will never be anything to read, closing it will propagate to Storage and that's enough.
  int pipefd[2];
  int ignoreResult = pipe(pipefd);

  ignoreResult = close(0);
  ignoreResult = dup(pipefd[0]);

  execl("/usr/bin/ssh",
    "ssh",
    "-q",
    "-i", sshKey,
    "-c", "blowfish",
    "-o", "StrictHostKeyChecking=no",
    "-o", "UserKnownHostsFile=/dev/null",
    "-o", "ServerAliveInterval=30",
    "-l", userName,
    hostName,

    "cd", cwd, "&&",

    executable, rank, parset, isBigEndian,

    static_cast<char *>(0)
  );

  ignoreResult = write(2, "exec failed\n", 12); // Logger uses mutex, hence write directly
  exit(1);
}


void Job::forkSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *rank, const char *parset, const char *cwd, const char *isBigEndian, int &storagePID)
{
  LOG_INFO_STR("Storage writer on " << hostName << ": starting as rank " << rank);
  LOG_DEBUG_STR("child will exec "
    "\"/usr/bin/ssh "
    "-q "
    "-i " << sshKey << " "
    "-c blowfish "
    "-o StrictHostKeyChecking=no "
    "-o UserKnownHostsFile=/dev/null "
    "-o ServerAliveInterval=30 "
    "-l " << userName << " "
    << hostName << " "
    "cd " << cwd << " && " << executable << " " << rank << " " << parset << " " << isBigEndian <<
    "\""
  );

  switch (storagePID = fork()) {
    case -1 : throw SystemCallException("fork", errno, THROW_ARGS);

    case  0 : execSSH(sshKey, userName, hostName, executable, rank, parset, cwd, isBigEndian);
  }
}


void Job::joinSSH(int childPID, const std::string &hostName, unsigned &timeout)
{
  if (childPID != 0) {
    int status;

    // always try at least one waitpid(). if child has not exited, optionally
    // sleep and try again.
    for (;;) {
      pid_t ret;

      if ((ret = waitpid(childPID, &status, WNOHANG)) == (pid_t)-1) {
        int error = errno;

        if (error == EINTR) {
          LOG_DEBUG_STR(itsLogPrefix << "Storage writer on " << hostName << " : waitpid() was interrupted -- retrying");
          continue;
        }

        // error
        LOG_WARN_STR(itsLogPrefix << "Storage writer on " << hostName << " : waitpid() failed with errno " << error);
        return;
      } else if (ret == 0) {
        // child still running
        if (timeout == 0) {
          break;
        }

        timeout--;
        sleep(1);
      } else {
        // child exited
        if (WIFSIGNALED(status) != 0)
          LOG_WARN_STR(itsLogPrefix << "SSH to storage writer on " << hostName << " was killed by signal " << WTERMSIG(status));
        else if (WEXITSTATUS(status) != 0) {
          const char *explanation;

          switch (WEXITSTATUS(status)) {
            default:
              explanation = "??";
              break;

            case 255:
              explanation = "Network or authentication error";
              break;
            case 127:
              explanation = "BASH: command/library not found";
              break;
            case 126:
              explanation = "BASH: command found but could not be executed (wrong architecture?)";
              break;

            case 128 + SIGHUP:
              explanation = "killed by SIGHUP";
              break;
            case 128 + SIGINT:
              explanation = "killed by SIGINT (Ctrl-C)";
              break;
            case 128 + SIGQUIT:
              explanation = "killed by SIGQUIT";
              break;
            case 128 + SIGILL:
              explanation = "illegal instruction";
              break;
            case 128 + SIGABRT:
              explanation = "killed by SIGABRT";
              break;
            case 128 + SIGKILL:
              explanation = "killed by SIGKILL";
              break;
            case 128 + SIGSEGV:
              explanation = "segmentation fault";
              break;
            case 128 + SIGPIPE:
              explanation = "broken pipe";
              break;
            case 128 + SIGALRM:
              explanation = "killed by SIGALRM";
              break;
            case 128 + SIGTERM:
              explanation = "killed by SIGTERM";
              break;
          }

          LOG_ERROR_STR(itsLogPrefix << "Storage writer on " << hostName << " exited with exit code " << WEXITSTATUS(status) << " (" << explanation << ")" );
        } else
          LOG_INFO_STR(itsLogPrefix << "Storage writer on " << hostName << " terminated normally");

        return;  
      }
    }

    // child did not exit within the given timeout

    LOG_WARN_STR(itsLogPrefix << "Storage writer on " << hostName << " : sending SIGTERM");
    kill(childPID, SIGTERM);

    if (waitpid(childPID, &status, 0) == -1) {
      LOG_WARN_STR(itsLogPrefix << "Storage writer on " << hostName << " : waitpid() failed");
    }

    LOG_WARN_STR(itsLogPrefix << "Storage writer on " << hostName << " terminated after sending SIGTERM");
  }
}


void Job::startStorageProcesses()
{
  itsStorageHostNames = itsParset.getStringVector("OLAP.Storage.hosts");

  std::string userName   = itsParset.getString("OLAP.Storage.userName");
  std::string sshKey     = itsParset.getString("OLAP.Storage.sshIdentityFile");
  std::string executable = itsParset.getString("OLAP.Storage.msWriter");
  std::string parset     = itsParset.name();

  char cwd[1024];

  if (getcwd(cwd, sizeof cwd) == 0) {
    throw SystemCallException("getcwd", errno, THROW_ARGS);
  }

  itsStoragePIDs.resize(itsStorageHostNames.size());

  for (unsigned rank = 0; rank < itsStorageHostNames.size(); rank ++)
    forkSSH(sshKey.c_str(),
	    userName.c_str(),
	    itsStorageHostNames[rank].c_str(),
	    executable.c_str(),
	    boost::lexical_cast<std::string>(rank).c_str(),
	    parset.c_str(),
	    cwd,
#if defined WORDS_BIGENDIAN
	    "1",
#else
	    "0",
#endif
	    itsStoragePIDs[rank]);
}


void Job::stopStorageProcesses()
{
  // warning: there could be zero storage processes
  unsigned timeleft = 10;

  for (unsigned rank = 0; rank < itsStoragePIDs.size(); rank ++)
    joinSSH(itsStoragePIDs[rank], itsStorageHostNames[rank], timeleft);
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
    jobQueue.itsReevaluate.broadcast();

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
  if (myPsetNumber == 0 || itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
    createCNstreams();
    createIONstreams();

    if (myPsetNumber == 0) {
      // PLC: DEFINE phase
      bool canStart = true;

      if (!checkParset()) {
        canStart = false;
      }

      if (!itsPLCClient) {
        // we are either not PLC controlled, or we're supposed to be but can't connect to
        // the ApplController
        LOG_INFO_STR( itsLogPrefix << "Not controlled by ApplController" );

        // perform some functions which ApplController would have us do

        // obey the stop time in the parset -- the first anotherRun() will broadcast it
        if(!pause( itsParset.stopTime() )) {
          LOG_ERROR_STR( itsLogPrefix << "Could not set observation stop time" );
          canStart = false;
        }
      }

      if (canStart) {
        // PLC: INIT phase
        if (itsParset.realTime())
          waitUntilCloseToStartOfObservation(10);

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

      // each node is expected to:
      // 1. agree() on starting, to allow the compute nodes to complain in preprocess()
      // 2. call anotherRun() until the end of the observation to synchronise the
      //    stop time.

      if (itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree)
	switch (itsParset.nrBitsPerSample()) {
	  case  4 : doObservation<i4complex>();
		    break;

	  case  8 : doObservation<i8complex>();
		    break;

	  case 16 : doObservation<i16complex>();
		    break;
	}
      else {
        if(agree(true)) { // we always agree on the fact that we can start
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

    deleteIONstreams();
  }

  delete this;
}


void Job::createCNstreams()
{
  std::vector<unsigned> usedCoresInPset = itsParset.usedCoresInPset();

  itsCNstreams.resize(usedCoresInPset.size());

  for (unsigned core = 0; core < usedCoresInPset.size(); core ++)
    itsCNstreams[core] = allCNstreams[usedCoresInPset[core]];

  if (itsHasPhaseOne || itsHasPhaseTwo) {
    std::vector<unsigned> phaseOneTwoCores = itsParset.phaseOneTwoCores();

    itsPhaseOneTwoCNstreams.resize(phaseOneTwoCores.size());

    for (unsigned core = 0; core < phaseOneTwoCores.size(); core ++)
      itsPhaseOneTwoCNstreams[core] = allCNstreams[phaseOneTwoCores[core]];
  }

  if (itsHasPhaseThree) {
    std::vector<unsigned> phaseThreeCores = itsParset.phaseThreeCores();

    itsPhaseThreeCNstreams.resize(phaseThreeCores.size());

    for (unsigned core = 0; core < phaseThreeCores.size(); core ++)
      itsPhaseThreeCNstreams[core] = allCNstreams[phaseThreeCores[core]];
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

  CN_Command	   command(CN_Command::PREPROCESS);
  CN_Configuration configuration(itsParset);
  
  LOG_DEBUG_STR(itsLogPrefix << "Configuring cores " << itsParset.usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++) {
    command.write(itsCNstreams[core]);
    configuration.write(itsCNstreams[core]);
  }

  for (unsigned core = 0; core < itsCNstreams.size(); core ++) {
    char failed;
    itsCNstreams[core]->read(&failed, sizeof failed);

    if (failed) {
      LOG_ERROR_STR(itsLogPrefix << "Core " << core << " failed to initialise");
      success = false;
    }
  }
  
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
    itsIsRunning   = !itsDoCancel;

    // only allow pset 0 to actually decide whether or not to stop
    broadcast(itsIsRunning);

    // sync updated stop times -- abuse atomicity of copying itsRequestedStopTime
    itsStopTime = itsRequestedStopTime;
    broadcast(itsStopTime);
  }

  bool done = !itsIsRunning;

  if (itsStopTime > 0.0) {
    // start time of last processed block
    double currentTime = itsParset.startTime() + itsBlockNumber * itsParset.CNintegrationTime();

    done = done || currentTime >= itsStopTime;
  }

  itsBlockNumber++;

  return !done;
}


template <typename SAMPLE_TYPE> void Job::doObservation()
{
  CN_Configuration configuration(itsParset);
  CN_ProcessingPlan<> plan(configuration);
  plan.removeNonOutputs();
  unsigned nrOutputTypes = plan.nrOutputTypes();
  std::vector<OutputSection *> outputSections(nrOutputTypes, 0);
  unsigned nrparts = itsParset.nrPartsPerStokes();
  unsigned nrbeams = itsParset.flysEye() ? itsParset.nrMergedStations() : itsParset.nrPencilBeams();

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

  // start output process threads
  for (unsigned output = 0; output < nrOutputTypes; output ++) {
    ProcessingPlan::planlet &p = plan.plan[output];

    unsigned phase, maxlistsize;
    int psetIndex;
    std::vector<std::pair<unsigned,std::string> > list; // list of filenames
    std::vector<unsigned> cores;

    std::string mask = itsParset.fileNameMask( p.info.storageParsetPrefix );

    switch (p.info.distribution) {
      case ProcessingPlan::DIST_SUBBAND:
        phase = 2;
        cores = itsParset.phaseOneTwoCores();
        psetIndex = itsParset.phaseTwoPsetIndex(myPsetNumber);

        if (psetIndex < 0) {
          // this pset does not participate for this output
          continue;
        }

        maxlistsize = itsParset.nrSubbandsPerPset();


        for (unsigned sb = 0; sb < itsParset.nrSubbandsPerPset(); sb ++) {
          unsigned s = psetIndex * itsParset.nrSubbandsPerPset() + sb;

          if (s < itsParset.nrSubbands()) {
            std::string filename = itsParset.constructSubbandFilename( mask, s );
            list.push_back(std::pair<unsigned,std::string>(s,filename));
          }
        }

        break;

      case ProcessingPlan::DIST_BEAM:
        phase = 3;
        cores = itsParset.phaseThreeCores();
        psetIndex = itsParset.phaseThreePsetIndex(myPsetNumber);

        if (psetIndex < 0) {
          // this pset does not participate for this outputlist
          continue;
        }

        if (itsParset.phaseThreeDisjunct()) {
          // simplification: each core produces at most 1 beam
          assert( itsParset.nrBeamsPerPset() <= itsParset.phaseThreeCores().size() );

          maxlistsize = itsParset.nrBeamsPerPset();
        } else {
          // simplification: each core produces at most 1 beam
          assert( itsParset.nrBeamsPerPset() <= itsParset.nrSubbandsPerPset() );

          // simplification: also each core which processes a beam also processes a subband
          assert( nrbeams <= itsParset.nrSubbands() );

          maxlistsize = itsParset.nrSubbandsPerPset();
        }

        for (unsigned beam = 0;  beam < itsParset.nrBeamsPerPset(); beam ++) {
          unsigned nrstokes = p.info.nrStokes;

          unsigned beamNumber = psetIndex * itsParset.nrBeamsPerPset() + beam;

          unsigned b = beamNumber / nrparts / nrstokes;
          unsigned s = beamNumber / nrparts % nrstokes;
          unsigned q = beamNumber % nrparts;

          if (b < nrbeams) {
            std::string filename = itsParset.constructBeamFormedFilename( mask, b, s, q );
            list.push_back(std::pair<unsigned,std::string>(beamNumber,filename));
          }
        }

        break;

      default:
        continue;
    }

    outputSections[output] = new OutputSection(itsParset, cores, list, maxlistsize, p, &createCNstream);
  }

  LOG_DEBUG_STR(itsLogPrefix << "doObservation processing input start");

  { // separate scope to ensure that the beamletbuffertocomputenode objects
    // only exist if the beamletbuffers exist in the inputsection
    std::vector<BeamletBuffer<SAMPLE_TYPE> *> noInputs;
    BeamletBufferToComputeNode<SAMPLE_TYPE>   beamletBufferToComputeNode(&itsParset, itsPhaseOneTwoCNstreams, itsHasPhaseOne ? static_cast<InputSection<SAMPLE_TYPE> *>(theInputSection)->itsBeamletBuffers : noInputs, myPsetNumber);

    ControlPhase3Cores controlPhase3Cores(&itsParset, itsPhaseThreeCNstreams);

    while (anotherRun()) {
      for (unsigned output = 0; output < nrOutputTypes; output ++) {
        if (outputSections[output])
          outputSections[output]->addIterations( 1 );
      }

      controlPhase3Cores.addIterations( 1 );

      beamletBufferToComputeNode.process();
    }

    LOG_DEBUG_STR(itsLogPrefix << "doObservation processing input done");
  }

  for (unsigned output = 0; output < nrOutputTypes; output ++)
    if (outputSections[output])
      outputSections[output]->noMoreIterations();

  for (unsigned output = 0; output < nrOutputTypes; output ++)
    delete outputSections[output];

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
  LOG_DEBUG_STR( itsLogPrefix << "Job: define(): check parset" );

  return checkParset();
}


bool Job::init()
{
  LOG_DEBUG_STR( itsLogPrefix << "Job: init(): allocate buffers / make connections" );

  return true;
}


bool Job::run()
{
  LOG_DEBUG_STR( itsLogPrefix << "Job: run(): run observation" );

  // we ignore this, since 'now' is both ill-defined and we need time
  // to communicate changes to other psets

  return true;
}


bool Job::pause( const double &when )
{
  char   buf[26];
  time_t whenRounded = static_cast<time_t>(when);

  ctime_r(&whenRounded, buf);
  buf[24] = '\0';
  
  LOG_DEBUG_STR( itsLogPrefix << "Job: pause(): pause observation at " << buf );

  // make sure we don't interfere with queue dynamics
  ScopedLock scopedLock(jobQueue.itsMutex);

  if (when == 0 || when <= itsParset.startTime()) { // yes we can compare a double to 0
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
  LOG_DEBUG_STR( itsLogPrefix << "Job: quit(): end observation" );

  // stop now

  if (!itsDoCancel) {
    ScopedLock scopedLock(jobQueue.itsMutex);

    cancel();
  }

  return true;
}


bool Job::observationRunning()
{
  LOG_DEBUG_STR( itsLogPrefix << "Job: observationRunning()" );

  return itsIsRunning;
}

} // namespace RTCP
} // namespace LOFAR

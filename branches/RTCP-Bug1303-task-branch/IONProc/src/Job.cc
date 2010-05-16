//#  Job.cc:
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
#include <Common/LofarLogger.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/Exceptions.h>
#include <Interface/PrintVector.h>
#include <Interface/RSPTimeStamp.h>
#include <InputSection.h>
#include <ION_main.h>
#include <Job.h>
#include <OutputSection.h>
#include <StreamMultiplexer.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


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
  itsIsRunning(false),
  itsDoCancel(false)
{
  checkParset();


  LOG_DEBUG_STR("Creating new observation, ObsID = " << itsParset.observationID());
  LOG_DEBUG_STR("ObsID = " << itsParset.observationID() << ", usedCoresInPset = " << itsParset.usedCoresInPset() << ", myPsetNumber = " << myPsetNumber );

  itsNrRuns = static_cast<unsigned>(ceil((itsParset.stopTime() - itsParset.startTime()) / itsParset.CNintegrationTime()));
  LOG_DEBUG_STR("itsNrRuns = " << itsNrRuns);

  // synchronize roughly every 5 seconds to see if the job is cancelled
  itsNrRunTokensPerBroadcast = static_cast<unsigned>(ceil(5.0 / itsParset.CNintegrationTime()));
  itsNrRunTokens	     = itsNrRunTokensPerBroadcast;

  itsHasPhaseOne   = itsParset.phaseOnePsetIndex(myPsetNumber) >= 0;
  itsHasPhaseTwo   = itsParset.phaseTwoPsetIndex(myPsetNumber) >= 0;
  itsHasPhaseThree = itsParset.phaseThreePsetIndex(myPsetNumber) >= 0;

  itsJobThread = new Thread(this, &Job::jobThread, 65536);
}


Job::~Job()
{
  delete itsJobThread;
  jobQueue.remove(this);

  LOG_INFO_STR("ObsID = " << itsObservationID << ": " << (itsIsRunning ? "ended" : "cancelled") << " successfully");
}


void Job::createIONstreams()
{
  if (myPsetNumber == 0) {
    std::vector<unsigned> involvedPsets = itsParset.usedPsets();

    for (unsigned i = 0; i < involvedPsets.size(); i ++)
      if (involvedPsets[i] != 0) // do not send to itself
	itsIONstreams.push_back(new MultiplexedStream(*allIONstreamMultiplexers[involvedPsets[i]], itsJobID));
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


template <typename T> void Job::broadcast(T &value)
{
  if (myPsetNumber == 0)
    for (unsigned i = 0; i < itsIONstreams.size(); i ++)
      itsIONstreams[i]->write(&value, sizeof value);
  else
    itsIONstreams[0]->read(&value, sizeof value);
}


void Job::execSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *rank, const char *parset)
{
  // DO NOT DO ANY CALL THAT GRABS A LOCK, since the lock may be held by a
  // thread that is no longer part of our address space

  execl("/usr/bin/ssh",
    "ssh",
    "-i", sshKey,
    "-c", "blowfish",
    "-o", "StrictHostKeyChecking=no",
    "-o", "UserKnownHostsFile=/dev/null",
    "-l", userName,
    hostName,
    executable,
    rank,
    parset,
    static_cast<char *>(0)
  );

  write(2, "exec failed\n", 12); // Logger uses mutex, hence write directly
  exit(1);
}


void Job::forkSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *rank, const char *parset, int &storagePID)
{
  LOG_INFO_STR("child will exec("
    "\"/usr/bin/ssh\", "
    "\"ssh\", "
    "\"-i\", \"" << sshKey << "\", "
    "\"-c\", \"blowfish\", "
    "\"-o\", \"StrictHostKeyChecking=no\", "
    "\"-o\", \"UserKnownHostsFile=/dev/null\", "
    "\"-l\", \"" << userName << "\", "
    "\"" << hostName << "\", "
    "\"" << executable << "\", "
    "\"" << rank << "\", "
    "\"" << parset << "\", "
    "0)"
  );

  switch (storagePID = fork()) {
    case -1 : throw SystemCallException("fork", errno, THROW_ARGS);

    case  0 : execSSH(sshKey, userName, hostName, executable, rank, parset);
  }
}


void Job::joinSSH(int childPID, const std::string &hostName)
{
  if (childPID != 0) {
    int status;

    if (waitpid(childPID, &status, 0) == -1)
      LOG_WARN_STR("storage writer on " << hostName << " : waitpid() failed");
    else if (WIFSIGNALED(status) != 0)
      LOG_WARN_STR("storage writer on " << hostName << " was killed by signal " << WTERMSIG(status));
    else if (WEXITSTATUS(status) != 0)
      LOG_WARN_STR("storage writer on " << hostName << " exited with exit code " << WEXITSTATUS(status));
    else
      LOG_INFO_STR("storage writer on " << hostName << " terminated normally");
  }
}


void Job::startStorageProcesses()
{
  //itsStorageHostNames = itsParset.getStringVector("Observation.VirtualInstrument.storageNodeList");
// use IP addresses, since the I/O node cannot resolve host names
  itsStorageHostNames = itsParset.getStringVector("OLAP.OLAP_Conn.IONProc_Storage_ServerHosts");

  std::string userName   = itsParset.getString("OLAP.Storage.userName");
  std::string sshKey     = itsParset.getString("OLAP.Storage.sshIdentityFile");
  std::string executable = itsParset.getString("OLAP.Storage.msWriter");
  std::string parset     = itsParset.name();

  itsStoragePIDs.resize(itsStorageHostNames.size());

  for (unsigned rank = 0; rank < itsStorageHostNames.size(); rank ++)
    forkSSH(sshKey.c_str(),
	    userName.c_str(),
	    itsStorageHostNames[rank].c_str(),
	    executable.c_str(),
	    boost::lexical_cast<std::string>(rank).c_str(),
	    parset.c_str(),
	    itsStoragePIDs[rank]);
}


void Job::stopStorageProcesses()
{
  for (unsigned rank = 0; rank < itsStorageHostNames.size(); rank ++)
    joinSSH(itsStoragePIDs[rank], itsStorageHostNames[rank]);
}


void Job::waitUntilCloseToStartOfObservation(time_t secondsPriorToStart)
{
  time_t closeToStart = static_cast<time_t>(itsParset.startTime()) - secondsPriorToStart;
  char   buf[26];

  ctime_r(&closeToStart, buf);
  buf[24] = '\0';
  
  LOG_DEBUG_STR("waiting for job " << itsObservationID << " to start: sleeping until " << buf);

  itsWallClockTime.waitUntil(closeToStart);
}


void Job::cancel()
{
  // note that JobQueue holds lock, so that this function executes atomically

  if (itsDoCancel) {
    LOG_WARN_STR("ObsID = " << itsObservationID << ": already cancelled");
  } else {
    itsDoCancel = true;
    jobQueue.itsReevaluate.broadcast();

    if (itsParset.realTime())
      itsWallClockTime.cancelWait();
  }
}


void Job::claimResources()
{
  LOG_INFO_STR("ObsID = " << itsObservationID << ": claiming resources ...");

  ScopedLock scopedLock(jobQueue.itsMutex);

retry:
  if (itsDoCancel)
    return;

  for (std::vector<Job *>::iterator job = jobQueue.itsJobs.begin(); job != jobQueue.itsJobs.end(); job ++)
    if ((*job)->itsIsRunning && ((*job)->itsParset.overlappingResources(itsParset) || !(*job)->itsParset.compatibleInputSection(itsParset))) {
      LOG_WARN_STR("ObsID = " << itsObservationID << ": postponed due to resource conflicts");
      jobQueue.itsReevaluate.wait(jobQueue.itsMutex);
      goto retry;
    }

  itsIsRunning = true;

  LOG_INFO_STR("ObsID = " << itsObservationID << ": claiming resources done");
}


void Job::jobThread()
{
  LOG_INFO_STR("ObsID = " << itsObservationID << ": initialising");

  if (myPsetNumber == 0 || itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
    LOG_INFO_STR("ObsID = " << itsObservationID << ": copying CN streams");
    createCNstreams();
    LOG_INFO_STR("ObsID = " << itsObservationID << ": copying ION streams");
    createIONstreams();

    bool storageStarted = false;

    if (myPsetNumber == 0) {
      LOG_INFO_STR("ObsID = " << itsObservationID << ": wait for start and start storage processes");
      if (itsParset.realTime())
	waitUntilCloseToStartOfObservation(10);

      claimResources();

      if (itsIsRunning && itsParset.hasStorage()) {
	startStorageProcesses();
	storageStarted = true;
      }
    }

    LOG_INFO_STR("ObsID = " << itsObservationID << ": broadcast that we're running");

    broadcast(itsIsRunning);

    if (itsIsRunning) {
      if (itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree)
	switch (itsParset.nrBitsPerSample()) {
	  case  4 : doObservation<i4complex>();
		    break;

	  case  8 : doObservation<i8complex>();
		    break;

	  case 16 : doObservation<i16complex>();
		    break;
	}
      else // force pset 0 to broadcast itsIsRunning periodically
	for (unsigned i = 0; i < itsNrRuns && !isCancelled(); i ++)
	  ;

      barrier();

      if (storageStarted)
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


void Job::configureCNs()
{
  CN_Command	   command(CN_Command::PREPROCESS);
  CN_Configuration configuration(itsParset);
  
  LOG_DEBUG_STR("configuring cores " << itsParset.usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++) {
    command.write(itsCNstreams[core]);
    configuration.write(itsCNstreams[core]);
  }
  
  LOG_DEBUG_STR("configuring cores " << itsParset.usedCoresInPset() << " done");
}


void Job::unconfigureCNs()
{
  CN_Command command(CN_Command::POSTPROCESS);

  LOG_DEBUG_STR("unconfiguring cores " << itsParset.usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++)
    command.write(itsCNstreams[core]);

  LOG_DEBUG_STR("unconfiguring cores " << itsParset.usedCoresInPset() << " done");
}


bool Job::isCancelled()
{
  if (-- itsNrRunTokens == 0) {
    itsNrRunTokens = itsNrRunTokensPerBroadcast;
    itsIsRunning   = !itsDoCancel;
    broadcast(itsIsRunning);
  }

  return !itsIsRunning;
}


template <typename SAMPLE_TYPE> void Job::doObservation()
{
  if (itsHasPhaseOne)
    attachToInputSection<SAMPLE_TYPE>();

  CN_Configuration configuration(itsParset);
  CN_ProcessingPlan<> plan(configuration, itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree);
  plan.removeNonOutputs();
  unsigned nrOutputTypes = plan.nrOutputTypes();
  std::vector<OutputSection *> outputSections(nrOutputTypes, 0);

  LOG_DEBUG("starting doObservation");

  // first: send configuration to compute nodes so they know what to expect
  configureCNs();

  LOG_DEBUG_STR("nrOutputs = " << nrOutputTypes );

  // start output process threads
  for (unsigned output = 0; output < nrOutputTypes; output ++) {
    unsigned phase, psetIndex;
    std::vector<signed> list; // list of subbands or beams

    const unsigned nrPsets = itsParset.phaseThreePsets().size();

    LOG_DEBUG_STR("setting up output " << output );

    switch (plan.plan[output].distribution) {
      case ProcessingPlan::DIST_SUBBAND:
        phase = 2;
        psetIndex = itsParset.phaseTwoPsetIndex(myPsetNumber);

        for (unsigned sb = 0; sb < itsParset.nrSubbandsPerPset(); sb ++) {
          unsigned subbandNumber = psetIndex * itsParset.nrSubbandsPerPset() + sb;

          if (subbandNumber < itsParset.nrSubbands()) {
            list.push_back(subbandNumber);
          } else {
            list.push_back(-1);
          }
        }
        LOG_DEBUG_STR("output " << output << " will generate subbands " << list );

        break;

      case ProcessingPlan::DIST_BEAM:
        phase = 3;
        psetIndex = itsParset.phaseThreePsetIndex(myPsetNumber);

        for (unsigned beam = 0;  beam < itsParset.nrSubbandsPerPset(); beam ++) {
          unsigned beamNumber = psetIndex + nrPsets * beam;

          if (beamNumber < itsParset.nrPencilBeams()) {
            list.push_back(beamNumber);
          } else {
            list.push_back(-1);
          }
        }
        LOG_DEBUG_STR("output " << output << " will generate beams " << list );

        break;

      default:
        continue;
    }

    outputSections[output] = new OutputSection(itsParset, list, output, &createCNstream);
  }

  LOG_DEBUG("doObservation processing input");

  unsigned				    run;
  std::vector<BeamletBuffer<SAMPLE_TYPE> *> noInputs;
  BeamletBufferToComputeNode<SAMPLE_TYPE>   beamletBufferToComputeNode(itsCNstreams, itsHasPhaseOne ? static_cast<InputSection<SAMPLE_TYPE> *>(theInputSection)->itsBeamletBuffers : noInputs, myPsetNumber);

  beamletBufferToComputeNode.preprocess(&itsParset);
        
  for (run = 0; run < itsNrRuns && !isCancelled(); run ++) {
    for (unsigned output = 0; output < nrOutputTypes; output ++) {
      outputSections[output]->addIterations( 1 );
    }

    beamletBufferToComputeNode.process();
  }

  for (unsigned output = 0; output < nrOutputTypes; output ++) {
    outputSections[output]->noMoreIterations();
  }

  beamletBufferToComputeNode.postprocess();

  unconfigureCNs();

  LOG_DEBUG("doObservation done processing input");

  for (unsigned output = 0; output < nrOutputTypes; output ++)
    delete outputSections[output];

  if (itsHasPhaseOne)
    detachFromInputSection<SAMPLE_TYPE>();

  LOG_DEBUG("doObservation finished");
}


void Job::checkParset() const
{
  if (itsParset.nrCoresPerPset() > nrCNcoresInPset) {
    LOG_ERROR_STR("nrCoresPerPset (" << itsParset.nrCoresPerPset() << ") cannot exceed " << nrCNcoresInPset);
    exit(1);
  }
}


void Job::printInfo() const
{
  LOG_INFO_STR("JobID = " << itsJobID << ", ObsID = " << itsObservationID << ", " << (itsIsRunning ? "running" : "not running"));
}


} // namespace RTCP
} // namespace LOFAR

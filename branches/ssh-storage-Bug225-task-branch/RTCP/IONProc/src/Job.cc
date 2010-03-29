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

#include <algorithm>


namespace LOFAR {
namespace RTCP {

unsigned	   Job::nextJobID = 1;
InputSectionParent *Job::theInputSection;
Mutex		   Job::theInputSectionMutex;
unsigned	   Job::theInputSectionRefCount = 0;

unsigned	   Job::theNrJobsCreated;
Semaphore	   Job::theNrJobsFinished;

Mutex		   Job::theJobQueueMutex;
Condition	   Job::theReevaluateJobQueue;
std::vector<Job *> Job::theRunningJobs;


Job::Job(const char *parsetName)
:
  itsParset(parsetName),
  itsJobID(nextJobID ++) // no need to make thread safe
{
  checkParset();

  itsObservationID = itsParset.observationID();

  LOG_DEBUG_STR("Creating new observation, ObsID = " << itsParset.observationID());
  LOG_DEBUG_STR("ObsID = " << itsParset.observationID() << ", usedCoresInPset = " << itsParset.usedCoresInPset());

  itsNrRuns = static_cast<unsigned>(ceil((itsParset.stopTime() - itsParset.startTime()) / itsParset.CNintegrationTime()));
  LOG_DEBUG_STR("itsNrRuns = " << itsNrRuns);

  itsHasPhaseOne   = itsParset.phaseOnePsetIndex(myPsetNumber) >= 0;
  itsHasPhaseTwo   = itsParset.phaseTwoPsetIndex(myPsetNumber) >= 0;
  itsHasPhaseThree = itsParset.phaseThreePsetIndex(myPsetNumber) >= 0;

  itsStopTime  = TimeStamp(static_cast<int64>(itsParset.stopTime() * itsParset.sampleRate()), itsParset.clockSpeed());

  createCNstreams();
  createIONstreams();

  itsJobThread = new Thread(this, &Job::jobThread, 65536);

  ++ theNrJobsCreated;
}


Job::~Job()
{
  LOG_DEBUG("Job::~Job()");

  delete itsJobThread;
  deleteIONstreams();

  theNrJobsFinished.up();
}


void Job::waitUntilAllJobsAreFinished()
{
  theNrJobsFinished.down(theNrJobsCreated);
}


void Job::allocateResources()
{
  if (itsHasPhaseOne)
    attachToInputSection();

  switch (itsParset.nrBitsPerSample()) {
    case  4 : itsCNthread = new Thread(this, &Job::CNthread<i4complex>, 65536);
	      break;

    case  8 : itsCNthread = new Thread(this, &Job::CNthread<i8complex>, 65536);
	      break;

    case 16 : itsCNthread = new Thread(this, &Job::CNthread<i16complex>, 65536);
	      break;
  }
}


void Job::deallocateResources()
{
  // WAIT for thread
  delete itsCNthread;
  LOG_DEBUG("CNthread joined");

  // STOP inputSection
  if (itsHasPhaseOne)
    detachFromInputSection();
}


void Job::barrier()
{
  char byte = 0;

  if (myPsetNumber == 0) {
    for (unsigned ion = 1; ion < nrPsets; ion ++)
      itsIONstreams[ion]->read(&byte, sizeof byte);

    for (unsigned ion = 1; ion < nrPsets; ion ++)
      itsIONstreams[ion]->write(&byte, sizeof byte);
  } else {
    itsIONstreams[0]->write(&byte, sizeof byte);
    itsIONstreams[0]->read(&byte, sizeof byte);
  }
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
    static_cast<void *>(0)
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

  const char *userName   = itsParset.getString("OLAP.Storage.userName").c_str();
  const char *sshKey     = (std::string("/globalhome/") + userName + "/.ssh/id_rsa").c_str();
  const char *executable = itsParset.getString("OLAP.Storage.msWriter").c_str();
  const char *parset     = itsParset.name().c_str();

  itsStoragePIDs.resize(itsStorageHostNames.size());

  for (unsigned rank = 0; rank < itsStorageHostNames.size(); rank ++)
    forkSSH(sshKey, userName, itsStorageHostNames[rank].c_str(), executable, boost::lexical_cast<std::string>(rank).c_str(), parset, itsStoragePIDs[rank]);
}



void Job::stopStorageProcesses()
{
  for (unsigned rank = 0; rank < itsStorageHostNames.size(); rank ++)
    joinSSH(itsStoragePIDs[rank], itsStorageHostNames[rank]);
}


void Job::jobThread()
{
  if (myPsetNumber == 0) {
    if (itsParset.realTime()) {
      // claim resources ten seconds before observation start
      WallClockTime wallClock;
      time_t     closeToStart = static_cast<time_t>(itsParset.startTime()) - 10;
      char       buf[26];
      ctime_r(&closeToStart, buf);
      buf[24] = '\0';
      
      LOG_DEBUG_STR("waiting for job " << itsObservationID << " to start: sleeping until " << buf);
      wallClock.waitUntil(closeToStart);
    }

    theJobQueueMutex.lock();

    retry:
      for (std::vector<Job *>::iterator job = theRunningJobs.begin(); job != theRunningJobs.end(); job ++)
	if ((*job)->itsParset.overlappingResources(itsParset) || !(*job)->itsParset.compatibleInputSection(itsParset)) {
	  LOG_WARN_STR("ObsID = " << itsObservationID << ": postponed due to resource conflicts");
	  theReevaluateJobQueue.wait(theJobQueueMutex);
	  goto retry;
	}

    theRunningJobs.push_back(this);

    theJobQueueMutex.unlock();

    if (itsParset.hasStorage())
      startStorageProcesses();
  }

  barrier();

  if (itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
    LOG_DEBUG_STR("claiming resources for observation " << itsObservationID);
    allocateResources();

    // do observation

    deallocateResources();
    LOG_DEBUG_STR("resources of job " << itsObservationID << " deallocated");
  }

  barrier();

  if (myPsetNumber == 0) {    
    if (itsParset.hasStorage())
      stopStorageProcesses();

    theJobQueueMutex.lock();
    theRunningJobs.erase(find(theRunningJobs.begin(), theRunningJobs.end(), this));
    theReevaluateJobQueue.broadcast();
    theJobQueueMutex.unlock();
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


void Job::createIONstreams()
{
  if (myPsetNumber == 0) {
    itsIONstreams.resize(nrPsets);

    for (unsigned ion = 1; ion < nrPsets; ion ++)
      itsIONstreams[ion] = new MultiplexedStream(*allIONstreamMultiplexers[ion], itsJobID);
  } else {
    itsIONstreams.push_back(new MultiplexedStream(*allIONstreamMultiplexers[0], itsJobID));
  }
}


void Job::deleteIONstreams()
{
  if (myPsetNumber == 0) {
    for (unsigned ion = 1; ion < nrPsets; ion ++)
      delete itsIONstreams[ion];
  } else {
    delete itsIONstreams[0];
  }

  itsIONstreams.clear();
}


void Job::attachToInputSection()
{
  theInputSectionMutex.lock();

  if (theInputSectionRefCount ++ == 0)
    switch (itsParset.nrBitsPerSample()) {
      case  4 : theInputSection = new InputSection<i4complex>(itsParset, myPsetNumber);
		break;

      case  8 : theInputSection = new InputSection<i8complex>(itsParset, myPsetNumber);
		break;

      case 16 : theInputSection = new InputSection<i16complex>(itsParset, myPsetNumber);
		break;
    }

  theInputSectionMutex.unlock();
}


void Job::detachFromInputSection()
{
  theInputSectionMutex.lock();

  if (-- theInputSectionRefCount == 0)
    delete theInputSection;

  theInputSectionMutex.unlock();
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


template <typename SAMPLE_TYPE> void Job::CNthread()
{
  CN_Configuration configuration(itsParset);
  CN_ProcessingPlan<> plan(configuration,itsHasPhaseOne,itsHasPhaseTwo,itsHasPhaseThree);
  plan.removeNonOutputs();
  unsigned nrOutputTypes = plan.nrOutputTypes();
  std::vector<OutputSection *> outputSections( nrOutputTypes, 0 );

  LOG_DEBUG("starting CNthread");

  // first: send configuration to compute nodes so they know what to expect
  configureCNs();

  // start output process threads
  for (unsigned output = 0; output < nrOutputTypes; output++) {
    unsigned phase, psetIndex, maxlistsize;
    std::vector<unsigned> list; // list of subbands or beams

    switch( plan.plan[output].distribution ) {
      case ProcessingPlan::DIST_SUBBAND:
        phase = 2;
        psetIndex = itsParset.phaseTwoPsetIndex( myPsetNumber );
        maxlistsize = itsParset.nrSubbandsPerPset();

        for (unsigned sb = 0; sb < itsParset.nrSubbandsPerPset(); sb++) {
          unsigned subbandNumber = psetIndex * itsParset.nrSubbandsPerPset() + sb;

          if (subbandNumber < itsParset.nrSubbands()) {
            list.push_back( subbandNumber );
          }
        }
        break;

      case ProcessingPlan::DIST_BEAM:
        phase = 3;
        psetIndex = itsParset.phaseThreePsetIndex( myPsetNumber );
        maxlistsize = itsParset.nrBeamsPerPset();

        for (unsigned beam = 0;  beam < itsParset.nrBeamsPerPset(); beam++) {
          unsigned beamNumber = psetIndex * itsParset.nrBeamsPerPset() + beam;

          if (beamNumber < itsParset.nrBeams()) {
            list.push_back( beamNumber );
          }
        }
        break;

      default:
        continue;
    }

    outputSections[output] = new OutputSection(&itsParset, itsNrRuns, list, maxlistsize, output, &createCNstream);
  }

  // forward input, if any
  LOG_DEBUG("CNthread processing input");

  unsigned run;
  std::vector<BeamletBuffer<SAMPLE_TYPE> *> noInputs;
  BeamletBufferToComputeNode<SAMPLE_TYPE>   beamletBufferToComputeNode(itsCNstreams, itsHasPhaseOne ? static_cast<InputSection<SAMPLE_TYPE> *>(theInputSection)->itsBeamletBuffers : noInputs, myPsetNumber);

  beamletBufferToComputeNode.preprocess(&itsParset);
        
  for (run = 0; run < itsNrRuns; run ++)
    beamletBufferToComputeNode.process();

  beamletBufferToComputeNode.postprocess();

  unconfigureCNs();

  LOG_DEBUG("CNthread done processing input");

  // wait for output process threads to finish 
  for (unsigned output = 0; output < nrOutputTypes; output ++) {
    delete outputSections[output];
  }

  LOG_DEBUG("CNthread finished");
}


void Job::checkParset() const
{
  if (itsParset.nrCoresPerPset() > nrCNcoresInPset) {
    LOG_ERROR_STR("nrCoresPerPset (" << itsParset.nrCoresPerPset() << ") cannot exceed " << nrCNcoresInPset);
    exit(1);
  }
}


} // namespace RTCP
} // namespace LOFAR

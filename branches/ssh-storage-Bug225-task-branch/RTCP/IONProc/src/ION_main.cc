//#  ION_main.cc:
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
//#  $Id$

#include <lofar_config.h>

#include <BeamletBufferToComputeNode.h>
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_Mapping.h>
#include <Interface/Exceptions.h>
#include <Interface/Mutex.h>
#include <Interface/Parset.h>
#include <Interface/PrintVector.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/Thread.h>
#include <ION_Allocator.h>
#include <InputSection.h>
#include <OutputSection.h>
#include <PLC/ACCmain.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Stream/SystemCallException.h>
#include <StreamMultiplexer.h>
#include <IONProc/Package__Version.h>

#include <algorithm>
#include <boost/multi_array.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
#include <FCNP/fcnp_ion.h>
#include <FCNP_ServerStream.h>
#endif

#ifdef USE_VALGRIND
extern "C" {
#include <valgrind/valgrind.h>

/*
 * Valgrind wrappers to replace functions which use Double Hummer instructions,
 * since valgrind can't cope with them.
 *
 * Outside valgrind, these functions are not used.
 */

void *I_WRAP_SONAME_FNNAME_ZZ(Za,memcpy)( void *b, const void *a, size_t n) {
    char *s1 = static_cast<char*>(b);
    const char *s2 = static_cast<const char*>(a);
    for(; 0<n; --n)*s1++ = *s2++;
    return b;
}

void *I_WRAP_SONAME_FNNAME_ZZ(Za,memset)( void *dest, int val, size_t len) {
  unsigned char *ptr = static_cast<unsigned char*>(dest);
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

}
#endif

// if exceptions are not caught, an attempt is made to create a backtrace
// from the place where the exception is thrown.
//
// JD: if exceptions are caught, a backtrace is produced now as well,
//     plus the actual exception is printed. So that's preferable as a default.
#define CATCH_EXCEPTIONS


#if !defined CATCH_EXCEPTIONS

void terminate_with_backtrace()
{
  LOG_FATAL("terminate_with_backtrace()");

  void *buffer[100];
  int  nptrs     = backtrace(buffer, 100);
  char **strings = backtrace_symbols(buffer, nptrs);

  for (int i = 0; i < nptrs; i ++)
    LOG_FATAL_STR(i << ": " << strings[i]);

  free(strings);
  abort();
}

#endif


namespace LOFAR {
namespace RTCP {

std::string				myIPaddress;

static unsigned			 	myPsetNumber, nrPsets;
static boost::multi_array<char, 2>	ipAddresses;
static std::vector<Stream *>		allCNstreams, allIONstreams;
static std::vector<StreamMultiplexer *> allIONstreamMultiplexers;

static const unsigned	     nrCNcoresInPset = 64; // TODO: how to figure out the number of CN cores?

#if defined USE_VALGRIND || !defined HAVE_FCNP // FIXME
static const std::string     streamType = "TCP";
#else
static const std::string     streamType = "FCNP";
#endif

#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
static bool	   fcnp_inited;
#endif

static Stream *createCNstream(unsigned core, unsigned channel)
{
  // translate logical to physical core number
  core = CN_Mapping::mapCoreOnPset(core, myPsetNumber);

#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
  if (streamType == "FCNP")
    return new FCNP_ServerStream(core, channel);
  else
#endif
  if (streamType == "NULL")
    return new NullStream;
  else if (streamType == "TCP") {
    LOG_DEBUG_STR("new SocketStream(\"127.0.0.1\", 5000 + " << core << " + 1000 * " << channel << ", ::TCP, ::Server");
    Stream *str = new SocketStream("127.0.0.1", 5000 + core + 1000 * channel, SocketStream::TCP, SocketStream::Server);
    LOG_DEBUG_STR("new SocketStream() done");
    return str;
  }
  else
    THROW(IONProcException, "unknown Stream type between ION and CN");
}


static void createAllCNstreams()
{
#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
  if (streamType == "FCNP" && !fcnp_inited) {
    FCNP_ION::init(true);
    fcnp_inited = true;
  }
#endif

  allCNstreams.resize(nrCNcoresInPset);

  for (unsigned core = 0; core < nrCNcoresInPset; core ++)
    allCNstreams[core] = createCNstream(core, 0);
}


static void deleteAllCNstreams()
{
  for (unsigned core = 0; core < nrCNcoresInPset; core ++)
    delete allCNstreams[core];

#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
  if (fcnp_inited) {
    FCNP_ION::end();
    fcnp_inited = false;
  }
#endif
}


static void stopCNs()
{
  LOG_DEBUG_STR("stopping " << nrCNcoresInPset << " cores ...");

  CN_Command command(CN_Command::STOP);

  for (unsigned core = 0; core < nrCNcoresInPset; core ++)
    command.write(allCNstreams[core]);

  LOG_DEBUG_STR("stopping " << nrCNcoresInPset << " cores done");
}


static void createAllIONstreams()
{
  LOG_DEBUG_STR("create streams between I/O nodes");

  if (myPsetNumber == 0) {
    allIONstreams.resize(nrPsets);
    allIONstreamMultiplexers.resize(nrPsets);

    for (unsigned ion = 1; ion < nrPsets; ion ++) {
      allIONstreams[ion] = new SocketStream(ipAddresses[ion].origin(), 4001, SocketStream::TCP, SocketStream::Client);
      allIONstreamMultiplexers[ion] = new StreamMultiplexer(*allIONstreams[ion]);
    }
  } else {
    allIONstreams.push_back(new SocketStream(ipAddresses[myPsetNumber].origin(), 4001, SocketStream::TCP, SocketStream::Server));
    allIONstreamMultiplexers.push_back(new StreamMultiplexer(*allIONstreams[0]));
  }

  LOG_DEBUG_STR("create streams between I/O nodes done");
}


static void deleteAllIONstreams()
{
  if (myPsetNumber == 0) {
    for (unsigned ion = 1; ion < nrPsets; ion ++) {
      delete allIONstreamMultiplexers[ion];
      delete allIONstreams[ion];
    }
  } else {
    delete allIONstreamMultiplexers[0];
    delete allIONstreams[0];
  }

  allIONstreamMultiplexers.clear();
  allIONstreams.clear();
}


static void enableCoreDumps()
{
  struct rlimit rlimit;

  rlimit.rlim_cur = RLIM_INFINITY;
  rlimit.rlim_max = RLIM_INFINITY;

  if (setrlimit(RLIMIT_CORE, &rlimit) < 0)
    perror("warning: setrlimit on unlimited core size failed");

  if (system("echo /tmp/%e.core >/proc/sys/kernel/core_pattern") < 0)
    LOG_WARN("could not change /proc/sys/kernel/core_pattern");

  LOG_DEBUG("coredumps enabled");
}


static void ignoreSigPipe()
{
  if (signal(SIGPIPE, SIG_IGN) < 0)
    perror("warning: ignoring SIGPIPE failed");
}


#if defined FLAT_MEMORY

static void   *flatMemoryAddress = reinterpret_cast<void *>(0x50000000);
static size_t flatMemorySize     = 1536 * 1024 * 1024;

static void mmapFlatMemory()
  /* 

  mmap a fixed area of flat memory space to increase performance. 
  currently only 1.5 GiB can be allocated, we mmap() the maximum
  available amount

  */
{
  int fd = open("/dev/flatmem", O_RDONLY);

  if (fd < 0) { 
    perror("open(\"/dev/flatmem\", O_RDONLY)");
    exit(1);
  }
 
  if (mmap(flatMemoryAddress, flatMemorySize, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0) == MAP_FAILED) {
    perror("mmap flat memory");
    exit(1);
  } 

  close(fd);
  LOG_DEBUG_STR("mapped " << flatMemorySize << " bytes of fast memory at " << flatMemoryAddress);
}

static void unmapFlatMemory()
{
  if (munmap(flatMemoryAddress, flatMemorySize) < 0)
    perror("munmap flat memory");
}

#endif


class Job
{
  public:
    Job(const char *parsetName);
    ~Job();

    const Parset itsParset;

  private:
    void	checkParset() const;
    void	createCNstreams();
    void	configureCNs(), unconfigureCNs();

    void	createIONstreams(), deleteIONstreams();

    void				 jobThread();
    template <typename SAMPLE_TYPE> void CNthread();

    void	allocateResources();
    void	deallocateResources();

    void	attachToInputSection();
    void	detachFromInputSection();

    void	barrier();

    std::vector<Stream *>		itsCNstreams, itsIONstreams;
    unsigned				itsNrRuns;
    TimeStamp                           itsStopTime;
    Thread				*itsJobThread, *itsCNthread;
    bool				itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree;
    unsigned				itsJobID, itsObservationID;
    static unsigned			nextJobID;

    static void				*theInputSection;
    static Mutex			theInputSectionMutex;
    static unsigned			theInputSectionRefCount;
};


unsigned		  Job::nextJobID = 1;
void			  *Job::theInputSection;
Mutex			  Job::theInputSectionMutex;
unsigned		  Job::theInputSectionRefCount = 0;

static std::vector<Job *> runningJobs;
static unsigned		  nrJobsCreated;
static Semaphore	  nrJobsFinished;

static Mutex		  jobQueueMutex;
static Condition	  reevalutateJobQueue;


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

  itsStopTime  = TimeStamp(static_cast<int64>(itsParset.stopTime() * itsParset.sampleRate()));

  createCNstreams();
  createIONstreams();

  itsJobThread = new Thread(this, &Job::jobThread, 65536);

  ++ nrJobsCreated;
}


Job::~Job()
{
  LOG_DEBUG("Job::~Job()");

  delete itsJobThread;
  deleteIONstreams();

  nrJobsFinished.up();
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

    jobQueueMutex.lock();

    retry:
      for (std::vector<Job *>::iterator job = runningJobs.begin(); job != runningJobs.end(); job ++)
	if ((*job)->itsParset.overlappingResources(&itsParset)) {
	  reevalutateJobQueue.wait(jobQueueMutex);
	  goto retry;
	}

    runningJobs.push_back(this);

    jobQueueMutex.unlock();
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
    jobQueueMutex.lock();
    runningJobs.erase(find(runningJobs.begin(), runningJobs.end(), this));
    reevalutateJobQueue.broadcast();
    jobQueueMutex.unlock();
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
      case  4 : theInputSection = new InputSection<i4complex>(&itsParset, myPsetNumber);
		break;

      case  8 : theInputSection = new InputSection<i8complex>(&itsParset, myPsetNumber);
		break;

      case 16 : theInputSection = new InputSection<i16complex>(&itsParset, myPsetNumber);
		break;
    }

  theInputSectionMutex.unlock();
}


void Job::detachFromInputSection()
{
  theInputSectionMutex.lock();

  if (-- theInputSectionRefCount == 0)
    switch (itsParset.nrBitsPerSample()) {
      case  4 : delete static_cast<InputSection<i4complex> *>(theInputSection);
		break;

      case  8 : delete static_cast<InputSection<i8complex> *>(theInputSection);
		break;

      case 16 : delete static_cast<InputSection<i16complex> *>(theInputSection);
		break;
    }

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


static bool quit = false;


static void handleCommand(const std::string &command)
{
  LOG_DEBUG_STR("command \"" << command << "\" received");

  if (command == "quit") {
    quit = true;
  } else if (command.compare(0, 7, "parset ") == 0) {
    try {
      new Job(command.substr(7).c_str());
    } catch (APSException &) { // if file could not be found
    }
  } else if (myPsetNumber == 0) {
    LOG_ERROR_STR("command \"" << command << "\" not understood");
  }
}


static void commandMaster()
{
  std::vector<MultiplexedStream *> ionStreams(nrPsets);

  for (unsigned ion = 1; ion < nrPsets; ion ++)
    ionStreams[ion] = new MultiplexedStream(*allIONstreamMultiplexers[ion], 0);

  while (!quit) {
    SocketStream sk("0.0.0.0", 4000, SocketStream::TCP, SocketStream::Server);

    try {
      while (!quit) {
	std::string command;

	for (char ch; sk.read(&ch, 1), ch != '\n';) // TODO: do not do a syscall per char
	  command.push_back(ch);

	LOG_DEBUG_STR("read command: " << command);
	unsigned size = command.size() + 1;

	//MPI_Bcast(&size, sizeof size, MPI_INT, 0, MPI_COMM_WORLD);
	//MPI_Bcast(const_cast<char *>(command.c_str()), size, MPI_CHAR, 0, MPI_COMM_WORLD);
	for (unsigned ion = 1; ion < nrPsets; ion ++) {
	  ionStreams[ion]->write(&size, sizeof size);
	  ionStreams[ion]->write(command.c_str(), size);
	}

	handleCommand(command);
      }
    } catch (Stream::EndOfStreamException &) {
    }
  }

  for (unsigned ion = 1; ion < nrPsets; ion ++)
    delete ionStreams[ion];
}


static void commandSlave()
{
  MultiplexedStream streamFromMaster(*allIONstreamMultiplexers[0], 0);

  while (!quit) {
    unsigned size;

    //MPI_Bcast(&size, sizeof size, MPI_INT, 0, MPI_COMM_WORLD);
    streamFromMaster.read(&size, sizeof size);

    char *command = new char[size];
    //MPI_Bcast(command, size, MPI_CHAR, 0, MPI_COMM_WORLD);
    streamFromMaster.read(command, size);
    handleCommand(command);
    delete [] command;
  }
}


static void commandServer()
{
  if (myPsetNumber == 0)
    commandMaster();
  else
    commandSlave();
}


static void master_thread(int argc, char **argv)
{
#if !defined HAVE_PKVERSION
  std::string type = "brief";
  Version::show<IONProcVersion> (std::clog, "IONProc", type);
#endif  
  
  LOG_DEBUG("starting master_thread");

  enableCoreDumps();
  ignoreSigPipe();

#if defined CATCH_EXCEPTIONS
  try {
#endif

#if defined FLAT_MEMORY
    mmapFlatMemory();
#endif

    setenv("AIPSPATH", "/globalhome/lofarsystem/packages/root/bgp_ion/", 0);

    if (argc < 2) {
      LOG_ERROR("unexpected number of arguments");
      exit(1);
    }

    createAllCNstreams();
    createAllIONstreams();

#if 1
    if (myPsetNumber == 0)
      for (int arg = 1; arg < argc; arg ++)
	LOG_DEBUG_STR("manually insert " << argv[arg]);

    commandServer();
#else
    for (int arg = 1; arg < argc; arg ++) {
      LOG_DEBUG_STR("creating new Job for ParameterSet " << argv[arg]);
      new Job(argv[arg]);
    }
#endif

    nrJobsFinished.down(nrJobsCreated);
    stopCNs();

#if defined FLAT_MEMORY
    unmapFlatMemory();
#endif

    deleteAllIONstreams();
    deleteAllCNstreams();

#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    LOG_FATAL_STR("main thread caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("main thread caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("main thread caught non-std::exception: ");
  }
#endif

  LOG_DEBUG("master thread finishes");
}


} // namespace RTCP
} // namespace LOFAR


int main(int argc, char **argv)
{
  using namespace LOFAR;
  using namespace LOFAR::RTCP;

#if !defined CATCH_EXCEPTIONS
  std::set_terminate(terminate_with_backtrace);
#endif
  
#if defined HAVE_MPI
#if 1
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    std::cerr << "MPI_Init failed" << std::endl;
    exit(1);
  }
#else
  int provided;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  if (provided != MPI_THREAD_MULTIPLE) {
    std::cerr << "MPI does not provide MPI_THREAD_MULTIPLE" << std::endl;
    exit(1);
  }
#endif

  MPI_Comm_rank(MPI_COMM_WORLD, reinterpret_cast<int *>(&myPsetNumber));
  MPI_Comm_size(MPI_COMM_WORLD, reinterpret_cast<int *>(&nrPsets));
#endif

#if defined HAVE_MPI && defined HAVE_BGP_ION
  ParameterSet personality("/proc/personality.sh");
  unsigned realPsetNumber = personality.getUint32("BG_PSETNUM");

  if (myPsetNumber != realPsetNumber) {
    std::cerr << "myPsetNumber (" << myPsetNumber << ") != realPsetNumber (" << realPsetNumber << ')' << std::endl;
    exit(1);
  }

  ipAddresses.resize(boost::extents[nrPsets][16]);
  myIPaddress = personality.getString("BG_IP");
  strcpy(ipAddresses[myPsetNumber].origin(), myIPaddress.c_str());

  for (unsigned root = 0; root < nrPsets; root ++)
    if (MPI_Bcast(ipAddresses[root].origin(), sizeof(char [16]), MPI_CHAR, root, MPI_COMM_WORLD) != MPI_SUCCESS) {
      std::cerr << "MPI_Bcast failed" << std::endl;
      exit(1);
    }
#endif
  
  std::stringstream sysInfo;
  sysInfo << basename(argv[0]) << "@" << myPsetNumber;
 
#if defined HAVE_BGP
  INIT_BGP_LOGGER(sysInfo.str());
#endif

  master_thread(argc, argv);

#if defined HAVE_MPI
  MPI_Finalize();
#endif

  return 0;
}

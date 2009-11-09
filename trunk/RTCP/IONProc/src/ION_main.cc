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
#include <Interface/Thread.h>
#include <ION_Allocator.h>
#include <InputSection.h>
#include <OutputSection.h>
#include <PLC/ACCmain.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Stream/SystemCallException.h>
#if !defined HAVE_PKVERSION
#include <Package__Version.h>
#endif
#include <algorithm>
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

#if defined HAVE_FCNP && defined __PPC__
#include <FCNP/fcnp_ion.h>
#include <FCNP_ServerStream.h>
#endif

#ifdef HAVE_VALGRIND
extern "C" {
#include <valgrind.h>

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


using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::LFDebug;


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


static unsigned		     myPsetNumber;
static std::vector<Stream *> allCNstreams;
static const unsigned	     nrCNcoresInPset = 64; // TODO: how to figure out the number of CN cores?
static pthread_mutex_t	     allocationMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t	     reevaluate	     = PTHREAD_COND_INITIALIZER;


#if defined HAVE_FCNP && defined __PPC__
static bool	   fcnp_inited;
#endif



static void createAllCNstreams(const std::string &streamType)
{
#if defined HAVE_FCNP && defined __PPC__
  if (streamType == "FCNP") {
    FCNP_ION::init(true);
    fcnp_inited = true;
  }
#endif

  allCNstreams.resize(nrCNcoresInPset);

  for (unsigned core = 0; core < nrCNcoresInPset; core ++) {
#if defined HAVE_FCNP && defined __PPC__
    if (streamType == "FCNP")
      allCNstreams[core] = new FCNP_ServerStream(core);
    else
#endif

    if (streamType == "NULL")
      allCNstreams[core] = new NullStream;
    else if (streamType == "TCP")
      allCNstreams[core] = new SocketStream("127.0.0.1", 5000 + core, SocketStream::TCP, SocketStream::Server);
    else
      THROW(IONProcException, "unknown Stream type between ION and CN");
  }
}


static void deleteAllCNstreams()
{
  for (unsigned core = 0; core < nrCNcoresInPset; core ++)
    delete allCNstreams[core];

#if defined HAVE_FCNP && defined __PPC__
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

    void				 jobThread();
    template <typename SAMPLE_TYPE> void toCNthread();
    void				 fromCNthread();

    void	allocateResources();
    void	deallocateResources();

    void	attachToInputSection();
    void	detachFromInputSection();

    std::vector<Stream *>		itsCNstreams;
    unsigned				itsNrRuns;
    Thread				*itsJobThread, *itsToCNthread, *itsFromCNthread;
    bool				itsHasInputSection, itsHasOutputSection;
    unsigned				itsObservationID;

    static void				*theInputSection;
    static Mutex			theInputSectionMutex;
    static unsigned			theInputSectionRefCount;
};


void			  *Job::theInputSection;
Mutex			  Job::theInputSectionMutex;
unsigned		  Job::theInputSectionRefCount = 0;

static std::vector<Job *> jobs;


Job::Job(const char *parsetName)
:
  itsParset(parsetName)
{
  checkParset();

  itsObservationID = itsParset.observationID();

  LOG_DEBUG_STR("Creating new observation, ObsID = " << itsParset.observationID());
  LOG_DEBUG_STR("ObsID = " << itsParset.observationID() << ", usedCoresInPset = " << itsParset.usedCoresInPset());

  itsHasInputSection  = itsParset.inputPsetIndex(myPsetNumber) >= 0;
  itsHasOutputSection = itsParset.outputPsetIndex(myPsetNumber) >= 0;

  if (itsHasInputSection || itsHasOutputSection)
    itsJobThread = new Thread(this, &Job::jobThread, 65536);
}


Job::~Job()
{
  if (itsHasInputSection || itsHasOutputSection)
    delete itsJobThread;
}


void Job::allocateResources()
{
  createCNstreams();

  itsNrRuns = static_cast<unsigned>(ceil((itsParset.stopTime() - itsParset.startTime()) / itsParset.CNintegrationTime()));
  LOG_DEBUG_STR("itsNrRuns = " << itsNrRuns);

  pthread_mutex_lock(&allocationMutex);

  // see if there is a resource conflict with any preceding job
  for (std::vector<Job *>::iterator job = jobs.begin(); *job != this;)
    if ((*job)->itsParset.overlappingResources(&itsParset)) {
      pthread_cond_wait(&reevaluate, &allocationMutex);
      job = jobs.begin();
    } else {
      job ++;
    }

  pthread_mutex_unlock(&allocationMutex);

  if (itsHasInputSection)
    attachToInputSection();

  switch (itsParset.nrBitsPerSample()) {
    case  4 : itsToCNthread = new Thread(this, &Job::toCNthread<i4complex>, 65536);
	      break;

    case  8 : itsToCNthread = new Thread(this, &Job::toCNthread<i8complex>, 65536);
	      break;

    case 16 : itsToCNthread = new Thread(this, &Job::toCNthread<i16complex>, 65536);
	      break;
  }
      

  if (itsHasOutputSection)
    itsFromCNthread = new Thread(this, &Job::fromCNthread, 65536);
}


void Job::deallocateResources()
{
  delete itsToCNthread;
  LOG_DEBUG("lofar__fini: to_CN thread joined");

  if (itsHasInputSection)
    detachFromInputSection();

  if (itsHasOutputSection) {
    delete itsFromCNthread;
    LOG_DEBUG("lofar__fini: from_CN thread joined");
  }

  pthread_mutex_lock(&allocationMutex);
  jobs.erase(find(jobs.begin(), jobs.end(), this));
  pthread_cond_broadcast(&reevaluate);
  pthread_mutex_unlock(&allocationMutex);
}



void Job::jobThread()
{
  if (itsParset.realTime()) {
    // claim resources two seconds before observation start
    WallClockTime wallClock;
    time_t     closeToStart = itsParset.startTime() - 2;
    char       buf[26];
    ctime_r(&closeToStart, buf);
    buf[24] = '\0';
    
    LOG_DEBUG("waiting for job " << itsObservationID << " to start: sleeping until " << buf);
    wallClock.waitUntil(closeToStart);
  }

  LOG_DEBUG("claiming resources for observation " << itsObservationID);
  allocateResources();

  // do observation

  deallocateResources();
  LOG_DEBUG("resources of job " << itsObservationID << " deallocated");
  delete this;
}


void Job::createCNstreams()
{
  std::vector<unsigned> usedCoresInPset = itsParset.usedCoresInPset();

  itsCNstreams.resize(usedCoresInPset.size());

  for (unsigned core = 0; core < usedCoresInPset.size(); core ++)
    itsCNstreams[core] = allCNstreams[CN_Mapping::mapCoreOnPset(usedCoresInPset[core], myPsetNumber)];
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


template <typename SAMPLE_TYPE> void Job::toCNthread()
{
  LOG_DEBUG("starting to_CN thread");
  configureCNs();

  std::vector<BeamletBuffer<SAMPLE_TYPE> *> noInputs;
  BeamletBufferToComputeNode<SAMPLE_TYPE>   beamletBufferToComputeNode(itsCNstreams, itsHasInputSection ? static_cast<InputSection<SAMPLE_TYPE> *>(theInputSection)->itsBeamletBuffers : noInputs, myPsetNumber);

  beamletBufferToComputeNode.preprocess(&itsParset);
	
  for (unsigned run = 0; run < itsNrRuns; run ++)
    beamletBufferToComputeNode.process();

  beamletBufferToComputeNode.postprocess();

  unconfigureCNs();
  LOG_DEBUG("to_CN thread finished");
}


void Job::fromCNthread()
{
  LOG_DEBUG("starting from_CN thread");
  OutputSection outputSection(myPsetNumber, itsCNstreams);

  outputSection.preprocess(&itsParset);

  for (unsigned run = 0; run < itsNrRuns; run ++)
    outputSection.process();

  outputSection.postprocess();
  LOG_DEBUG("from_CN thread finished");
}


void Job::checkParset() const
{
  if (itsParset.nrCoresPerPset() > nrCNcoresInPset) {
    LOG_ERROR_STR("nrCoresPerPset (" << itsParset.nrCoresPerPset() << ") cannot exceed " << nrCNcoresInPset);
    exit(1);
  }
}


void master_thread(int argc, char **argv)
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

    setenv("AIPSPATH", "/cephome/romein/packages/casacore-0.3.0/stage/", 0); // FIXME

    if (argc < 2) {
      LOG_ERROR("unexpected number of arguments");
      exit(1);
    }

    //createAllCNstreams(parset.getTransportType("OLAP.OLAP_Conn.IONProc_CNProc"));
#if defined HAVE_VALGRIND // FIXME
    createAllCNstreams("TCP");
#else
    createAllCNstreams("FCNP");
#endif

    pthread_mutex_lock(&allocationMutex);

    for (int arg = 1; arg < argc; arg ++) {
      LOG_DEBUG_STR("creating new Job for ParameterSet " << argv[arg]);
      Job *job = new Job(argv[arg]);

      // insert into sorted jobs list
      std::vector<Job *>::iterator prev = jobs.begin();

      while (prev != jobs.end() && (*prev)->itsParset.precedes(&job->itsParset))
	++ prev;

      jobs.insert(prev, job);
    }

    while (jobs.size() > 0)
      pthread_cond_wait(&reevaluate, &allocationMutex);

    pthread_mutex_unlock(&allocationMutex);
    stopCNs();

#if defined FLAT_MEMORY
    unmapFlatMemory();
#endif

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


int main(int argc, char **argv)
{
#if !defined CATCH_EXCEPTIONS
  std::set_terminate(terminate_with_backtrace);
#endif
  
#if defined HAVE_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, reinterpret_cast<int *>(&myPsetNumber));
#endif

#if defined HAVE_MPI && defined HAVE_BGP_ION
  ParameterSet personality("/proc/personality.sh");
  unsigned realPsetNumber = personality.getUint32("BG_PSETNUM");

  if (myPsetNumber != realPsetNumber) {
    LOG_ERROR_STR("myPsetNumber (" << myPsetNumber << ") != realPsetNumber (" << realPsetNumber << ')');
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

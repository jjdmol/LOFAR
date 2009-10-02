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
#include <PLC/ACCmain.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_Mapping.h>
#include <Interface/Mutex.h>
#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
#include <ION_Allocator.h>
#include <InputSection.h>
#include <OutputSection.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Stream/SystemCallException.h>
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>
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
class JobParent;
static std::vector<JobParent *> jobs;


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


class JobParent // untemplated superclass of Job<SAMPLE_TYPE>
{
  public:
	    JobParent(const Parset *);
    virtual ~JobParent();

    const Parset *itsParset;
};


JobParent::JobParent(const Parset *parset)
:
  itsParset(parset)
{
}


JobParent::~JobParent()
{
}


template <typename SAMPLE_TYPE> class Job : public JobParent
{
  public:
    Job(const Parset *);
    virtual ~Job();

  private:
    void	createCNstreams();
    void	configureCNs(), unconfigureCNs();

    void	jobThread();
    static void *jobThreadStub(void *);

    void	toCNthread();
    static void	*toCNthreadStub(void *);

    void	fromCNthread();
    static void	*fromCNthreadStub(void *);

    void	allocateResources();
    void	deallocateResources();

    void	attachToInputSection();
    void	detachFromInputSection();

    std::vector<Stream *>		itsCNstreams;
    unsigned				itsNrRuns;
    pthread_t				itsJobID, itsToCNthreadID, itsFromCNthreadID;
    bool				itsHasInputSection, itsHasOutputSection;
    unsigned				itsObservationID;

    static InputSection<SAMPLE_TYPE>	*theInputSection;
    static Mutex			theInputSectionMutex;
    static unsigned			theInputSectionRefCount;
};


template <typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE> *Job<SAMPLE_TYPE>::theInputSection;
template <typename SAMPLE_TYPE> Mutex			   Job<SAMPLE_TYPE>::theInputSectionMutex;
template <typename SAMPLE_TYPE> unsigned		   Job<SAMPLE_TYPE>::theInputSectionRefCount = 0;


template <typename SAMPLE_TYPE> Job<SAMPLE_TYPE>::Job(const Parset *parset)
:
  JobParent(parset)
{
  itsObservationID = parset->observationID();

  LOG_DEBUG_STR("Creating new observation, ObsID = " << parset->observationID());
  LOG_DEBUG_STR("ObsID = " << parset->observationID() << ", parset = " << (void *) parset << ", usedCoresInPset = " << parset->usedCoresInPset());

  itsHasInputSection  = parset->inputPsetIndex(myPsetNumber) >= 0;
  itsHasOutputSection = parset->outputPsetIndex(myPsetNumber) >= 0;

  if (itsHasInputSection || itsHasOutputSection) {
    if (pthread_create(&itsJobID, 0, jobThreadStub, static_cast<void *>(this)) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }
}


template <typename SAMPLE_TYPE> Job<SAMPLE_TYPE>::~Job()
{
  if (itsHasInputSection || itsHasOutputSection) {
#if 0
    if (pthread_join(itsJobID, 0) != 0) {
      perror("pthread join");
      exit(1);
    }
#else
    if (pthread_detach(pthread_self()) != 0) {
      perror("pthread detach");
      exit(1);
    }
#endif
  }

  delete itsParset; // FIXME: not here
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::allocateResources()
{
  createCNstreams();

  itsNrRuns = static_cast<unsigned>(ceil((itsParset->stopTime() - itsParset->startTime()) / itsParset->CNintegrationTime()));
  LOG_DEBUG_STR("itsNrRuns = " << itsNrRuns);

  pthread_mutex_lock(&allocationMutex);

  // see if there is a resource conflict with any preceding job
  for (std::vector<JobParent *>::iterator job = jobs.begin(); *job != this;)
    if ((*job)->itsParset->overlappingResources(itsParset)) {
      pthread_cond_wait(&reevaluate, &allocationMutex);
      job = jobs.begin();
    } else {
      job ++;
    }

  pthread_mutex_unlock(&allocationMutex);

  if (itsHasInputSection)
    attachToInputSection();

  pthread_attr_t attr;

  if (pthread_attr_init(&attr) != 0)
    throw SystemCallException("pthread_attr_init", errno, THROW_ARGS);

  if (pthread_attr_setstacksize(&attr, 65536) != 0)
    throw SystemCallException("pthread_attr_setstacksize", errno, THROW_ARGS);

  if (pthread_create(&itsToCNthreadID, &attr, toCNthreadStub, static_cast<void *>(this)) != 0)
    throw SystemCallException("pthread_create toCN thread", errno, THROW_ARGS);

  if (itsHasOutputSection && pthread_create(&itsFromCNthreadID, &attr, fromCNthreadStub, static_cast<void *>(this)) != 0)
    throw SystemCallException("pthread_create fromCN thread", errno, THROW_ARGS);

  if (pthread_attr_destroy(&attr) != 0)
    throw SystemCallException("pthread_attr_destroy", errno, THROW_ARGS);
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::deallocateResources()
{
  if (pthread_join(itsToCNthreadID, 0) != 0) {
    perror("pthread join");
    exit(1);
  }

  LOG_DEBUG("lofar__fini: to_CN section joined");

  if (itsHasInputSection)
    detachFromInputSection();

  if (itsHasOutputSection) {
    if (pthread_join(itsFromCNthreadID, 0) != 0) {
      perror("pthread join");
      exit(1);
    }

    LOG_DEBUG("lofar__fini: output section joined");
  }

  pthread_mutex_lock(&allocationMutex);
  jobs.erase(find(jobs.begin(), jobs.end(), this));
  pthread_cond_broadcast(&reevaluate);
  pthread_mutex_unlock(&allocationMutex);
}



template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::jobThread()
{
  if (itsParset->realTime()) {
    // claim resources two seconds before observation start
    WallClockTime wallClock;
    TimeStamp     closeToStart(static_cast<int64>((itsParset->startTime() - 2) * itsParset->sampleRate()));
    
    wallClock.waitUntil(closeToStart);
  }

  LOG_DEBUG("claiming resources for observation " << itsObservationID);
  allocateResources();

  // do observation

  deallocateResources();
  LOG_DEBUG("resources of job " << itsObservationID << " deallocated");
  delete this;
}


template <typename SAMPLE_TYPE> void *Job<SAMPLE_TYPE>::jobThreadStub(void *job)
{
#if defined CATCH_EXCEPTIONS
  try {
#endif
    static_cast<Job<SAMPLE_TYPE> *>(job)->jobThread();
#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    LOG_FATAL_STR("job thread caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("job thread caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("job thread caught non-std::exception: ");
  }
#endif

  return 0;
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::createCNstreams()
{
  std::vector<unsigned> usedCoresInPset = itsParset->usedCoresInPset();

  itsCNstreams.resize(usedCoresInPset.size());

  for (unsigned core = 0; core < usedCoresInPset.size(); core ++)
    itsCNstreams[core] = allCNstreams[CN_Mapping::mapCoreOnPset(usedCoresInPset[core], myPsetNumber)];
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::attachToInputSection()
{
  theInputSectionMutex.lock();

  // while (theInputSectionRefCount > 0 && !compatible(parset)
  //   theInputSectionRetry.wait(theInputSectionMutex)

  if (theInputSectionRefCount ++ == 0)
    theInputSection = new InputSection<SAMPLE_TYPE>(itsParset, myPsetNumber);

  theInputSectionMutex.unlock();
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::detachFromInputSection()
{
  theInputSectionMutex.lock();

  if (-- theInputSectionRefCount == 0) {
    delete theInputSection;
    //theInputSectionRetry.broadcast();
  }

  theInputSectionMutex.unlock();
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::configureCNs()
{
  CN_Command	   command(CN_Command::PREPROCESS);
  CN_Configuration configuration(*itsParset);
  LOG_DEBUG_STR("ION thinks that there are " << configuration.nrPencilBeams() << " pencil beams");
  
  LOG_DEBUG_STR("configuring cores " << itsParset->usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++) {
    LOG_DEBUG_STR("sending message 1 to core " << core << " ...");
    command.write(itsCNstreams[core]);
    LOG_DEBUG_STR("sending message 2 to core " << core << " ...");
    configuration.write(itsCNstreams[core]);
  }
  
  LOG_DEBUG_STR("configuring cores " << itsParset->usedCoresInPset() << " done");
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::unconfigureCNs()
{
  CN_Command command(CN_Command::POSTPROCESS);

  LOG_DEBUG_STR("unconfiguring cores " << itsParset->usedCoresInPset() << " ...");

  for (unsigned core = 0; core < itsCNstreams.size(); core ++)
    command.write(itsCNstreams[core]);

  LOG_DEBUG_STR("unconfiguring cores " << itsParset->usedCoresInPset() << " done");
}


template<typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::toCNthread()
{
  LOG_DEBUG_STR("Granted CN access to Job " << itsObservationID);

  configureCNs();

  std::vector<BeamletBuffer<SAMPLE_TYPE> *> noInputs;
  BeamletBufferToComputeNode<SAMPLE_TYPE>   beamletBufferToComputeNode(itsCNstreams, itsHasInputSection ? theInputSection->itsBeamletBuffers : noInputs, myPsetNumber);

  beamletBufferToComputeNode.preprocess(itsParset);
	
  for (unsigned run = 0; run < itsNrRuns; run ++)
    beamletBufferToComputeNode.process();

  beamletBufferToComputeNode.postprocess();

  unconfigureCNs();
}


template <typename SAMPLE_TYPE> void *Job<SAMPLE_TYPE>::toCNthreadStub(void *job)
{
  LOG_DEBUG("starting to_CN section");

#if defined CATCH_EXCEPTIONS
  try {
#endif
    static_cast<Job<SAMPLE_TYPE> *>(job)->toCNthread();
#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    LOG_FATAL_STR("to_CN section caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("to_CN section caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("to_CN section caught non-std::exception: ");
  }
#endif

  LOG_DEBUG("to_CN section finished");
  return 0;
}


template <typename SAMPLE_TYPE> void Job<SAMPLE_TYPE>::fromCNthread()
{
  OutputSection outputSection(myPsetNumber, itsCNstreams);

  outputSection.preprocess(itsParset);

  for (unsigned run = 0; run < itsNrRuns; run ++)
    outputSection.process();

  outputSection.postprocess();
}


template <typename SAMPLE_TYPE> void *Job<SAMPLE_TYPE>::fromCNthreadStub(void *job)
{
  LOG_DEBUG("starting output section");

#if defined CATCH_EXCEPTIONS
  try {
#endif
    static_cast<Job<SAMPLE_TYPE> *>(job)->fromCNthread();
#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    LOG_FATAL_STR("output section caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("output section caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("output section caught non-std::exception: ");
  }
#endif

  LOG_DEBUG("output section finished");
  return 0;
}


static void checkParset(const Parset *parset)
{
  if (parset->nrCoresPerPset() > nrCNcoresInPset) {
    LOG_ERROR_STR("nrCoresPerPset (" << parset->nrCoresPerPset() << ") cannot exceed " << nrCNcoresInPset);
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

    //createAllCNstreams(parset->getTransportType("OLAP.OLAP_Conn.IONProc_CNProc"));
#if defined HAVE_VALGRIND // FIXME
    createAllCNstreams("TCP");
#else
    createAllCNstreams("FCNP");
#endif

    pthread_mutex_lock(&allocationMutex);

    for (int arg = 1; arg < argc; arg ++) {
      LOG_DEBUG_STR("trying to use " << argv[arg] << " as ParameterSet");

      Parset *parset = new Parset(argv[arg]);

#if 0
      // OLAP.parset is deprecated, as everything will be in the parset given on the command line
      try {
	LOG_WARN("Reading OLAP.parset is deprecated");
	parset->adoptFile("OLAP.parset");
      } catch (APSException &ex) {
	LOG_WARN_STR("could not read OLAP.parset: " << ex);
      }
#endif

      checkParset(parset);

      JobParent *job;

      LOG_DEBUG("creating new Job");

      switch (parset->nrBitsPerSample()) {
	case  4 : job = new Job<i4complex>(parset); // parset deleted by ~Job()
		  break;

	case  8 : job = new Job<i8complex>(parset);
		  break;

	case 16 : job = new Job<i16complex>(parset);
		  break;

	default : delete parset;
		  THROW(IONProcException, "unsupported number of bits per sample");
      }

      LOG_DEBUG("creating new Job done");

      // insert into sorted jobs list
      std::vector<JobParent *>::iterator prev = jobs.begin();

      while (prev != jobs.end() && (*prev)->itsParset->precedes(job->itsParset))
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

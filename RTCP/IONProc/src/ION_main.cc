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

#include <CommandServer.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Mapping.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <ION_Allocator.h>
#include <Job.h>
#include <JobQueue.h>
#include <Stream/NamedPipeStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <StreamMultiplexer.h>
#include <IONProc/Package__Version.h>

#include <boost/multi_array.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

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

unsigned			   myPsetNumber, nrPsets;
static boost::multi_array<char, 2> ipAddresses;
std::vector<Stream *>		   allCNstreams, allIONstreams;
std::vector<StreamMultiplexer *>   allIONstreamMultiplexers;

#if defined HAVE_BGP
unsigned			   nrCNcoresInPset = 64; // TODO: how to figure out the number of CN cores?
#else
unsigned			   nrCNcoresInPset = 1;
#endif

static const char		   *cnStreamType;

#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
static bool			   fcnp_inited;
#endif

Stream *createCNstream(unsigned core, unsigned channel)
{
  // translate logical to physical core number
  core = CN_Mapping::mapCoreOnPset(core, myPsetNumber);

  if (strcmp(cnStreamType, "NULL") == 0) {
    return new NullStream;
  } else if (strcmp(cnStreamType, "TCP") == 0) {
    LOG_DEBUG_STR("new SocketStream(\"127.0.0.1\", 5000 + " << core << " + " << nrCNcoresInPset << " * " << channel << ", ::TCP, ::Server");
    Stream *str = new SocketStream("127.0.0.1", 5000 + core + nrCNcoresInPset * channel, SocketStream::TCP, SocketStream::Server);
    LOG_DEBUG_STR("new SocketStream() done");
    return str;
  } else if (strcmp(cnStreamType, "PIPE") == 0) {
    char pipe[128];
    sprintf(pipe, "/tmp/ion-cn-%u-%u-%u", myPsetNumber, core, channel);
    LOG_DEBUG_STR("new NamedPipeStream(\"" << pipe << "\")");
    return new NamedPipeStream(pipe);
#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
  } else if (strcmp(cnStreamType, "FCNP") == 0) {
    return new FCNP_ServerStream(core, channel);
#endif
  } else {
    throw IONProcException("unknown Stream type between ION and CN", THROW_ARGS);
  }
}


static void createAllCNstreams()
{
  const char *streamType = getenv("CN_STREAM_TYPE");

  if (streamType != 0)
    cnStreamType = streamType;
  else
#if !defined HAVE_BGP_ION
    cnStreamType = "NULL";
#elif defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
    cnStreamType = "FCNP";
#else
    cnStreamType = "TCP";
#endif

#if defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
  if (cnStreamType == "FCNP" && !fcnp_inited) {
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
      allIONstreams[ion] = new SocketStream(ipAddresses[ion].origin(), 4000 + ion, SocketStream::TCP, SocketStream::Client);
      allIONstreamMultiplexers[ion] = new StreamMultiplexer(*allIONstreams[ion]);
    }
  } else {
    allIONstreams.push_back(new SocketStream(ipAddresses[myPsetNumber].origin(), 4000 + myPsetNumber, SocketStream::TCP, SocketStream::Server));
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

#if defined HAVE_BGP
  if (system("echo /tmp/%e.core >/proc/sys/kernel/core_pattern") < 0)
    LOG_WARN("could not change /proc/sys/kernel/core_pattern");
#endif

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


static void master_thread()
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

    if (getenv("AIPSPATH") == 0)
      setenv("AIPSPATH", "/globalhome/lofarsystem/packages/root/bgp_ion/", 0);

    createAllCNstreams();
    createAllIONstreams();

    commandServer();

    jobQueue.waitUntilAllJobsAreFinished();
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

#if defined HAVE_MPI
  ipAddresses.resize(boost::extents[nrPsets][16]);

#if defined HAVE_BGP_ION
  ParameterSet personality("/proc/personality.sh");
  unsigned realPsetNumber = personality.getUint32("BG_PSETNUM");

  if (myPsetNumber != realPsetNumber) {
    std::cerr << "myPsetNumber (" << myPsetNumber << ") != realPsetNumber (" << realPsetNumber << ')' << std::endl;
    exit(1);
  }

  std::string myIPaddress = personality.getString("BG_IP");
  strcpy(ipAddresses[myPsetNumber].origin(), myIPaddress.c_str());
#else
  const char *uri = getenv("OMPI_MCA_orte_local_daemon_uri");

  if (uri == 0) {
    std::cerr << "\"OMPI_MCA_orte_local_daemon_uri\" not in environment" << std::endl;
    exit(1);
  }

  if (sscanf(uri, "%*u.%*u;tcp://%[0-9.]:%*u", ipAddresses[myPsetNumber].origin()) != 1) {
    std::cerr << "could not parse environment variable \"OMPI_MCA_orte_local_daemon_uri\"" << std::endl;
    exit(1);
  }
#endif

  for (unsigned root = 0; root < nrPsets; root ++)
    if (MPI_Bcast(ipAddresses[root].origin(), sizeof(char [16]), MPI_CHAR, root, MPI_COMM_WORLD) != MPI_SUCCESS) {
      std::cerr << "MPI_Bcast failed" << std::endl;
      exit(1);
    }
#endif
  
  std::stringstream sysInfo;
  sysInfo << basename(argv[0]) << "@" << myPsetNumber;
 
#if defined HAVE_BGP
  INIT_LOGGER_WITH_SYSINFO(sysInfo.str());
#elif defined HAVE_LOG4CPLUS
  lofarLoggerInitNode();
#elif defined HAVE_LOG4CXX
  Context::initialize();
  setLevel("Global", 8);
#else
  INIT_LOGGER_WITH_SYSINFO(sysInfo.str());
#endif

  master_thread();

#if defined HAVE_MPI
  MPI_Finalize();
#endif

  return 0;
}

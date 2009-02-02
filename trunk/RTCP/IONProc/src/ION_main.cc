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

#include <PLC/ACCmain.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_Mapping.h>
#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
#include <ION_Allocator.h>
#include <InputSection.h>
#include <OutputSection.h>
#include <IONProc/Lock.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
//#include <TH_ZoidServer.h>
#include <Package__Version.h>

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

#if defined HAVE_ZOID
extern "C" {
#include <lofar.h>
}
#endif

#if defined HAVE_FCNP && defined __PPC__
#include <FCNP/fcnp_ion.h>
#include <FCNP_ServerStream.h>
#endif

// if exceptions are not caught, an attempt is made to create a backtrace
// from the place where the exception is thrown.
#define CATCH_EXCEPTIONS


using namespace LOFAR;
using namespace LOFAR::RTCP;


#if !defined CATCH_EXCEPTIONS

void terminate_with_backtrace()
{
  cerr_logger("terminate_with_backtrace()");

  void *buffer[100];
  int  nptrs     = backtrace(buffer, 100);
  char **strings = backtrace_symbols(buffer, nptrs);

  for (int i = 0; i < nptrs; i ++)
    cerr_logger(i << ": " << strings[i]);

  free(strings);
  abort();
}

#endif


static char	   **global_argv;
static std::string blockID;
static unsigned    myPsetNumber, nrPsets, nrCoresPerPset, nrRuns;
static std::vector<Stream *> clientStreams;

#if defined HAVE_FCNP && defined __PPC__
static bool	   fcnp_inited;
#endif


static void checkParset(const Parset &parset)
{
  if (parset.nrPsets() > nrPsets) {
    cerr_logger("needs " << parset.nrPsets() << " psets; only " << nrPsets << " available");
    exit(1);
  }

#if 0
  if (parset.sizeBeamletAndSubbandList);
    cerr_logger("size of the beamletlist must be equal to the size of subbandlist.");

  parset.parseBeamletList();
#endif 
}


static void createClientStreams(unsigned nrClients, const std::string &streamType)
{
#if defined HAVE_FCNP && defined __PPC__
  if (streamType == "FCNP") {
    FCNP_ION::init(false);
    fcnp_inited = true;
  }
#endif

  clientStreams.resize(nrClients);

  for (unsigned core = 0; core < nrClients; core ++) {
#if defined HAVE_ZOID
    if (streamType == "ZOID")
      clientStreams[core] = new ZoidServerStream(core);
    else
#endif

#if defined HAVE_FCNP && defined __PPC__
    if (streamType == "FCNP")
      clientStreams[core] = new FCNP_ServerStream(core);
    else
#endif

    if (streamType == "NULL")
      clientStreams[core] = new NullStream;
    else if (streamType == "TCP")
      clientStreams[core] = new SocketStream("127.0.0.1", 5000 + core, SocketStream::TCP, SocketStream::Server);
    else
      THROW(IONProcException, "unknown Stream type between ION and CN");
  }
}


static void deleteClientStreams()
{
  for (unsigned core = 0; core < clientStreams.size(); core ++)
    delete clientStreams[core];

#if defined HAVE_FCNP && defined __PPC__
  if (fcnp_inited) {
    FCNP_ION::end();
    fcnp_inited = false;
  }
#endif
}


static void configureCNs(const Parset &parset)
{
  CN_Command	    command(CN_Command::PREPROCESS);
  CN_Configuration configuration;
  std::vector<Parset::StationRSPpair> inputs = parset.getStationNamesAndRSPboardNumbers(myPsetNumber);
  Matrix<double>    &phaseCentres      = configuration.phaseCentres();
  Matrix<double>    &manualPencilBeams = configuration.manualPencilBeams();
  
  configuration.nrStations()              = parset.nrStations();
  configuration.nrBitsPerSample()	  = parset.nrBitsPerSample();
  configuration.nrChannelsPerSubband()	  = parset.nrChannelsPerSubband();
  configuration.nrSamplesPerIntegration() = parset.CNintegrationSteps();
  configuration.nrSamplesPerStokesIntegration() = parset.stokesIntegrationSteps();
  configuration.nrSamplesToCNProc()       = parset.nrSamplesToCNProc();
  configuration.nrUsedCoresPerPset()      = parset.nrCoresPerPset();
  configuration.nrSubbandsPerPset()       = parset.nrSubbandsPerPset();
  configuration.delayCompensation()       = parset.delayCompensation();
  configuration.correctBandPass()	  = parset.correctBandPass();
  configuration.sampleRate()              = parset.sampleRate();
  configuration.inputPsets()              = parset.getUint32Vector("OLAP.CNProc.inputPsets");
  configuration.outputPsets()             = parset.getUint32Vector("OLAP.CNProc.outputPsets");
  configuration.tabList()                 = parset.getUint32Vector("OLAP.CNProc.tabList");
  configuration.refFreqs()                = parset.subbandToFrequencyMapping();
  configuration.nrPencilRings()           = parset.nrPencilRings();
  configuration.pencilRingSize()          = parset.pencilRingSize();
  configuration.nrManualPencilBeams()     = parset.nrManualPencilBeams();
  configuration.refPhaseCentre()          = parset.getRefPhaseCentres();
  configuration.mode()                    = CN_Mode(parset);
  configuration.outputIncoherentStokesI() = parset.outputIncoherentStokesI();

  phaseCentres.resize( inputs.size(), 3 );
  for( unsigned stat = 0; stat < inputs.size(); stat++ ) {
    std::vector<double> phaseCentre = parset.getPhaseCentresOf( inputs[stat].station );

    for( unsigned dim = 0; dim < 3; dim++ ) {
      phaseCentres[stat][dim] = phaseCentre[dim];
    }
  }

  manualPencilBeams.resize( parset.nrManualPencilBeams(), 2 );
  for( unsigned beam = 0; beam < parset.nrManualPencilBeams(); beam++ ) {
    std::vector<double> coordinates = parset.getManualPencilBeam( beam );

    for( unsigned dim = 0; dim < 2; dim++ ) {
      manualPencilBeams[beam][dim] = coordinates[dim];
    }
  }
 
  std::stringstream logStr;
  
  logStr << "configuring " << nrCoresPerPset << " cores ...";

  for (unsigned core = 0; core < nrCoresPerPset; core ++) {
    logStr << ' ' << core;
    command.write(clientStreams[core]);
    configuration.write(clientStreams[core]);
  }

  logStr << " done";
  
  clog_logger(logStr.str());
}


static void unconfigureCNs()
{
  std::stringstream logStr;
  logStr << "unconfiguring " << nrCoresPerPset << " cores ...";

  CN_Command command(CN_Command::POSTPROCESS);

  for (unsigned core = 0; core < nrCoresPerPset; core ++) {
    logStr << ' ' << core;
    command.write(clientStreams[core]);
  }

  logStr << " done";
  clog_logger(logStr.str());
}


static void stopCNs()
{
  std::stringstream logStr;
  logStr << "stopping " << nrCoresPerPset << " cores ... ";

  CN_Command command(CN_Command::STOP);

  for (unsigned core = 0; core < nrCoresPerPset; core ++)
    command.write(clientStreams[core]);

  logStr << " done";
  clog_logger(logStr.str());
}


template<typename SAMPLE_TYPE> static void inputTask(Parset *parset)
{
  InputSection<SAMPLE_TYPE> inputSection(clientStreams, myPsetNumber);

  inputSection.preprocess(parset);

  for (unsigned run = 0; run < nrRuns; run ++)
    inputSection.process();

  inputSection.postprocess();
}


static void *inputSection(void *parset)
{
  clog_logger("starting input section");

#if defined CATCH_EXCEPTIONS
  try {
#endif

    Parset *ps = static_cast<Parset *>(parset);

    switch (ps->nrBitsPerSample()) {
      case 4  : inputTask<i4complex>(ps);
		break;

      case 8  : inputTask<i8complex>(ps);
		break;

      case 16 : inputTask<i16complex>(ps);
		break;

      default : THROW(IONProcException, "unsupported number of bits per sample");
    }

#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    cerr_logger("input section caught Exception: " << ex);
  } catch (std::exception &ex) {
    cerr_logger("input section caught std::exception: " << ex.what());
  } catch (...) {
    cerr_logger("input section caught non-std::exception: ");
  }
#endif

  clog_logger("input section finished");
  return 0;
}


static void *outputSection(void *parset)
{
  clog_logger("starting output section");

#if defined CATCH_EXCEPTIONS
  try {
#endif

    OutputSection outputSection(myPsetNumber, clientStreams);

    outputSection.preprocess(static_cast<Parset *>(parset));

    for (unsigned run = 0; run < nrRuns; run ++)
      outputSection.process();

    outputSection.postprocess();

#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    cerr_logger("output section caught Exception: " << ex);
  } catch (std::exception &ex) {
    cerr_logger("output section caught std::exception: " << ex.what());
  } catch (...) {
    cerr_logger("output section caught non-std::exception: ");
  }
#endif

  clog_logger("output section finished");
  return 0;
}


static void enableCoreDumps()
{
  struct rlimit rlimit;

  rlimit.rlim_cur = RLIM_INFINITY;
  rlimit.rlim_max = RLIM_INFINITY;

  if (setrlimit(RLIMIT_CORE, &rlimit) < 0)
    perror("warning: setrlimit on unlimited core size failed");

  if (system("echo /tmp/%e.core >/proc/sys/kernel/core_pattern") < 0)
    cerr_logger("warning: could not change /proc/sys/kernel/core_pattern");

  clog_logger("coredumps enabled");
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
  clog_logger("mapped " << flatMemorySize << " bytes of fast memory at " << flatMemoryAddress);
}

static void unmapFlatMemory()
{
  if (munmap(flatMemoryAddress, flatMemorySize) < 0)
    perror("munmap flat memory");
}

#endif

void *master_thread(void *)
{
  std::string type = "brief";
  Version::show<IONProcVersion> (std::clog, "IONProc", type);
  
  clog_logger("starting master_thread");

  enableCoreDumps();
  ignoreSigPipe();

#if defined CATCH_EXCEPTIONS
  try {
#endif

#if defined FLAT_MEMORY
    mmapFlatMemory();
#endif

    setenv("AIPSPATH", "/cephome/romein/packages/casacore-0.3.0/stage/", 0); // FIXME

    pthread_t input_thread_id, output_thread_id;
    clog_logger("trying to use " << global_argv[1] << " as ParameterSet");
    ParameterSet parameterSet(global_argv[1]);
    Parset parset(&parameterSet);

    parset.adoptFile("OLAP.parset");
    checkParset(parset);

    nrRuns = static_cast<unsigned>(ceil((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime()));
    clog_logger("nrRuns = " << nrRuns);

    bool hasInputSection  = parset.inputPsetIndex(myPsetNumber) >= 0;
    bool hasOutputSection = parset.outputPsetIndex(myPsetNumber) >= 0;

#if !defined HAVE_ZOID
    nrCoresPerPset = parset.nrCoresPerPset();
    string streamType = parset.getTransportType("OLAP.OLAP_Conn.IONProc_CNProc");
    createClientStreams(nrCoresPerPset, streamType);
#endif

    configureCNs(parset);

    if (hasInputSection && pthread_create(&input_thread_id, 0, inputSection, static_cast<void *>(&parset)) != 0) {
      perror("pthread_create");
      exit(1);
    }

    if (hasOutputSection && pthread_create(&output_thread_id, 0, outputSection, static_cast<void *>(&parset)) != 0) {
      perror("pthread_create");
      exit(1);
    }

    if (!hasInputSection && hasOutputSection) {
      // quick hack to send PROCESS commands to CNs

      CN_Command command(CN_Command::PROCESS);
      unsigned	  core = 0;

      for (int count = nrRuns * parset.nrSubbandsPerPset(); -- count >= 0;) {
	command.write(clientStreams[CN_Mapping::mapCoreOnPset(core, myPsetNumber)]);

	if (++ core == nrCoresPerPset)
	  core = 0;
      }
    }

    if (hasInputSection) {
      if (pthread_join(input_thread_id, 0) != 0) {
	perror("pthread join");
	exit(1);
      }

      clog_logger("lofar__fini: input section joined");
    }

    unconfigureCNs();
    stopCNs();

    if (hasOutputSection) {
      if (pthread_join(output_thread_id, 0) != 0) {
	perror("pthread join");
	exit(1);
      }

      clog_logger("lofar__fini: output thread joined");
    }

#if defined FLAT_MEMORY
    unmapFlatMemory();
#endif

#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    cerr_logger("main thread caught Exception: " << ex);
  } catch (std::exception &ex) {
    cerr_logger("main thread caught std::exception: " << ex.what());
  } catch (...) {
    cerr_logger("main thread caught non-std::exception: ");
  }
#endif

  if (pthread_detach(pthread_self()) != 0) {
    cerr_logger("could not detach master thread");
  }

#if defined HAVE_ZOID
  if (global_argv != 0) {
    for (char **arg = &global_argv[0]; *arg != 0; arg ++)
      free(*arg);

    delete [] global_argv;
  }
#endif

  clog_logger("master thread finishes");
  return 0;
}


extern "C"
{
  void lofar__init(int);
  void lofar__fini(void);
}


inline static void redirect_output()
{
#if defined HAVE_ZOID
  int  fd;
  char file_name[64];
  
  sprintf(file_name, "run.IONProc.%s.%u", blockID.c_str(), myPsetNumber);

  if ((fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0 || dup2(fd, 1) < 0 || dup2(fd, 2) < 0)
      perror("redirecting stdout/stderr");
#endif
}


static void tryToGetPersonality()
{
  myPsetNumber = 0;
  nrPsets      = 1;

  try {
    ParameterSet personality("/proc/personality.sh");

#if defined HAVE_ZOID // compiler bug: exceptions cause crashes
    try {
      myPsetNumber = personality.getUint32("BGL_PSETNUM");
      nrPsets	   = personality.getUint32("BGL_NUMPSETS");
      blockID	   = personality.getString("BGL_BLOCKID");

      clog_logger(" myPsetNumber : " << myPsetNumber
                    << " nrPsets : " << nrPsets
		    << " blockID : " << blockID);
    } catch (std::exception& ex) {
      cerr_logger(ex.what());

    } catch (...) {
      cerr_logger("Caught unknown exception");
      
    }
#else
    try {
      myPsetNumber = personality.getUint32("BG_PSETNUM");
      nrPsets	   = personality.getUint32("BG_NUMPSETS");
      blockID	   = personality.getString("BG_BLOCKID");
    } catch (...) {
    }
#endif
  } catch (...) {
  }
}


#if defined HAVE_ZOID

void lofar__init(int nrComputeCores)
{
  nrCoresPerPset = nrComputeCores;
  tryToGetPersonality();
  redirect_output();
  clog_logger("begin of lofar__init");

  createClientStreams(nrComputeCores, "ZOID");

  global_argv = 0;
}



void lofar_init(char   **argv /* in:arr2d:size=+1 */,
		size_t * /*lengths*/ /* in:arr:size=+1 */,
		int    argc /* in:obj */)

{
  clog_logger("ok, begin of lofar_init(..., ..., " << argc << ")");
  

  for (int i = 0; i < argc; i ++)
    clog_logger("lofar_init(): arg = " << argv[i]);

  // copy argv
  global_argv = new char * [argc + 1];

  for (int arg = 0; arg < argc; arg ++)
    global_argv[arg] = strdup(argv[arg]);

  global_argv[argc] = 0; // terminating zero pointer

  if (argc == 3) {
    cerr_logger("WARNING: specifying nrRuns is deprecated --- ignored");
  } else if (argc != 2) {
    cerr_logger("unexpected number of arguments");
    exit(1);
  }

  pthread_t master_thread_id;

  if (pthread_create(&master_thread_id, 0, master_thread, 0) != 0) {
    perror("pthread_create");
    exit(1);
  }
}


void lofar__fini(void)
{
  clog_logger("begin of lofar__fini");
  deleteClientStreams();
  clog_logger("end of lofar__fini");
}

#else

int main(int argc, char **argv)
{
#if !defined CATCH_EXCEPTIONS
  std::set_terminate(terminate_with_backtrace);
#endif

  global_argv = argv;
  
  pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
  Lock::setMutex(mutex);
  
  
  if (argc == 3) {
    cerr_logger("WARNING: specifying nrRuns is deprecated --- ignored");
  } else if (argc != 2) {
    cerr_logger("unexpected number of arguments");
    exit(1);
  }

  tryToGetPersonality();
  master_thread(0);
  deleteClientStreams();
  return 0;
}

#endif

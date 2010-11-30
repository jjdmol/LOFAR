//#  Storage_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h> 
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>
#include <Common/LofarLocators.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <Thread/Thread.h>
#include <Storage/SubbandWriter.h>
#include <Storage/Package__Version.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <stdexcept>
#include <cstdio>
#include <cmath>
#include <cstdlib>

#include <map>
#include <vector>
#include <string>

#include <boost/format.hpp>
using boost::format;


using namespace LOFAR;
using namespace LOFAR::RTCP;


#if 0
class Job
{
  public:
    Job(const char *parsetName, int rank, int size);

    void	  mainLoop();

  private:
    const Parset  itsParset;
    SubbandWriter itsSubbandWriter;
};


static std::vector<Job *> jobs;

Job::Job(const char *parsetName, int rank, int size)
:
  itsParset(parsetName),
  itsSubbandWriter(&itsParset, rank, size)
{
  LOG_INFO_STR("Created job with parset " << parsetName);
}


static void child(int argc, char *argv[], int rank, int size)
{
  try {
#if !defined HAVE_PKVERSION
    std::string type = "brief";
    Version::show<StorageVersion> (std::cout, "Storage", type);  
#endif    
    
#if 0
    // OLAP.parset is depricated, as everything is now in the parset given on the command line
    try {
      parset.adoptFile("OLAP.parset");
    } catch( APSException &ex ) {
      LOG_WARN_STR("could not read OLAP.parset: " << ex);
    }
#endif

  for (int i = 1; i < argc; i ++)
    jobs.push_back(new Job(argv[i], rank, size));

  for (unsigned i = 0; i < jobs.size(); i ++) // TODO: let job delete itself
    delete jobs[i];

  } catch (Exception &ex) {
    LOG_FATAL_STR("caught Exception: " << ex);
    exit(1);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("caught std::exception: " << ex.what());
    exit(1);
  } catch (...) {
    LOG_FATAL("caught unknown exception");
    exit(1);
  }
}
#endif


class ExitOnClosedStdin {
public:
  ExitOnClosedStdin(): observationDone(false) {}

  bool observationDone;

  void mainLoop();
};

void ExitOnClosedStdin::mainLoop()
{
  // an empty read on stdin means the SSH connection closed, which indicates that we should abort

  while (!observationDone) {
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(0,&fds);

    struct timeval timeval;

    timeval.tv_sec = 1;
    timeval.tv_usec = 0;

    switch (select(1, &fds, 0, 0, &timeval)) {
      case -1 : throw SystemCallException("select", errno, THROW_ARGS);
      case  0 : continue;
    }

    char buf[1024];
    ssize_t numbytes;
    numbytes = ::read(0, buf, sizeof buf);

    if( numbytes == 0 ) {
      LOG_FATAL("Lost stdin -- aborting"); // this most likely won't arrive, since stdout/stderr are probably closed as well
      exit(1);
    }
  }
}

int main(int argc, char *argv[])
{
  string logPrefix = "[obs unknown] ";

#if defined HAVE_LOG4CPLUS
  INIT_LOGGER( "Storage" );
#elif defined HAVE_LOG4CXX
  #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
  Context::initialize();
  setLevel("Global",8);
#else
  INIT_LOGGER_WITH_SYSINFO(str(format("Storage@%02d") % (argc > 1 ? atoi(argv[1]) : -1)));
#endif

#if 0
#if defined HAVE_MPI
  int rank;
  int size;

#if 0
  int thread_model_provided;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &thread_model_provided);
  if (thread_model_provided != MPI_THREAD_SERIALIZED) {
    LOG_WARN_STR("Failed to set MPI thread model to MPI_THREAD_SERIALIZED");
  }
#else
  MPI_Init(&argc, &argv);
#endif

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#else
  int rank = 0;
  int size = 1;
#endif

  // Do the real work in a forked process.  This allows us to catch a crashing
  // process, and fool mpirun that the process terminated normally.  If mpirun
  // would have seen a crashing member, it would kill all other storage
  // writers.  It is better to let the other members continue, to minimize the
  // amount of data loss during an observation.

  int status;

  switch (fork()) {
    case -1 : perror("fork");
	      break;

    case 0  : child(argc, argv, rank, size);
	      _exit(0);

    default : if (wait(&status) < 0)
		perror("wait");
	      else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		LOG_ERROR_STR("child returned exit status " << WEXITSTATUS(status));
		
	      else if (WIFSIGNALED(status))
	        LOG_ERROR_STR("child killed by signal " << WTERMSIG(status));

	      break;
  }

#if defined HAVE_MPI
  MPI_Finalize();
#endif

#else
  try {
    if (argc != 4)
      throw StorageException(str(format("usage: %s rank parset is_bigendian") % argv[0]), THROW_ARGS);

    char stdoutbuf[1024], stderrbuf[1024];
    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stdoutbuf, _IOLBF, sizeof stderrbuf);

    LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2] << ' ' << argv[3]);

    unsigned			 myRank = boost::lexical_cast<unsigned>(argv[1]);
    Parset			 parset(argv[2]);
    bool			 isBigEndian = boost::lexical_cast<bool>(argv[3]);

    CN_Configuration             configuration(parset);
    CN_ProcessingPlan<>          plan(configuration);
    std::vector<SubbandWriter *> subbandWriters;

    plan.removeNonOutputs();

    logPrefix = str(format("[obs %u] ") % parset.observationID());

    vector<string> hosts = parset.getStringVector("OLAP.Storage.hosts");
    ASSERT( myRank < hosts.size() );
    const string myhost = hosts[myRank];
    unsigned nrparts = parset.nrPartsPerStokes();
    unsigned  nrbeams = parset.flysEye() ? parset.nrMergedStations() : parset.nrPencilBeams();

    // start all writers
    for (unsigned output = 0; output < plan.nrOutputTypes(); output ++) {
      ProcessingPlan::planlet &p = plan.plan[output];
      string mask = parset.fileNameMask( p.info.storageParsetPrefix );

      switch (p.info.distribution) {
        case ProcessingPlan::DIST_SUBBAND:
          for (unsigned s = 0; s < parset.nrSubbands(); s++) {
            string filename = parset.constructSubbandFilename( mask, s );
            string host = parset.targetHost( p.info.storageParsetPrefix, filename );

            if (host == myhost) {
              string dir  = parset.targetDirectory( p.info.storageParsetPrefix, filename );
              unsigned index = s;

              subbandWriters.push_back(new SubbandWriter(parset, p, index, host, dir, filename, isBigEndian));
            }
          }
          
          break;

        case ProcessingPlan::DIST_BEAM:

          for (unsigned b = 0; b < nrbeams; b++) {
            unsigned nrstokes = p.info.nrStokes;

            for (unsigned s = 0; s < nrstokes; s++) {
              for (unsigned q = 0; q < nrparts; q++) {
                string filename = parset.constructBeamFormedFilename( mask, b, s, q );
                string host = parset.targetHost( p.info.storageParsetPrefix, filename );

                if (host == myhost) {
                  string dir  = parset.targetDirectory( p.info.storageParsetPrefix, filename );
                  unsigned index = (b * nrstokes + s ) * nrparts + q;

	          subbandWriters.push_back(new SubbandWriter(parset, p, index, host, dir, filename, isBigEndian));
                }
              }
            }
          }

          break;

        case ProcessingPlan::DIST_UNKNOWN:
        case ProcessingPlan::DIST_STATION:
          continue;
      }
    }

    ExitOnClosedStdin stdinWatcher;
    Thread stdinWatcherThread(&stdinWatcher,&ExitOnClosedStdin::mainLoop,logPrefix + "[stdinWatcherThread] ",65535);

    for (unsigned writer = 0; writer < subbandWriters.size(); writer ++)
      delete subbandWriters[writer];

    stdinWatcher.observationDone = true;
  } catch (Exception &ex) {
    LOG_FATAL_STR(logPrefix << "Caught Exception: " << ex);
    exit(1);
  } catch (std::exception &ex) {
    LOG_FATAL_STR(logPrefix << "Caught std::exception: " << ex.what());
    exit(1);
  } catch (...) {
    LOG_FATAL_STR(logPrefix << "Caught non-std::exception: ");
    exit(1);
  }
#endif

  LOG_INFO_STR(logPrefix << "Program end");
  return 0;
}

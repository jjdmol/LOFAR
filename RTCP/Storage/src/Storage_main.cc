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
#include <Interface/CN_ProcessingPlan.h>
#include <Thread/Thread.h>
#include <Storage/SubbandWriter.h>
#include <Storage/Package__Version.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>

#include <stdexcept>
#include <cstdio>


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
      LOG_FATAL("lost stdin -- aborting"); // this most likely won't arrive, since stdout/stderr are probably closed as well
      exit(1);
    }
  }
}

int main(int argc, char *argv[])
{
#if defined HAVE_LOG4CPLUS
  using namespace log4cplus;
  using namespace log4cplus::helpers;

  lofarLoggerInitNode();
  helpers::Properties traceProp;
  traceProp.setProperty("log4cplus.rootLogger", "DEBUG, STDOUT");
  traceProp.setProperty("log4cplus.logger.TRC", "DEBUG");
  traceProp.setProperty("log4cplus.appender.STDOUT", "log4cplus::ConsoleAppender");
  traceProp.setProperty("log4cplus.appender.STDOUT.layout", "log4cplus::PatternLayout");
  traceProp.setProperty("log4cplus.appender.STDOUT.layout.ConversionPattern", "%-5p|%x|%m%n");
  
  PropertyConfigurator(traceProp).configure();
#elif defined HAVE_LOG4CXX
  Context::initialize();
  setLevel("Global",8);
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
    if (argc != 3)
      throw StorageException(std::string("usage: ") + argv[0] + " rank parset", THROW_ARGS);

    char stdoutbuf[1024], stderrbuf[1024];
    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stdoutbuf, _IOLBF, sizeof stderrbuf);

    LOG_INFO_STR("started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2]);

    unsigned			 myRank = boost::lexical_cast<unsigned>(argv[1]);
    Parset			 parset(argv[2]);
    std::vector<unsigned>	 storageNodeListSubbands = parset.subbandStorageList();
    std::vector<unsigned>	 storageNodeListBeams    = parset.beamStorageList();
    std::vector<SubbandWriter *> subbandWriters;

    CN_Configuration             configuration(parset);
    CN_ProcessingPlan<>          plan(configuration);
    plan.removeNonOutputs();

    // FIXME: implement beamformed data writer
    for (unsigned output = 0; output < plan.plan.size(); output ++) {
      switch (plan.plan[output].distribution) {
        case ProcessingPlan::DIST_SUBBAND:
          for (unsigned subband = 0; subband < storageNodeListSubbands.size(); subband ++)
            if (storageNodeListSubbands[subband] == myRank)
	      subbandWriters.push_back(new SubbandWriter(parset, subband, output));
          break;

        case ProcessingPlan::DIST_BEAM:  
          for (unsigned beam = 0; beam < storageNodeListBeams.size(); beam ++)
            if (storageNodeListBeams[beam] == myRank)
	      subbandWriters.push_back(new SubbandWriter(parset, beam, output));
          break;

        default:
          continue;
      }    
    }      

    ExitOnClosedStdin stdinWatcher;
    Thread stdinWatcherThread(&stdinWatcher,&ExitOnClosedStdin::mainLoop,65535);

    for (unsigned writer = 0; writer < subbandWriters.size(); writer ++)
      delete subbandWriters[writer];

    stdinWatcher.observationDone = true;
  } catch (Exception &ex) {
    LOG_FATAL_STR("caught Exception: " << ex);
    exit(1);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("caught std::exception: " << ex.what());
    exit(1);
  } catch (...) {
    LOG_FATAL_STR("caught non-std::exception: ");
    exit(1);
  }
#endif

  LOG_INFO("Program end");
  return 0;
}

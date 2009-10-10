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
#include <Interface/Thread.h>
#include <Storage/SubbandWriter.h>

#if !defined HAVE_PKVERSION
#include <Storage/Package__Version.h>
#endif
#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>

#include <stdexcept>


using namespace LOFAR;
using namespace LOFAR::RTCP;


class Job
{
  public:
    Job(const char *parsetName, int rank, int size);

    void	  mainLoop();

  private:
    const Parset  itsParset;
    SubbandWriter itsSubbandWriter;
    Thread	  itsThread;
};


static std::vector<Job *> jobs;


Job::Job(const char *parsetName, int rank, int size)
:
  itsParset(parsetName),
  itsSubbandWriter(&itsParset, rank, size),
  itsThread(this, &Job::mainLoop)
{
  LOG_INFO_STR("Created job with parset " << parsetName);
}


void Job::mainLoop()
{
  itsSubbandWriter.preprocess();
  itsSubbandWriter.process();
  itsSubbandWriter.postprocess();
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
#elif !defined LOG4CXX
  Context::initialize();
  setLevel("Global",8);
#endif

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
	      exit(0);

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

  LOG_INFO("Program end");
  return 0; // always return 0, otherwise mpirun kills other storage processes
}

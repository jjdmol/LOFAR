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
#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
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

using namespace log4cplus;
using namespace log4cplus::helpers;

static void child(int argc, char *argv[], int rank, int size)
{
  try {
    if (argc == 3)
      LOG_WARN("specifying nrRuns is deprecated --- ignored");
    else if (argc != 2)
      THROW(StorageException, std::string("usage: ") << argv[0] << " parset");
#if !defined HAVE_PKVERSION
    std::string type = "brief";
    Version::show<StorageVersion> (std::cout, "Storage", type);  
#endif    
    
    LOG_INFO_STR("trying to use parset \"" << argv[1] << '"');
    
    Parset parset(argv[1]);

    // OLAP.parset is depricated, as everything is now in the parset given on the command line
    try {
      parset.adoptFile("OLAP.parset");
    } catch( APSException &ex ) {
      LOG_WARN_STR("could not read OLAP.parset: " << ex);
    }

    SubbandWriter subbandWriter(&parset, rank, size);

    subbandWriter.preprocess();
    subbandWriter.process();
    subbandWriter.postprocess();
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


using namespace log4cplus;
using namespace log4cplus::helpers;

int main(int argc, char *argv[])
{
#ifdef HAVE_LOG4CPLUS
  lofarLoggerInitNode();
  helpers::Properties traceProp;
  traceProp.setProperty("log4cplus.rootLogger", "DEBUG, STDOUT");
  traceProp.setProperty("log4cplus.logger.TRC", "DEBUG");
  traceProp.setProperty("log4cplus.appender.STDOUT", "log4cplus::ConsoleAppender");
  traceProp.setProperty("log4cplus.appender.STDOUT.layout", "log4cplus::PatternLayout");
  traceProp.setProperty("log4cplus.appender.STDOUT.layout.ConversionPattern", "%-5p|%x|%m%n");
  
  PropertyConfigurator(traceProp).configure();
#else
  Context::initialize();
  setLevel("Global",8);
#endif

#if defined HAVE_MPI
  int rank;
  int size;

  MPI_Init(&argc, &argv);
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

  return 0; // always return 0, otherwise mpirun kills other storage processes
}

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

static void child(int argc, char *argv[], int rank)
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
    
    ParameterSet parameterSet(argv[1]);
    Parset parset(&parameterSet);
    parset.adoptFile("OLAP.parset");

    SubbandWriter subbandWriter(&parset, rank);

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
  helpers::Properties traceProp;
  traceProp.setProperty("log4cplus.rootLogger", "DEBUG, STDOUT");
  traceProp.setProperty("log4cplus.appender.STDOUT", "log4cplus::ConsoleAppender");
  traceProp.setProperty("log4cplus.appender.STDOUT.layout", "log4cplus::PatternLayout");
  traceProp.setProperty("log4cplus.appender.STDOUT.layout.ConversionPattern", "%-5p|%c{3}|%m%n");
  traceProp.setProperty("log4cplus.appender.STDOUT.filters.1", "log4cplus::spi::StringMatchFilter");
  traceProp.setProperty("log4cplus.appender.STDOUT.filters.1.AcceptOnMatch", "false");
  traceProp.setProperty("log4cplus.appender.STDOUT.filters.1.StringToMatch", "data");
  traceProp.setProperty("log4cplus.logger.TRC", "DEBUG");
  PropertyConfigurator(traceProp).configure();
#else
  Context::initialize();
  setLevel("Global",8);
#endif

#if defined HAVE_MPI
  int rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
  int rank = 0;
#endif

  int status;
  
  switch (fork()) {
    case -1 : perror("fork");
	      break;

    case 0  : child(argc, argv, rank);
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

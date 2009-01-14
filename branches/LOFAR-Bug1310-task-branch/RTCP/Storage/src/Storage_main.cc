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
#include <Storage/Package__Version.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>

#include <stdexcept>


using namespace LOFAR;
using namespace LOFAR::RTCP;


static void child(int argc, char *argv[], int rank)
{
  try {
    if (argc == 3)
      std::cerr << "WARNING: specifying nrRuns is deprecated --- ignored" << std::endl;
    else if (argc != 2)
      THROW(StorageException, std::string("usage: ") << argv[0] << " parset");

    std::string type = "brief";
    Version::show<StorageVersion> (std::cout, "Storage", type);  
    
    ConfigLocator aCL;
    string        progName = basename(argv[0]);
    string        logPropFile(progName + ".log_prop");
    INIT_LOGGER (aCL.locate(logPropFile).c_str());
    LOG_DEBUG_STR("Initialized logsystem with: " << aCL.locate(logPropFile));

    std::clog << "trying to use parset \"" << argv[1] << '"' << std::endl;
    ParameterSet parameterSet(argv[1]);
    Parset parset(&parameterSet);
    parset.adoptFile("OLAP.parset");

    SubbandWriter subbandWriter(&parset, rank);

    subbandWriter.preprocess();
    subbandWriter.process();
    subbandWriter.postprocess();
  } catch (Exception &ex) {
    std::cerr << "caught Exception: " << ex << std::endl;
    exit(1);
  } catch (std::exception &ex) {
    std::cerr << "caught std::exception: " << ex.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << "caught unknown exception" << std::endl;
    exit(1);
  }
}


int main(int argc, char *argv[])
{
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
		std::cerr << "child returned exit status " << WEXITSTATUS(status) << std::endl;
	      else if (WIFSIGNALED(status))
		std::cerr << "child killed by signal " << WTERMSIG(status) << std::endl;

	      break;
  }

#if defined HAVE_MPI
  MPI_Finalize();
#endif

  return 0; // always return 0, otherwise mpirun kills other storage processes
}

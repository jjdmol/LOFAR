//#  CS1_Storage_main.cc:
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
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Storage/SubbandWriter.h>
#include <CS1_Storage/Package__Version.h>
#include <PLC/ACCmain.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <stdexcept>


using namespace LOFAR;
using namespace LOFAR::CS1;


int main(int argc, char *argv[])
{
  std::string type = "brief";
  Version::show<CS1_StorageVersion> (std::cout, "CS1_Storage", type);  
  
  ConfigLocator aCL;
  string        progName = basename(argv[0]);
  string        logPropFile(progName + ".log_prop");
  INIT_LOGGER (aCL.locate(logPropFile).c_str());
  LOG_DEBUG_STR("Initialized logsystem with: " << aCL.locate(logPropFile));

#if defined HAVE_MPI
  int rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
  int rank = 0;
#endif

  try {
    if (argc == 3)
      std::cerr << "WARNING: specifying nrRuns is deprecated --- ignored" << std::endl;
    else if (argc != 2)
      throw std::runtime_error(std::string("usage: ") + argv[0] + " parset");

    std::clog << "trying to use parset \"" << argv[1] << '"' << std::endl;
    ACC::APS::ParameterSet parameterSet(argv[1]);
    CS1_Parset parset(&parameterSet);
    parset.adoptFile("OLAP.parset");

    SubbandWriter subbandWriter(&parset, rank);

    subbandWriter.preprocess();
    subbandWriter.process();
    subbandWriter.postprocess();
  } catch (Exception &ex) {
    std::cerr << "caught Exception: " << ex.what() << std::endl;
    exit(1);
  } catch (std::exception &ex) {
    std::cerr << "caught std::exception: " << ex.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << "caught unknown exception" << std::endl;
    exit(1);
  }

#if defined HAVE_MPI
  MPI_Finalize();
#endif

  return 0;
}

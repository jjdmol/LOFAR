//#  tAH_TestStorage.cc:
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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

#include <stdexcept>


using namespace LOFAR;
using namespace LOFAR::RTCP;


int main(int argc, char *argv[])
{
  std::string type = "brief";
  Version::show<StorageVersion> (std::cout, "Storage", type);  
  
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
      THROW(StorageException, std::string("usage: ") << argv[0] << " parset");

    std::clog << "trying to use parset \"" << argv[1] << '"' << std::endl;
    ParameterSet parameterSet(argv[1]);
    Parset parset(&parameterSet);
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


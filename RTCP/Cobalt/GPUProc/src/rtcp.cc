//# rtcp.cc: Real-Time Central Processor application, GPU cluster version
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <omp.h>

#include "Common/LofarLogger.h"
#include "Common/Exception.h"
#include "CoInterface/Parset.h"

#include "global_defines.h"
#include "OpenMP_Support.h"
#include "OpenCL_Support.h"
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
#include "Pipelines/UHEP_Pipeline.h"
#include "Storage/StorageProcesses.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;

// Use our own terminate handler
Exception::TerminateHandler t(OpenCL_Support::terminate);

void usage(char **argv)
{
  std::cerr << "usage: " << argv[0] << " parset" << " [-t correlator|beam|UHEP] [-p]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -t: select pipeline type" << std::endl;
  std::cerr << "  -p: enable profiling" << std::endl;
}

enum SELECTPIPELINE { correlator, beam, UHEP,unittest};

// Coverts the input argument from string to a valid 'function' name
SELECTPIPELINE to_select_pipeline(char *argument)
{
  if (!strcmp(argument,"correlator"))
    return correlator;

  if (!strcmp(argument,"beam"))
    return beam;

  if (!strcmp(argument,"UHEP"))
    return UHEP;

  std::cout << "incorrect third argument supplied." << std::endl;
  exit(1);
}

int main(int argc, char **argv)
{
  //Allow usage of nested omp calls
  omp_set_nested(true);

  using namespace LOFAR::Cobalt;

  INIT_LOGGER("rtcp");
  LOG_INFO_STR("running ...");

  // Set parts of the environment
  if (setenv("DISPLAY", ":0", 1) < 0)
  {
    perror("error setting DISPLAY");
    exit(1);
  }

#if 0 && defined __linux__
  set_affinity(0);   //something with processor affinity, define at start of rtcp
#endif

  SELECTPIPELINE option = correlator;
  int opt;

  // parse all command-line options
  while ((opt = getopt(argc, argv, "t:p")) != -1) {
    switch (opt) {
    case 't':
      option = to_select_pipeline(optarg);
      break;

    case 'p':
      profiling = true;
      break;

    default:       /* '?' */
      usage(argv);
      exit(1);
    }
  }

  // we expect a parset filename as an additional parameter
  if (optind >= argc) {
    usage(argv);
    exit(1);
  }

  // Create a parameters set object based on the inputs
  Parset ps(argv[optind]);

  // Set the number of stations: Code is currently non functional
  //bool set_num_stations = false;
  //if (set_num_stations)
  //{
  //    const char *str = getenv("NR_STATIONS");
  //    ps.nrStations() = str ? atoi(str) : 77;
  //}
  LOG_DEBUG_STR("nr stations = " << ps.nrStations());

  // Select number of GPUs to run on

  // Spawn the output processes (only do this once globally)
  StorageProcesses storageProcesses(ps, "");

  // use a switch to select between modes
  switch (option)
  {
  case correlator:
    LOG_INFO_STR("Correlator pipeline selected");
    CorrelatorPipeline(ps).doWork();
    break;

  case beam:
    LOG_INFO_STR("BeamFormer pipeline selected");
    BeamFormerPipeline(ps).doWork();
    break;

  case UHEP:
    LOG_INFO_STR("UHEP pipeline selected");
    UHEP_Pipeline(ps).doWork();
    break;

  default:
    LOG_WARN_STR("No pipeline selected, do nothing");
    break;
  }

  // COMPLETING stage
  time_t completing_start = time(0);

  // retrieve and forward final meta data
  // TODO: Increase timeouts when FinalMetaDataGatherer starts working
  // again
  storageProcesses.forwardFinalMetaData(completing_start + 2);

  // graceful exit
  storageProcesses.stop(completing_start + 10);

  return 0;
}


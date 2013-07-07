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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <omp.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <CoInterface/OutputTypes.h>

#include <InputProc/OMPThread.h>
#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>

#include "global_defines.h"
#include "OpenMP_Lock.h"
#include <GPUProc/Station/StationInput.h>
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
//#include "Pipelines/UHEP_Pipeline.h"
#include "Storage/StorageProcesses.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

void usage(char **argv)
{
  cerr << "usage: " << argv[0] << " parset" << " [-p]" << endl;
  cerr << endl;
  cerr << "  -p: enable profiling" << endl;
}

void runPipeline(const Parset &ps, const vector<size_t> subbands)
{
  bool correlatorEnabled = ps.settings.correlator.enabled;
  bool beamFormerEnabled = ps.settings.beamFormer.enabled;

  if (correlatorEnabled && beamFormerEnabled) {
    LOG_ERROR_STR("Commensal observations (correlator+beamformer) not supported yet.");
    exit(1);
  }

  LOG_INFO_STR("Processing subbands " << subbands);

  if (correlatorEnabled) {
    LOG_INFO_STR("Correlator pipeline selected");
    CorrelatorPipeline(ps, subbands).processObservation(CORRELATED_DATA);
  } else if (beamFormerEnabled) {
    LOG_INFO_STR("BeamFormer pipeline selected");
    BeamFormerPipeline(ps, subbands).processObservation(BEAM_FORMED_DATA);
  } else {
    LOG_FATAL_STR("No pipeline selected, do nothing");
  }
}

int main(int argc, char **argv)
{
  // Make sure all time is dealt with and reported in UTC
  setenv("TZ", "UTC", 1);

  // Restrict access to (tmp build) files we create to owner
  umask(S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

  // Set parts of the environment
  if (setenv("DISPLAY", ":0", 1) < 0)
  {
    perror("error setting DISPLAY");
    exit(1);
  }

  // Rank in MPI set of hosts, or 0 if no MPI is used
  int rank = 0;

  // Number of MPI hosts, or 1 if no MPI is used
  int nrHosts = 1;

#ifdef HAVE_MPI
  // Initialise and query MPI
  int provided_mpi_thread_support;
  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_mpi_thread_support) != MPI_SUCCESS) {
    cerr << "MPI_Init_thread failed" << endl;
    exit(1);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);
#endif

#ifdef HAVE_LOG4CPLUS
  INIT_LOGGER(str(format("rtcp@%02d") % rank));
#else
  INIT_LOGGER_WITH_SYSINFO(str(format("rtcp@%02d") % rank));
#endif

#ifdef HAVE_MPI
  LOG_INFO_STR("MPI rank " << rank << " out of " << nrHosts << " hosts");
#else
  LOG_WARN_STR("Running without MPI!");
#endif

  // parse all command-line options
  int opt;
  while ((opt = getopt(argc, argv, "p")) != -1) {
    switch (opt) {
    case 'p':
      profiling = true;
      break;

    default: /* '?' */
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

  LOG_DEBUG_STR("nr stations = " << ps.nrStations());
  LOG_DEBUG_STR("nr subbands = " << ps.nrSubbands());
  LOG_DEBUG_STR("bitmode     = " << ps.nrBitsPerSample());

  // Only ONE host should start the Storage processes
  SmartPtr<StorageProcesses> storageProcesses;

  if (rank == 0) {
    storageProcesses = new StorageProcesses(ps, "");
  }

  // Distribute the subbands over the MPI ranks
  SubbandDistribution subbandDistribution; // rank -> [subbands]

  for( size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = subband % nrHosts;

    subbandDistribution[receiverRank].push_back(subband);
  }

  sendInputInit(ps);

  #pragma omp parallel sections
  {
    #pragma omp section
    {
      // Read and forward station data
      #pragma omp parallel for num_threads(ps.nrStations())
      for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
        sendInputToPipeline(ps, stat, subbandDistribution);
      }
    }

    #pragma omp section
    {
      // Process station data
      if (!subbandDistribution[rank].empty()) {
        runPipeline(ps, subbandDistribution[rank]);
      }
    }
  }

  /*
   * COMPLETING stage
   */
  if (storageProcesses) {
    time_t completing_start = time(0);

    // retrieve and forward final meta data
    // TODO: Increase timeouts when FinalMetaDataGatherer starts working again
    storageProcesses->forwardFinalMetaData(completing_start + 2);

    // graceful exit
    storageProcesses->stop(completing_start + 10);
  }

  LOG_INFO_STR("Done");

#ifdef HAVE_MPI
  MPI_Finalize();
#endif

  return 0;
}


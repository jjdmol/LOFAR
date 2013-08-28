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
#include <sys/resource.h>
#include <sys/mman.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

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

#include <GPUProc/cpu_utils.h>

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

int main(int argc, char **argv)
{
  /*
   * Parse command-line options
   */

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

  /*
   * Extract rank/size from environment, because we need
   * to fork during initialisation, which we want to do
   * BEFORE calling MPI_Init_thread. Once MPI is initialised,
   * forking can lead to crashes.
   */

  // Rank in MPI set of hosts, or 0 if no MPI is used
  int rank = 0;

  // Number of MPI hosts, or 1 if no MPI is used
  int nrHosts = 1;

#ifdef HAVE_MPI
  const char *rankstr, *sizestr;

  // OpenMPI rank
  if ((rankstr = getenv("OMPI_COMM_WORLD_RANK")) != NULL)
    rank = boost::lexical_cast<int>(rankstr);

  // MVAPICH2 rank
  if ((rankstr = getenv("MV2_COMM_WORLD_RANK")) != NULL)
    rank = boost::lexical_cast<int>(rankstr);

  // OpenMPI size
  if ((sizestr = getenv("OMPI_COMM_WORLD_SIZE")) != NULL)
    nrHosts = boost::lexical_cast<int>(sizestr);

  // MVAPICH2 size
  if ((sizestr = getenv("MV2_COMM_WORLD_SIZE")) != NULL)
    nrHosts = boost::lexical_cast<int>(sizestr);
#endif

  /*
   * Initialise logger.
   */

#ifdef HAVE_LOG4CPLUS
  // Set ${MPIRANK}, which is used by our log_prop file.
  if (setenv("MPIRANK", str(format("%02d") % rank).c_str(), 1) < 0)
  {
    perror("error setting MPIRANK");
    exit(1);
  }

  INIT_LOGGER("rtcp");
#else
  INIT_LOGGER_WITH_SYSINFO(str(format("rtcp@%02d") % rank));
#endif

  LOG_INFO_STR("===== INIT =====");

#ifdef HAVE_MPI
  LOG_INFO_STR("MPI rank " << rank << " out of " << nrHosts << " hosts");
#else
  LOG_WARN_STR("Running without MPI!");
#endif

  /*
   * Initialise the system environment
   */

  // Make sure all time is dealt with and reported in UTC
  if (setenv("TZ", "UTC", 1) < 0)
    THROW_SYSCALL("setenv(TZ)");

  // Tie to local X server (TODO: does CUDA really need this?)
  if (setenv("DISPLAY", ":0", 1) < 0)
    THROW_SYSCALL("setenv(DISPLAY)");

  // Restrict access to (tmp build) files we create to owner
  umask(S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

  // Remove limits on pinned (locked) memory
  struct rlimit unlimited = { RLIM_INFINITY, RLIM_INFINITY };

  if (setrlimit(RLIMIT_MEMLOCK, &unlimited) < 0) {
    int err = errno;
    LOG_WARN_STR("setrlimit(RLIMIT_MEMLOCK, unlimited): " << strerror(err));
  }

  /*
   * Initialise OpenMP
   */

  LOG_INFO_STR("----- Initialising OpenMP");

  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

  /*
   * INIT stage
   */

  LOG_INFO_STR("----- Reading Parset");

  // Create a parameters set object based on the inputs
  Parset ps(argv[optind]);

  LOG_DEBUG_STR("nr stations = " << ps.nrStations());
  LOG_DEBUG_STR("nr subbands = " << ps.nrSubbands());
  LOG_DEBUG_STR("bitmode     = " << ps.nrBitsPerSample());

  LOG_INFO_STR("----- Initialising GPUs");

  gpu::Platform platform;
  vector<gpu::Device> allDevices(platform.devices());

  LOG_INFO_STR("----- Initialising NUMA bindings");

  // TODO: How to migrate the memory that's currently in use
  // (and mlocked!) to the selected CPU?

  // The set of GPUs we're allowed to use
  vector<gpu::Device> devices;
  // If we are testing we do not want dependency on hardware specific cpu configuration
  // Just use all gpu's
  if(rank >= 0 && (size_t)rank < ps.settings.nodes.size()) {
    // set the processor affinity before any threads are created
    int cpuId = ps.settings.nodes[rank].cpu;
    setProcessorAffinity(cpuId);

    // derive the set of gpus we're allowed to use
    const vector<unsigned> &gpuIds = ps.settings.nodes[rank].gpus;
    LOG_DEBUG_STR("Binding to GPUs " << gpuIds);
    for (size_t i = 0; i < gpuIds.size(); ++i)
      devices.push_back(allDevices[i]);
  } else {
    LOG_WARN_STR("Rank " << rank << " not present in node list -- using all cores and GPUs");
    devices = allDevices;
  }

  // Bindings are done -- Lock everything in memory
  if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
    // THROW_SYSCALL("mlockall");
    int err = errno;
    LOG_WARN_STR("mlockall: " << strerror(err));
  } else {
    LOG_DEBUG_STR("All memory is now pinned.");
  }

  // Only ONE host should start the Storage processes
  SmartPtr<StorageProcesses> storageProcesses;

  if (rank == 0) {
    LOG_INFO_STR("----- Starting OutputProc");
    storageProcesses = new StorageProcesses(ps, "");
  }

  LOG_INFO_STR("----- Initialising Pipeline");

  // Distribute the subbands over the MPI ranks
  SubbandDistribution subbandDistribution; // rank -> [subbands]

  for( size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = subband % nrHosts;

    subbandDistribution[receiverRank].push_back(subband);
  }

  bool correlatorEnabled = ps.settings.correlator.enabled;
  bool beamFormerEnabled = ps.settings.beamFormer.enabled;

  if (correlatorEnabled && beamFormerEnabled) {
    LOG_ERROR_STR("Commensal observations (correlator+beamformer) not supported yet.");
    exit(1);
  }

  SmartPtr<Pipeline> pipeline;
  OutputType outputType;

  // Creation of pipelines cause fork/exec, which we need to
  // do before we start doing anything fancy with libraries and threads.
  if (correlatorEnabled) {
    pipeline = new CorrelatorPipeline(ps, subbandDistribution[rank], devices);
    outputType = CORRELATED_DATA;
  } else if (beamFormerEnabled) {
    pipeline = new BeamFormerPipeline(ps, subbandDistribution[rank], devices);
    outputType = BEAM_FORMED_DATA;
  } else {
    LOG_FATAL("No pipeline selected.");
    exit(1);
  }


#ifdef HAVE_MPI
  /*
   * Initialise MPI (we are done forking)
   */

  // Initialise and query MPI
  int provided_mpi_thread_support;

  LOG_INFO_STR("----- Initialising MPI");
  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_mpi_thread_support) != MPI_SUCCESS) {
    cerr << "MPI_Init_thread failed" << endl;
    exit(1);
  }

  // Verify the rank/size settings we assumed earlier
  int real_rank;
  int real_size;

  MPI_Comm_rank(MPI_COMM_WORLD, &real_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &real_size);

  ASSERTSTR(rank == real_rank, 
            "rank = " << rank << ", real_rank = " << real_rank);
  ASSERTSTR(nrHosts == real_size,
            "nrHosts = " << nrHosts << ", real_size = " << real_size);
#else
  // Create the DirectInput instance
  DirectInput::instance(&ps);
#endif

  /*
   * RUN stage
   */

  LOG_INFO_STR("===== LAUNCH =====");

  LOG_INFO_STR("Processing subbands " << subbandDistribution[rank]);

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
        pipeline->processObservation(outputType);
      }
    }
  }

  /*
   * COMPLETING stage
   */
  LOG_INFO_STR("===== FINALISE =====");

  if (storageProcesses) {
    time_t completing_start = time(0);

    LOG_INFO("----- Processing final metadata (broken antenna information)");

    // retrieve and forward final meta data
    // TODO: Increase timeouts when FinalMetaDataGatherer starts working again
    storageProcesses->forwardFinalMetaData(completing_start + 2);

    LOG_INFO("Stopping Storage processes");

    // graceful exit
    storageProcesses->stop(completing_start + 10);

    LOG_INFO("Writing LTA feedback to disk");

    // obtain LTA feedback
    Parset feedbackLTA;
    feedbackLTA.adoptCollection(storageProcesses->feedbackLTA());

    // augment LTA feedback with global information
    feedbackLTA.add("LOFAR.ObsSW.Observation.DataProducts.nrOfOutput_Beamformed_", str(format("%u") % ps.nrStreams(BEAM_FORMED_DATA)));
    feedbackLTA.add("LOFAR.ObsSW.Observation.DataProducts.nrOfOutput_Correlated_", str(format("%u") % ps.nrStreams(CORRELATED_DATA)));

    // write LTA feedback to disk
    const char *LOFARROOT = getenv("LOFARROOT");
    if (LOFARROOT != NULL) {
      string feedbackFilename = str(format("%s/var/run/Observation_%s.feedback") % LOFARROOT % ps.observationID());

      try {
        feedbackLTA.writeFile(feedbackFilename, false);
      } catch (APSException &ex) {
        LOG_ERROR_STR("Could not write feedback file " << feedbackFilename << ": " << ex);
      }
    } else {
      LOG_WARN_STR("Could not write feedback file: $LOFARROOT not set.");
    }
  }
  LOG_INFO_STR("===== SUCCESS =====");

#ifdef HAVE_MPI
  MPI_Finalize();
#endif

  return 0;
}


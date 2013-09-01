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

#ifdef HAVE_LIBNUMA
#include <numa.h>
#include <numaif.h>
#endif

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
  // JD: Don't do that! We want to be able to clean up each other's
  // mess.
  // umask(S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

  // Remove limits on pinned (locked) memory
  struct rlimit unlimited = { RLIM_INFINITY, RLIM_INFINITY };

  if (setrlimit(RLIMIT_MEMLOCK, &unlimited) < 0)
    THROW_SYSCALL("setrlimit(RLIMIT_MEMLOCK, unlimited)");

  /*
   * INIT stage
   */

  LOG_INFO_STR("----- Reading Parset");

  // Create a parameters set object based on the inputs
  Parset ps(argv[optind]);

  if (rank == 0) {
    LOG_INFO_STR("nr stations = " << ps.nrStations());
    LOG_INFO_STR("nr subbands = " << ps.nrSubbands());
    LOG_INFO_STR("bitmode     = " << ps.nrBitsPerSample());
  }

  LOG_INFO_STR("----- Initialising GPUs");

  gpu::Platform platform;
  vector<gpu::Device> allDevices(platform.devices());

  LOG_INFO_STR("----- Initialising NUMA bindings");

  // The set of GPUs we're allowed to use
  vector<gpu::Device> devices;

  // If we are testing we do not want dependency on hardware specific cpu configuration
  // Just use all gpu's
  if(rank >= 0 && (size_t)rank < ps.settings.nodes.size()) {
    // set the processor affinity before any threads are created
    int cpuId = ps.settings.nodes[rank].cpu;
    setProcessorAffinity(cpuId);

#ifdef HAVE_LIBNUMA
    // force node + memory binding for future allocations
    struct bitmask *numa_node = numa_allocate_nodemask();
    numa_bitmask_clearall(numa_node);
    numa_bitmask_setbit(numa_node, cpuId);
    numa_bind(numa_node);
    numa_bitmask_free(numa_node);

    // only allow allocation on this node in case
    // the numa_alloc_* functions are used
    numa_set_strict(1);

    // retrieve and report memory binding
    numa_node = numa_get_membind();
    vector<string> nodestrs;
    for (size_t i = 0; i < numa_node->size; i++)
      if (numa_bitmask_isbitset(numa_node, i))
        nodestrs.push_back(str(format("%s") % i));

    // migrate currently used memory to our node
    numa_migrate_pages(0, numa_all_nodes_ptr, numa_node);

    numa_bitmask_free(numa_node);

    LOG_DEBUG_STR("Bound to memory on nodes " << nodestrs);
#else
    LOG_WARN_STR("Cannot bind memory (no libnuma support)");
#endif

    // Bindings are done -- Lock everything in memory
    if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0)
      THROW_SYSCALL("mlockall");

    // derive the set of gpus we're allowed to use
    const vector<unsigned> &gpuIds = ps.settings.nodes[rank].gpus;
    vector<string> gpuPciIds;
    for (size_t i = 0; i < gpuIds.size(); ++i) {
      gpu::Device &d = allDevices[gpuIds[i]];

      devices.push_back(d);
      gpuPciIds.push_back(d.pciId());
    }
    LOG_DEBUG_STR("Binding to GPUs " << gpuIds << " = " << gpuPciIds);

    // Select on the local NUMA InfiniBand interface (OpenMPI only, for now)
    const string nic = ps.settings.nodes[rank].nic;

    if (nic != "") {
      LOG_DEBUG_STR("Binding to interface " << nic);

      if (setenv("OMPI_MCA_btl_openib_if_include", nic.c_str(), 1) < 0)
        THROW_SYSCALL("setenv(OMPI_MCA_btl_openib_if_include)");
    }
  } else {
    LOG_WARN_STR("Rank " << rank << " not present in node list -- using full machine");
    devices = allDevices;
  }

  /*
   * Initialise OpenMP
   */

  LOG_INFO_STR("----- Initialising OpenMP");

  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

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

  ASSERT(rank    == real_rank);
  ASSERT(nrHosts == real_size);
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


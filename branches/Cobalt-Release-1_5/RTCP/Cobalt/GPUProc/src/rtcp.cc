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
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <omp.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/mman.h>

#ifdef HAVE_LIBNUMA
#include <numa.h>
#include <numaif.h>
#endif

#ifdef HAVE_MPI
#include <mpi.h>
#include <InputProc/Transpose/MPIUtil2.h>
#endif

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
#include <Common/StringUtil.h>
#include <CoInterface/Parset.h>
#include <CoInterface/OutputTypes.h>
#include <CoInterface/OMPThread.h>
#include <InputProc/SampleType.h>
#include <InputProc/WallClockTime.h>
#include <InputProc/Buffer/StationID.h>

#include <ApplCommon/PVSSDatapointDefs.h>
#include <ApplCommon/StationInfo.h>

#include "global_defines.h"
#include "OpenMP_Lock.h"
#include <GPUProc/Station/StationInput.h>
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
//#include "Pipelines/UHEP_Pipeline.h"
#include "Storage/StorageProcesses.h"
#include "Storage/SSH.h"

#include <GPUProc/cpu_utils.h>
#include <GPUProc/SysInfoLogger.h>
#include <GPUProc/Package__Version.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

/* Tuning parameters */

// Number of seconds to schedule for the allocation of resources. That is,
// we start allocating resources at startTime - allocationTimeout.
const time_t defaultAllocationTimeout = 15;

// Deadline for the FinalMetaDataGatherer, in seconds
const time_t defaultFinalMetaDataTimeout = 2 * 60;

// Deadline for outputProc, in seconds.
const time_t defaultOutputProcTimeout = 60;

// Amount of seconds to stay alive after Observation.stopTime
// has passed.
const time_t defaultRtcpTimeout = 5 * 60;

static void usage(const char *argv0)
{
  cerr << "RTCP: Real-Time Central Processing of the LOFAR radio telescope." << endl;
  cerr << "RTCP provides correlation for the Standard Imaging mode and" << endl;
  cerr << "beam-forming for the Pulsar mode." << endl;
  // one of the roll-out scripts greps for the version x.y
  cerr << "GPUProc version " << GPUProcVersion::getVersion() << " r" << GPUProcVersion::getRevision() << endl;
  cerr << endl;
  cerr << "Usage: " << argv0 << " parset" << " [-p]" << endl;
  cerr << endl;
  cerr << "  -p: enable profiling" << endl;
  cerr << "  -h: print this message" << endl;
}

int main(int argc, char **argv)
{
  /*
   * Parse command-line options
   */

  int opt;
  while ((opt = getopt(argc, argv, "ph")) != -1) {
    switch (opt) {
    case 'p':
      profiling = true;
      break;

    case 'h':
      usage(argv[0]);
      exit(0);

    default: /* '?' */
      usage(argv[0]);
      exit(1);
    }
  }

  // we expect a parset filename as an additional parameter
  if (optind >= argc) {
    usage(argv[0]);
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

  // Use LOG_*() for c-strings (incl cppstr.c_str()), and LOG_*_STR() for std::string.
  LOG_INFO("===== INIT =====");

#ifdef HAVE_MPI
  LOG_INFO_STR("MPI rank " << rank << " out of " << nrHosts << " hosts");
#else
  LOG_WARN("Running without MPI!");
#endif

  LOG_INFO_STR("GPUProc version " << GPUProcVersion::getVersion() << " r" << GPUProcVersion::getRevision());

  /*
   * Initialise the system environment
   */

  // Ignore SIGPIPE, as we handle disconnects ourselves
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    THROW_SYSCALL("signal(SIGPIPE)");

  // Make sure all time is dealt with and reported in UTC
  if (setenv("TZ", "UTC", 1) < 0)
    THROW_SYSCALL("setenv(TZ)");

  // Create a parameters set object based on the inputs
  LOG_INFO("----- Reading Parset");
  Parset ps(argv[optind]);

  /* Tuning parameters */

  // Number of seconds to schedule for the allocation of resources. That is,
  // we start allocating resources at startTime - allocationTimeout.
  const time_t allocationTimeout = 
    ps.ParameterSet::getTime("Cobalt.Tuning.allocationTimeout", 
			     defaultAllocationTimeout);

  // Deadline for the FinalMetaDataGatherer, in seconds
  const time_t finalMetaDataTimeout = 
    ps.ParameterSet::getTime("Cobalt.Tuning.finalMetaDataTimeout",
			     defaultFinalMetaDataTimeout);

  // Deadline for outputProc, in seconds.
  const time_t outputProcTimeout = 
    ps.ParameterSet::getTime("Cobalt.Tuning.outputProcTimeout",
			     defaultOutputProcTimeout);

  // Amount of seconds to stay alive after Observation.stopTime
  // has passed.
  const time_t rtcpTimeout = 
    ps.ParameterSet::getTime("Cobalt.Tuning.rtcpTimeout",
			     defaultRtcpTimeout);

  LOG_DEBUG_STR(
    "Tuning parameters:" <<
    "\n  allocationTimeout    : " << allocationTimeout << "s" <<
    "\n  finalMetaDataTimeout : " << finalMetaDataTimeout << "s" <<
    "\n  outputProcTimeout    : " << outputProcTimeout << "s" <<
    "\n  rtcpTimeout          : " << rtcpTimeout << "s");

  if (ps.realTime() && getenv("COBALT_NO_ALARM") == NULL) {
    // First of all, make sure we can't freeze for too long
    // by scheduling an alarm() some time after the observation
    // ends.

    const time_t now = time(0);
    const double stopTime = ps.stopTime();

    if (now < stopTime + rtcpTimeout) {
      size_t maxRunTime = stopTime + rtcpTimeout - now;

      LOG_INFO_STR("RTCP will self-destruct in " << maxRunTime << " seconds");
      alarm(maxRunTime);
    } else {
      LOG_ERROR_STR("Observation.stopTime has passed more than " << rtcpTimeout << " seconds ago, but observation is real time. Nothing to do. Bye bye.");
      return 0;
    }
  }

  // Remove limits on pinned (locked) memory
  struct rlimit unlimited = { RLIM_INFINITY, RLIM_INFINITY };
  if (setrlimit(RLIMIT_MEMLOCK, &unlimited) < 0)
  {
    if (ps.settings.realTime)
      THROW_SYSCALL("setrlimit(RLIMIT_MEMLOCK, unlimited)");
    else
      LOG_WARN("Cannot setrlimit(RLIMIT_MEMLOCK, unlimited)");
  }

  /*
   * Initialise OpenMP
   */

  LOG_INFO("----- Initialising OpenMP");

  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

  /*
   * INIT stage
   */

  // Send identification string to the MAC Log Processor
  string fmtStr(createPropertySetName(PSN_COBALTGPU_PROC, "",
                                      ps.getString("_DPname")));
  format prFmt;
  prFmt.exceptions(boost::io::no_error_bits); // avoid throw
  prFmt.parse(fmtStr);
  LOG_INFO_STR("MACProcessScope: " << str(prFmt
                   % toUpper(myHostname(false))
                   % (ps.settings.nodes.size() > size_t(rank) ? 
                      ps.settings.nodes[rank].cpu : 0)));

  if (rank == 0) {
    LOG_INFO_STR("nr stations = " << ps.nrStations());
    LOG_INFO_STR("nr subbands = " << ps.nrSubbands());
    LOG_INFO_STR("bitmode     = " << ps.nrBitsPerSample());
  }

  LOG_INFO("----- Initialising GPUs");

  gpu::Platform platform;
  LOG_INFO_STR("GPU platform " << platform.getName());
  vector<gpu::Device> allDevices(platform.devices());

  LOG_INFO("----- Initialising NUMA bindings");

  // The set of GPUs we're allowed to use
  vector<gpu::Device> devices;
#if 1
  // If we are testing we do not want dependency on hardware specific cpu configuration
  // Just use all gpu's
  if(rank >= 0 && (size_t)rank < ps.settings.nodes.size()) {
    // set the processor affinity before any threads are created
    int cpuId = ps.settings.nodes.at(rank).cpu;
    setProcessorAffinity(cpuId);

#ifdef HAVE_LIBNUMA
    if (numa_available() != -1) {
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
    } else {
      LOG_WARN("Cannot bind memory (libnuma says there is no numa available)");
    }
#else
    LOG_WARN("Cannot bind memory (no libnuma support)");
#endif

    // derive the set of gpus we're allowed to use
    const vector<unsigned> &gpuIds = ps.settings.nodes[rank].gpus;
    for (size_t i = 0; i < gpuIds.size(); ++i) {
      ASSERTSTR(gpuIds[i] < allDevices.size(), "Request to use GPU #" << gpuIds[i] << ", but found only " << allDevices.size() << " GPUs");

      gpu::Device &d = allDevices[gpuIds[i]];

      devices.push_back(d);
    }

    // Select on the local NUMA InfiniBand interface (OpenMPI only, for now)
    const string nic = ps.settings.nodes[rank].nic;

    if (nic != "") {
      LOG_DEBUG_STR("Binding to interface " << nic);

      if (setenv("OMPI_MCA_btl_openib_if_include", nic.c_str(), 1) < 0)
        THROW_SYSCALL("setenv(OMPI_MCA_btl_openib_if_include)");
    }
  } else {
#else
  {
#endif
    LOG_WARN_STR("Rank " << rank << " not present in node list -- using full machine");
    devices = allDevices;
  }

  for (size_t i = 0; i < devices.size(); ++i)
    LOG_INFO_STR("Bound to GPU #" << i << ": " << devices[i].pciId() << " " << devices[i].getName() << ". Compute capability: " <<
                 devices[i].getComputeCapabilityMajor() << "." <<
                 devices[i].getComputeCapabilityMinor() <<
                 " global memory: " << (devices[i].getTotalGlobalMem() / 1024 / 1024) << " Mbyte");
#if 1
  // Bindings are done -- Lock everything in memory
  if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0)
  {
    if (ps.settings.realTime)
      THROW_SYSCALL("mlockall");
    else
      LOG_WARN("Cannot mlockall(MCL_CURRENT | MCL_FUTURE)");
  } else {
    LOG_DEBUG("All memory is now pinned.");
  }
#endif

  LOG_INFO("----- Initialising Pipeline");

  // Distribute the subbands over the MPI ranks
  SubbandDistribution subbandDistribution; // rank -> [subbands]

  for( size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = subband % nrHosts;

    subbandDistribution[receiverRank].push_back(subband);
  }

  bool correlatorEnabled = ps.settings.correlator.enabled;
  bool beamFormerEnabled = ps.settings.beamFormer.enabled;

  if (correlatorEnabled && beamFormerEnabled) {
    LOG_ERROR("Commensal observations (correlator+beamformer) not supported yet.");
    exit(1);
  }

  SmartPtr<Pipeline> pipeline;

  // Creation of pipelines cause fork/exec, which we need to
  // do before we start doing anything fancy with libraries and threads.
  if (subbandDistribution[rank].empty()) {
    // no operation -- don't even create a pipeline!
    pipeline = NULL;
  } else if (correlatorEnabled) {
    pipeline = new CorrelatorPipeline(ps, subbandDistribution[rank], devices);
  } else if (beamFormerEnabled) {
    pipeline = new BeamFormerPipeline(ps, subbandDistribution[rank], devices, rank);
  } else {
    LOG_FATAL("No pipeline selected.");
    exit(1);
  }

  LOG_INFO("Bailing out now. This was just a test run");
  exit(0);
  
 // Only ONE host should start the Storage processes
  SmartPtr<StorageProcesses> storageProcesses;

  LOG_INFO("----- Initialising SSH library");
  SSH_Init();

  if (rank == 0) {
    LOG_INFO("----- Starting OutputProc");
    storageProcesses = new StorageProcesses(ps, "");
  }

#ifdef HAVE_MPI
  /*
   * Initialise MPI (we are done forking)
   */

  // Initialise and query MPI
  int provided_mpi_thread_support;

  LOG_INFO("----- Initialising MPI");
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

  MPIPoll::instance().start();
#else
  // Create the DirectInput instance
  DirectInput::instance(&ps);
#endif

  // Periodically log system information
  SysInfoLogger siLogger(ps.startTime(), ps.stopTime());

  /*
   * RUN stage
   */

  LOG_INFO("===== LAUNCH =====");

  LOG_INFO_STR("Processing subbands " << subbandDistribution[rank]);

  if (ps.realTime()) {
    // Wait just before the obs starts to allocate resources,
    // both the UDP sockets and the GPU buffers!
    LOG_INFO_STR("Waiting to start obs running from " << TimeStamp::convert(ps.settings.startTime, ps.settings.clockHz()) << " to " << TimeStamp::convert(ps.settings.stopTime, ps.settings.clockHz()));

    const time_t deadline = floor(ps.settings.startTime) - allocationTimeout;
    WallClockTime waiter;
    waiter.waitUntil(deadline);
  }

  #pragma omp parallel sections num_threads(2)
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
        pipeline->processObservation();
      }
    }
  }

  pipeline = 0;

  /*
   * COMPLETING stage
   */
  LOG_INFO("===== FINALISE =====");

  if (storageProcesses) {
    LOG_INFO("----- Processing final metadata (broken antenna information)");

    // retrieve and forward final meta data
    time_t completing_start = time(0);
    storageProcesses->forwardFinalMetaData(completing_start + finalMetaDataTimeout);

    LOG_INFO("Stopping Storage processes");

    // graceful exit
    storageProcesses->stop(completing_start + finalMetaDataTimeout + outputProcTimeout);

    LOG_INFO("Writing LTA feedback to disk");

    // obtain LTA feedback
    Parset feedbackLTA;
    feedbackLTA.adoptCollection(storageProcesses->feedbackLTA());

    // augment LTA feedback with global information
    feedbackLTA.adoptCollection(ps.getGlobalLTAFeedbackParameters());

    // write LTA feedback to disk
    const char *LOFARROOT = getenv("LOFARROOT");
    if (LOFARROOT != NULL) {
      string feedbackFilename = str(format("%s/var/run/Observation%s_feedback") % LOFARROOT % ps.observationID());

      try {
        feedbackLTA.writeFile(feedbackFilename, false);
      } catch (APSException &ex) {
        LOG_ERROR_STR("Could not write feedback file " << feedbackFilename << ": " << ex);
      }
    } else {
      LOG_WARN("Could not write feedback file: $LOFARROOT not set.");
    }

    // final cleanup
    storageProcesses = 0;
  }
  LOG_INFO("===== SUCCESS =====");

  SSH_Finalize();

#ifdef HAVE_MPI
  MPIPoll::instance().stop();

  MPI_Finalize();
#endif

  return 0;
}


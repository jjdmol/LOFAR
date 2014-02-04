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
#include <time.h>

#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
#include <Common/StringUtil.h>
#include <CoInterface/Parset.h>
#include <CoInterface/OutputTypes.h>

#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>

#include <ApplCommon/PVSSDatapointDefs.h>
#include <ApplCommon/StationInfo.h>

#include "global_defines.h"
#include "MPISetup.h"
#include "SystemSetup.h"
#include <GPUProc/Station/StationInput.h>
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
//#include "Pipelines/UHEP_Pipeline.h"
#include "Storage/StorageProcesses.h"
#include "Storage/SSH.h"

#include <GPUProc/SysInfoLogger.h>
#include <GPUProc/Package__Version.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

/* Tuning parameters */

// Deadline for the FinalMetaDataGatherer, in seconds
const time_t finalMetaDataTimeout = 2 * 60;

// Deadline for outputProc, in seconds.
const time_t outputProcTimeout = 2 * 60;

// Amount of seconds to stay alive after Observation.stopTime
// has passed.
const time_t rtcpTimeout = 5 * 60;

static void usage(const char *argv0)
{
  cerr << "RTCP: Real-Time Central Processing of the LOFAR radio telescope." << endl;
  cerr << "RTCP provides correlation for the Standard Imaging mode and" << endl;
  cerr << "beam-forming for the Pulsar mode." << endl;
  cerr << endl;
  cerr << "Usage: " << argv0 << " parset" << " [-p]" << endl;
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
      usage(argv[0]);
      exit(1);
    }
  }

  // we expect a parset filename as an additional parameter
  if (optind >= argc) {
    usage(argv[0]);
    exit(1);
  }

  MPISetup mpi;

  /*
   * Initialise logger.
   */

#ifdef HAVE_LOG4CPLUS
  INIT_LOGGER("rtcp");
#else
  INIT_LOGGER_WITH_SYSINFO(str(format("rtcp@%02d") % mpi.rank));
#endif

  // Use LOG_*() for c-strings (incl cppstr.c_str()), and LOG_*_STR() for std::string.
  LOG_INFO("===== INIT =====");

#ifdef HAVE_MPI
  LOG_INFO_STR("MPI rank " << mpi.rank << " out of " << mpi.size << " hosts");
#else
  LOG_WARN("Running without MPI!");
#endif

  LOG_INFO_STR("GPUProc version " << GPUProcVersion::getVersion() << " r" << GPUProcVersion::getRevision());

  // Create a parameters set object based on the inputs
  LOG_INFO_STR("----- Reading Parset " << argv[optind]);
  Parset ps(argv[optind]);

  if (ps.realTime()) {
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
                   % (ps.settings.nodes.size() > size_t(mpi.rank) ? 
                      ps.settings.nodes[mpi.rank].cpu : 0)));

  if (mpi.rank == 0) {
    LOG_INFO_STR("nr stations = " << ps.nrStations());
    LOG_INFO_STR("nr subbands = " << ps.nrSubbands());
    LOG_INFO_STR("bitmode     = " << ps.nrBitsPerSample());
  }

  // The node we're going to bind to
  struct ObservationSettings::Node *node;

  try {
    node = &ps.settings.nodes.at(mpi.rank);
  } catch(std::out_of_range&) {
    LOG_WARN_STR("Rank " << mpi.rank << " not present in node list -- using full machine");
    node = NULL;
  }

  SystemSetup system(node);

  LOG_INFO("----- Initialising Pipeline");

  // Distribute the subbands over the MPI ranks
  SubbandDistribution subbandDistribution; // rank -> [subbands]

  for( size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = subband % mpi.size;

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
  if (subbandDistribution[mpi.rank].empty()) {
    // no operation -- don't even create a pipeline!
    pipeline = NULL;
  } else if (correlatorEnabled) {
    pipeline = new CorrelatorPipeline(ps, subbandDistribution[mpi.rank], system.gpus);
  } else if (beamFormerEnabled) {
    pipeline = new BeamFormerPipeline(ps, subbandDistribution[mpi.rank], system.gpus, mpi.rank);
  } else {
    LOG_FATAL("No pipeline selected.");
    exit(1);
  }

  // Only ONE host should start the Storage processes
  SmartPtr<StorageProcesses> storageProcesses;

  if (mpi.rank == 0) {
    LOG_INFO("----- Starting OutputProc");
    storageProcesses = new StorageProcesses(ps, "");
  }

#ifdef HAVE_MPI
  /*
   * Initialise MPI (we are done forking)
   */

  mpi.init(argc, argv);
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

  LOG_INFO_STR("Processing subbands " << subbandDistribution[mpi.rank]);

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
      if (!subbandDistribution[mpi.rank].empty()) {
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

  mpi.finalise();

  LOG_INFO("===== DONE =====");

  return 0;
}


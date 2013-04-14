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
#include <map>
#include <vector>
#include <string>
#include <omp.h>
#include "boost/format.hpp"

#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <CoInterface/Parset.h>

#include <InputProc/OMPThread.h>
#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>
#include <InputProc/Buffer/BufferSettings.h>
#include <InputProc/Buffer/BlockReader.h>
#include <InputProc/Station/PacketsToBuffer.h>
#include <InputProc/Station/PacketFactory.h>
#include <InputProc/Station/PacketStream.h>
#include <InputProc/Delays/Delays.h>

#ifdef HAVE_MPI
#include <mpi.h>
#include <InputProc/Transpose/MPISendStation.h>
#endif

#include "global_defines.h"
#include "OpenMP_Support.h"
#include "OpenCL_Support.h"
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
//#include "Pipelines/UHEP_Pipeline.h"
#include "Storage/StorageProcesses.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

// Use our own terminate handler
Exception::TerminateHandler t(OpenCL_Support::terminate);

void usage(char **argv)
{
  cerr << "usage: " << argv[0] << " parset" << " [-t correlator|beam|UHEP] [-p]" << endl;
  cerr << endl;
  cerr << "  -t: select pipeline type" << endl;
  cerr << "  -p: enable profiling" << endl;
}

// Rank in MPI set of hosts
int rank = 0;

// Number of MPI hosts
int nrHosts = 1;

// Which MPI rank receives which subbands.
map<int, vector<size_t> > subbandDistribution; // rank -> [subbands]

// An MPI send process, sending data for one station.
template<typename SampleT> void sender(const Parset &ps, size_t stationIdx)
{
  const TimeStamp from(ps.startTime() * ps.subbandBandwidth(), ps.clockSpeed());
  const TimeStamp to(ps.stopTime() * ps.subbandBandwidth(), ps.clockSpeed());

  /*
   * Construct our stationID.
   */

  // fetch station name (f.e. CS001HBA0)
  const string fullFieldName = ps.settings.stations[stationIdx].name;

  // split into station name and antenna field name
  const string stationName = fullFieldName.substr(0,5); // CS001
  const string fieldName   = fullFieldName.substr(5);   // HBA0

  struct StationID stationID(stationName, fieldName, ps.settings.clockMHz, ps.settings.nrBitsPerSample);

  /*
   * For now, we run the circular buffer
   */
  struct BufferSettings settings(stationID, false);

  settings.setBufferSize(2.0);

  // fetch input streams
  vector<string> inputStreamDescs = ps.getStringVector(str(format("PIC.Core.Station.%s.RSP.ports") % fullFieldName), true);
  vector< SmartPtr<Stream> > inputStreams(inputStreamDescs.size());

  for (size_t board = 0; board < inputStreamDescs.size(); ++board) {
    const string desc = inputStreamDescs[board];

    if (desc == "factory:") {
      PacketFactory factory(settings);
      inputStreams[board] = new PacketStream(factory, from, to, board);
    } else {
      inputStreams[board] = createStream(desc, true);
    }
  }

  ASSERTSTR(inputStreams.size() > 0, "No input streams for station " << fullFieldName);
  settings.nrBoards = inputStreams.size();

  // Force buffer reader/writer syncing if observation is non-real time
  SyncLock syncLock(settings);

  if (!ps.realTime()) {
    settings.sync = true;
    settings.syncLock = &syncLock;
  }

  // Set up the circular buffer
  MultiPacketsToBuffer station(settings, inputStreams);

  /*
   * Stream the data.
   */

  #pragma omp parallel sections
  {
    // Start a circular buffer
    #pragma omp section
    { 
      LOG_INFO_STR("Starting circular buffer");
      station.process();
    }

    // Send data to receivers
    #pragma omp section
    {
      /*
       * Set up delay compensation.
       */
      Delays delays(ps, stationIdx, from, ps.nrSamplesPerSubband());
      delays.start();

      Delays::AllDelays delaySet1(ps), delaySet2(ps);
      Delays::AllDelays *delaysAtBegin  = &delaySet1;
      Delays::AllDelays *delaysAfterEnd = &delaySet2;

      // Get delays at begin of first block
      delays.getNextDelays(*delaysAtBegin);

      // Fetch buffer settings from SHM.
      struct BufferSettings s(stationID, true);

      LOG_INFO_STR("Detected " << s);
      LOG_INFO_STR("Connecting to receivers to send " << from << " to " << to);

      /*
       * Set up circular buffer data reader.
       */
      vector<size_t> beamlets(ps.nrSubbands());
      for( size_t i = 0; i < beamlets.size(); ++i) {
        // Determine the beamlet number of subband i for THIS station
        unsigned board = ps.settings.stations[stationIdx].rspBoardMap[i];
        unsigned slot  = ps.settings.stations[stationIdx].rspSlotMap[i];

        unsigned beamlet = board * s.nrBeamletsPerBoard + slot;

        beamlets[i] = beamlet;
      }

      BlockReader<SampleT> reader(s, beamlets, ps.nrHistorySamples(), 0.25);

#ifdef HAVE_MPI
      /*
       * Set up the MPI send engine.
       */
      MPISendStation sender(s, rank, subbandDistribution);
#else
#error Not implemented -- MPI required
#endif

      /*
       * Transfer all blocks.
       */
      LOG_INFO_STR("Sending to receivers");
      for (TimeStamp current = from; current + ps.nrSamplesPerSubband() < to; current += ps.nrSamplesPerSubband()) {
        // Fetch end delays (start delays are set by the previous block, or
        // before the loop).
        delays.getNextDelays(*delaysAfterEnd);

        // Delay compensation is performed in two parts. First, a coarse
        // correction is done by shifting the block to read by a whole
        // number of samples. Then, in the GPU, the remainder of the delay
        // will be corrected for using a phase shift.
        vector<ssize_t> coarseDelaysSamples(ps.settings.SAPs.size()); // [sap], in samples
        vector<double>  coarseDelaysSeconds(ps.settings.SAPs.size()); // [sap], in seconds
        for (size_t sap = 0; sap < ps.nrBeams(); ++sap) {
          double delayAtBegin  = delaysAtBegin->SAPs[sap].SAP.delay;
          double delayAfterEnd = delaysAfterEnd->SAPs[sap].SAP.delay;

          // The coarse delay compensation is based on the average delay
          // between begin and end.
          coarseDelaysSamples[sap] = static_cast<ssize_t>(floor(0.5 * (delayAtBegin + delayAfterEnd) * ps.subbandBandwidth() + 0.5));
          coarseDelaysSeconds[sap] = coarseDelaysSamples[sap] / ps.subbandBandwidth();
        }

        // Compute the offsets at which each subband is read
        vector<ssize_t> read_offsets(ps.nrSubbands(), 0);

        for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
          unsigned sap = ps.settings.subbands[subband].SAP;

          // Mystery unary minus
          read_offsets[subband] = -coarseDelaysSamples[sap];
        }

        // Read the next block from the circular buffer.
        SmartPtr<struct BlockReader<SampleT>::LockedBlock> block(reader.block(current, current + ps.nrSamplesPerSubband(), read_offsets));

        LOG_INFO_STR("Block read");

        vector<SubbandMetaData> metaDatas(ps.nrSubbands());

        // Add the delays to metaDatas.
        for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
          unsigned sap = ps.settings.subbands[subband].SAP;

          metaDatas[subband].stationBeam.delayAtBegin  = delaysAtBegin->SAPs[sap].SAP.delay - coarseDelaysSeconds[sap];
          metaDatas[subband].stationBeam.delayAfterEnd = delaysAfterEnd->SAPs[sap].SAP.delay - coarseDelaysSeconds[sap];
        }

        // Send the block to the receivers
        sender.sendBlock<SampleT>(*block, metaDatas);

        LOG_INFO_STR("Block sent");

        // Swap delay sets to accomplish delaysAtBegin = delaysAfterEnd
        swap(delaysAtBegin, delaysAfterEnd);
      }

      /*
       * The end.
       */
      station.stop();
    }
  }
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

  cout << "incorrect third argument supplied." << endl;
  exit(1);
}

int main(int argc, char **argv)
{
  // Make sure all time is dealt with and reported in UTC
  setenv("TZ", "UTC", 1);

  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow thread registration
  OMPThread::init();

  using namespace LOFAR::Cobalt;

  INIT_LOGGER("rtcp");
#ifdef HAVE_LOG4CPLUS
#else
  LOGCOUT_SETLEVEL(8);
#endif

  LOG_INFO_STR("running ...");

  // Set parts of the environment
  if (setenv("DISPLAY", ":0", 1) < 0)
  {
    perror("error setting DISPLAY");
    exit(1);
  }

#ifdef HAVE_MPI
  // Initialise and query MPI
  int mpi_thread_support;

  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &mpi_thread_support) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    exit(1);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

  LOG_INFO_STR("MPI rank " << rank << " out of " << nrHosts << " hosts");
#else
  LOG_INFO_STR("MPI not enabled -- running in stand-alone mode");
#endif

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

  LOG_DEBUG_STR("nr stations = " << ps.nrStations());
  LOG_DEBUG_STR("nr subbands = " << ps.nrSubbands());
  LOG_DEBUG_STR("bitmode     = " << ps.nrBitsPerSample());

  // Distribute the subbands over the receivers
  for( size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = ps.nrStations(); // for now, this rank receives all

    subbandDistribution[receiverRank].push_back(subband);
  }

  // This is currently the only supported case
  ASSERT(nrHosts >= (int)ps.nrStations() + 1);

  // Decide course to take based on rank.
  if (rank < (int)ps.nrStations()) {
    /*
     * Send station data
     */
   
    switch (ps.nrBitsPerSample()) {
    default:
    case 16: 
      sender< SampleType<i16complex> >(ps, rank);
      break;

    case 8: 
      sender< SampleType<i8complex> >(ps, rank);
      break;

    case 4: 
      sender< SampleType<i4complex> >(ps, rank);
      break;
    }

  } else if (subbandDistribution.find(rank) != subbandDistribution.end()) {
    /*
     * Receive and process station data
     */

    // TODO: Honour subbandDistribution by forwarding it to the pipeline
      
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
      //UHEP_Pipeline(ps).doWork();
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
  } else {
    LOG_WARN_STR("Superfluous MPI rank");
  }

  LOG_INFO_STR("Done");

#ifdef HAVE_MPI
  MPI_Finalize();
#endif

  return 0;
}


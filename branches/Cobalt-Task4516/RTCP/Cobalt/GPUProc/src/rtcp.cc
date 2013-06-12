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
#include <map>
#include <vector>
#include <string>
#include <omp.h>

#ifdef HAVE_MPI
#include <mpi.h>
#include <InputProc/Transpose/MPISendStation.h>
#else
#include <GPUProc/DirectInput.h>
#endif

#include <boost/format.hpp>

#include <Common/LofarLogger.h>
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

#include "global_defines.h"
#include "OpenMP_Lock.h"
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
  cerr << "usage: " << argv[0] << " parset" << " [-t correlator|beam|UHEP] [-p]" << endl;
  cerr << endl;
  cerr << "  -t: select pipeline type" << endl;
  cerr << "  -p: enable profiling" << endl;
}

// Rank in MPI set of hosts, or 0 if no MPI is used
int rank = 0;

// Number of MPI hosts, or 1 if no MPI is used
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

  // fetch station name (e.g. CS001HBA0)
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

  // Remove lingering buffers
  removeSampleBuffers(settings);

  // fetch input streams
  vector<string> inputStreamDescs = ps.getStringVector(str(format("PIC.Core.Station.%s.RSP.ports") % fullFieldName), true);
  vector< SmartPtr<Stream> > inputStreams(inputStreamDescs.size());

  for (size_t board = 0; board < inputStreamDescs.size(); ++board) {
    const string desc = inputStreamDescs[board];

    LOG_DEBUG_STR("Input stream for board " << board << ": " << desc);

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
#endif

      /*
       * Set up delay compensation.
       */
      Delays delays(ps, stationIdx, from, ps.nrSamplesPerSubband());
      delays.start();

      // We keep track of the delays at the beginning and end of each block.
      // After each block, we'll swap the afterEnd delays into atBegin.
      Delays::AllDelays delaySet1(ps), delaySet2(ps);
      Delays::AllDelays *delaysAtBegin  = &delaySet1;
      Delays::AllDelays *delaysAfterEnd = &delaySet2;

      // Get delays at begin of first block
      delays.getNextDelays(*delaysAtBegin);

      /*
       * Transfer all blocks.
       */
      LOG_INFO_STR("Sending to receivers");

      vector<SubbandMetaData> metaDatas(ps.nrSubbands());
      vector<ssize_t> read_offsets(ps.nrSubbands());

      for (TimeStamp current = from; current + ps.nrSamplesPerSubband() < to; current += ps.nrSamplesPerSubband()) {
        // Fetch end delays (start delays are set by the previous block, or
        // before the loop).
        delays.getNextDelays(*delaysAfterEnd);

        // Compute the next set of metaData and read_offsets from the new
        // delays pair.
        delays.generateMetaData(*delaysAtBegin, *delaysAfterEnd, metaDatas, read_offsets);

        //LOG_DEBUG_STR("Delays obtained");

        // Read the next block from the circular buffer.
        SmartPtr<struct BlockReader<SampleT>::LockedBlock> block(reader.block(current, current + ps.nrSamplesPerSubband(), read_offsets));

        //LOG_INFO_STR("Block read");

#ifdef HAVE_MPI
        // Send the block to the receivers
        sender.sendBlock<SampleT>(*block, metaDatas);
#else
        // Send the block to the stationDataQueues global object
        for (size_t subband = 0; subband < block->beamlets.size(); ++subband) {
          const struct Block<SampleT>::Beamlet &beamlet = block->beamlets[subband];

          /* create new block */
          SmartPtr<struct InputBlock> pblock = new InputBlock;

          pblock->samples.resize((ps.nrHistorySamples() + ps.nrSamplesPerSubband()) * sizeof(SampleT));

          /* copy metadata */
          pblock->metaData = metaDatas[subband];

          /* copy data */
          const size_t nrBytesRange0 = (beamlet.ranges[0].to - beamlet.ranges[0].from) * sizeof(SampleT);

          memcpy(&pblock->samples[0], beamlet.ranges[0].from, nrBytesRange0);

          if (beamlet.nrRanges > 1) {
            const size_t nrBytesRange1 = (beamlet.ranges[1].to - beamlet.ranges[1].from) * sizeof(SampleT);

            memcpy(&pblock->samples[nrBytesRange0], beamlet.ranges[1].from, nrBytesRange1);
          }

          /* obtain flags (after reading the data!) */
          pblock->metaData.flags = beamlet.flagsAtBegin | block->flags(subband);

          /* send to pipeline */
          stationDataQueues[stationIdx][subband]->append(pblock);
        }
#endif

        //LOG_INFO_STR("Block sent");

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

void sender(const Parset &ps, size_t stationIdx)
{
  switch (ps.nrBitsPerSample()) {
    default:
    case 16: 
      sender< SampleType<i16complex> >(ps, stationIdx);
      break;

    case 8: 
      sender< SampleType<i8complex> >(ps, stationIdx);
      break;

    case 4: 
      sender< SampleType<i4complex> >(ps, stationIdx);
      break;
  }
}

enum SELECTPIPELINE { correlator, beam, UHEP,unittest};

// Converts the input argument from string to a valid 'function' name
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

void runPipeline(SELECTPIPELINE pipeline, const Parset &ps, const vector<size_t> subbands)
{
  LOG_INFO_STR("Processing subbands " << subbands);

  // use a switch to select between modes
  switch (pipeline)
  {
  case correlator:
    LOG_INFO_STR("Correlator pipeline selected");
    CorrelatorPipeline(ps, subbands).doWork();
    break;

  case beam:
    LOG_INFO_STR("BeamFormer pipeline selected");
    //BeamFormerPipeline(ps).doWork();
    break;

  case UHEP:
    LOG_INFO_STR("UHEP pipeline selected");
    //UHEP_Pipeline(ps).doWork();
    break;

  default:
    LOG_WARN_STR("No pipeline selected, do nothing");
    break;
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

  using namespace LOFAR::Cobalt;

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
    cerr << "MPI_Init failed" << endl;
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

  // Distribute the subbands over the receivers
#ifdef HAVE_MPI
  size_t nrReceivers = nrHosts - ps.nrStations();
  size_t firstReceiver = ps.nrStations();
#else
  size_t nrReceivers = 1;
  size_t firstReceiver = 0;
#endif

  for( size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = firstReceiver + subband % nrReceivers;

    subbandDistribution[receiverRank].push_back(subband);
  }

#ifdef HAVE_MPI
  // This is currently the only supported case
  ASSERT(nrHosts >= (int)ps.nrStations() + 1);

  // Decide course to take based on rank.
  if (rank < (int)ps.nrStations()) {
    ASSERTSTR(subbandDistribution.find(rank) == subbandDistribution.end(), "An MPI rank can't both send station data and receive it.");

    /*
     * Send station data for station #rank
     */
    sender(ps, rank);
  } else if (subbandDistribution.find(rank) != subbandDistribution.end()) {
    /*
     * Receive and process station data
     */
    runPipeline(option, ps, subbandDistribution[rank]);
  } else {
    LOG_WARN_STR("Superfluous MPI rank");
  }
#else
  // No MPI -- run the same stuff, but multithreaded

  // Create queues to forward station data
  stationDataQueues.resize(boost::extents[ps.nrStations()][ps.nrSubbands()]);

  for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
    for (size_t sb = 0; sb < ps.nrSubbands(); ++sb) {
      stationDataQueues[stat][sb] = new BestEffortQueue< SmartPtr<struct InputBlock> >(1, ps.realTime());
    }
  }

  #pragma omp parallel sections
  {
    #pragma omp section
    {
      // Read and forward station data
      #pragma omp parallel for num_threads(ps.nrStations())
      for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
        sender(ps, stat);
      }
    }

    #pragma omp section
    {
      // Process station data
      runPipeline(option, ps, subbandDistribution[rank]);
    }
  }
#endif

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


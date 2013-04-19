/* tMPITransfer.cc
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#include <lofar_config.h>

#include <string>
#include <vector>
#include <map>
#include <omp.h>
#include <mpi.h>

#include <boost/format.hpp>

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Stream.h>
#include <CoInterface/RSPTimeStamp.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>

#include <InputProc/OMPThread.h>
#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>
#include <InputProc/Buffer/BufferSettings.h>
#include <InputProc/Buffer/BlockReader.h>
#include <InputProc/Station/PacketFactory.h>
#include <InputProc/Station/Generator.h>
#include <InputProc/Station/PacketsToBuffer.h>
#include <InputProc/Transpose/MPIReceiveStations.h>
#include <InputProc/Transpose/MPISendStation.h>
#include <InputProc/Transpose/MapUtil.h>

#include <map>
#include <vector>

#define DURATION 3
#define BLOCKSIZE 0.2
#define NRSTATIONS 3
#define NRBEAMLETS 4

using namespace LOFAR;
using namespace Cobalt;
using namespace std;
using boost::format;

const size_t clockMHz = 200;
const size_t clockHz = clockMHz * 1000 * 1000;
typedef SampleType<i16complex> SampleT;
const TimeStamp from(time(0L) + 10, 0, clockHz);
const TimeStamp to(time(0L) + 10 + DURATION, 0, clockHz);
const size_t blockSize = BLOCKSIZE * clockHz / 1024;
map<int, std::vector<size_t> > beamletDistribution;
const size_t nrHistorySamples = 16;

SubbandMetaData metaData;

// Rank in MPI set of hosts
int rank;

// Number of MPI hosts
int nrHosts;

// Number of MPI hosts acting as stations.
int nrStations;

// Number of MPI hosts acting as receivers.
int nrReceivers;

void sender()
{
  struct StationID stationID(str(format("CS%03d") % rank), "LBA", clockMHz, 16);
  struct BufferSettings settings(stationID, false);

  omp_set_nested(true);
  //omp_set_num_threads(32);
  OMPThread::init();

  // Transfer of packets from generator to buffer
  std::vector< SmartPtr<Stream> > inputStreams(4);
  std::vector< SmartPtr<Stream> > outputStreams(4);
  for (size_t i = 0; i < inputStreams.size(); ++i) {
    const string desc = str(format("tcp:127.0.0.%d:%u") % (rank + 1) % (4346+i));


    #pragma omp parallel sections
    {
      #pragma omp section
      inputStreams[i] = createStream(desc, true);

      #pragma omp section
      outputStreams[i] = createStream(desc, false);
    }
  }

  MultiPacketsToBuffer station( settings, inputStreams );
  PacketFactory factory( settings );
  Generator generator( settings, outputStreams, factory );

  #pragma omp parallel sections
  {
    // Generate the data to send
    #pragma omp section
    generator.process();

    // Start a circular buffer
    #pragma omp section
    station.process();

    // Send data to receivers
    #pragma omp section
    {
      struct BufferSettings s(stationID, true);

      LOG_INFO_STR("Detected " << s);
      LOG_INFO_STR("Connecting to receivers to send " << from << " to " << to);
      BlockReader<SampleT> reader(s, values(beamletDistribution), nrHistorySamples, 0.1);
      MPISendStation sender(s, rank, beamletDistribution);

      LOG_INFO_STR("Sending to receivers");
      for (TimeStamp current = from; current + blockSize < to; current += blockSize) {
        SmartPtr<struct BlockReader<SampleT>::LockedBlock> block(reader.block(current, current + blockSize, std::vector<ssize_t>(values(beamletDistribution).size(), 0)));

        std::vector<SubbandMetaData> metaDatas(block->beamlets.size(), metaData);

        sender.sendBlock<SampleT>(*block, metaDatas);
      }

      generator.stop();
      station.stop();
    }
  }
}

void receiver()
{
  const std::vector<size_t> &beamlets = beamletDistribution[rank];
  const size_t nrBeamlets = beamlets.size();

  std::vector<int> stationRanks(nrStations);

  for (size_t i = 0; i < stationRanks.size(); i++)
    stationRanks[i] = i;

  LOG_INFO_STR("Receiver node " << rank << " starts, handling " << beamlets.size() << " subbands from " << nrStations << " stations." );
  LOG_INFO_STR("Connecting to senders to receive " << from << " to " << to);

  MPIReceiveStations receiver(stationRanks, beamlets, blockSize + nrHistorySamples);

  // create space for the samples
  MultiDimArray<SampleT, 3> samples(boost::extents[nrStations][nrBeamlets][blockSize + nrHistorySamples]);

  // create blocks -- they all have to be the right size already
  std::vector< struct MPIReceiveStations::Block<SampleT> > blocks(nrStations);

  for (int s = 0; s < nrStations; ++s) {
    blocks[s].beamlets.resize(nrBeamlets);

    for (size_t b = 0; b < nrBeamlets; ++b) {
      blocks[s].beamlets[b].samples = &samples[s][b][0];
    }
  }

  // start transfer
  size_t blockIdx = 0;

  for(TimeStamp current = from; current + blockSize < to; current += blockSize) {
    receiver.receiveBlock<SampleT>(blocks);

    // validate meta data
    for (int s = 0; s < nrStations; ++s) {
      for (size_t b = 0; b < nrBeamlets; ++b) {
        ASSERT(blocks[s].beamlets[b].metaData.flags == metaData.flags);
      }
    }

    // calculate flagging average
    const size_t nrSamples = nrStations * nrBeamlets * (blockSize + nrHistorySamples);
    size_t nrFlaggedSamples = 0;

    for (int s = 0; s < nrStations; ++s) {
      for (size_t b = 0; b < nrBeamlets; ++b) {
        nrFlaggedSamples = blocks[s].beamlets[b].metaData.flags.count();
      }
    }

    float flagPerc = 100.0f * nrFlaggedSamples / nrSamples;

    LOG_INFO_STR("Receiver " << rank << " received block " << blockIdx << " flags: " << flagPerc << "%" );
    ++blockIdx;
  }

  LOG_INFO_STR("Receiver " << rank << " done");
}

int main( int argc, char **argv )
{
  INIT_LOGGER( "tMPITransfer" );

  // Prevent stalling.
  alarm(30);

  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

  // Need at least one sender and one receiver
  ASSERT( nrHosts >= 2 );

  // Use half of the nodes as stations
  nrStations = nrHosts/2;
  nrReceivers = nrHosts - nrStations;

  // Divide the subbands over the receivers
  // Make sure not to use the identity list to detect
  // mixups of stationBeamlet and beamletIdx.
  for (unsigned i = 0; i < NRBEAMLETS; ++i) {
    unsigned stationBeamlet = NRBEAMLETS - i - 1;
    int receiverNr = i % nrReceivers;

    beamletDistribution[nrStations + receiverNr].push_back(stationBeamlet);
  }

  if (rank < nrStations) {
    sender();
  } else {
    receiver();
  }

  MPI_Finalize();
}


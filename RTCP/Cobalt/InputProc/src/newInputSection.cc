//# newInputSection.cc
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
//# $Id: $

#include <lofar_config.h>

#include <string>
#include <vector>
#include <map>
#include <omp.h>
#if defined HAVE_MPI
#include <mpi.h>
#else
#error Cannot build input section without HAVE_MPI
#endif
#include <boost/format.hpp>

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Stream.h>
#include <CoInterface/RSPTimeStamp.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>

#include "OMPThread.h"
#include "SampleType.h"
#include "WallClockTime.h"
#include "Buffer/StationID.h"
#include "Buffer/BufferSettings.h"
#include "Station/Generator.h"
#include "Station/PacketsToBuffer.h"
#include "Transpose/MPITransferStations.h"


#define DURATION 60
#define BLOCKSIZE 0.005
#define NRSTATIONS 3
#define NR_TAPS 16

using namespace LOFAR;
using namespace Cobalt;


int main( int argc, char **argv )
{
  size_t clock = 200 * 1000 * 1000;

  typedef SampleType<i16complex> SampleT;
  const TimeStamp from(time(0L) + 1, 0, clock);
  const TimeStamp to(time(0L) + 1 + DURATION, 0, clock);
  const size_t blockSize = BLOCKSIZE * clock / 1024 + NR_TAPS;
  std::map<unsigned, std::vector<size_t> > beamlets;

  struct StationID stationID("RS106", "LBA", clock, 16);
  struct BufferSettings settings(stationID, false);

  settings.setBufferSize(5.0);

  INIT_LOGGER(argv[0]);

  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  int nrHosts, rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

  int nrStations = NRSTATIONS;

  for (unsigned i = 0; i < 244; ++i)
    beamlets[nrStations + i % (nrHosts - nrStations)].push_back(i);

  if (rank > nrStations - 1) {
    // receiver
    LOG_INFO_STR("Receiver " << rank << " starts, handling " << beamlets[rank].size() << " subbands from " << nrStations << " stations." );

    std::vector<int> stationRanks(nrStations);

    for (size_t i = 0; i < stationRanks.size(); i++)
      stationRanks[i] = i;

    {
      MPIReceiveStations<SampleT> receiver(settings, stationRanks, beamlets[rank], blockSize);

      for(size_t block = 0; block < (to - from) / blockSize + 1; ++block) {
        receiver.receiveBlock();

        // data is now in receiver.lastBlock

        //LOG_INFO_STR("Receiver " << rank << " received block " << block);
      }
    }

    LOG_INFO_STR("Receiver " << rank << " done");

    MPI_Finalize();
    return 0;
  }

  omp_set_nested(true);
  omp_set_num_threads(32);
  OMPThread::init();

  std::vector<std::string> inputStreams(4);
  inputStreams[0] = "udp:127.0.0.1:4346";
  inputStreams[1] = "udp:127.0.0.1:4347";
  inputStreams[2] = "udp:127.0.0.1:4348";
  inputStreams[3] = "udp:127.0.0.1:4349";

  if(rank == 0) {
    MultiPacketsToBuffer station( settings, inputStreams );
    Generator generator( settings, inputStreams );

    #pragma omp parallel sections num_threads(4)
    {
      #pragma omp section
      { station.process();
      }

      #pragma omp section
      { generator.process();
      }

      #pragma omp section
      { sleep(DURATION + 1);
        station.stop();
        sleep(1);
        generator.stop();
      }

      #pragma omp section
      {
        struct StationID lookup("RS106", "HBA0");
        struct BufferSettings s(stationID, true);

        LOG_INFO_STR("Detected " << s);
        #pragma omp parallel for num_threads(nrHosts - nrStations)
        for (int i = nrStations; i < nrHosts; ++i) {
          LOG_INFO_STR("Connecting to receiver " << i );
          MPISendStation< SampleT > streamer(s, from, to, blockSize, NR_TAPS, beamlets[i], i );

          LOG_INFO_STR("Sending to receiver " << i );
          streamer.process( 0.0 );
        }
      }
    }
  } else {
    struct StationID lookup("RS106", "HBA0");
    struct BufferSettings s(stationID, true);

    LOG_INFO_STR("Detected " << s);
      #pragma omp parallel for num_threads(nrHosts - nrStations)
    for (int i = nrStations; i < nrHosts; ++i) {
      LOG_INFO_STR("Connecting to receiver " << i );
      MPISendStation< SampleT > streamer(s, from, to, blockSize, NR_TAPS, beamlets[i], i );

      LOG_INFO_STR("Sending to receiver " << i );
      streamer.process( 0.0 );
    }
  }

  MPI_Finalize();
}


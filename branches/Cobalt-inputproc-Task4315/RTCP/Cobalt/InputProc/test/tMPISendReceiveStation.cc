/* tMPISendReceiveStation.cc
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
#include <mpi.h>

#include <boost/format.hpp>

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>

#include "SampleType.h"
#include "Transpose/MPIReceiveStations.h"
#include "Transpose/MPISendStation.h"
#include "Transpose/MPIUtil.h"

#include <UnitTest++.h>

#include <map>
#include <vector>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;


typedef SampleType<i16complex> SampleT;
std::vector<char> metaDataBlob(100, 42);

// Rank in MPI set of hosts
int rank;

// Number of MPI hosts
int nrHosts;

SmartPtr<MPISendStation> sender;
SmartPtr<MPIReceiveStations> receiver;


TEST(Flags) {
  // Create structures for input and output
  SparseSet<int64> flags_in;
  SparseSet<int64> flags_out;
  vector<char> flags_out_buffer(sender.flagsSize());

  flags_in.include(10, 20);
  flags_in.include(30, 40);
  flags_in += rank; // make flags unique

  // Post requests
  vector<MPI_Request> requests(2);
  
  requests[0] = sender.sendFlags(rank, 42, flags_in);
  requests[1] = receiver.receiveFlags(0, 42, flags_out_buffer);

  // Wait for results
  waitAll(requests);

  // Process results
  flags_out.unmarshall(&flags_out_buffer[0]);

  // Validate results
  CHECK_EQUAL(flags_in, flags_out);
}


int main( int argc, char **argv )
{
  INIT_LOGGER( "tMPISendReceiveStation" );

  // Prevent stalling.
  alarm(30);

  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

  // Set up
  struct StationID stationID(str(format("CS%03d") % rank), "LBA", 200, 16);
  struct BufferSettings settings(stationID, false);

  size_t blockSize = 1024;

  map<int, std::vector<size_t> > beamletDistribution;
  beamletDistribution[rank] = std::vector<size_t>(1,0);

  sender = new MPISendStation(settings, 0, beamletDistribution);
  receiver = new MPIReceiveStations(std::vector<int>(1,rank), beamletDistribution[rank], blockSize);

  // Run tests
  int result = UnitTest::RunAllTests();

  // Tear down
  receiver = 0;
  sender = 0;

  MPI_Finalize();

  return result;
}


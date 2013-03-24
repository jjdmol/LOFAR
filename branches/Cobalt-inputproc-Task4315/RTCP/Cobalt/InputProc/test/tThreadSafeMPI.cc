/* tThreadSafeMPI.cc
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
#include <Transpose/ThreadSafeMPI.h>

#include <UnitTest++.h>
#include <mpi.h>
#include <omp.h>
#include <unistd.h>

#include <map>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace LOFAR::Cobalt::ThreadSafeMPI;
using namespace std;

// Rank in MPI set of hosts
int rank;

// Number of MPI hosts
int nrHosts;

/*
 * Test whether a single transfer to self works.
 */
TEST(Self_Transfer) {
  const size_t value = 42;

#pragma omp parallel sections num_threads(2)
  {
#pragma omp section
    {
      size_t sendValue = value;

      MPI_Send(&sendValue, sizeof sendValue, rank, 0);
    }

#pragma omp section
    {
      size_t recvValue = 0;

      MPI_Recv(&recvValue, sizeof recvValue, rank, 0);

      CHECK_EQUAL(value, recvValue);
    }
  }

  LOG_INFO_STR("Self transfer succeeded");
}


/*
 * Test whether an all-to-all works in non-blocking mode.
 */
TEST(AllToAll_Nonblocking) {
#pragma omp parallel sections num_threads(2)
  {
#pragma omp section
    {
      std::vector<MPI_Request> requests(nrHosts);
      int sendValue = rank;

      for (int i = 0; i < nrHosts; ++i) {
        requests[i] = MPI_Isend(&sendValue, sizeof sendValue, i, 0);
      }

      for (int i = 0; i < nrHosts; ++i) {
        MPI_Wait(requests[i]);
      }
    }

#pragma omp section
    {
      std::vector<MPI_Request> requests(nrHosts);
      std::vector<int> recvValues(nrHosts);

      for (int i = 0; i < nrHosts; ++i) {
        requests[i] = MPI_Irecv(&recvValues[i], sizeof recvValues[i], i, 0);
      }

      for (int i = 0; i < nrHosts; ++i) {
        MPI_Wait(requests[i]);

        CHECK_EQUAL(i, recvValues[i]);
      }
    }
  }

  LOG_INFO_STR("All-to-all non-blocking transfer succeeded");
}


/*
 * Test whether an all-to-all works in blocking mode.
 */
TEST(AllToAll_Blocking) {
#pragma omp parallel sections num_threads(2)
  {
#pragma omp section
    {
#     pragma omp parallel for num_threads(nrHosts)
      for (int i = 0; i < nrHosts; ++i) {
        int sendValue = rank;

        MPI_Send(&sendValue, sizeof sendValue, i, 0);
      }
    }

#pragma omp section
    {
#     pragma omp parallel for num_threads(nrHosts)
      for (int i = 0; i < nrHosts; ++i) {
        int recvValue;

        MPI_Recv(&recvValue, sizeof recvValue, i, 0);

        CHECK_EQUAL(i, recvValue);
      }
    }
  }

  LOG_INFO_STR("All-to-all blocking transfer succeeded");
}

int main( int argc, char **argv )
{
  INIT_LOGGER( "tThreadSafeMPI" );

  omp_set_nested(true);

  // Prevent stalling.
  alarm(10);

  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

  // Instantiate request manager
  Singleton<MPIRequestManager>::instance();

  int result;

# pragma omp parallel sections num_threads(2)
  {
# pragma omp section
    Singleton<MPIRequestManager>::instance().process();

# pragma omp section
    {
      result = UnitTest::RunAllTests();

      Singleton<MPIRequestManager>::instance().stop();
    }
  }

  MPI_Finalize();

  return result;
}


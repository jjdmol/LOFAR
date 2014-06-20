/* tMPIUtil2.cc
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
 * $Id$
 */

#include <lofar_config.h>

#include <InputProc/Transpose/MPIUtil.h>

#include <vector>

#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

const size_t BUFSIZE = 1024;

LOFAR::Cobalt::MPI mpi;

SUITE(waitAll)
{
  TEST(One)
  {
    char inbuf[BUFSIZE], outbuf[BUFSIZE];
    vector<MPI_Request> requests;

    requests.push_back(Guarded_MPI_Irecv(&inbuf, sizeof inbuf, mpi.rank(), 1));
    requests.push_back(Guarded_MPI_Isend(&outbuf, sizeof outbuf, mpi.rank(), 1));

    RequestSet rs(requests, true, "waitAll::One");
    rs.waitAll();
  }

  TEST(MultiThreadOne)
  {
#   pragma omp parallel sections num_threads(2)
    {
#     pragma omp section
      {
        char outbuf[BUFSIZE];

        vector<MPI_Request> requests;
        {
          ScopedLock sl(MPIMutex);
          requests.push_back(Guarded_MPI_Isend(&outbuf, sizeof outbuf, mpi.rank(), 1));
        }

        RequestSet rs(requests, true, "waitAll::MultiThreadOne - sender");
        rs.waitAll();
      }

#     pragma omp section
      {
        char inbuf[BUFSIZE];

        vector<MPI_Request> requests;
        {
          ScopedLock sl(MPIMutex);
          requests.push_back(Guarded_MPI_Irecv(&inbuf, sizeof inbuf, mpi.rank(), 1));
        }

        RequestSet rs(requests, true, "waitAll::MultiThreadOne - receiver");
        rs.waitAll();
      }
    }
  }
}

SUITE(waitAny)
{
  TEST(One)
  {
    char inbuf[BUFSIZE], outbuf[BUFSIZE];
    vector<MPI_Request> requests;

    requests.push_back(Guarded_MPI_Irecv(&inbuf, sizeof inbuf, mpi.rank(), 1));
    requests.push_back(Guarded_MPI_Isend(&outbuf, sizeof outbuf, mpi.rank(), 1));

    RequestSet rs(requests, false, "waitAny::One");

    size_t sum = 0;

    for (size_t i = 0; i < requests.size(); ++i) {
      sum += rs.waitAny();
    }

    CHECK_EQUAL(1UL, sum); // 0 + 1
  }

  TEST(TwoPartial)
  {
    char inbuf1[BUFSIZE], outbuf1[BUFSIZE];
    char inbuf2[BUFSIZE], outbuf2[BUFSIZE];

    vector<MPI_Request> requests;

    // start 2 but don't finish it
    requests.push_back(Guarded_MPI_Irecv(&inbuf2, sizeof inbuf2, mpi.rank(), 2));

    // start and finish 1
    requests.push_back(Guarded_MPI_Irecv(&inbuf1, sizeof inbuf1, mpi.rank(), 1));
    requests.push_back(Guarded_MPI_Isend(&outbuf1, sizeof outbuf1, mpi.rank(), 1));

    RequestSet rs(requests, false, "waitAny::TwoPartial - set 1");

    size_t sum = 0;

    for (size_t i = 0; i < 2; ++i) {
      sum += rs.waitAny();
    }

    CHECK_EQUAL(3UL, sum); // 1 + 2

    // finish 2
    vector<MPI_Request> requests2;

    requests2.push_back(Guarded_MPI_Isend(&outbuf2, sizeof outbuf2, mpi.rank(), 2));

    RequestSet rs2(requests2, false, "waitAny::TwoPartial - set 2");
    CHECK_EQUAL(0UL, rs2.waitAny());

    // recv of 2 should be finished now as well
    CHECK_EQUAL(0UL, rs.waitAny());
  }
}

int main( int argc, char **argv )
{
  INIT_LOGGER("tMPIUtil2");

  mpi.init(argc, argv);

  return UnitTest::RunAllTests() > 0;
}

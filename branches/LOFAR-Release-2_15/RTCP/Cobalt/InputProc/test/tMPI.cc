/* tMPI.cc
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
#include <vector>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <CoInterface/MultiDimArray.h>
#include <InputProc/Transpose/MPIUtil.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

const size_t BUFSIZE = 128 * 1024 * 1024;

const size_t nrRuns = 10;

LOFAR::Cobalt::MPI mpi;

void test()
{
  MultiDimArray<char, 2> send_buffers(boost::extents[mpi.size()][BUFSIZE], 1, mpiAllocator);
  MultiDimArray<char, 2> receive_buffers(boost::extents[mpi.size()][BUFSIZE], 1, mpiAllocator);

  NSTimer timer("MPI I/O", true, false);

  for (size_t run = 0; run < nrRuns; ++run) {
    LOG_INFO_STR("Run " << run);

    vector<MPI_Request> requests;

    timer.start();

    {
      ScopedLock sl(MPIMutex);

    // post our sends
    for (int n = 0; n < mpi.size(); ++n) {
      if (n == mpi.rank()) continue;

      requests.push_back(Guarded_MPI_Issend(&send_buffers[n][0], BUFSIZE, n, 1000 * mpi.rank() + 200 + n));
    }

    // post our receives
    for (int n = 0; n < mpi.size(); ++n) {
      if (n == mpi.rank()) continue;

      requests.push_back(Guarded_MPI_Irecv(&receive_buffers[n][0], BUFSIZE, n, 1000 * n + 200 + mpi.rank()));
    }

    }

    // wait for all to finish
    RequestSet rs(requests, true);
    rs.waitAll();

    timer.stop();
  }

  const size_t bytesPerNode = BUFSIZE * mpi.size();

  LOG_INFO_STR("Avr. data rate in & out (per node): " << (8.0 * bytesPerNode / 1024 / 1024 / 1024) / timer.getAverage() << " Gbit/s");
  LOG_INFO_STR("Avr. data rate in & out (total)   : " << (8.0 * bytesPerNode * mpi.size() / 1024 / 1024 / 1024) / timer.getAverage() << " Gbit/s");
}

int main( int argc, char **argv )
{
  INIT_LOGGER("tMPI");

  mpi.init(argc, argv);

  test();

  return 0;
}

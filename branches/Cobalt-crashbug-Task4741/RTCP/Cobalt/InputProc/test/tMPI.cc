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
#include <sys/time.h>
#include <sys/resource.h>
#include <boost/format.hpp>
#include <fstream>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/PrintVector.h>
#include <InputProc/Transpose/MPIUtil.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

int rank;
int nrHosts;

const size_t BUFSIZE = 128 * 1024 * 1024;

const size_t nrRuns = 20;

void test()
{
  MultiDimArray<char, 2> send_buffers(boost::extents[nrHosts][BUFSIZE], 64);
  MultiDimArray<char, 2> receive_buffers(boost::extents[nrHosts][BUFSIZE], 64);

  NSTimer timer("MPI I/O", true, false);

  for (size_t run = 0; run < nrRuns; ++run) {
    LOG_INFO_STR("Run " << run);

    vector<MPI_Request> requests;

    if (run > 1)
      timer.start();

    // post our sends
    for (int n = 0; n < nrHosts; ++n) {
      if (n == rank) continue;

      requests.push_back(Guarded_MPI_Isend(&send_buffers[n][0], BUFSIZE, n, 1000 * rank + 200 + n));
    }

    // post our receives
    for (int n = 0; n < nrHosts; ++n) {
      if (n == rank) continue;

      requests.push_back(Guarded_MPI_Irecv(&receive_buffers[n][0], BUFSIZE, n, 1000 * n + 200 + rank));
    }

    // wait for all to finish
    waitAll(requests);

    if (run > 1)
      timer.stop();
  }

  const size_t bytesPerNode = BUFSIZE * nrHosts;

  LOG_INFO_STR("Avr. data rate in & out (per node): " << (8.0 * bytesPerNode / 1024 / 1024 / 1024) / timer.getAverage() << " Gbit/s");
  LOG_INFO_STR("Avr. data rate in & out (total)   : " << (8.0 * bytesPerNode * nrHosts / 1024 / 1024 / 1024) / timer.getAverage() << " Gbit/s");
}
    // Set the correct processer affinity
    void setProcessorAffinity(unsigned cpuId)
    {
      // Get the number of cores (32)
      unsigned numCores = sysconf(_SC_NPROCESSORS_ONLN);


      // Determine the cores local to the specified cpuId
      vector<unsigned> localCores;

      for (unsigned core = 0; core < numCores; ++core) {
        // The file below contains an integer indicating the physical CPU
        // hosting this core.
        std::ifstream fs(str(boost::format("/sys/devices/system/cpu/cpu%u/topology/physical_package_id") % core).c_str());

        unsigned physical_cpu;
        fs >> physical_cpu;

        if (!fs.good())
          continue;

        // Add this core to the mask if it matches the requested CPU
        if (physical_cpu == cpuId)
          localCores.push_back(core);
      }

      if (localCores.empty())
        THROW(GPUProcException, "Request to bind to non-existing CPU: " << cpuId);

      LOG_DEBUG_STR("Binding to CPU " << cpuId << ": cores " << localCores);

      // put localCores in a cpu_set
      cpu_set_t mask;  

      CPU_ZERO(&mask); 

      for (vector<unsigned>::const_iterator i = localCores.begin(); i != localCores.end(); ++i)
        CPU_SET(*i, &mask);

      // now assign the mask and set the affinity
      int sched_return = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
      int localerrno = errno;
      if (sched_return != 0)
        throw SystemCallException("sched_setaffinity", localerrno, THROW_ARGS);
    }

int main( int argc, char **argv )
{
  INIT_LOGGER( "tMPI" );

  struct rlimit unlimited = { RLIM_INFINITY, RLIM_INFINITY };

  setrlimit(RLIMIT_MEMLOCK, &unlimited);

  int provided_threading_support;

  for (size_t i = 0; i < argc; ++i)
    cout << "arg " << i << ": " << argv[i] << endl;

  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_threading_support) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  LOG_INFO_STR("Threading support level : " << provided_threading_support << ", and MPI_THREAD_MULTIPLE = " << MPI_THREAD_MULTIPLE);

  char *local_rank = getenv("MV2_COMM_WORLD_LOCAL_RANK");

  if (local_rank[0] == '0') {
    setProcessorAffinity(0);
  } else {
    setProcessorAffinity(1);
  }


  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

  test();

  MPI_Finalize();

  return 0;
}

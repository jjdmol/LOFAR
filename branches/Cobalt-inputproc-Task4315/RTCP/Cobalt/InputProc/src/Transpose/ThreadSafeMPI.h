/* ThreadSafeMPI.h
 * Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_INPUT_PROC_THREADSAFE_MPI_H
#define LOFAR_INPUT_PROC_THREADSAFE_MPI_H

#include <Common/Singleton.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Semaphore.h>

#include <mpi.h>
#include <limits.h>

#include <map>

namespace LOFAR {
  namespace Cobalt {

namespace ThreadSafeMPI {

/*
 * MPIRequestManager keeps track of a set of outstanding
 * MPI requests across threads. Any thread can call
 *    wait(request)
 * which will return once `request' has completed.
 *
 * The process() function must be running for the request
 * manager to work, and can be stopped through the stop()
 * function.
 *
 * The process() function will poll actively if there are
 * requests pending, and call pthread_yield to allow
 * other threads to use the CPU.
 */

class MPIRequestManager {
public:
  MPIRequestManager();
  ~MPIRequestManager();

  // Process wait() requests. Keep this function
  // running while MPI transfers are active.
  void process();

  // Signal process() to stop.
  void stop();

  // Wait for a specific request to finish. Returns
  // false if the wait was interrupted by a cancellation.
  bool wait( const MPI_Request &request );

private:
  static const int INTERNAL_COMM_TAG = INT_MAX;
  volatile bool done;

  Mutex requestsMutex;
  Condition requestAdded;

  std::map<MPI_Request, Semaphore*> requests;

  bool empty() const {
    return requests.empty();
  }

  void addRequest( const MPI_Request &request, Semaphore *semaphore );

  // Tests for completed requests. Returns the number of requests
  // that finished.
  int handleAny();
};

/*
 * Send [ptr, ptr + numBytes) to destRank.
 */
MPI_Request MPI_Isend(void *ptr, size_t numBytes, int destRank, int tag);
void MPI_Send(void *ptr, size_t numBytes, int destRank, int tag);

/*
 * Receive [ptr, ptr + numBytes) from destRank.
 */
MPI_Request MPI_Irecv(void *ptr, size_t numBytes, int destRank, int tag);
void MPI_Recv(void *ptr, size_t numBytes, int destRank, int tag);

/*
 * Wait for an MPI_I* transfer to finish.
 */
void MPI_Wait(const MPI_Request &request);

}
}
}

#endif


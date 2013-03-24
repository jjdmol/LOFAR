/* ThreadSafeMPI.cc
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

#include <lofar_config.h>
#include "ThreadSafeMPI.h"

#include <Common/LofarLogger.h>
#include <Common/Singleton.h>

#include <pthread.h>

using namespace std;

namespace LOFAR {
  namespace Cobalt {

namespace ThreadSafeMPI {

Mutex MPIMutex;

MPIRequestManager::MPIRequestManager()
  :
  done(false)
{
}


MPIRequestManager::~MPIRequestManager()
{
  stop();
}


void MPIRequestManager::stop() {
  done = true;

  // unlock waits for request
  requestAdded.signal();
}


bool MPIRequestManager::wait( const MPI_Request &request ) {
  Semaphore doneWaiting;

  addRequest(request, &doneWaiting);

  // Wait for request to finish
  return doneWaiting.down();
}

void MPIRequestManager::process() {
  LOG_DEBUG_STR("[MPIRequestManager] Process begin");

  // If done, empty the queue.
  while(!done ||!empty()) {
    // wait for at least one pending request
    {
      ScopedLock sl(requestsMutex);

      while(!done && empty()) {
        //LOG_DEBUG_STR("[MPIRequestManager] Waiting for requests");
        requestAdded.wait(requestsMutex);
      }

      if (empty())
        break;
    }

    if(handleAny() == 0) {
      /*
       * SPIN
       */

      pthread_yield();
    }
  }

  LOG_DEBUG_STR("[MPIRequestManager] Process end");
}


void MPIRequestManager::addRequest( const MPI_Request &request, Semaphore *semaphore ) {
  ScopedLock sl(requestsMutex);

  // Can't add requests if finishing up.
  ASSERT(!done);

  // MPI_REQUEST_NULL is used to signal a completed request.
  ASSERT(request != MPI_REQUEST_NULL);

  // Request may not already be present.
  ASSERT(requests.find(request) == requests.end());

  requests[request] = semaphore;

  // Signal that a request has been added
  requestAdded.signal();
}


int MPIRequestManager::handleAny() {
  ScopedLock sl(MPIMutex);

  // Convert the requests map to a vector of request identifiers
  std::vector<MPI_Request> ids;

  {
    ScopedLock sl(requestsMutex);

    ids.reserve(requests.size());
    for(map<MPI_Request, Semaphore*>::const_iterator i = requests.begin(); i != requests.end(); ++i) {
      ids.push_back(i->first);
    }
  }

  // MPI_Testany wants something to test
  ASSERT(ids.size() > 0);

  // Test if any request has finished
  std::vector<int> readyIds(ids.size());
  int readyCount;

  // NOTE: MPI_Testsome sets a completed request to MPI_REQUEST_NULL in the
  // ids array! So we need to create a copy in order to lookup the original
  // MPI_Request.
  std::vector<MPI_Request> ids_copy(ids);
  int error = ::MPI_Testsome(ids_copy.size(), &ids_copy[0], &readyCount, &readyIds[0], MPI_STATUS_IGNORE);
  ASSERT(error == MPI_SUCCESS);

  ASSERT(readyCount != MPI_UNDEFINED);

  for (int i = 0; i < readyCount; ++i) {
    int readyIdx = readyIds[i];

    ASSERT(readyIdx != MPI_UNDEFINED);

    // A request is finished. Remove it from the map and raise the
    // associated Semaphore.
    ScopedLock sl(requestsMutex);

    Semaphore *result = requests.at(ids[readyIdx]);
    requests.erase(ids[readyIdx]);

    if (result) {
      // signal client
      result->up();
    }
  }

  return readyCount;
}


/*
 * Send [ptr, ptr + numBytes) to destRank.
 */
MPI_Request MPI_Isend(void *ptr, size_t numBytes, int destRank, int tag) {
  ASSERT(tag >= 0); // Silly MPI requirement

  MPI_Request request;

  {
    ScopedLock sl(MPIMutex);

    int error;

    error = ::MPI_Isend(ptr, numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
    ASSERT(error == MPI_SUCCESS);
  }

  return request;
}

/*
 * Receive [ptr, ptr + numBytes) from destRank.
 */
MPI_Request MPI_Irecv(void *ptr, size_t numBytes, int destRank, int tag) {
  ASSERT(tag >= 0); // Silly MPI requirement

  MPI_Request request;

  {
    ScopedLock sl(MPIMutex);

    int error;

    error = ::MPI_Irecv(ptr, numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
    ASSERT(error == MPI_SUCCESS);
  }

  return request;
}

void MPI_Wait(const MPI_Request &request) {
  Singleton<MPIRequestManager>::instance().wait(request);
}

void MPI_Send(void *ptr, size_t numBytes, int destRank, int tag) {
  MPI_Request request = MPI_Isend(ptr, numBytes, destRank, tag);

  MPI_Wait(request);
}

void MPI_Recv(void *ptr, size_t numBytes, int destRank, int tag) {
  MPI_Request request = MPI_Irecv(ptr, numBytes, destRank, tag);

  MPI_Wait(request);
}

}
}
}


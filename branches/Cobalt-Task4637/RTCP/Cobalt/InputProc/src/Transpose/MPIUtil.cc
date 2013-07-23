#include <lofar_config.h>
#include "MPIUtil.h"

#include <iomanip>

#include <Common/LofarLogger.h>

//#define DEBUG_MPI

#ifdef DEBUG_MPI
#define DEBUG(str)  LOG_DEBUG_STR(__PRETTY_FUNCTION__ << ": " << str)
#else
#define DEBUG(str)
#endif

#include <ctime>
#include <Common/Thread/Mutex.h>
#include <CoInterface/SmartPtr.h>

using namespace std;

namespace LOFAR {

  namespace Cobalt {

    static Mutex MPIMutex;

    /*
     * Returns (and caches) whether the MPI library is thread safe.
     */
    static bool MPI_threadSafe() {
      /*
       * The level of threading support as reported by the MPI library.
       *
       * For multi-threading support, we need at least MPI_THREAD_MULTIPLE.
       *
       * Note that OpenMPI will claim to support the safe-but-useless
       * MPI_THREAD_SINGLE, while it supports MPI_THREAD_SERIALIZED in practice.
       */
      static int provided_mpi_thread_support;
      static const int minimal_thread_level = MPI_THREAD_MULTIPLE;

      static bool initialised = false;

      if (!initialised) {
        ScopedLock sl(MPIMutex);

        int error = MPI_Query_thread(&provided_mpi_thread_support);
        ASSERT(error == MPI_SUCCESS);

        initialised = true;

        LOG_INFO_STR("MPI is thread-safe: " << (provided_mpi_thread_support >= minimal_thread_level ? "yes" : "no"));
      }

      return provided_mpi_thread_support >= minimal_thread_level;
    }

    int waitAny( std::vector<MPI_Request> &requests )
    {
      DEBUG("entry");

      int idx;


      if (MPI_threadSafe()) {
        int error = MPI_Waitany(requests.size(), &requests[0], &idx, MPI_STATUS_IGNORE);
        ASSERT(error == MPI_SUCCESS);
      } else {
        int flag;

        do {
          {
            ScopedLock sl(MPIMutex);

            int error = MPI_Testany(requests.size(), &requests[0], &idx, &flag, MPI_STATUS_IGNORE);
            ASSERT(error == MPI_SUCCESS);
          }

          // sleep (with lock released)
          if (!flag) {
            const struct timespec req = { 0, 10000000 }; // 10 ms
            nanosleep(&req, NULL);
          }
        } while(!flag);
      }

      ASSERT(idx != MPI_UNDEFINED);

      // NOTE: MPI_Waitany/MPI_Testany will overwrite completed
      // entries with MPI_REQUEST_NULL.
      ASSERT(requests[idx] == MPI_REQUEST_NULL);

      DEBUG("index " << idx);

      return idx;
    }


    void waitAll( std::vector<MPI_Request> &requests )
    {
      DEBUG("entry: " << requests.size() << " requests");

      if (requests.size() > 0) {
        if (MPI_threadSafe()) {
          int error = MPI_Waitall(requests.size(), &requests[0], MPI_STATUS_IGNORE);
          ASSERT(error == MPI_SUCCESS);
        } else {
          int flag;

          do {
            {
              ScopedLock sl(MPIMutex);

              int error = MPI_Testall(requests.size(), &requests[0], &flag, MPI_STATUS_IGNORE);
              ASSERT(error == MPI_SUCCESS);
            }

            // sleep (with lock released)
            if (!flag) {
              const struct timespec req = { 0, 10000000 }; // 10 ms
              nanosleep(&req, NULL);
            }
          } while(!flag);
        }
      }

      // NOTE: MPI_Waitall/MPI_Testall will overwrite completed
      // entries with MPI_REQUEST_NULL.

      DEBUG("exit");
    }


    MPI_Request Guarded_MPI_Isend(const void *ptr, size_t numBytes, int destRank, int tag) {
      DEBUG("size " << numBytes << " tag " << hex << tag << dec << " to " << destRank);

      ASSERT(numBytes > 0);
      ASSERT(tag >= 0); // Silly MPI requirement

      SmartPtr<ScopedLock> sl = MPI_threadSafe() ? 0 : new ScopedLock(MPIMutex);

      MPI_Request request;

      int error;

      error = ::MPI_Isend(const_cast<void*>(ptr), numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
      ASSERT(error == MPI_SUCCESS);

      return request;
    }


    MPI_Request Guarded_MPI_Irecv(void *ptr, size_t numBytes, int srcRank, int tag) {
      DEBUG("size " << numBytes << " tag " << hex << tag);

      ASSERT(tag >= 0); // Silly MPI requirement

      SmartPtr<ScopedLock> sl = MPI_threadSafe() ? 0 : new ScopedLock(MPIMutex);

      MPI_Request request;

      int error;

      error = ::MPI_Irecv(ptr, numBytes, MPI_CHAR, srcRank, tag, MPI_COMM_WORLD, &request);
      ASSERT(error == MPI_SUCCESS);

      return request;
    }

  }
}


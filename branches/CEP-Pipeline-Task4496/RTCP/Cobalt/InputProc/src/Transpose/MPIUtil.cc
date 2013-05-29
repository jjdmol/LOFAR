#include <lofar_config.h>
#include "MPIUtil.h"

#include <Common/LofarLogger.h>

namespace LOFAR {

  namespace Cobalt {

    int waitAny( std::vector<MPI_Request> &requests )
    {
      int idx;

      // Wait for any header request to finish
      // NOTE: MPI_Waitany will overwrite completed entries with
      // MPI_REQUEST_NULL.
      int error = MPI_Waitany(requests.size(), &requests[0], &idx, MPI_STATUS_IGNORE);
      ASSERT(error == MPI_SUCCESS);

      ASSERT(idx != MPI_UNDEFINED);
      ASSERT(requests[idx] == MPI_REQUEST_NULL);

      return idx;
    }


    void waitAll( std::vector<MPI_Request> &requests )
    {
      if (requests.size() > 0) {
        // NOTE: MPI_Waitall will write MPI_REQUEST_NULL into requests array.
        int error = MPI_Waitall(requests.size(), &requests[0], MPI_STATUS_IGNORE);
        ASSERT(error == MPI_SUCCESS);
      }
    }


    MPI_Request Guarded_MPI_Isend(const void *ptr, size_t numBytes, int destRank, int tag) {
      ASSERT(tag >= 0); // Silly MPI requirement

      MPI_Request request;

      int error;

      error = ::MPI_Isend(const_cast<void*>(ptr), numBytes, MPI_CHAR, destRank, tag, MPI_COMM_WORLD, &request);
      ASSERT(error == MPI_SUCCESS);

      return request;
    }


    MPI_Request Guarded_MPI_Irecv(void *ptr, size_t numBytes, int srcRank, int tag) {
      ASSERT(tag >= 0); // Silly MPI requirement

      MPI_Request request;

      int error;

      error = ::MPI_Irecv(ptr, numBytes, MPI_CHAR, srcRank, tag, MPI_COMM_WORLD, &request);
      ASSERT(error == MPI_SUCCESS);

      return request;
    }

  }
}


#ifndef LOFAR_INPUTPROC_MPIUTIL_H
#define LOFAR_INPUTPROC_MPIUTIL_H

#include <mpi.h>
#include <vector>

namespace LOFAR {

  namespace Cobalt {

    // Wait for any request to finish. Returns the index of the request that
    // finished. Finished requests are set to MPI_REQUEST_NULL and ignored in
    // subsequent calls.
    int waitAny( std::vector<MPI_Request> &requests );


    // Wait for all given requests to finish. Finished requests are set to
    // MPI_REQUEST_NULL and ignored in subsequent calls.
    void waitAll( std::vector<MPI_Request> &requests );

    /*
     * A guarded version of MPI_Isend with fewer parameters.
     */
    MPI_Request Guarded_MPI_Isend(void *ptr, size_t numBytes, int destRank, int tag);

    /*
     * A guarded version of MPI_Irecv with fewer parameters.
     */
    MPI_Request Guarded_MPI_Irecv(void *ptr, size_t numBytes, int srcRank, int tag);

  }

}

#endif


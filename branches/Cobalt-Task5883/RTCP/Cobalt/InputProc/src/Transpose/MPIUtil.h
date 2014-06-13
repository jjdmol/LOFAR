#ifndef LOFAR_INPUTPROC_MPIUTIL_H
#define LOFAR_INPUTPROC_MPIUTIL_H

#include <mpi.h>
#include <vector>

#include <Common/Thread/Mutex.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/SmartPtr.h>

namespace LOFAR {

  namespace Cobalt {

    // MPI routines lock this mutex, unless annotated with "NOT LOCKED"
    extern Mutex MPIMutex;

    /*
     * Wrapper for the MPI Engine:
     *
     * - wraps MPI_Init
     * - makes sure MPI_Finalise is called
     * - manages rank & size both before and after MPI_Init
     *
     * We'll create a static MPI object, which makes sure that
     * MPI_Finalize is called after all buffers are freed with
     * MPI_Free_mem.
     */
    class MPI {
    public:
      MPI();
      ~MPI();

      // Call MPI_Init_thread to initialise the MPI engine
      void init(int argc, char **argv);

      bool initialised() const { return itsIsInitialised; }
      int rank() const { return itsRank; }
      int size() const { return itsSize; }

    public:
      // Rank in MPI set of hosts, or 0 if no MPI is used
      int itsRank;

      // Number of MPI hosts, or 1 if no MPI is used
      int itsSize;

    private:
      // Whether MPI_Init has been called
      bool itsIsInitialised;
    };

    extern MPI mpi;

    // An allocator using MPI_Alloc_mem/MPI_Free_mem
    class MPIAllocator : public Allocator
    {
    public:
      // NOTE: alignment must be 1!
      virtual void                *allocate(size_t size, size_t alignment = 1);

      virtual void                deallocate(void *);
    };

    extern MPIAllocator mpiAllocator;

    // Free functor for CoInterface/SmartPtr
    template <typename T = void>
    class SmartPtrMPI
    {
    public:
      static void free( T *ptr )
      {
        mpiAllocator.deallocate(ptr);
      }
    };

    /*
     * A guarded version of MPI_Issend with fewer parameters.
     *
     * Issend: Waits for receive to be posted before sending data,
     *         allowing direct transfers/preventing copying.
     *
     * NOT LOCKED
     */
    MPI_Request Guarded_MPI_Issend(const void *ptr, size_t numBytes, int destRank, int tag);

    /*
     * A guarded version of MPI_Ibsend with fewer parameters.
     *
     * Ibsend: Requires user buffers to be set up (MPI_Buffer_init)
     *
     * NOT LOCKED
     */
    MPI_Request Guarded_MPI_Ibsend(const void *ptr, size_t numBytes, int destRank, int tag);
    /*
     * A guarded version of MPI_Irsend with fewer parameters.
     *
     * Irsend: Requires matching receive to have been posted.
     *
     * NOT LOCKED
     */
    MPI_Request Guarded_MPI_Irsend(const void *ptr, size_t numBytes, int destRank, int tag);

    /*
     * A guarded version of MPI_Isend with fewer parameters.
     *
     * Isend: Allows MPI to chose the best send method (buffer or sync)
     *
     * NOT LOCKED
     */
    MPI_Request Guarded_MPI_Isend(const void *ptr, size_t numBytes, int destRank, int tag);

    /*
     * A guarded version of MPI_Irecv with fewer parameters.
     *
     * NOT LOCKED
     */
    MPI_Request Guarded_MPI_Irecv(void *ptr, size_t numBytes, int srcRank, int tag);
  }

}

#endif


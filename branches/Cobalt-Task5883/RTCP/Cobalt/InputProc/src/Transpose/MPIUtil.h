#ifndef LOFAR_INPUTPROC_MPIUTIL_H
#define LOFAR_INPUTPROC_MPIUTIL_H

#include <mpi.h>
#include <vector>

#include <Common/Thread/Mutex.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Condition.h>
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

    class RequestSet;

    typedef MPI_Request handle_t;

    class MPIPoll {
    public:
      MPIPoll();
      ~MPIPoll();

      // Register a set of MPI requests
      //
      // MPIPoll does NOT take ownership, and
      // will unregister the set once all requests
      // have been completed.
      //
      // It is the programmer's responsibility to
      // make sure that `set' is destructed only
      // after all its requests are finished.
      void add( RequestSet *set );

      // Unregister a set of MPI requests
      void remove( RequestSet *set );

      // Whether a certain set is registered.
      bool have( RequestSet *set );

      // Start watching the registered MPI requests.
      void start();

      // Stop watching the registered MPI requests.
      void stop();

      // Return whether we've been started
      bool is_started() const { return started; }

    private:
      bool started;
      bool done;
      std::vector<RequestSet *> requests;
      Mutex mutex;
      Condition newRequest;

      // The thread that periodically polls MPI
      SmartPtr<Thread> thread;

      // Unregister a set of MPI requests (doesn't grab mutex)
      void _remove( RequestSet *set );

      // Call MPI to test which requests have finished.
      //
      // A vector of indices is returned of the requests that
      // have finished. Also, all finished handles are set
      // to MPI_REQUEST_NULL as a side-effect from the MPI call used.
      //
      // Indices are of type `int' instead of `size_t' because that is
      // what MPI_TestSome returns.
      std::vector<int> testSome( std::vector<handle_t> &handles ) const;

      // Test all registered requests, and for finished ones:
      //
      //   1. Update their status
      //   2. Inform their owner
      //
      // If all requests from a RequestSet are completed, the set
      // will be unregistered.
      bool handleRequests();

      // Keep polling for new requests, and handle the registered ones
      // periodically.
      void pollThread();
    };


    class RequestSet {
    public:
      // Register a set of handles to watch
      // for completion.
      RequestSet(const std::vector<handle_t> &handles, bool willWaitAll, const std::string &name = "<anonymous>");

      ~RequestSet();

      // Wait for one request to finish, and
      // return its index.
      size_t waitAny();

      // Wait for one or more requests to finish,
      // and return a vector of their indices.
      std::vector<size_t> waitSome();

      // Wait for all requests to finish.
      void waitAll();

    private:
      // non-copyable
      RequestSet(const RequestSet &);

      friend class MPIPoll;

      // An identifier for this set, used for debugging purposes
      const std::string name;

      // true:  caller will use waitAll()
      // false: caller will use waitAny()/waitSome()
      const bool willWaitAll;

      // MPI handles to watch
      const std::vector<handle_t> handles;

      // ACTIVE:   Transfer is still in operation.
      // FINISHED: MPI reported the transfer as finished.
      // REPORTED: This transfer has been reported as finished
      //           to our callers (using waitAny/waitSome/waitAll).
      enum State { ACTIVE, FINISHED, REPORTED };

      // State of each handle
      std::vector<State> states;

      // How many requests have been completed
      size_t nrFinished;

      Mutex mutex;
      Condition oneFinished;
      Condition allFinished;
   };

  }

}

#endif


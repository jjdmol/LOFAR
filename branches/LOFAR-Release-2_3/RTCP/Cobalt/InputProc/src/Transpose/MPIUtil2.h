#ifndef LOFAR_INPUTPROC_MPIUTIL2_H
#define LOFAR_INPUTPROC_MPIUTIL2_H

#include <mpi.h>
#include <vector>
#include <string>

#include <Common/Singleton.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Condition.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/SmartPtr.h>

namespace LOFAR {

  namespace Cobalt {

    class RequestSet;

    typedef MPI_Request handle_t;

    class MPIPoll {
    public:
      static MPIPoll &instance() { return Singleton<MPIPoll>::instance(); }

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
      MPIPoll();
      friend class Singleton<MPIPoll>;

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


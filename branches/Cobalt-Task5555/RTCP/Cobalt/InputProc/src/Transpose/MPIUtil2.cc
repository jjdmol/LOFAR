#include <lofar_config.h>
#include "MPIUtil2.h"
#include "MPIUtil.h"

#include <Common/LofarLogger.h>
#include <Common/Singleton.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/SmartPtr.h>
#include <sys/time.h>
#include <algorithm>

//#define DEBUG_MPI

#ifdef DEBUG_MPI
#define DEBUG(str)  LOG_DEBUG_STR(str)
#else
#define DEBUG(str)
#endif

using namespace std;

namespace LOFAR {

  namespace Cobalt {

    MPIPoll::MPIPoll()
    :
      started(false),
      done(false)
    {
    }


    void MPIPoll::add( RequestSet *set ) {
      ScopedLock sl(mutex);

      requests.push_back(set);
      newRequest.signal();
    }


    void MPIPoll::remove( RequestSet *set ) {
      ScopedLock sl(mutex);

      const std::vector<RequestSet*>::iterator i = std::find(requests.begin(), requests.end(), set);

      if (i == requests.end())
        return;

      requests.erase(i);
    }


    bool MPIPoll::have( RequestSet *set ) {
      ScopedLock sl(mutex);

      return std::find(requests.begin(), requests.end(), set) != requests.end();
    }


    void MPIPoll::start() {
      started = true;

      thread = new Thread(this, &MPIPoll::pollThread, "MPIPoll::pollThread");
    }


    void MPIPoll::stop() {
      done = true;

      // Unlock thread if it is waiting for a new request
      newRequest.signal();

      // Wait for thread to finish
      thread = 0;
    }

    std::vector<int> MPIPoll::testSome( std::vector<handle_t> &handles ) const {
      vector<int> doneset;

      if (handles.empty())
        return doneset;

      doneset.resize(handles.size());

      int outcount;

      {
        ScopedLock sl(MPIMutex);

        // MPI_Testsome will put the indices of finished requests in doneset,
        // and set the respective handle to MPI_REQUEST_NULL.
        //
        // Note that handles that are MPI_REQUEST_NULL on input are ignored.
        MPI_Testsome(handles.size(), &handles[0], &outcount, &doneset[0], MPI_STATUSES_IGNORE);
      }

      // Cut off doneset at the actual number of returned indices
      doneset.resize(outcount);

      return doneset;
    }

    namespace {
      struct handle_ref {
        RequestSet *set;
        size_t index;

        handle_ref(RequestSet *set, size_t index): set(set), index(index) {}
      };
    };

    void MPIPoll::handleRequests()
    {
      // Collect all ACTIVE requests, and keep track of their index
      vector<handle_t> handles;

      vector<handle_ref> references;

      for (size_t i = 0; i < requests.size(); ++i) {
        ScopedLock sl(requests[i]->mutex);

        for (size_t j = 0; j < requests[i]->handles.size(); ++j) {
          if (requests[i]->states[j] != RequestSet::ACTIVE)
            continue;

          handles.push_back(requests[i]->handles[j]);
          references.push_back(handle_ref(requests[i], j));
        }
      }

      // Ask MPI which requests have finished
      //
      // NOTE: Finished requests are set to MPI_REQUEST_NULL in `handles'.
      const vector<int> finishedIndices = testSome(handles);

      // Process finished requests
      for(size_t i = 0; i < finishedIndices.size(); ++i) {
        struct handle_ref &ref = references[finishedIndices[i]];
        RequestSet &set = *(ref.set);

        ScopedLock sl(set.mutex);

        // Mark as FINISHED
        set.states[ref.index] = RequestSet::FINISHED;
        set.nrFinished++;

        // Inform waitAny/waitSome threads
        set.oneFinished.signal();

        if (set.nrFinished == set.handles.size()) {
          // Inform waitAll threads
          set.allFinished.signal();

          // Remove this set from the requests to watch
          remove(&set);
        }
      }
    }

    void MPIPoll::pollThread() {
      ScopedLock sl(mutex);

      while(!done) {
        if (requests.empty()) {
          // wait for request, with lock released
          newRequest.wait(mutex);
        } else {
          // poll all handles
          handleRequests();

          // if there are still pending requests, release
          // the lock and just wait with a timeout
          if (!requests.empty()) {
            struct timeval now;
            gettimeofday(&now, NULL);

            struct timespec deadline;

            deadline.tv_sec  = now.tv_sec;
            deadline.tv_nsec = now.tv_usec * 1000 + 1000000; // 1 ms

            newRequest.wait(mutex, deadline);
          }
        }
      }
    }


   RequestSet::RequestSet(const std::vector<handle_t> &handles)
   :
     handles(handles),
     states(handles.size(), ACTIVE),
     nrFinished(0)
   {
     // Requests shouldn't be MPI_REQUEST_NULL,
     // because those will never be reported as FINISHED
     for (size_t i = 0; i < states.size(); ++i) {
       ASSERT(handles[i] != MPI_REQUEST_NULL);
     }

     // register ourselves
     MPIPoll::instance().add(this);
   }

   RequestSet::~RequestSet()
   {
     // all requests should be finished and reported by now
     {
       ScopedLock sl(mutex);

       ASSERT(nrFinished == handles.size());

       for (size_t i = 0; i < states.size(); ++i) {
         ASSERT(states[i] == REPORTED);
       }
     }

     // we should have been unregistered once our last
     // request was FINISHED
     {
       ScopedLock sl(mutex);
       ASSERT(!MPIPoll::instance().have(this));
     }
   }

   size_t RequestSet::waitAny() {
     ScopedLock sl(mutex);

     // There has to be something to wait for
     ASSERT(nrFinished < handles.size());

     for(;;) {
       // Look for a finished request that hasn't been
       // reported yet.
       for (size_t i = 0; i < states.size(); ++i) {
         if (states[i] == FINISHED) {
           states[i] = REPORTED;
           return i;
         }
       }

       // Wait for another request to finish
       oneFinished.wait(mutex);
     }
   }

   vector<size_t> RequestSet::waitSome() {
     ScopedLock sl(mutex);

     // There has to be something to wait for
     ASSERT(nrFinished < handles.size());

     vector<size_t> finished;

     do {
       // Look for all finished requests that haven't been
       // reported yet.
       for (size_t i = 0; i < states.size(); ++i) {
         if (states[i] == FINISHED) {
           states[i] = REPORTED;

           finished.push_back(i);
         }
       }

       if (!finished.empty()) {
         // Wait for another request to finish
         oneFinished.wait(mutex);
       }
     } while (finished.empty());

     return finished;
   }

   void RequestSet::waitAll() {
     ScopedLock sl(mutex);

     while (nrFinished < handles.size()) {
       // Wait for all requests to finish
       allFinished.wait(mutex);
     }
   }

  }
}


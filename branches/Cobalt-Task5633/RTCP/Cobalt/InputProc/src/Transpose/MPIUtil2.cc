#include <lofar_config.h>
#include "MPIUtil2.h"
#include "MPIUtil.h"

#include <Common/LofarLogger.h>
#include <Common/Singleton.h>
#include <Common/Timer.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/TimeFuncs.h>
#include <sys/time.h>
#include <algorithm>

//#define DEBUG_MPI

#ifdef DEBUG_MPI
#define DEBUG(str)  LOG_INFO_STR(str)
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
      ASSERT(set != NULL);

      DEBUG("MPIPoll::add " << set->name);

      ASSERT(MPIPoll::instance().is_started());

      ScopedLock sl(mutex);

      requests.push_back(set);
      newRequest.signal();
    }


    void MPIPoll::remove( RequestSet *set ) {
      ScopedLock sl(mutex);

      _remove(set);
    }

    void MPIPoll::_remove( RequestSet *set ) {
      ASSERT(set != NULL);

      DEBUG("MPIPoll::_remove " << set->name);

      const std::vector<RequestSet*>::iterator i = std::find(requests.begin(), requests.end(), set);

      if (i == requests.end())
        return;

      DEBUG("MPIPoll::_remove found and removing " << set->name);

      requests.erase(i);
    }


    bool MPIPoll::have( RequestSet *set ) {
      ScopedLock sl(mutex);

      return std::find(requests.begin(), requests.end(), set) != requests.end();
    }


    void MPIPoll::start() {
      DEBUG("MPIPoll::start");

      started = true;

      thread = new Thread(this, &MPIPoll::pollThread, "MPIPoll::pollThread");
    }


    void MPIPoll::stop() {
      DEBUG("MPIPoll::stop");

      done = true;

      // Unlock thread if it is waiting for a new request
      newRequest.signal();

      // Wait for thread to finish
      thread = 0;

      DEBUG("MPIPoll::stop stopped");
    }

    // Track the time spent on lock contention
    NSTimer MPIMutexTimer("MPIPoll::MPIMutex lock()", true, true);

    // Track the time spent in MPI_Testsome
    NSTimer MPITestsomeTimer("MPIPoll::MPI_Testsome", true, true);

    std::vector<int> MPIPoll::testSome( std::vector<handle_t> &handles ) const {
      DEBUG("MPIPoll::testSome on " << handles.size() << " handles");

      vector<int> doneset;

      if (handles.empty())
        return doneset;

      doneset.resize(handles.size());

      int outcount;

      {
        MPIMutexTimer.start();
        ScopedLock sl(MPIMutex);
        MPIMutexTimer.stop();

        // MPI_Testsome will put the indices of finished requests in doneset,
        // and set the respective handle to MPI_REQUEST_NULL.
        //
        // Note that handles that are MPI_REQUEST_NULL on input are ignored.
        MPITestsomeTimer.start();
        MPI_Testsome(handles.size(), &handles[0], &outcount, &doneset[0], MPI_STATUSES_IGNORE);
        MPITestsomeTimer.stop();
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

    bool MPIPoll::handleRequests()
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
        DEBUG("MPIPoll::handleRequest: marking " << set.name << "[" << ref.index << "] as FINISHED");
        ASSERT(set.states[ref.index] == RequestSet::ACTIVE);
        set.states[ref.index] = RequestSet::FINISHED;

        set.nrFinished++;

        // Inform waitAny/waitSome threads
        if (!set.willWaitAll)
          set.oneFinished.signal();

        if (set.nrFinished == set.handles.size()) {
          DEBUG("MPIPoll::handleRequest: all requests in " << set.name << " are FINISHED");

          // Inform waitAll threads
          if (set.willWaitAll)
            set.allFinished.signal();

          // Remove this set from the requests to watch
          _remove(&set);
        }
      }

      return !finishedIndices.empty();
    }

    void MPIPoll::pollThread() {
      Thread::ScopedPriority sp(SCHED_FIFO, 10);

      ScopedLock sl(mutex);

      while(!done) {
        // next poll will be in 0.1 ms
        //
        // NOTE: MPI is VERY sensitive to this, requiring
        //       often enough polling to keep transfers
        //       running smoothly.

        if (requests.empty()) {
          // wait for request, with lock released
          newRequest.wait(mutex);
        } else {
          // poll all handles
          (void)handleRequests();

          // if there are still pending requests, release
          // the lock and just wait with a timeout
          if (!requests.empty()) {
            struct timespec deadline = TimeSpec::now();
            TimeSpec::inc(deadline, 0.0001);

            newRequest.wait(mutex, deadline);
          }
        }
      }
    }


   RequestSet::RequestSet(const std::vector<handle_t> &handles, bool willWaitAll, const std::string &name)
   :
     name(name),
     willWaitAll(willWaitAll),
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
     ASSERT(!willWaitAll);

     ScopedLock sl(mutex);

     for(;;) {
       // Look for a finished request that hasn't been
       // reported yet.
       for (size_t i = 0; i < states.size(); ++i) {
         if (states[i] == FINISHED) {
           states[i] = REPORTED;

           DEBUG("RequestSet::waitAny: set " << name << " finished request " << i);
           return i;
         }
       }

       // Wait for another request to finish
       DEBUG("RequestSet::waitAny: set " << name << " waits for a request to finish");

       // There has to be something to wait for
       ASSERT(nrFinished < handles.size());
       oneFinished.wait(mutex);
     }
   }

   vector<size_t> RequestSet::waitSome() {
     ASSERT(!willWaitAll);

     ScopedLock sl(mutex);

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

       if (finished.empty()) {
         // Wait for another request to finish
         DEBUG("RequestSet::waitSome: set " << name << " waits for a request to finish");

         // There has to be something to wait for
         ASSERT(nrFinished < handles.size());
         oneFinished.wait(mutex);
       }
     } while (finished.empty());

     DEBUG("RequestSet::waitSome: set " << name << " finished " << finished.size() << " requests");

     return finished;
   }

   void RequestSet::waitAll() {
     ASSERT(willWaitAll);

     ScopedLock sl(mutex);

     while (nrFinished < handles.size()) {
       DEBUG("RequestSet::waitAll: set " << name << " has " << nrFinished << "/" << handles.size() << " requests finished");

       // Wait for all requests to finish
       allFinished.wait(mutex);
     }

     DEBUG("RequestSet::waitAll: set " << name << " finished all requests");

     // Mark all requests as reported
     for (size_t i = 0; i < states.size(); ++i) {
       ASSERT(states[i] >= FINISHED);

       states[i] = REPORTED;
     }
   }

  }
}


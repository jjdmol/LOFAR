//# OMPThread.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_INPUT_PROC_OMP_THREAD_H 
#define LOFAR_INPUT_PROC_OMP_THREAD_H

#include <ctime>
#include <csignal>
#include <pthread.h>
#include <vector>
#include <algorithm>

#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>

#include <CoInterface/SmartPtr.h>
#include <CoInterface/TimeFuncs.h>
#include <CoInterface/Exceptions.h>

namespace LOFAR
{
  namespace Cobalt
  {

  /*
   * Represents an OpenMP thread. To use,
   * call start() and stop() at the beginning and end
   * of an OpenMP thread. The kill() command can then
   * be used by another thread to kill the OpenMP thread.
   *
   * The thread is killed by sending SIGHUP to it until
   * stop() is called by the thread.
   *
   * To be able to use this class properly, please call
   * OMPThread::init() to clear the SIGHUP handler.
   *
   * Note: do NOT use this on threads which continue operating
   * through omp 'nowait' parallelism, due to race conditions.
   */
  class OMPThread
  {
  public:
    OMPThread() : id(pthread_self()), stopped(false)
    {
    }

    ~OMPThread()
    {
      // Ensure any wait()s finish
      stop();
    }

    // Return whether this OMPThread object represents this thread
    bool isMe() const {
      ScopedLock sl(mutex);

      // If the object has been stop()ped, it may represent
      // an OpenMP thread that has already been assigned to
      // a different task. We effectively consider stopped
      // threads as non-existing.
      return id == pthread_self() && !stopped;
    }

    // Unregister the current thread
    void stop()
    {
      ScopedLock sl(mutex);

      if (stopped)
        return;

      stopped = true;

      stopSignal.broadcast();
    }

    bool wait(const struct timespec &abstime) {
      ScopedLock sl(mutex);

      do {
        if (stopped)
          return true;
      } while(stopSignal.wait(mutex, abstime));

      return false;
    }

    // Kill the registered thread. If no thread is registered,
    // kill() will wait.
    void kill()
    {
      try {
        struct timespec deadline = TimeSpec::now();

        do {
          sendSIGHUP();

          TimeSpec::inc(deadline, 0.2);
        } while (!wait(deadline));
      } catch(Exception &ex) {
        LOG_ERROR_STR("Caught exception: " << ex);
      }
    }

    static void init()
    {
      signal(SIGHUP, sighandler);
#if 0
      // We avoid cancellation exception for OpenMP threads.
      // Allow signalling them ourselves to interrupt some blocking syscalls.
      struct sigaction sa;
      sa.sa_handler = sighandler;
      ::sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      int err = ::sigaction(SIGHUP, &sa, NULL);
      if (err != 0) {
        LOG_WARN("Failed to register a handler for SIGHUP: OpenMP threads may not terminate!");
      }
#endif
    }

  private:
    const pthread_t id;
    bool stopped;

    mutable Mutex mutex;
    Condition stopSignal;

    static void sighandler(int)
    {
      /* no-op. We use SIGHUP only
       * to interrupt system calls.
       */
    }

    void sendSIGHUP() {
      ScopedLock sl(mutex);

      if (stopped)
        return;

      // Interrupt blocking system calls (most notably, read()),
      // possibly multiple in a row.
      // Note that the thread will stick around until the end
      // of pragma parallel, so the thread id is always valid
      // once it has been set.

      // Do not use THROW_SYSCALL(), because pthread_*() does not set errno,
      // but returns it.
      int error = pthread_kill(id, SIGHUP);
      if (error != 0)
        throw SystemCallException("pthread_kill", error, THROW_ARGS);
    }
  };

  /*
   * Manage a set of OMPThreads.
   *
   * Create a OMPThreadSet `threadSet' object.
   * Each OpenMP thread registers itself using OMPThreadSet::ScopedRun sr(threadSet);
   *
   * The threadSet.killAll() function will then kill each thread in the set. Any thread
   * added to the set after killAll() will abort by throwing a CannotStartException.
   */
  class OMPThreadSet {
  public:
    EXCEPTION_CLASS(CannotStartException, Exception);

    OMPThreadSet(): stopped(false) {}

    class ScopedRun {
    public:
      // We add this thread to the set, but will never remove it.
      //
      // We can't access `set' at removal, because removal might
      // be due to OMPThreadSet::killAll, which holds the mutex
      // that's needed for removal as well.
      ScopedRun( OMPThreadSet &set ): thread(set.registerMe()) {}
      ~ScopedRun()                             { thread.stop(); }
    private:
      OMPThread &thread;
    };

    /*
     * Waits until `abstime' for all threads to finish, and kill
     * all that did not.
     *
     * Returns the number of killed processes.
     */
    size_t killAll( const timespec &abstime = TimeSpec::big_bang ) {
      ScopedLock sl(mutex);

      if (stopped)
        return 0;

      stopped = true;

      std::vector<bool> killed(threads.size(), false);

#     pragma omp parallel for num_threads(threads.size())
      for (size_t i = 0; i < threads.size(); ++i) {
        // Give the thread until `abstime' to finish up
        if (threads[i]->wait(abstime)) {
          LOG_DEBUG_STR("Thread " << i << ": exited normally");
        } else {
          // Kill the thread
          LOG_DEBUG_STR("Thread " << i << ": killing...");
          threads[i]->kill();
          LOG_DEBUG_STR("Thread " << i << ": killed");

          killed[i] = true;
        }
      }

      // Return the number of killed processes
      return std::count(killed.begin(), killed.end(), true);
    }

  private:
    Mutex mutex;

    std::vector< SmartPtr<OMPThread> > threads;
    bool stopped;

    // Add this thread to the set
    OMPThread &registerMe() {
      ScopedLock sl(mutex);

      if (stopped)
        THROW(CannotStartException, "ThreadSet was ordered to stop before this thread started");

      SmartPtr<OMPThread> t = new OMPThread;

      threads.push_back(t);

      return **threads.rbegin();
    }

    // Call stop() for this thread in the set, to mark
    // the thread as stopped.
    //
    // Because there is still a ScopedRun object with a reference
    // to the OMPThread, we can't actually free the OMPThread object
    // associated with this thread.
    void unregisterMe() {
      ScopedLock sl(mutex);

      if (stopped)
        return;

      for (size_t i = 0; i < threads.size(); ++i) {
        if (threads[i]->isMe()) {
          threads[i]->stop();
          return;
        }
      }

      THROW(CoInterfaceException, "Unregistering thread that was not registered");
    }
  };

  }
}

#endif


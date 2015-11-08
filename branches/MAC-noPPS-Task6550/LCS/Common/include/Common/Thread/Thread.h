//#  Thread.h:
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_LCS_COMMON_THREAD_H
#define LOFAR_LCS_COMMON_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#ifdef USE_THREADS

#include <pthread.h>
#include <signal.h>
#include <sched.h>

#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <Common/Thread/Semaphore.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Cancellation.h>

#include <boost/algorithm/string.hpp>

#include <string>
#include <vector>
#include <memory>

namespace LOFAR {

class Thread
{
  public:
    // Spawn a method by creating a Thread object as follows:
    //
    // class C {
    //   public:
    //     C() : thread(this, &C::method) {}
    //   private:
    //     void method() { std::cout << "runs asynchronously" << std::endl; }
    //     Thread thread;
    // };
    //
    // The thread is joined in the destructor of the Thread object
      
    template <typename T> Thread(T *object, void (T::*method)(), const std::string &name = "<anon>", const std::string &logPrefix = "", size_t stackSize = 0);

    // ~Thread() is NOT virtual, because Thread should NOT be inherited. An
    // DerivedThread class would partially destruct itself before reaching
    // the pthread_join in ~Thread(), which in turn would result in a running
    // thread controlled by a partially deconstructed Thread object.
			  ~Thread(); // join a thread

    void		  cancel();
    void		  cancel(const struct timespec &); // cancel if thread is not finished at deadline

    void		  wait();
    bool		  wait(const struct timespec &);
    bool      isDone();


    // Returns whether the thread threw an exception. This function will wait for the thread
    // to finish.
    bool      caughtException();

    class ScopedPriority
    {
    public:
      /* see man pthread_setschedparam */
      ScopedPriority(int policy, int priority)
      {
        int retval;

        if ((retval = pthread_getschedparam(pthread_self(), &origPolicy, &origParam)) != 0)
          throw SystemCallException("pthread_getschedparam", retval, THROW_ARGS);

        struct sched_param newParam;
        newParam.sched_priority = priority;

        if ((retval = pthread_setschedparam(pthread_self(), policy, &newParam)) != 0)
          try {
            throw SystemCallException("pthread_setschedparam", retval, THROW_ARGS);
          } catch (Exception &ex) {
            LOG_WARN_STR("Could not change thread priority to policy " << policyName(policy) << " priority " << priority << ": " << ex.what());
          }
      }

      ~ScopedPriority()
      {
        int retval;

        if ((retval = pthread_setschedparam(pthread_self(), origPolicy, &origParam)) != 0)
          try {
            throw SystemCallException("pthread_setschedparam", retval, THROW_ARGS);
          } catch (Exception &ex) {
            LOG_ERROR_STR("Exception in destructor: " << ex);
          }
      }

    private:
      int origPolicy;
      struct sched_param origParam;

      std::string policyName(int policy) const
      {
        switch(policy) {
          case SCHED_OTHER:
            return "SCHED_OTHER (normal)";

#ifdef SCHED_BATCH
          case SCHED_BATCH:
            return "SCHED_BATCH (cpu intensive)";
#endif

#ifdef SCHED_IDLE
          case SCHED_IDLE:
            return "SCHED_IDLE (idle)";
#endif

          case SCHED_FIFO:
            return "SCHED_FIFO (real time)";

          case SCHED_RR:
            return "SCHED_RR (real time)";

          default:
            return "(unknown)";
        };
      }
    };

  private:
    Thread(const Thread&);
    Thread& operator=(const Thread&);

    template <typename T> struct Args {
      Args(T *object, void (T::*method)(), Thread *thread, const std::string &name) : object(object), method(method), thread(thread), name(name) {}

      T	     *object;
      void   (T::*method)();
      Thread *thread;

      std::string name;
    };

    template <typename T> void	      stub(Args<T> *);
    template <typename T> static void *stub(void *);

    const std::string logPrefix;
    const std::string name;
    Semaphore	      started, finished;
    pthread_t	      thread;

    bool              caught_exception;
};

class ThreadMap {
public:
  typedef std::map<pthread_t,std::string> mapType;

  static ThreadMap &instance();

  void report();

  class ScopedRegistration {
  public:
    ScopedRegistration( ThreadMap &tm, const std::string &desc ): tm(tm) {
      ScopedLock sl(tm.mutex);
      tm.map[id()] = desc;
    }

    ~ScopedRegistration() {
      ScopedLock sl(tm.mutex);
      tm.map.erase(id());
    }

  private:
    ThreadMap &tm;

    pthread_t id() const { return pthread_self(); }
  };

private:
  mapType map;
  Mutex   mutex;
};

template <typename T> inline Thread::Thread(T *object, void (T::*method)(), const std::string &name, const std::string &logPrefix, size_t stackSize)
:
  logPrefix(logPrefix),
  name(name),
  caught_exception(false)
{
  int retval;

  if (stackSize != 0) {
    pthread_attr_t attr;

    if ((retval = pthread_attr_init(&attr)) != 0)
      throw SystemCallException("pthread_attr_init", retval, THROW_ARGS);

    if ((retval = pthread_attr_setstacksize(&attr, stackSize)) != 0)
      throw SystemCallException("pthread_attr_setstacksize", retval, THROW_ARGS);

    if ((retval = pthread_create(&thread, &attr, &Thread::stub<T>, new Args<T>(object, method, this, name))) != 0)
      throw SystemCallException("pthread_create", retval, THROW_ARGS);

    if ((retval = pthread_attr_destroy(&attr)) != 0)
      throw SystemCallException("pthread_attr_destroy", retval, THROW_ARGS);
  } else {
    if ((retval = pthread_create(&thread, 0, &Thread::stub<T>, new Args<T>(object, method, this, name))) != 0)
      throw SystemCallException("pthread_create", retval, THROW_ARGS);
  }
}


inline Thread::~Thread()
{
  ScopedDelayCancellation dc; // pthread_join is a cancellation point

  int retval;

  if ((retval = pthread_join(thread, 0)) != 0)
    try {
      throw SystemCallException("pthread_join", retval, THROW_ARGS);
    } catch (Exception &ex) {
      LOG_ERROR_STR("Exception in destructor: " << ex);
    }
}


inline void Thread::cancel()
{
  started.down();
  started.up(); // allow multiple cancels

  (void)pthread_cancel(thread); // could return ESRCH ==> ignore
}


inline void Thread::cancel(const struct timespec &timespec)
{
  try {
    if (!wait(timespec))
      cancel();
  } catch(...) {
    // Guarantee cancel() will be called.
    cancel();
    throw;
  }
}


inline void Thread::wait()
{
  finished.down();
  finished.up(); // allow multiple waits
}


inline bool Thread::wait(const struct timespec &timespec)
{
  bool ok = finished.tryDown(1, timespec);

  if (ok)
    finished.up(); // allow multiple waits

  return ok;
}


inline bool Thread::isDone()
{
  struct timespec deadline = { 0, 0 };

  return wait(deadline);
}


inline bool Thread::caughtException()
{
  // thread must be finished for caught_exception to make sense
  wait();

  return caught_exception;
}


template <typename T> inline void Thread::stub(Args<T> *args)
{
  // (un)register WITHIN the thread, since the thread id
  // can be reused once the thread finishes.
  Cancellation::ScopedRegisterThread rt;

  ThreadLogger threadLogger;

  LOG_DEBUG_STR(logPrefix << "Thread started");

  ThreadMap::ScopedRegistration sr(ThreadMap::instance(), args->name);

  try {
#if defined(_LIBCPP_VERSION)
    int retval;

    // Set name WITHIN the thread, to avoid race conditions
    if ((retval = pthread_setname_np(args->name.substr(0,15).c_str())) != 0)
      throw SystemCallException("pthread_setname_np", retval, THROW_ARGS);
#else
# if defined(_GNU_SOURCE) && __GLIBC_PREREQ(2, 12)
    int retval;

    // Set name WITHIN the thread, to avoid race conditions
    if ((retval = pthread_setname_np(pthread_self(), args->name.substr(0,15).c_str())) != 0)
      throw SystemCallException("pthread_setname_np", retval, THROW_ARGS);
# endif
#endif

    // allow cancellation from here, to guarantee finished.up()
    started.up();

    (args->object->*args->method)();
  } catch (Exception &ex) {
    // split exception message into lines to be able to add the logPrefix to each line
    std::vector<std::string> exlines;
    std::ostringstream	     exstrs;
    exstrs << ex;
    std::string		     exstr = exstrs.str();

    boost::split(exlines, exstr, boost::is_any_of("\n"));
    LOG_ERROR_STR(logPrefix << "Caught Exception: " << exlines[0]);

    for (unsigned i = 1; i < exlines.size(); i ++)
      LOG_ERROR_STR(logPrefix << exlines[i]);

    caught_exception = true;
  } catch (std::exception &ex) {
    LOG_ERROR_STR(logPrefix << "Caught std::exception: " << ex.what());

    caught_exception = true;
  } catch (...) {
    LOG_DEBUG_STR(logPrefix << "Thread cancelled");

    finished.up();

    throw;
  }

  finished.up();

  LOG_DEBUG_STR(logPrefix << "Thread stopped");
}

template <typename T> inline void *Thread::stub(void *arg)
{
  std::auto_ptr<Args<T> > args(static_cast<Args<T> *>(arg));
  args->thread->stub(args.get());
  return 0;
}


} // namespace LOFAR

#endif

#endif

//#  Thread.h:
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

#ifndef LOFAR_LCS_THREAD_THREAD_H
#define LOFAR_LCS_THREAD_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <pthread.h>
#include <signal.h>

#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <Thread/Semaphore.h>
#include <Thread/Cancellation.h>

#include <boost/algorithm/string.hpp>

#include <string>
#include <vector>

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
      
    template <typename T> Thread(T *object, void (T::*method)(), const std::string &logPrefix = "", size_t stackSize = 0);
			  ~Thread(); // join a thread

    void		  cancel();

    void		  wait();
    bool		  wait(const struct timespec &);

  private:
    template <typename T> struct Args {
      Args(T *object, void (T::*method)(), Thread *thread) : object(object), method(method), thread(thread) {}

      T	     *object;
      void   (T::*method)();
      Thread *thread;
    };

    template <typename T> void	      stub(Args<T> *);
    template <typename T> static void *stub(void *);

    const std::string logPrefix;
    Semaphore	      finished;
    pthread_t	      thread;
};


template <typename T> inline Thread::Thread(T *object, void (T::*method)(), const std::string &logPrefix, size_t stackSize)
:
  logPrefix(logPrefix)
{
  int retval;

  if (stackSize != 0) {
    pthread_attr_t attr;

    if ((retval = pthread_attr_init(&attr)) != 0)
      throw SystemCallException("pthread_attr_init", retval, THROW_ARGS);

    if ((retval = pthread_attr_setstacksize(&attr, stackSize)) != 0)
      throw SystemCallException("pthread_attr_setstacksize", retval, THROW_ARGS);

    if ((retval = pthread_create(&thread, &attr, &Thread::stub<T>, new Args<T>(object, method, this))) != 0)
      throw SystemCallException("pthread_create", retval, THROW_ARGS);

    if ((retval = pthread_attr_destroy(&attr)) != 0)
      throw SystemCallException("pthread_attr_destroy", retval, THROW_ARGS);
  } else {
    if ((retval = pthread_create(&thread, 0, &Thread::stub<T>, new Args<T>(object, method, this))) != 0)
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
      LOG_FATAL_STR("Exception in destructor: " << ex);
    }
}


inline void Thread::cancel()
{
  pthread_cancel(thread); // could return ESRCH ==> ignore
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


template <typename T> inline void Thread::stub(Args<T> *args)
{
  pthread_t myid = pthread_self(); // this->thread might not be initialised yet
  Cancellation::register_thread(myid);

  try {
    (args->object->*args->method)();
  } catch (Exception &ex) {
    // split exception message into lines to be able to add the logPrefix to each line
    std::vector<std::string> exlines;
    std::ostringstream	     exstrs;
    exstrs << ex;
    std::string		     exstr = exstrs.str();

    boost::split(exlines, exstr, boost::is_any_of("\n"));
    LOG_FATAL_STR(logPrefix << "Caught Exception: " << exlines[0]);

    for (unsigned i = 1; i < exlines.size(); i ++)
      LOG_FATAL_STR(logPrefix << exlines[i]);
  } catch (std::exception &ex) {
    LOG_FATAL_STR(logPrefix << "Caught std::exception: " << ex.what());
  } catch (...) {
    LOG_DEBUG_STR(logPrefix << "Cancelled");

    finished.up();
    Cancellation::unregister_thread(myid);  
    throw;
  }

  finished.up();

  // unregister WITHIN the thread, since the thread id
  // can be reused once the thread finishes.
  Cancellation::unregister_thread(myid);  
}


template <typename T> inline void *Thread::stub(void *arg)
{
  std::auto_ptr<Args<T> > args(static_cast<Args<T> *>(arg));
  args->thread->stub(args.get());
  return 0;
}


} // namespace LOFAR

#endif

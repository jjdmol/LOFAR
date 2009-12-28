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

#ifndef LOFAR_RTCP_INTERFACE_THREAD_H
#define LOFAR_RTCP_INTERFACE_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <Common/LofarLogger.h>
#include <Stream/SystemCallException.h>


namespace LOFAR {
namespace RTCP {


class Thread
{
  public:
    // Spawn a method by creating a Thread object as follows:
    //
    // class C {
    //   public:
    //     C() : thread(this, &C::method) { thread.start(); }
    //   private:
    //     void method() { std::cout << "runs asynchronously" << std::endl; }
    //     Thread thread;
    // };
    //
    // The thread is joined in the destructor of the Thread object (or detached
    // if the thread deletes itself)
     
    // object:          object containing the thread function to run
    // method:          name of the thread function
    // name:            name of the thread, mentioned in error logs
    // stackSize:       stack size to assign

    template <typename T> Thread(T *object, void (T::*method)(), string name = "", size_t stackSize = 0);

    // join/detach a thread
    ~Thread();

    // start the thread
    void start();

    // send signal
    void kill(int sig);

    // abort the thread and wait for it to die
    void abort();

    // name of the thread
    const string name;

    // true when the thread should wrap up
    volatile bool stop;

  private:
    // thread state
    bool started;
    volatile bool stopped;

    // capture SIGUSR1 for use in abort()
    static void setSigHandler();
    static void sigHandler(int);

    // thread parameters
    pthread_attr_t attr;
    pthread_t thread;
    void *arg;

    // thread wrapper function
    template <typename T> static void *stub(void *);

    // deletes any templated data, to be called by the destructor
    template <typename T> static void *destructTemplate(Thread *);

    // pointers to templated functions
    void *(*stubInstance)(void*);
    void *(*destructTemplateInstance)(Thread *);

    // all parameters that we need in stub()
    template <typename T> struct stubParams {
      T *object;
      void (T::*method)();
      Thread *thread;
    };

};


template <typename T> inline Thread::Thread(T *object, void (T::*method)(), string name, size_t stackSize)
:
  name(name),
  stop(false),
  started(false),
  stopped(false),
  stubInstance(&Thread::stub<T>),
  destructTemplateInstance(&Thread::destructTemplate<T>)
{
  int retval;

  setSigHandler();

  stubParams<T> *params = new stubParams<T>();
  params->object = object;
  params->method = method;
  params->thread = this;
  arg = params;

  if ((retval = pthread_attr_init(&attr)) != 0)
    throw SystemCallException("pthread_attr_init", retval, THROW_ARGS);

  if (stackSize > 0) {
    if ((retval = pthread_attr_setstacksize(&attr, stackSize)) != 0)
      throw SystemCallException("pthread_attr_setstacksize", retval, THROW_ARGS);
  }
}


inline void Thread::start()
{
  // we do not start the thread in the constructor for two reasons:
  // 1) the constructor might throw an exception after thread start,
  //    leaving the object unconstructed but the thread started
  // 2) the started thread might want to refer to this thread, which
  //    is not necessarily constructed yet when the thread is started,
  //    leading to a race condition between
  //        t = new Thread(...)
  //    and accessing (for instance) t->stop in the started thread

  int retval;

  if ((retval = pthread_create(&thread, &attr, stubInstance, arg)) != 0)
    throw SystemCallException("pthread_create", retval, THROW_ARGS);

  started = true;
}


inline Thread::~Thread()
{
  int retval;

  if (started) {
    if (thread == pthread_self()) {
      if ((retval = pthread_detach(thread)) != 0)
        throw SystemCallException("pthread_detach", retval, THROW_ARGS);
    } else {
      if ((retval = pthread_join(thread, 0)) != 0)
        throw SystemCallException("pthread_join", retval, THROW_ARGS);
    }
  }

  if ((retval = pthread_attr_destroy(&attr)) != 0)
    throw SystemCallException("pthread_attr_destroy", retval, THROW_ARGS);

  destructTemplateInstance( this );
}


inline void Thread::kill(int sig)
{
  int retval;

  if ((retval = pthread_kill(thread, sig)) != 0)
    throw SystemCallException("pthread_kill", retval, THROW_ARGS);
}


inline void Thread::abort()
{
  stop = true;

  while (!stopped) {
    try {
      kill(SIGUSR1); // interrupt blocking system call
      usleep(25000);
    } catch (SystemCallException &) {
      // ignore the case that the thread just exited
    }  
  }

  // our destructor will do the pthread_join
}


template <typename T> inline void *Thread::stub(void *arg)
{
  stubParams<T> *params = static_cast<stubParams<T>*>(arg);
  T	                *object        = params->object;
  void			(T::*method)() = params->method;
  Thread                *thread        = params->thread;

  try {
    (object->*method)();
  } catch (Exception &ex) {
    LOG_FATAL_STR("Thread " << thread->name << " caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("Thread " << thread->name << " caught Exception: " << ex.what());
  } catch (...) {
    LOG_FATAL_STR("Thread " << thread->name << " caught non-std::exception");
  }

  thread->stopped = true;

  return 0;
}


template <typename T> inline void *Thread::destructTemplate(Thread *thread)
{
  stubParams<T> *params = static_cast<stubParams<T>*>(thread->arg);

  delete params;
}

} // namespace RTCP
} // namespace LOFAR

#endif

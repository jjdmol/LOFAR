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

#include <Common/LofarLogger.h>
#include <Common/Semaphore.h>
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
    //     C() : thread(this, &C::method) {}
    //   private:
    //     void method() { std::cout << "runs asynchronously" << std::endl; }
    //     Thread thread;
    // };
    //
    // The thread is joined in the destructor of the Thread object (or detached
    // if the thread deletes itself)
      
    template <typename T> Thread(T *object, void (T::*method)(), size_t stackSize = 0);

    // join/detach a thread
    ~Thread();

  protected:
    template <typename T> struct Args {
      Args(T *object, void (T::*method)()) : object(object), method(method) {}

      T	   *object;
      void (T::*method)();
    };

    template <typename T>	      Thread(void * (*stub)(void *), Args<T> *args, size_t stackSize);
    template <typename T> static void *stub(void *);

    pthread_t thread;

  private:
    template <typename T> void	      create(void * (*stub)(void *), Args<T> *args, size_t stackSize);
};


class InterruptibleThread : public Thread
{
  public:
    template <typename T> InterruptibleThread(T *object, void (T::*method)(), size_t stackSize = 0);
			  ~InterruptibleThread();

    void		  abort();

  private:
    struct ExitState {
      ExitState() : finished(false) {}

      Semaphore     mayExit;
      volatile bool finished;
    };

    template <typename T> struct Args : public Thread::Args<T> {
      Args(T *object, void (T::*method)(), ExitState *exitState) : Thread::Args<T>(object, method), exitState(exitState) {}

      ExitState *exitState;
    };

    template <typename T> static void *stub(void *);

    void			      installSignalHandler();
    static void			      signalHandler(int sig);

    ExitState			      *exitState;
};


template <typename T> inline Thread::Thread(T *object, void (T::*method)(), size_t stackSize)
{
  create(&Thread::stub<T>, new Args<T>(object, method), stackSize);
}


template <typename T> inline Thread::Thread(void * (*stub)(void *), Args<T> *args, size_t stackSize)
{
  create(stub, args, stackSize);
}


template <typename T> inline void Thread::create(void * (*stub)(void *), Args<T> *args, size_t stackSize)
{
  int retval;

  if (stackSize != 0) {
    pthread_attr_t attr;

    if ((retval = pthread_attr_init(&attr)) != 0)
      throw SystemCallException("pthread_attr_init", retval, THROW_ARGS);

    if ((retval = pthread_attr_setstacksize(&attr, stackSize)) != 0)
      throw SystemCallException("pthread_attr_setstacksize", retval, THROW_ARGS);

    if ((retval = pthread_create(&thread, &attr, stub, args)) != 0)
      throw SystemCallException("pthread_create", retval, THROW_ARGS);

    if ((retval = pthread_attr_destroy(&attr)) != 0)
      throw SystemCallException("pthread_attr_destroy", retval, THROW_ARGS);
  } else {
    if ((retval = pthread_create(&thread, 0, stub, args)) != 0)
      throw SystemCallException("pthread_create", retval, THROW_ARGS);
  }
}


inline Thread::~Thread()
{
  int retval;

  if (thread == pthread_self()) {
    if ((retval = pthread_detach(thread)) != 0)
      throw SystemCallException("pthread_detach", retval, THROW_ARGS);
  } else {
    if ((retval = pthread_join(thread, 0)) != 0)
      throw SystemCallException("pthread_join", retval, THROW_ARGS);
  }
}


template <typename T> inline InterruptibleThread::InterruptibleThread(T *object, void (T::*method)(), size_t stackSize)
:
  Thread(&InterruptibleThread::stub<T>, (exitState = new ExitState, new Args<T>(object, method, exitState)), stackSize)
{
  installSignalHandler();
}


inline InterruptibleThread::~InterruptibleThread()
{
  exitState->mayExit.up();
}


template <typename T> inline void *Thread::stub(void *arg)
{
  Args<T> *args = static_cast<Args<T> *>(arg);

  try {
    (args->object->*args->method)();
  } catch (Exception &ex) {
    LOG_FATAL_STR("caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("caught non-std::exception");
  }

  delete args;

  return 0;
}


template <typename T> inline void *InterruptibleThread::stub(void *arg)
{
  Args<T>   *args      = static_cast<Args<T> *>(arg);
  ExitState *exitState = args->exitState;

  Thread::stub<T>(arg);

  exitState->finished = true;
  exitState->mayExit.down(); // make sure that no signal is sent to a thread that just exited
  delete exitState;

  return 0;
}


inline void InterruptibleThread::installSignalHandler()
{
  struct sigaction sa;
  int		   retval;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags   = 0;
  sa.sa_handler = signalHandler;

  if ((retval = sigaction(SIGUSR1, &sa, 0)) != 0)
    throw SystemCallException("sigaction", retval, THROW_ARGS);
}


inline void InterruptibleThread::signalHandler(int)
{
}


inline void InterruptibleThread::abort()
{
  // interrupt I/O-related system calls
  assert(thread != pthread_self());

  while (!exitState->finished) {
    int retval;

    if ((retval = pthread_kill(thread, SIGUSR1)) != 0)
      throw SystemCallException("pthread_kill", retval, THROW_ARGS);

    usleep(25000);
  }
}


} // namespace RTCP
} // namespace LOFAR

#endif

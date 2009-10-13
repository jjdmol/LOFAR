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
      
    template <typename T> Thread(T *object, void (T::*method)());
    template <typename T> Thread(T *object, void (T::*method)(), size_t stackSize);

    // join/detach a thread
    ~Thread();

    // send signal
    void kill(int sig);

  private:
    template <typename T> static void *stub(void *);

    pthread_t thread;
};


template <typename T> inline Thread::Thread(T *object, void (T::*method)())
{
  if (pthread_create(&thread, 0, &Thread::stub<T>, new std::pair<T *, void (T::*)()>(object, method)) != 0)
    throw SystemCallException("pthread_create", errno, THROW_ARGS);
}


template <typename T> inline Thread::Thread(T *object, void (T::*method)(), size_t stackSize)
{
  pthread_attr_t attr;

  if (pthread_attr_init(&attr) != 0)
    throw SystemCallException("pthread_attr_init", errno, THROW_ARGS);

  if (pthread_attr_setstacksize(&attr, stackSize) != 0)
    throw SystemCallException("pthread_attr_setstacksize", errno, THROW_ARGS);

  if (pthread_create(&thread, &attr, &Thread::stub<T>, new std::pair<T *, void (T::*)()>(object, method)) != 0)
    throw SystemCallException("pthread_create", errno, THROW_ARGS);

  if (pthread_attr_destroy(&attr) != 0)
    throw SystemCallException("pthread_attr_destroy", errno, THROW_ARGS);
}


inline Thread::~Thread()
{
  if (thread == pthread_self()) {
    if (pthread_detach(thread) != 0)
      throw SystemCallException("pthread_detach", errno, THROW_ARGS);
  } else {
    if (pthread_join(thread, 0) != 0)
      throw SystemCallException("pthread_join", errno, THROW_ARGS);
  }
}


inline void Thread::kill(int sig)
{
  if (pthread_kill(thread, sig) != 0)
    throw SystemCallException("pthread_kill", errno, THROW_ARGS);
}


template <typename T> inline void *Thread::stub(void *arg)
{
  std::pair<T *, void (T::*)()> *object_method = static_cast<std::pair<T *, void (T::*)()> *>(arg);
  T				*object	       = object_method->first;
  void				(T::*method)() = object_method->second;

  delete object_method;

  try {
    (object->*method)();
  } catch (Exception &ex) {
    LOG_FATAL_STR("caught Exception: " << ex);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("caught non-std::exception");
  }

  return 0;
}


} // namespace RTCP
} // namespace LOFAR

#endif

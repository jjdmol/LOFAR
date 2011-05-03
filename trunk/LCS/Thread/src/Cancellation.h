//#  Cancellation.h:
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
//#  $Id: Thread.h 16592 2010-10-22 13:04:23Z mol $

#ifndef LOFAR_LCS_THREAD_CANCELLATION_H
#define LOFAR_LCS_THREAD_CANCELLATION_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#ifdef USE_THREADS
#include <Common/LofarLogger.h>
#include <Thread/Mutex.h>

#include <pthread.h>
#include <map>
#endif

namespace LOFAR {


class Cancellation {
public:
  // allows cancellation here explicitly
  static void point();

  // return the current state of cancellability of this thread.
  static bool get();

  // push_disable() and pop_disable() maintain a reference count on how many objects
  // requested disabling cancellation. After the first push_disable(), cancellation
  // is disabled until the last pop_disable(), after which the original cancellation
  // state is restored.
  static void push_disable();
  static void pop_disable();

#ifdef USE_THREADS
  // register threads explicitly to avoid the state maps growing unbounded
  static void register_thread( pthread_t id );
  static void unregister_thread( pthread_t id );
#endif

private:  
  // set (enable or disable) the cancellability of this thread.
  // returns the previous value.
  static bool set( bool enable );

  static bool disable() { return set( false ); }
  static bool enable()  { return set( true ); }

#ifdef USE_THREADS
  struct thread_state {
    unsigned refcount;
    int oldstate;

    thread_state(): refcount(0), oldstate(false) {}
  };

  static std::map<pthread_t, struct thread_state> thread_states; 
  static Mutex mutex;
#endif  
};


class ScopedDelayCancellation {
public:
  ScopedDelayCancellation() {
    Cancellation::push_disable();
  };

  ~ScopedDelayCancellation() {
    Cancellation::pop_disable();
  }
};


#ifdef USE_THREADS
class ScopedRegisterThread {
public:
  ScopedRegisterThread(): id(pthread_self()) {
    Cancellation::register_thread( id );
  };

  ~ScopedRegisterThread() {
    Cancellation::unregister_thread( id );
  }
private:
  pthread_t id;
};
#endif


inline void Cancellation::point() {
#ifdef USE_THREADS
  pthread_testcancel();
#endif  
}


inline bool Cancellation::set( bool enable ) {
#ifdef USE_THREADS
  int oldState;

  pthread_setcanceltype( enable ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, &oldState );

  return oldState == PTHREAD_CANCEL_ENABLE;
#else
  (void)enable;

  // return whatever value we consider default if no threads are used
  return get();
#endif  
}


inline bool Cancellation::get() {
#ifdef USE_THREADS
  // pthread_getcanceltype doesn't exist unfortunately, so emulate it.
  bool state = disable(); // never accidently toggle to enabled!

  set( state );
  return state;
#else
  return false;
#endif  
}


inline void Cancellation::push_disable() {
#ifdef USE_THREADS
  pthread_t myid = pthread_self();

  ScopedLock sl(mutex);

  // the main thread cannot be registered before
  // other code triggers a push_disable(), so be nice
  // and don't check for it.

  // map::operator[] will call the default constructor
  // if myid is not in the map
  struct thread_state &state = thread_states[myid];

  if (state.refcount++ == 0)
    state.oldstate = disable();
#endif  
}


inline void Cancellation::pop_disable() {
#ifdef USE_THREADS
  pthread_t myid = pthread_self();

  ScopedLock sl(mutex);

  // by now, the thread should be in the list, put there by push_disable
  ASSERT( thread_states.find(myid) != thread_states.end() );

  struct thread_state &state = thread_states[myid];

  ASSERT( state.refcount > 0 );

  if (--state.refcount == 0)
    set( state.oldstate );
#endif    
}


#ifdef USE_THREADS
inline void Cancellation::register_thread( pthread_t id ) {
  ScopedLock sl(mutex);

  ASSERT( thread_states.find(id) == thread_states.end() );

  thread_states[id] = thread_state();
}
#endif  


#ifdef USE_THREADS
inline void Cancellation::unregister_thread( pthread_t id ) {
  ScopedLock sl(mutex);

  ASSERT( thread_states.find(id) != thread_states.end() );

  ASSERT( thread_states[id].refcount == 0 );

  thread_states.erase(id);
}
#endif  


} // namespace LOFAR

#endif

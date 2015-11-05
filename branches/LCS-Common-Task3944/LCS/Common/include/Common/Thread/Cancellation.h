//#  Cancellation.h: Control whether the current thread can be cancelled by pthread_cancel
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

#ifndef LOFAR_LCS_COMMON_CANCELLATION_H
#define LOFAR_LCS_COMMON_CANCELLATION_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#ifdef USE_THREADS
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

  // The set/disable/enable functions interfere with the reference counting
  // of the routines above. Use with extreme caution.

  // set (enable or disable) the cancellability of this thread.
  // returns the previous value.
  static bool set( bool enable );

  static bool disable() { return set( false ); }
  static bool enable()  { return set( true ); }

  class ScopedRegisterThread {
#ifdef USE_THREADS
  public:
    ScopedRegisterThread(): id(pthread_self()) {
      register_thread( id );
    };

    ~ScopedRegisterThread() {
      unregister_thread( id );
    }
  private:
    const pthread_t id;
#endif
  };

private:  
  Cancellation();
  Cancellation(const Cancellation&);
  Cancellation& operator=(const Cancellation&);

#ifdef USE_THREADS
  struct thread_state {
    unsigned refcount;
    int oldstate;

    thread_state(): refcount(0), oldstate(false) {}
  };

  typedef std::map<pthread_t, struct thread_state> thread_states_t;

  static thread_states_t& getThreadStates(); 

  // register threads explicitly to avoid the state maps growing unbounded
  // note that the main thread won't be registered since there is no way
  // to do that before ScopedDelayCancellation is used by some global
  // object initialization.
  static void register_thread( pthread_t id );
  static void unregister_thread( pthread_t id );

  static pthread_mutex_t mutex; // can't use Mutex class due to cyclical references through Exception.h

  class ScopedLock {
  public:
    ScopedLock() {
      pthread_mutex_lock( &mutex );
    }
    ~ScopedLock() {
      pthread_mutex_unlock( &mutex );
    }
  };
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
private:
  ScopedDelayCancellation(const ScopedDelayCancellation&);
  ScopedDelayCancellation& operator=(const ScopedDelayCancellation&);
};




inline void Cancellation::point() {
#ifdef USE_THREADS
  pthread_testcancel();
#endif  
}


inline bool Cancellation::set( bool enable ) {
#ifdef USE_THREADS
  int oldState;

  pthread_setcancelstate( enable ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, &oldState );

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

  ScopedLock sl;

  // the main thread cannot be registered before
  // other code triggers a push_disable(), so be nice
  // and don't check for it.

  // map::operator[] will call the default constructor
  // if myid is not in the map
  struct thread_state &state = getThreadStates()[myid];

  if (state.refcount++ == 0)
    state.oldstate = disable();
#endif  
}


inline void Cancellation::pop_disable() {
#ifdef USE_THREADS
  pthread_t myid = pthread_self();

  ScopedLock sl;

  struct thread_state &state = getThreadStates()[myid];

  if (--state.refcount == 0)
    set( state.oldstate );
#endif    
}


#ifdef USE_THREADS
inline void Cancellation::register_thread( pthread_t id ) {
  ScopedLock sl;

  getThreadStates()[id] = thread_state();
}
#endif  


#ifdef USE_THREADS
inline void Cancellation::unregister_thread( pthread_t id ) {
  ScopedLock sl;

  getThreadStates().erase(id);
}
#endif  


} // namespace LOFAR

#endif

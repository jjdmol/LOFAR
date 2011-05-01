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
#include <pthread.h>
#include <map>
#endif

namespace LOFAR {


class Cancellation {
public:
  // allows cancellation here explicitly
  static void point();

  // set (enable or disable) the cancellability of this thread.
  // returns the previous value.
  static bool set( bool enable );

  // return the current state of cancellability of this thread.
  static bool get();

  static bool disable() { return set( false ); }
  static bool enable()  { return set( true ); }


  // push_disable() and pop_disable() maintain a reference count on how many objects
  // requested disabling cancellation. After the first push_disable(), cancellation
  // is disabled until the last pop_disable(), after which the original cancellation
  // state is restored.
  static void push_disable();
  static void pop_disable();

private:
#ifdef USE_THREADS
  static std::map<pthread_t, unsigned> refcounts; 
  static std::map<pthread_t, int> oldstates; 
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

  // can't use refcounts[myid] directly since the default constructor of unsigned
  // won't set it to 0 if myid is not in refcounts
  unsigned oldcount = refcounts.find(myid) == refcounts.end() ? 0 : refcounts[myid];

  refcounts[myid] = oldcount + 1;

  if (oldcount == 0)
    oldstates[myid] = disable();
#endif  
}


inline void Cancellation::pop_disable() {
#ifdef USE_THREADS
  pthread_t myid = pthread_self();

  if (--refcounts[myid] == 0)
    set( oldstates[myid] );
#endif    
}


} // namespace LOFAR

#endif

//# tThread.cc: Test program for Thread
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id: tSingleton.cc 14057 2009-09-18 12:26:29Z diepen $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h>

//# Includes
#include <Common/Thread/Cancellation.h>
#include <Common/lofar_iostream.h>
#include <unistd.h>
#include <time.h>

using namespace LOFAR;

void test_simple() {
  bool cancellable = Cancellation::get(); // we should always return to this state

  // disable/set pair (we can't ensure that cancellability can ever be enabled)
  Cancellation::disable();
  ASSERT( !Cancellation::get() );
  Cancellation::set( cancellable );
  ASSERT( Cancellation::get() == cancellable );

  // push/pop pair
  Cancellation::push_disable();
  ASSERT( !Cancellation::get() );
  Cancellation::pop_disable();
  ASSERT( Cancellation::get() == cancellable );

  // constructor/destructor pair
  {
    ScopedDelayCancellation dc;

    ASSERT( !Cancellation::get() );
  }
  ASSERT( Cancellation::get() == cancellable );

  // this should be harmless
  Cancellation::point();
}

#ifdef USE_THREADS

#include <Common/Thread/Thread.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Semaphore.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Queue.h>

Semaphore s,t;
Mutex m;

class A {
public:
  void preamble() {
    cancelled = false;

    // signal main thread that we're ready
    s.up();
  }

  void delayCancellation() {
    {
      ScopedDelayCancellation dc;

      preamble();

      // wait for main thread to cancel us
      t.down();

      // <-- we were now cancelled
      cancelled = true;
    }

    Cancellation::point(); // will trigger cancellation

    cancelled = false; // should be dead code
  }

  void cancelCondition() {
    preamble();

    Condition c;

    try {
      ScopedLock sl(m);

      // blocks, but should be cancellable
      c.wait(m);
    } catch (std::exception &) {
    } catch (...) {
      cancelled = true;
      throw;
    }
  }

  void cancelSemaphore() {
    preamble();

    try {
      // blocks, but should be cancellable
      t.down();
    } catch (std::exception &) {
    } catch (...) {
      cancelled = true;
      throw;
    }
  }

  void cancelQueue() {
    preamble();

    try {
      Queue<int> q;

      // blocks, but should be cancellable
      (void)q.remove();
    } catch (std::exception &) {
    } catch (...) {
      cancelled = true;
      throw;
    }
  }

  bool cancelled;
};

void test_mt() {
  A a;

  {
    Thread ta(&a,&A::delayCancellation);

    s.down();
    ta.cancel();
    t.up();
  }
  ASSERT( a.cancelled );

  {
    Thread ta(&a,&A::cancelCondition);

    s.down();
    ta.cancel();
  }
  ASSERT( a.cancelled );
  ASSERT( m.trylock() ); // m, the mutex in the tested condition, should have been automatically unlocked
  m.unlock();

  {
    Thread ta(&a,&A::cancelSemaphore);

    s.down();
    ta.cancel();
  }
  ASSERT( a.cancelled );

  {
    Thread ta(&a,&A::cancelQueue);

    s.down();
    ta.cancel();
  }
  ASSERT( a.cancelled );
}

#endif


int main()
{
  INIT_LOGGER("tCancellation");
  LOG_INFO("Starting up...");

  // kill any deadlocks
  alarm(10);

  test_simple();
#ifdef USE_THREADS  
  test_mt();
#endif  

  LOG_INFO("Program terminated successfully");
  return 0;
}

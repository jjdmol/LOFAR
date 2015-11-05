//# tMutex.cc: Test program for Mutexes
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

//# Includes
#include <Common/Thread/Thread.h>
#include <Common/Thread/Mutex.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <unistd.h>

// BARRIER forces the compiler and CPU to stay in-order. An external function
// call should do the job.
#define BARRIER         sleep(0)

using namespace LOFAR;

// try simple functionality which should succeed even if routines
// are a no-op (!USE_THREADS)
void test_simple() {
  Mutex mutex;
  
  mutex.lock();
  mutex.unlock();

  mutex.trylock();
  mutex.unlock();

  {
    ScopedLock sl(mutex);
  }
}

void test_trylock() {
#ifdef USE_THREADS  
  Mutex mutex;
  
  ASSERT( mutex.trylock() );
  mutex.unlock();

  mutex.lock();
  ASSERT( !mutex.trylock() ); // locks are non-recursive
  mutex.unlock();

  {
    ScopedLock sl(mutex);
    ASSERT( !mutex.trylock() );
  }
#endif  
}

#ifdef USE_THREADS
Mutex mutex;
volatile bool a_started = false;
volatile bool b_started = false;
volatile unsigned counter = 0;

class A {
public:
  void mainLoop() {
    ScopedLock sl(mutex);

    a_started = true;

    BARRIER;

    while( !b_started )
      BARRIER;

    BARRIER;  

    // <--- both threads are started, A has lock  

    sleep(1); // make sure we are slow (there are no guarantees however -- B could be stalled even more 
    counter = 1;
  }
};
class B {
public:
  void mainLoop() {
    ASSERT( counter == 0 ); // A could not have incremented yet

    b_started = true;

    BARRIER;

    while( !a_started )
      BARRIER;

    BARRIER;  

    // <--- both threads are started, A has lock  

    mutex.lock();
    ASSERT( counter == 1 ); // A has to have incremented
    mutex.unlock();
  }
};
#endif

void test_criticalsection() {
#ifdef USE_THREADS

  A a;
  B b;

  {
    Thread ta(&a,&A::mainLoop);
    Thread tb(&b,&B::mainLoop);
  }  
#endif
}

int main()
{
  INIT_LOGGER("tMutex");
  LOG_INFO("Starting up...");

  // kill any deadlocks
  alarm(10);

  test_simple();
  test_trylock();
  test_criticalsection();

#ifdef USE_THREADS
#else
#endif
  LOG_INFO("Program terminated successfully");
  return 0;
}

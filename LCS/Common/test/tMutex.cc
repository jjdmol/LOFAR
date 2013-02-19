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
#include <Common/SystemCallException.h>
#include <unistd.h>

// BARRIER forces the compiler and CPU to stay in-order. An external function
// call should do the job.
#define BARRIER         sleep(0)

#define TEST_ALL(x) \
  std::cerr << "TEST_ALL(" << #x << ")" << std::endl; \
  test_simple(x); \
  test_trylock(x); \
  test_recursive_lock(x); \
  test_criticalsection(x);

using namespace LOFAR;

// try simple functionality which should succeed even if routines
// are a no-op (!USE_THREADS)
void test_simple(Mutex::Type type) {
  Mutex mutex(type);
 
  // lock + unlock must succeed
  mutex.lock();
  mutex.unlock();

  // trylock + unlock must succeed
  mutex.trylock();
  mutex.unlock();

  {
    // ScopedLock must not block if mutex is available
    ScopedLock sl(mutex);
  }
}

void relock_trylock(Mutex::Type type, Mutex &mutex) {
  switch(type) {
    default:
      ASSERT( !mutex.trylock() );
      break;

    case Mutex::RECURSIVE:
      // relocking using trylock works with a recursive lock
      ASSERT( mutex.trylock() );
      mutex.unlock();
      break;

    case Mutex::ERRORCHECK:
      // mutex.trylock() can cause a dead-lock in the same thread,
      // and throws an exception.

      try {
        mutex.trylock();

        ASSERTSTR(false, "lock + trylock must fail on error checked mutex");
      } catch (LOFAR::SystemCallException&) {
      }
      break;
  }
}

void test_trylock(Mutex::Type type) {
#ifdef USE_THREADS
  Mutex mutex(type);
 
  // trylock + unlock must succeed for ALL types
  ASSERT( mutex.trylock() );
  mutex.unlock();

  // try lock + relocking
  mutex.lock();
  relock_trylock(type, mutex);
  mutex.unlock();

  {
    // ScopedLock test
    ScopedLock sl(mutex);

    // try relocking
    relock_trylock(type, mutex);
  }
#else
  (void)type;
#endif  
}

void test_recursive_lock(Mutex::Type type)
{
#ifdef USE_THREADS
  Mutex mutex(type);
  ScopedLock sl(mutex);
  
  switch(type) {
  default:
    // Normal mutexes (the default) will dead-lock when locked recursively.
    // Don't attempt to do so.
    break;

  case Mutex::RECURSIVE:
    // Recursive lock must succeed on a recursive mutex
    {
      ScopedLock sl(mutex);
    }
    break;

  case Mutex::ERRORCHECK:
    // Recursive lock must fail with a SystemCallException on an error
    // checking mutex.
    try {
      ScopedLock sl(mutex);
      ASSERTSTR(false, "Recursive lock must fail on error checked mutex");
    } catch (SystemCallException&) {
    }
    break;
  }
#else
  (void)type;
#endif
}

#ifdef USE_THREADS
volatile bool a_started;
volatile bool b_started;
volatile unsigned counter;

class A {
  Mutex& mutex;
public:
  A(Mutex& mtx) : mutex(mtx) {}
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
  Mutex& mutex;
public:
  B(Mutex& mtx) : mutex(mtx) {}
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

void test_criticalsection(Mutex::Type type) {
#ifdef USE_THREADS
  Mutex mutex(type);
  a_started = false;
  b_started = false;
  counter = 0;
  
  A a(mutex);
  B b(mutex);

  {
    Thread ta(&a,&A::mainLoop);
    Thread tb(&b,&B::mainLoop);
  }  
#else
  (void)type;
#endif
}

int main()
{
  INIT_LOGGER("tMutex");
  LOG_INFO("Starting up...");

  // kill any deadlocks
  alarm(10);

  TEST_ALL(Mutex::NORMAL);
  TEST_ALL(Mutex::RECURSIVE);
  TEST_ALL(Mutex::ERRORCHECK);
  TEST_ALL(Mutex::DEFAULT);

#ifdef USE_THREADS
#else
#endif
  LOG_INFO("Program terminated successfully");
  return 0;
}

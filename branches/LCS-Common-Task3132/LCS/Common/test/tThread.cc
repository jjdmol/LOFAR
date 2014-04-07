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

#ifdef USE_THREADS

// BARRIER forces the compiler and CPU to stay in-order. An external function
// call should do the job.
#define BARRIER         sleep(0)

//# Includes
#include <Common/Thread/Thread.h>
#include <Common/lofar_iostream.h>
#include <unistd.h>
#include <time.h>
#include <exception>

using namespace LOFAR;

class F {
public:
  F(): member(42) {}

  void mainLoop() {
    ASSERT( member == 42 ); // make sure *this was carried over correctly
  }
private:
  int member;
};

class G {
public:
  void mainLoop() {
    while (1)
      sleep(1); // sleep() is a cancellation point
  }
};

class H {
public:
  H(): cancelled(false) {}
  void mainLoop() {
    try {
      while (1)
        sleep(1); // sleep() is a cancellation point
    } catch(std::exception &) {
    } catch(...) {
      cancelled = true;
      throw;
    }
  }

  bool cancelled;
};

void test_simple() {
  F f;
  G g;
  H h;

  {
    Thread t(&f,&F::mainLoop);
  }  

  {
    Thread t(&f,&F::mainLoop);

    t.wait();
  }

  {
    Thread t(&f,&F::mainLoop);

    struct timespec timespec = { time(0L), 0 };
    t.wait( timespec );
  }

  {
    Thread t(&f,&F::mainLoop);

    struct timespec timespec = { 0, 0 };
    t.wait( timespec );
  }

  {
    Thread t(&f,&F::mainLoop);

    struct timespec timespec = { time(0L)+1, 0 };
    t.wait( timespec );
  }

  {
    Thread t(&g,&G::mainLoop);
    t.cancel();
  }

  {
    Thread t(&h,&H::mainLoop);
    t.cancel();
  }
  ASSERT( h.cancelled );
}

// We can't count on mutexes etc to work, so use atomicity of word-sized
// variables and spinlocks.

volatile bool a_started = false;
volatile bool b_started = false;
volatile bool a_can_start = false;
volatile bool b_can_start = false;

class A {
public:
  void mainLoop() {
    while (!a_can_start)
      BARRIER;

    BARRIER;

    a_started = true;
    b_can_start = true;
  }
};

class B {
public:
  void mainLoop() {
    while (!b_can_start)
      BARRIER;

    BARRIER;  

    b_started = true;
  }
};

void test_mt() {
  A a;
  B b;

  {
    // First we start the threads, then the main thread triggers A, which triggers B,
    // which we check on. This scheme fails if the Thread class for example simply
    // executes the mainLoop functions without creating additional threads.
    Thread ta(&a,&A::mainLoop);
    Thread tb(&b,&B::mainLoop);

    BARRIER;

    a_can_start = true;
  }

  BARRIER;

  ASSERT( a_started );
  ASSERT( b_started );
}


int main()
{
  INIT_LOGGER("tThread");
  LOG_INFO("Starting up...");

  // kill any deadlocks
  alarm(10);

  test_simple();
  test_mt();

  LOG_INFO("Program terminated successfully");
  return 0;
}

#else // USE_THREADS

int main()
{
  INIT_LOGGER("tThread");
  LOG_INFO("Starting up...");
  LOG_INFO("Program terminated successfully");
  return 0;
}

#endif

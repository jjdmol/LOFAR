//# tCondition.cc: Test program for Condition
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

//# Includes
#include <Common/Thread/Thread.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>
#include <Common/lofar_iostream.h>
#include <unistd.h>
#include <time.h>

using namespace LOFAR;

void test_simple() {
  Condition cond;
  Mutex mutex;

  // these should not block

  cond.broadcast();

  cond.signal();

  {
    ScopedLock sl(mutex);
    struct timespec timespec = { time(0L), 0 };
    ASSERT( !cond.wait(mutex, timespec) );
  }

  {
    ScopedLock sl(mutex);
    struct timespec timespec = { 0, 0 };
    ASSERT( !cond.wait(mutex, timespec) );
  }

  // this should block for 1 second
  {
    ScopedLock sl(mutex);
    struct timespec timespec = { time(0L)+1, 0 };
    ASSERT( !cond.wait(mutex, timespec) );
  }
}

Condition cond;
Mutex mutex;
volatile bool barrier = false;

class A {
public:
  void mainLoop() {
    ScopedLock sl(mutex);

    barrier = true;

    cond.wait(mutex);
  }
};

class B {
public:
  void mainLoop() {
    // make sure we don't signal before A is waiting
    while( !barrier )
      continue;

    ScopedLock sl(mutex);

    cond.signal();
  }
};

void test_signalling() {
  A a;
  B b;

  {
    Thread ta(&a,&A::mainLoop);
    Thread tb(&b,&B::mainLoop);
  }  
}


int main()
{
  INIT_LOGGER("tCondition");
  LOG_INFO("Starting up...");

  // kill any deadlocks
  alarm(10);

  test_simple();
  test_signalling();

  LOG_INFO("Program terminated successfully");
  return 0;
}

#else // USE_THREADS

int main()
{
  INIT_LOGGER("tCondition");
  LOG_INFO("Starting up...");
  LOG_INFO("Program terminated successfully");
  return 0;
}

#endif

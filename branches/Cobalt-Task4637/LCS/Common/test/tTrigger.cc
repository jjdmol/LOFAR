//# tTrigger.cc: Test program for Semaphore
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
#include <Common/Thread/Trigger.h>
#include <Common/lofar_iostream.h>
#include <unistd.h>
#include <time.h>

using namespace LOFAR;

void test_simple() {
  {
    Trigger t;

    // these should not block
    t.trigger();
    t.wait();
  }

  {
    Trigger t;

    struct timespec timespec = { time(0L), 0 };
    ASSERT( !t.wait(timespec) );
  }

  {
    Trigger t;

    struct timespec timespec = { 0, 0 };
    ASSERT( !t.wait(timespec) );
  }

  // this should block for 1 second
  {
    Trigger t;

    struct timespec timespec = { time(0L)+1, 0 };
    ASSERT( !t.wait(timespec) );
  }
}

Trigger t;

class A {
public:
  void mainLoop() {
    t.wait();
  }
};

class B {
public:
  void mainLoop() {
    t.trigger();
  }
};

void test_signalling() {
  A a;
  B b1, b2;

  // one thread waiting
  {
    Thread ta(&a,&A::mainLoop);
    Thread tb(&b1,&B::mainLoop);
  }  

  // two threads waiting
  {
    Thread ta(&a,&A::mainLoop);
    Thread tb1(&b1,&B::mainLoop);
    Thread tb2(&b2,&B::mainLoop);
  }  
}


int main()
{
  INIT_LOGGER("tTrigger");
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
  INIT_LOGGER("tTrigger");
  LOG_INFO("Starting up...");
  LOG_INFO("Program terminated successfully");
  return 0;
}

#endif

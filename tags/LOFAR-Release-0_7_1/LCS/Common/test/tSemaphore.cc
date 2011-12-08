//# tSemaphore.cc: Test program for Semaphore
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
#include <Common/Thread/Semaphore.h>
#include <Common/lofar_iostream.h>
#include <unistd.h>
#include <time.h>

using namespace LOFAR;

void test_simple() {
  Semaphore s;

  // these should not block
  s.up();
  s.down();

  ASSERT( !s.tryDown() );

  s.up();
  ASSERT( s.tryDown() );

  s.up(2);
  s.down();
  s.down();

  s.up();
  s.up();
  s.down(2);

  {
    struct timespec timespec = { time(0L), 0 };
    ASSERT( !s.tryDown(1, timespec) );
  }

  {
    struct timespec timespec = { 0, 0 };
    ASSERT( !s.tryDown(1, timespec) );
  }

  // this should block for 1 second
  {
    struct timespec timespec = { time(0L)+1, 0 };
    ASSERT( !s.tryDown(1, timespec) );
  }
}

Semaphore s1, s2, s3;

class A {
public:
  void mainLoop() {
    s1.up();

    s2.up();
    s2.up();
    s2.noMore();

    s3.up();
    s3.noMore();
  }
};

class B {
public:
  void mainLoop() {
    s1.down();

    while( s2.down(1) )
      ;

    while( s3.down(2) )
      ;
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
  INIT_LOGGER("tSemaphore");
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
  INIT_LOGGER("tSemaphore");
  LOG_INFO("Starting up...");
  LOG_INFO("Program terminated successfully");
  return 0;
}

#endif

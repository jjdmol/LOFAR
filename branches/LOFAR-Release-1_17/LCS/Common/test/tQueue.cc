//# tQueue.cc: Test program for Queue
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
#include <Common/Thread/Queue.h>
#include <Common/lofar_iostream.h>
#include <unistd.h>

using namespace LOFAR;

void test_simple() {
  Queue<int> q;

  ASSERT( q.empty() );
  ASSERT( q.size() == 0 );

  // these should not block
  q.append(1);
  ASSERT( !q.empty() );
  ASSERT( q.size() == 1 );
  ASSERT( q.remove() == 1 );

  q.append(1);
  q.append(2);
  ASSERT( !q.empty() );
  ASSERT( q.size() == 2 );
  ASSERT( q.remove() == 1 );
  ASSERT( !q.empty() );
  ASSERT( q.size() == 1 );
  ASSERT( q.remove() == 2 );
}

Queue<int> q;

class A {
public:
  void mainLoop() {
    sleep(1); // make "sure" B blocks on q.remove()

    for (int i = 0; i < 10; i++)
      q.append(i);
  }
};

class B {
public:
  void mainLoop() {
    for (int i = 0; i < 10; i++)
      ASSERT( q.remove() == i );
  }
};

void test_mt() {
  A a;
  B b;

  {
    Thread ta(&a,&A::mainLoop);
    Thread tb(&b,&B::mainLoop);
  }  
}


int main()
{
  INIT_LOGGER("tQueue");
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
  INIT_LOGGER("tQueue");
  LOG_INFO("Starting up...");
  LOG_INFO("Program terminated successfully");
  return 0;
}

#endif

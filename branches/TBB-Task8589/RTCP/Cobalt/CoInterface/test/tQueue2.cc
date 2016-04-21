//# tQueue.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id$

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <CoInterface/Queue.h>

#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;

SUITE(Basic) {
  TEST(OneItem) {
    Queue<int> q("Basic::OneItem");

    q.append(42);
    CHECK(!q.empty());

    CHECK_EQUAL(42, q.remove());
    CHECK(q.empty());
  }

  TEST(FIFO) {
    Queue<size_t> q("Basic::FIFO");

    for(size_t i = 0; i < 1000; i++) {
      CHECK_EQUAL(i, q.size());
      q.append(i);
    }

    for(size_t i = 0; i < 1000; i++) {
      CHECK_EQUAL(1000UL - i, q.size());
      CHECK_EQUAL(i, q.remove());
    }

    CHECK(q.empty());
  }

  TEST(LIFO) {
    Queue<size_t> q("Basic::LIFO");

    for(size_t i = 0; i < 1000; i++) {
      CHECK_EQUAL(i, q.size());
      q.prepend(i);
    }

    for(int i = 999; i >= 0; i--) {
      CHECK_EQUAL((size_t)i, q.remove());
      CHECK_EQUAL((size_t)i, q.size());
    }

    CHECK(q.empty());
  }
}

SUITE(Speed) {
  TEST(append) {
    NSTimer timer("Speed::append", true, true);
    Queue<int> q("Speed::append");

    timer.start();
    for (size_t i = 0; i < 10000; ++i) {
      q.append(i);
    }
    timer.stop();
  }

  TEST(remove) {
    NSTimer timer("Speed::remove", true, true);
    Queue<int> q("Speed::remove");

    for (size_t i = 0; i < 10000; ++i) {
      q.append(i);
    }

    timer.start();
    for (size_t i = 0; i < 10000; ++i) {
      q.remove();
    }
    timer.stop();
  }

  TEST(appendremove1) {
    NSTimer timer("Speed::appendremove1", true, true);
    Queue<int> q("Speed::appendremove1");

    for (size_t i = 0; i < 100; ++i) {
      q.append(i);
    }

    timer.start();
    for (size_t i = 0; i < 10000; ++i) {
      q.append(i);
      q.remove();
    }
    timer.stop();
  }

  TEST(appendremove10) {
    NSTimer timer("Speed::appendremove10", true, true);
    Queue<int> q("Speed::appendremove10");

    for (size_t i = 0; i < 100; ++i) {
      q.append(i);
    }

    timer.start();
    for (size_t i = 0; i < 10000; ++i) {
      for (size_t j = 0; j < 10; ++j) {
        q.append(i);
      }
      for (size_t j = 0; j < 10; ++j) {
        q.remove();
      }
    }
    timer.stop();
  }
}

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

int main(void)
{
  INIT_LOGGER("tQueue");

  return UnitTest::RunAllTests() > 0;
}


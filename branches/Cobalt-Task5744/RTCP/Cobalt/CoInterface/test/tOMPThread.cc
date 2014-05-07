//# tOMPThread.cc
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
#include <Common/Thread/Semaphore.h>
#include <CoInterface/OMPThread.h>

#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

void sleepUntilKilled()
{
  LOG_INFO("Sleeping...");

  while (usleep(999999) == 0)
    LOG_INFO("Zzz");

  int saved_errno = errno;

  LOG_INFO("Woken up");

  // Our code relies on interruptions being
  // flagged as EINTR.
  CHECK_EQUAL(EINTR, saved_errno);
}

SUITE(OMPThread) {
  TEST(Basic) {
    OMPThread t;
  }

  TEST(StopSelf) {
    OMPThread t;

    t.stop();
  }

  TEST(WaitStopped) {
    OMPThread t;

    t.stop();
    CHECK(t.wait(TimeSpec::big_bang));
  }

  TEST(Kill) {
    OMPThread *t = NULL;
    Semaphore constructed;

#   pragma omp parallel sections num_threads(2)
    {
#     pragma omp section
      {
        t = new OMPThread;
        constructed.up();

        sleepUntilKilled();

        // Force ~t() to be called, to
        // let kill() know we've finished
        delete t;
      }

#     pragma omp section
      {
        constructed.down();

        LOG_INFO("Killing");
        t->kill();
        LOG_INFO("Killed");
      }
    }
  }
}

SUITE(OMPThreadSet) {
  TEST(EmptySet) {
    OMPThreadSet set;

    CHECK_EQUAL(0UL, set.killAll());
  }

  TEST(KillRunning) {
    OMPThreadSet set;
    Semaphore constructed;

#   pragma omp parallel sections num_threads(2)
    {
#     pragma omp section
      {
        OMPThreadSet::ScopedRun sr(set);
        constructed.up();

        sleepUntilKilled();
      }

#     pragma omp section
      {
        constructed.down();

        LOG_INFO("Killing");
        CHECK_EQUAL(1UL, set.killAll());
        LOG_INFO("Killed");
      }
    }
  }

  TEST(KillUnstarted) {
    OMPThreadSet set;
    Semaphore killed;

    set.killAll();
    CHECK_THROW(OMPThreadSet::ScopedRun sr(set), OMPThreadSet::CannotStartException);
  }

  TEST(KillStopped) {
    OMPThreadSet set;

    {
      OMPThreadSet::ScopedRun sr(set);
    }

    CHECK_EQUAL(0UL, set.killAll());
  }

  TEST(PidReuse) {
    OMPThreadSet set;

    { OMPThreadSet::ScopedRun sr(set); }
    { OMPThreadSet::ScopedRun sr(set); }

    // Should have marked both instances as stopped,
    // resulting in no kills.
    CHECK_EQUAL(0UL, set.killAll());
  }
}

int main(void)
{
  INIT_LOGGER("tOMPThread");

  OMPThread::init();

  return UnitTest::RunAllTests() > 0;
}


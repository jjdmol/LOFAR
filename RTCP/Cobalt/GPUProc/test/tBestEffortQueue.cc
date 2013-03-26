//# tBestEffortQueue.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
//# $Id: $

#include <lofar_config.h>

#include <unistd.h>

#include <Common/LofarLogger.h>

#include <opencl-incl.h>
#include <BestEffortQueue.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

void test_drop()
{
  // check if blocks are dropped if queue is full
  size_t queueSize = 10;
  BestEffortQueue<size_t> queue(queueSize, true);

  // queue has free space -- append should succeed
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.append(100 + i));
    ASSERT(queue.size() == i + 1);
  }

  // queue is full -- append should fail
  ASSERT(!queue.append(1000));
  ASSERT(queue.size() == queueSize);

  // removal should succeed
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.remove() == 100 + i);
    ASSERT(queue.size() == queueSize - i - 1);
  }

  ASSERT(queue.empty());
}

void test_nondrop()
{
  size_t queueSize = 10;
  BestEffortQueue<size_t> queue(queueSize, false);

  // queue has free space -- append should succeed
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.append(100 + i));
    ASSERT(queue.size() == i + 1);
  }

# pragma omp parallel sections
  {
#   pragma omp section
    {
      // push more -- append should always succeed
      for (size_t i = 0; i < queueSize; ++i) {
        ASSERT(queue.append(100 + i));
      }
    }

#   pragma omp section
    {
      // pop everything
      for (size_t i = 0; i < queueSize * 2; ++i) {
        ASSERT(queue.remove() > 0);
      }
    }
  }

  ASSERT(queue.empty());
}

void test_nomore()
{
  size_t queueSize = 10;
  BestEffortQueue<size_t> queue(queueSize, false);

  // fill queue
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.append(100 + i));
  }

  // end-of-stream
  queue.noMore();

  // can't append anymore
  ASSERT(!queue.append(1));

  // should be able to empty queue until we hit 0
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.remove() > 0);
  }

  // 0 signals end-of-queue
  ASSERT(queue.remove() == 0);
  ASSERT(queue.empty());

  // can't append anymore
  ASSERT(!queue.append(1));
}

int main()
{
  INIT_LOGGER( "tBestEffortQueue" );

  // abort program if code blocks
  alarm(5);

  test_drop();
  test_nondrop();
  test_nomore();

  return 0;
}


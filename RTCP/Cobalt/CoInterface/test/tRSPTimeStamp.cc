/* tRSPTimeStamp.cc
 * Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include <lofar_config.h>

#include <CoInterface/RSPTimeStamp.h>

#include <stdint.h>

#include <Common/LofarLogger.h>


#define SAMPLERATE 195312.5
// start the test at INT32_MAX * SAMPLERATE
#define TESTSTART static_cast<int64>(0x7fffffff * SAMPLERATE)
// we don't want the test to take too long
#define TESTEND   static_cast<int64>(0x7fffff00 * SAMPLERATE)

using namespace LOFAR;
using LOFAR::Cobalt::TimeStamp;

int main()
{
  unsigned clock = static_cast<unsigned>(1024 * SAMPLERATE);

  for (int64 timecounter = TESTSTART; timecounter >= TESTEND; timecounter--) {
    TimeStamp one(timecounter, clock);
    TimeStamp other(one.getSeqId(), one.getBlockId(), clock);
    ASSERTSTR(one == other, one << " == " << other << " counter was " << timecounter);
  }

  return 0;
}


//#  tRSPTimeStamp.cc: test for the RSPTimeStamp
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <stdint.h>

#include <CS1_Interface/RSPTimeStamp.h>

#define SAMPLERATE 195312.5
// start the test at INT32_MAX * SAMPLERATE
#define TESTSTART (0x7fffffff * SAMPLERATE)
// we don't want the test to take too long
#define TESTEND   (0x7ffff000 * SAMPLERATE) 

using namespace LOFAR;
using LOFAR::CS1::TimeStamp;

int main(int argc, char* argv[]) {
  TimeStamp::setMaxBlockId(SAMPLERATE);
  for (int64 timecounter = TESTSTART; timecounter >= TESTEND; timecounter--) {
    TimeStamp one(timecounter);
    TimeStamp other(one.getSeqId(), one.getBlockId());
    ASSERTSTR(one == other, one << " == " << other << " counter was " << timecounter);
  }
  return 0;
}

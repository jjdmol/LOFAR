//# tSelfDestructTimer.cc: Test wrapper class for main functionality
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
//# $Id: tGPUProcIO.cc 31587 2015-05-08 12:28:21Z mol $

#include <lofar_config.h>

#include <CoInterface/SelfDestructTimer.h>

#include <UnitTest++.h>
#include <time.h>

using namespace std;
using namespace LOFAR;
using namespace Cobalt;


Parset basicParset() {
  Parset par;
  par.add("Observation.nrBeams", "1");
  par.add("Observation.Beam[0].subbandList", "[300..301]");
  par.add("Observation.VirtualInstrument.stationList", "[CS002]");
  par.add("Observation.rspBoardList", "[0, 0]");
  par.add("Observation.rspSlotList", "[0, 1]");

  return par;
}

string timeStr(int seconds_from_now) {
  time_t t;
  struct tm *tm;
  char buf[26];

  t = time(0) + seconds_from_now;
  tm = gmtime(&t);
  strftime(buf, sizeof buf, "%F %T", tm);

  return string(buf);
}

TEST(RT_TimeOutPastObs)
{
  Parset par = basicParset();
  par.add("Observation.startTime", "2011-03-22 18:16:00");
  par.add("Observation.stopTime",  "2011-03-22 18:16:00");
  par.add("Cobalt.realTime",       "true");
  par.updateSettings();

  CHECK_EQUAL(0UL, getMaxRunTime(par, 60));
}

TEST(RT_TimeOutFutureObs)
{

  Parset par = basicParset();
  par.add("Observation.startTime", timeStr(10));
  par.add("Observation.stopTime",  timeStr(20));
  par.add("Cobalt.realTime",       "true");
  par.updateSettings();

  CHECK(getMaxRunTime(par, 40) > 0UL);
  CHECK(getMaxRunTime(par, 40) <= 60UL);
}

TEST(NonRT_TimeOutPastObs)
{
  Parset par = basicParset();
  par.add("Observation.startTime", "2011-03-22 18:16:00");
  par.add("Observation.stopTime",  "2011-03-22 18:16:00");
  par.add("Cobalt.realTime",       "false");
  par.updateSettings();

  CHECK_EQUAL(0UL, getMaxRunTime(par, 60));
}

TEST(NonRT_TimeOutFutureObs)
{

  Parset par = basicParset();
  par.add("Observation.startTime", timeStr(10));
  par.add("Observation.stopTime",  timeStr(20));
  par.add("Cobalt.realTime",       "false");
  par.updateSettings();

  CHECK_EQUAL(0UL, getMaxRunTime(par, 60));
}

int main()
{
  INIT_LOGGER("tSelfDestructTimer");

  return UnitTest::RunAllTests() > 0;
}


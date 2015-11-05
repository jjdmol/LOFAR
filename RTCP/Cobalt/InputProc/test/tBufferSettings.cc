//# tBufferSettings.cc: stand-alone test program for BufferSettings class
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <string>
#include <iostream>

#include <UnitTest++.h>

#include <Common/LofarLogger.h>

#include <InputProc/Buffer/BufferSettings.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

SUITE(nrBeamletsPerBoard)
{
  TEST(16bit)
  {
    struct StationID stationID("RS106", "LBA", 200, 16);
    struct BufferSettings settings(stationID, false);

    CHECK_EQUAL(61U, settings.nrBeamletsPerBoard());
  }

  TEST(8bit)
  {
    struct StationID stationID("RS106", "LBA", 200, 8);
    struct BufferSettings settings(stationID, false);

    CHECK_EQUAL(122U, settings.nrBeamletsPerBoard());
  }

  TEST(4bit)
  {
    struct StationID stationID("RS106", "LBA", 200, 4);
    struct BufferSettings settings(stationID, false);

    CHECK_EQUAL(244U, settings.nrBeamletsPerBoard());
  }
}

int main()
{
  INIT_LOGGER("tBufferSettings");

  return UnitTest::RunAllTests() > 0;
}


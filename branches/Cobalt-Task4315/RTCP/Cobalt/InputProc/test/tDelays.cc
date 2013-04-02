/* tDelays.cc
 * Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <ctime>
#include <UnitTest++.h>

#include <Common/LofarLogger.h>

#include <InputProc/Delays/Delays.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

TEST(Basic) {
  Parset ps;

  ps.add( "Observation.referencePhaseCenter", "[0, 0, 0]" ); // center of earth
  ps.add( "PIC.Core.STATION.phaseCenter", "[0, 0, 299792458]" ); // 1 lightsecond away from earth center
  ps.add( "OLAP.storageStationNames", "[STATION]" );

  ps.add( "Observation.nrBeams", "1" );
  ps.add( "Observation.Beam[0].directionType", "AZEL" );
  ps.add( "Observation.Beam[0].angle1", "0" );
  ps.add( "Observation.Beam[0].angle2", "0" );
  ps.add( "Observation.Beam[0].nrTiedArrayBeams", "0" );
  ps.updateSettings();

  // blockSize is ~1s
  Delays delays(ps, "STATION", TimeStamp(time(0), 0, 200000000), 192315);
  delays.start();

  Delays::AllDelays delaySet;

  for (size_t block = 0; block < 1024; ++block)
    delays.getNextDelays(delaySet);
}


int main()
{
  INIT_LOGGER( "tDelays" );

  // Don't run forever if communication fails for some reason
  alarm(10);

  return UnitTest::RunAllTests() > 0;
}


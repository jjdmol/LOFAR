//# printDelays.cc: Print all delays generated for an observation
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
//# $Id: capture.cc 25540 2013-07-02 13:20:36Z mol $

#include <lofar_config.h>

#include <iostream>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>

#include <CoInterface/Parset.h>
#include <InputProc/Delays/Delays.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;
using boost::format;

string printDouble3Vector( const vector<double> &vec )
{
  ASSERT(vec.size() == 3);

  return str(format("[%.3f, %.3f, %.3f]") % vec[0] % vec[1] % vec[2]);
}

int main( int argc, char **argv )
{
  INIT_LOGGER( "printDelays" );

  if (argc < 3) {
    cerr << "Syntax: printDelays parset stationname" << endl;
    exit(1);
  }

  Parset ps(argv[1]);
  const string stationName = argv[2];

  const ssize_t stationIdx = ps.settings.stationIndex(stationName);

  if (stationIdx < 0) {
    LOG_ERROR_STR("Could not find station in parset: " << stationName);
    exit(1);
  }

  /* Determine start/stop/blocksize parameters */
  const TimeStamp from(ps.startTime() * ps.subbandBandwidth(), ps.clockSpeed());
  const TimeStamp to(ps.stopTime() * ps.subbandBandwidth(), ps.clockSpeed());
  ssize_t block = -1;
  size_t blockSize = ps.nrSamplesPerSubband();

  /* Print header */
  cout << "# Parset:           " << argv[1] << endl;
  cout << "# Delay comp?:      " << ps.settings.delayCompensation.enabled << endl;
  cout << "# Clock corr?:      " << ps.settings.corrections.clock << endl;
  cout << "# Ref Phase center: " << printDouble3Vector(ps.settings.delayCompensation.referencePhaseCenter) << endl;
  cout << "#" << endl;
  cout << "# Station:          " << stationName << endl;
  cout << "# Phase center:     " << printDouble3Vector(ps.settings.stations[stationIdx].phaseCenter) << endl;
  cout << "# Clock correction: " << str(format("%.15f") % ps.settings.stations[stationIdx].clockCorrection) << endl;
  cout << "# Start:            " << from << endl;
  cout << "# Stop:             " << to << endl;
  cout << "# BlockSize:        " << blockSize << " samples" << endl;
  cout << "#" << endl;
  cout << "# Applied delay := delay + clockCorrection" << endl;
  cout << "# block timestamp dir.x dir.y dir.z delay" << endl;
  cout.flush();

  /* Start delay compensation thread */
  Delays delays(ps, stationIdx, from, blockSize);
  delays.start();

  /* Produce and print delays for the whole observation */
  for (TimeStamp current = from + block * blockSize; current + blockSize < to; current += blockSize, ++block) 
  {
    Delays::AllDelays delaySet(ps);

    delays.getNextDelays(delaySet);


    Delays::Delay d = delaySet.SAPs[0].SAP;
    cout << str(format("%u %.15f %.15f %.15f %.15f %.15f")
          % block
          % current.getSeconds()
          % d.direction[0] % d.direction[1] % d.direction[2]
          % d.delay) << endl;
  }

  return 0;
}



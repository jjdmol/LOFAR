//# fixlsmeta.cc: Program to fix the meta info of the LofarStMan
//# Copyright (C) 2009
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
//# $Id$

#include <lofar_config.h>
#include <LofarStMan/LofarStMan.h>
#include <Common/LofarLogger.h>
#include <casa/IO/AipsIO.h>
#include <casa/Containers/BlockIO.h>
#include <casa/Inputs/Input.h>
#include <casa/iostream.h>

using namespace casa;
using namespace LOFAR;

int main (int argc, char* argv[])
{
  try {
    Input inputs(1);
    // Define the input keywords
    inputs.version("20100401GvD");
    inputs.create ("ms", "",
		   "Name of input MS",
		   "string");
    inputs.create ("timedivisor", "1",
		   "Factor to divide the time interval by",
		   "float");
    inputs.create ("timemultiplier", "1",
		   "Factor to multiply the time interval with",
		   "float");
    // Fill the input value from the command line.
    inputs.readArguments (argc, argv);

    // Get and check the input specification.
    String msin (inputs.getString("ms"));
    if (msin == "") {
      throw AipsError(" an input MS must be given");
    }
    // Get the time divisor.
    double timeDivisor (inputs.getDouble("timedivisor"));
    double timeMultiplier (inputs.getDouble("timemultiplier"));

    Bool asBigEndian;
    uInt alignment;
    Block<int32> ant1;
    Block<int32> ant2;
    double startTime;
    double timeIntv;
    uint32 nChan;
    uint32 nPol;
    double maxNrSample;
    int version
;    {
      // Open and read the meta file.
      AipsIO aio(msin + "/table.f0meta");
      version = aio.getstart ("LofarStMan");
      ASSERTSTR (version <= 3,
                 "fixlsmeta can only handle up to version 3");
      aio >> ant1 >> ant2 >> startTime >> timeIntv >> nChan
          >> nPol >> maxNrSample >> alignment >> asBigEndian;
      aio.getend();
    }
    // Fix the interval.
    cout << "time interval changed from " << timeIntv << " to ";
    timeIntv /= timeDivisor;
    timeIntv *= timeMultiplier;
    cout <<timeIntv << endl;
    {
      // Create and write the meta file.
      AipsIO aio(msin + "/table.f0meta", ByteIO::New);
      version = aio.putstart ("LofarStMan", version);
      aio << ant1 << ant2 << startTime << timeIntv << nChan
          << nPol << maxNrSample << alignment << asBigEndian;
      aio.putend();
    }

  } catch (AipsError x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}

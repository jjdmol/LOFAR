//# createHeaders.cc: Generates all .h5/.MS files given a (OLAP) parset
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

#include <cstdlib>
#include <string>
#include <iostream>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include "OutputThread.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

using boost::format;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char *argv[])
{
  INIT_LOGGER("createHeaders");

  if (argc != 2) {
    cout << str(format("usage: %s parset") % argv[0]) << endl;
    cout << endl;
    cout << "parset: the filename of the parset to process." << endl;
    return 1;
  }

  Parset parset(argv[1]);
  MACIO::RTmetadata rtmd(parset.settings.observationID, "", ""); // dummy

  // Process correlated data
  if (parset.settings.correlator.enabled) {
    for (size_t fileIdx = 0; fileIdx < parset.settings.correlator.files.size(); ++fileIdx)
    {
      string logPrefix = str(format("[correlated stream %3u] ") % fileIdx);

      Pool<StreamableData> outputPool(logPrefix, true);

      SubbandOutputThread writer(parset, fileIdx, outputPool, rtmd, "rtmd key prefix", logPrefix, ".");
      writer.createMS();
    }
  }

  // Process beam-formed data
  if (parset.settings.beamFormer.enabled) {
    for (size_t fileIdx = 0; fileIdx < parset.settings.beamFormer.files.size(); ++fileIdx)
    {
      string logPrefix = str(format("[beamformed stream %3u] ") % fileIdx);

      Pool<TABTranspose::BeamformedData> outputPool(logPrefix, true);

      TABOutputThread writer(parset, fileIdx, outputPool, rtmd, "rtmd key prefix", logPrefix, ".");
      writer.createMS();
    }
  }

  return 0;
}


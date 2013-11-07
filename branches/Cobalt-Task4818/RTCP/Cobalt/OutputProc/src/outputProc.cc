//# outputProc.cc
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <cstdio> // for setvbuf
#include <omp.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <Common/CasaLogSink.h>
#include <Common/Exceptions.h>
#include <Common/NewHandler.h>
#include <Stream/PortBroker.h>
#include <CoInterface/Exceptions.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Stream.h>
#include "Writer.h"
#include "GPUProcIO.h"
#include "IOPriority.h"

// install a new handler to produce backtraces for bad_alloc
LOFAR::NewHandler h(LOFAR::BadAllocException::newHandler);

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

char stdoutbuf[1024], stderrbuf[1024];

int main(int argc, char *argv[])
{
  INIT_LOGGER("outputProc");

  CasaLogSink::attach();

  if (argc < 3)
    throw StorageException(str(boost::format("usage: %s obsid rank") % argv[0]), THROW_ARGS);

  setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
  setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

  omp_set_nested(true);

  LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2]);

  int observationID = boost::lexical_cast<int>(argv[1]);
  size_t myRank = boost::lexical_cast<size_t>(argv[2]);

  setIOpriority();
  setRTpriority();
  lockInMemory();

  PortBroker::createInstance(storageBrokerPort(observationID));

  // retrieve the parset
  string resource = getStorageControlDescription(observationID, myRank);
  PortBroker::ServerStream controlStream(resource);

  process(controlStream, myRank);

  LOG_INFO_STR("Program end");

  return 0;
}


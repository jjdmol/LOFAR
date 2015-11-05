//# outputProc.cc
//# Copyright (C) 2008-2014  ASTRON (Netherlands Institute for Radio Astronomy)
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
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <omp.h>
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
#include <OutputProc/Package__Version.h>
#include "GPUProcIO.h"
#include "IOPriority.h"

#define STDLOG_BUFFER_SIZE     1024

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

// install a new handler to produce backtraces for bad_alloc
LOFAR::NewHandler h(LOFAR::BadAllocException::newHandler);

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

static char stdoutbuf[STDLOG_BUFFER_SIZE];
static char stderrbuf[STDLOG_BUFFER_SIZE];

static void usage(const char *argv0)
{
  cerr << "OutputProc: Data writer for the Real-Time Central Processing of the" << endl;
  cerr << "LOFAR radio telescope." << endl;
  cerr << "OutputProc provides CASA Measurement Set files with correlated data" << endl;
  cerr << "for the Standard Imaging mode and HDF5 files with beamformed data" << endl;
  cerr << "for the Pulsar mode." << endl; 
  // one of the roll-out scripts greps for the version x.y
  cerr << "OutputProc version " << OutputProcVersion::getVersion() << " r" << OutputProcVersion::getRevision() << endl;
  cerr << endl;
  cerr << "Usage: " << argv0 << " ObservationID mpi_rank" << endl;
  cerr << endl;
  cerr << "  -h: print this message" << endl;
}

int main(int argc, char *argv[])
{
  setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
  setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

  int opt;
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;

    default: /* '?' */
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (argc != 3) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  INIT_LOGGER("outputProc"); // also attaches to CasaLogSink

  LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2]);
  LOG_INFO_STR("OutputProc version " << OutputProcVersion::getVersion() << " r" << OutputProcVersion::getRevision());

  int observationID = boost::lexical_cast<int>(argv[1]);
  unsigned myRank = boost::lexical_cast<unsigned>(argv[2]);

  omp_set_nested(true);

  setIOpriority();
  setRTpriority();
  lockInMemory();

  PortBroker::createInstance(storageBrokerPort(observationID));

  // retrieve control stream to receive the parset and report back
  string resource = getStorageControlDescription(observationID, myRank);
  PortBroker::ServerStream controlStream(resource);

  if (process(controlStream, myRank)) {
    LOG_INFO("Program terminated succesfully");
    return EXIT_SUCCESS;
  } else {
    LOG_ERROR("Program terminated with errors");
    return EXIT_FAILURE;
  }
}


/* outputProc.cc
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>
#include <libgen.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <Common/LofarLogger.h>
#include <Common/CasaLogSink.h>
#include <Common/StringUtil.h>
#include <Common/Exceptions.h>
#include <Common/NewHandler.h>
#include <Common/Thread/Thread.h>
#include <ApplCommon/Observation.h>
#include <Stream/PortBroker.h>
#include <CoInterface/Exceptions.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Stream.h>
#include <CoInterface/FinalMetaData.h>
#include <OutputProc/Package__Version.h>
#include "SubbandWriter.h"
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
#if defined HAVE_LOG4CPLUS
  char *dirc = strdup(argv[0]);

  INIT_LOGGER(string(getenv("LOFARROOT") ? : dirname(dirc)) + "/../etc/Storage_main.log_prop");

  free(dirc);
#elif defined HAVE_LOG4CXX
  #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
  Context::initialize();
  setLevel("Global",8);
#else
  INIT_LOGGER_WITH_SYSINFO(str(boost::format("Storage@%02d") % (argc > 2 ? atoi(argv[2]) : -1)));
#endif

  CasaLogSink::attach();

  try {
    if (argc != 4)
      throw StorageException(str(boost::format("usage: %s obsid rank is_bigendian") % argv[0]), THROW_ARGS);

    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

    LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2] << ' ' << argv[3]);

    int observationID = boost::lexical_cast<int>(argv[1]);
    unsigned myRank = boost::lexical_cast<unsigned>(argv[2]);
    bool isBigEndian = boost::lexical_cast<bool>(argv[3]);

    setIOpriority();
    setRTpriority();
    lockInMemory();

    PortBroker::createInstance(storageBrokerPort(observationID));

    // retrieve the parset
    string resource = getStorageControlDescription(observationID, myRank);
    PortBroker::ServerStream controlStream(resource);

    Parset parset(&controlStream);
    Observation obs(&parset, false, parset.totalNrPsets());

    vector<string> hostnames = parset.getStringVector("OLAP.Storage.hosts", true);
    ASSERT(myRank < hostnames.size());
    string myHostName = hostnames[myRank];

    string obsLogPrefix = str(boost::format("[obs %u] ") % parset.observationID());

    {
      // make sure "parset" stays in scope for the lifetime of the SubbandWriters

      vector<SmartPtr<SubbandWriter> > subbandWriters;

      for (OutputType outputType = FIRST_OUTPUT_TYPE; outputType < LAST_OUTPUT_TYPE; outputType++) {
        for (unsigned streamNr = 0; streamNr < parset.nrStreams(outputType); streamNr++) {
          if (parset.getHostName(outputType, streamNr) == myHostName) {
            unsigned writerNr = 0;

            // lookup PVSS writer number for this file
            for (unsigned i = 0; i < obs.streamsToStorage.size(); i++) {
              Observation::StreamToStorage &s = obs.streamsToStorage[i];

              if (s.dataProductNr == static_cast<unsigned>(outputType) && s.streamNr == streamNr) {
                writerNr = s.writerNr;
                break;
              }
            }

            string sbLogPrefix = str(boost::format("[obs %u type %u stream %3u writer %3u] ") % parset.observationID() % outputType % streamNr % writerNr);

            try {
              subbandWriters.push_back(new SubbandWriter(parset, outputType, streamNr, isBigEndian, sbLogPrefix));
            } catch (Exception &ex) {
              LOG_WARN_STR(sbLogPrefix << "Could not create writer: " << ex);
            } catch (exception &ex) {
              LOG_WARN_STR(sbLogPrefix << "Could not create writer: " << ex.what());
            }
          }
        }
      }

      // Add final meta data (broken tile information, etc)
      // that is obtained after the end of an observation.
      LOG_INFO_STR(obsLogPrefix << "Waiting for final meta data");
      FinalMetaData finalMetaData;
      finalMetaData.read(controlStream);

      LOG_INFO_STR(obsLogPrefix << "Processing final meta data");
      for (size_t i = 0; i < subbandWriters.size(); ++i)
        try {
          subbandWriters[i]->augment(finalMetaData);
        } catch (Exception &ex) {
          LOG_WARN_STR(obsLogPrefix << "Could not add final meta data: " << ex);
        }
    }
  } catch (Exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught Exception: " << ex);
    return 1;
  }

  LOG_INFO_STR("[obs unknown] Program end");
  return 0;
}


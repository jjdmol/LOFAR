//#  FinalMetaDataGatherer.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id: Storage_main.cc 22339 2012-10-15 09:33:57Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/CasaLogSink.h>
#include <Common/Exceptions.h>
#include <Common/NewHandler.h>
#include <ApplCommon/Observation.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <Interface/Stream.h>
#include <Interface/FinalMetaData.h>
#include <Stream/PortBroker.h>
#include <Storage/ExitOnClosedStdin.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>
#include <libgen.h>

#include <boost/format.hpp>


// install a new handler to produce backtraces for bad_alloc
LOFAR::NewHandler h(LOFAR::BadAllocException::newHandler);

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace std;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

char stdoutbuf[1024], stderrbuf[1024];

int main(int argc, char *argv[])
{
#if defined HAVE_LOG4CPLUS
  char *dirc = strdup(argv[0]);

  INIT_LOGGER(string(getenv("LOFARROOT") ? : dirname(dirc)) + "/../etc/FinalMetaDataGatherer.log_prop");

  free(dirc);
#elif defined HAVE_LOG4CXX
  #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
  Context::initialize();
  setLevel("Global",8);
#else
  INIT_LOGGER_WITH_SYSINFO(str(boost::format("FinalMetaDataGatherer@%02d") % (argc > 2 ? atoi(argv[2]) : -1)));
#endif

  CasaLogSink::attach();

  try {
    if (argc != 2)
      throw StorageException(str(boost::format("usage: %s obsid") % argv[0]), THROW_ARGS);

    ExitOnClosedStdin			  stdinWatcher;
    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

    LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1]);

    int				          observationID = boost::lexical_cast<int>(argv[1]);

    PortBroker::createInstance(storageBrokerPort(observationID));

    // retrieve the parset
    string resource = getStorageControlDescription(observationID, -1);
    PortBroker::ServerStream controlStream(resource);

    Parset parset(&controlStream);

    FinalMetaData finalMetaData;

    finalMetaData.write(controlStream);
  } catch (Exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught Exception: " << ex);
    return 1;
  }

  LOG_INFO_STR("[obs unknown] Program end");
  return 0;
}

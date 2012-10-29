//#  Storage_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/CasaLogSink.h>
#include <Common/StringUtil.h>
#include <Common/Exceptions.h>
#include <Common/NewHandler.h>
#include <ApplCommon/Observation.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <Interface/Stream.h>
#include <Interface/FinalMetaData.h>
#include <Common/Thread/Thread.h>
#include <Stream/PortBroker.h>
#include <Storage/SubbandWriter.h>
#include <Storage/IOPriority.h>
#include <Storage/ExitOnClosedStdin.h>
#include <Storage/Package__Version.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/select.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libgen.h>

#include <stdexcept>
#include <string>
#include <vector>

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

    ExitOnClosedStdin			  stdinWatcher;
    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stderrbuf, _IOLBF, sizeof stderrbuf);

    LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2] << ' ' << argv[3]);

    int				          observationID = boost::lexical_cast<int>(argv[1]);
    unsigned				  myRank = boost::lexical_cast<unsigned>(argv[2]);
    bool				  isBigEndian = boost::lexical_cast<bool>(argv[3]);

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

    {
      // make sure "parset" stays in scope for the lifetime of the SubbandWriters

      vector<SmartPtr<SubbandWriter> > subbandWriters;

      for (OutputType outputType = FIRST_OUTPUT_TYPE; outputType < LAST_OUTPUT_TYPE; outputType ++) {
        for (unsigned streamNr = 0; streamNr < parset.nrStreams(outputType); streamNr ++) {
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

            string logPrefix = str(boost::format("[obs %u type %u stream %3u writer %3u] ") % parset.observationID() % outputType % streamNr % writerNr);

            try {
              subbandWriters.push_back(new SubbandWriter(parset, outputType, streamNr, isBigEndian, logPrefix));
            } catch (Exception &ex) {
              LOG_WARN_STR(logPrefix << "Could not create writer: " << ex);
            } catch (exception &ex) {
              LOG_WARN_STR(logPrefix << "Could not create writer: " << ex.what());
            }
          }
        }
      }   
    }
  } catch (Exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught Exception: " << ex);
    return 1;
  }

  LOG_INFO_STR("[obs unknown] Program end");
  return 0;
}

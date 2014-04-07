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
#include <Common/StringUtil.h>
#include <Common/Exceptions.h>
#include <Common/NewHandler.h>
#include <ApplCommon/Observation.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <Common/Thread/Thread.h>
#include <Storage/SubbandWriter.h>
#include <Storage/IOPriority.h>
#include <Storage/Package__Version.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif

#include <sys/select.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

#include <stdexcept>
#include <string>
#include <vector>

#include <boost/format.hpp>


// install a new handler to produce backtraces for std::bad_alloc
LOFAR::NewHandler h(LOFAR::BadAllocException::newHandler);


using namespace LOFAR;
using namespace LOFAR::RTCP;


class ExitOnClosedStdin
{
  public:
    ExitOnClosedStdin();
    ~ExitOnClosedStdin();

  private:
    void   mainLoop();
    Thread itsThread;
};


ExitOnClosedStdin::ExitOnClosedStdin()
:
  itsThread(this, &ExitOnClosedStdin::mainLoop, "[obs unknown] [stdinWatcherThread] ", 65535)
{
}


ExitOnClosedStdin::~ExitOnClosedStdin()
{
  itsThread.cancel();
}


void ExitOnClosedStdin::mainLoop()
{
  // an empty read on stdin means the SSH connection closed, which indicates that we should abort

  while (true) {
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(0, &fds);

    struct timeval timeval;

    timeval.tv_sec  = 1;
    timeval.tv_usec = 0;

    switch (select(1, &fds, 0, 0, &timeval)) {
      case -1 : throw SystemCallException("select", errno, THROW_ARGS);
      case  0 : continue;
    }

    char buf[1];
    ssize_t numbytes;
    numbytes = ::read(0, buf, sizeof buf);

    if (numbytes == 0) {
      LOG_FATAL("Lost stdin -- aborting"); // this most likely won't arrive, since stdout/stderr are probably closed as well
      exit(1);
    } else {
      // slow down reading data (IONProc will be spamming us with /dev/zero)
      if (usleep(999999) < 0)
        throw SystemCallException("usleep", errno, THROW_ARGS);
    }  
  }
}


int main(int argc, char *argv[])
{
#if defined HAVE_LOG4CPLUS
  INIT_LOGGER(string(getenv("LOFARROOT") ? : ".") + "/etc/Storage_main.log_prop");
#elif defined HAVE_LOG4CXX
  #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
  Context::initialize();
  setLevel("Global",8);
#else
  INIT_LOGGER_WITH_SYSINFO(str(boost::format("Storage@%02d") % (argc > 1 ? atoi(argv[1]) : -1)));
#endif

  try {
    if (argc != 4)
      throw StorageException(str(boost::format("usage: %s rank parset is_bigendian") % argv[0]), THROW_ARGS);

    char stdoutbuf[1024], stderrbuf[1024];
    setvbuf(stdout, stdoutbuf, _IOLBF, sizeof stdoutbuf);
    setvbuf(stderr, stdoutbuf, _IOLBF, sizeof stderrbuf);

    LOG_DEBUG_STR("Started: " << argv[0] << ' ' << argv[1] << ' ' << argv[2] << ' ' << argv[3]);

    ExitOnClosedStdin			  stdinWatcher;

    unsigned				  myRank = boost::lexical_cast<unsigned>(argv[1]);
    Parset				  parset(argv[2]);
    bool				  isBigEndian = boost::lexical_cast<bool>(argv[3]);
    std::vector<SmartPtr<SubbandWriter> > subbandWriters;
    std::string				  myHostName = parset.getStringVector("OLAP.Storage.hosts", true)[myRank];

    setIOpriority();
    setRTpriority();
    lockInMemory();

    Observation obs(&parset, false);

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

	  std::string logPrefix = str(boost::format("[obs %u type %u stream %3u writer %3u] ") % parset.observationID() % outputType % streamNr % writerNr);

	  try {
	    subbandWriters.push_back(new SubbandWriter(parset, outputType, streamNr, isBigEndian, logPrefix));
	  } catch (Exception &ex) {
	    LOG_WARN_STR(logPrefix << "Could not create writer: " << ex);
	  } catch (std::exception &ex) {
	    LOG_WARN_STR(logPrefix << "Could not create writer: " << ex.what());
	  }
	}
      }
    }
  } catch (Exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught Exception: " << ex);
    exit(1);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught std::exception: " << ex.what());
    exit(1);
  } catch (...) {
    LOG_FATAL_STR("[obs unknown] Caught non-std::exception");
    exit(1);
  }

  LOG_INFO_STR("[obs unknown] Program end");
  return 0;
}

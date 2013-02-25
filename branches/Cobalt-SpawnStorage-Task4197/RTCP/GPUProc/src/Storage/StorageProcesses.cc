#include "lofar_config.h"
#include <Storage/StorageProcesses.h>
#include <sys/time.h>
#include <unistd.h>
#include <Common/Thread/Thread.h>
#include <Stream/PortBroker.h>
#include <boost/format.hpp>

namespace LOFAR {
namespace RTCP {

using namespace std;
using boost::format;


StorageProcess::StorageProcess( StorageProcesses &manager, const Parset &parset, const string &logPrefix, int rank, const string &hostname )
:
  itsManager(manager),
  itsParset(parset),
  itsLogPrefix(str(boost::format("%s [StorageWriter rank %2d host %s] ") % logPrefix % rank % hostname)),
  itsRank(rank),
  itsHostname(hostname)
{
}


StorageProcess::~StorageProcess()
{
  ScopedDelayCancellation dc; // stop() is a cancellation point

  // stop immediately
  struct timespec immediately = { 0, 0 };
  stop(immediately);
}


void StorageProcess::start()
{
  ASSERTSTR(!itsThread, "StorageProcess has already been started");

  itsThread = new Thread(this, &StorageProcess::controlThread, itsLogPrefix + "[ControlThread] ", 65535);
}


void StorageProcess::stop(struct timespec deadline)
{
  if (!itsThread) {
    // not started
    return;
  }

  if (!itsThread->wait(deadline))
    itsThread->cancel();
}


bool StorageProcess::isDone()
{
  return itsThread->isDone();
}


void StorageProcess::controlThread()
{
  // Start Storage
  std::string userName   = itsParset.getString("OLAP.Storage.userName");
  std::string sshKey     = itsParset.getString("OLAP.Storage.sshIdentityFile");
  std::string executable = itsParset.getString("OLAP.Storage.msWriter");

  if (userName == "") {
    // No username given -- use $USER
    const char *USER = getenv("USER");

    ASSERTSTR(USER, "$USER not set.");

    userName = USER;
  }

  if (sshKey == "") {
    // No SSH key given -- try to discover it

    char privkey[1024];

    if (discover_ssh_privkey(privkey, sizeof privkey))
      sshKey = privkey;
  }

  char cwd[1024];

  if (getcwd(cwd, sizeof cwd) == 0)
    throw SystemCallException("getcwd", errno, THROW_ARGS);

  std::string commandLine = str(boost::format("cd %s && %s%s %u %d %u")
    % cwd
#if defined USE_VALGRIND
    % "valgrind --leak-check=full "
#else
    % ""
#endif
    % executable
    % itsParset.observationID()
    % itsRank
#if defined WORDS_BIGENDIAN
    % 1
#else
    % 0
#endif
  );

  SSHconnection sshconn(itsLogPrefix, itsHostname, commandLine, userName, sshKey, 0);
  sshconn.start();

  // Connect control stream
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] connecting...");
  std::string resource = getStorageControlDescription(itsParset.observationID(), itsRank);
  PortBroker::ClientStream stream(itsHostname, storageBrokerPort(itsParset.observationID()), resource, 0);

  // Send parset
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] connected -- sending parset");
  itsParset.write(&stream);
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sent parset");

  // Send final meta data once it is available
  itsManager.itsFinalMetaDataAvailable.down();

  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sending final meta data");
  itsManager.itsFinalMetaData.write(stream);
  LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sent final meta data");

  // Wait for Storage to finish properly
  sshconn.wait();
}


StorageProcesses::StorageProcesses( const Parset &parset, const std::string &logPrefix )
:
  itsParset(parset),
  itsLogPrefix(logPrefix)
{
  start();
}

StorageProcesses::~StorageProcesses()
{
  // never let any processes linger
  stop(0);
}

void StorageProcesses::start()
{
  vector<string> hostnames = itsParset.getStringVector("OLAP.Storage.hosts");

  itsStorageProcesses.resize(hostnames.size());

  LOG_DEBUG_STR(itsLogPrefix << "Starting " << itsStorageProcesses.size() << " Storage processes");

  // Start all processes
  for (unsigned rank = 0; rank < itsStorageProcesses.size(); rank ++) {
    itsStorageProcesses[rank] = new StorageProcess(*this, itsParset, itsLogPrefix, rank, hostnames[rank]);
    itsStorageProcesses[rank]->start();
  }  
}


void StorageProcesses::stop( time_t deadline )
{
  LOG_DEBUG_STR(itsLogPrefix << "Stopping storage processes");

  struct timespec deadline_ts = { deadline, 0 };

  // Stop all processes
  for (unsigned rank = 0; rank < itsStorageProcesses.size(); rank ++) {
    itsStorageProcesses[rank]->stop(deadline_ts);
    itsStorageProcesses[rank] = 0;
  }

  itsStorageProcesses.clear();

  LOG_DEBUG_STR(itsLogPrefix << "Storage processes are stopped");
}


void StorageProcesses::forwardFinalMetaData( time_t deadline )
{
  struct timespec deadline_ts = { deadline, 0 };

  Thread thread(this, &StorageProcesses::finalMetaDataThread, itsLogPrefix + "[FinalMetaDataThread] ", 65536);

  thread.cancel(deadline_ts);
}


void StorageProcesses::finalMetaDataThread()
{
  std::string hostName    = itsParset.getString("OLAP.FinalMetaDataGatherer.host");
  std::string userName    = itsParset.getString("OLAP.FinalMetaDataGatherer.userName");
  std::string sshKey      = itsParset.getString("OLAP.FinalMetaDataGatherer.sshIdentityFile");
  std::string executable  = itsParset.getString("OLAP.FinalMetaDataGatherer.executable");

  char cwd[1024];

  if (getcwd(cwd, sizeof cwd) == 0)
    throw SystemCallException("getcwd", errno, THROW_ARGS);

  std::string commandLine = str(boost::format("cd %s && %s %d 2>&1")
      % cwd
      % executable
      % itsParset.observationID()
      );

  // Start the remote process
  SSHconnection sshconn(itsLogPrefix + "[FinalMetaData] ", hostName, commandLine, userName, sshKey);
  sshconn.start();

  // Connect
  LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] connecting...");
  std::string resource = getStorageControlDescription(itsParset.observationID(), -1);
  PortBroker::ClientStream stream(hostName, storageBrokerPort(itsParset.observationID()), resource, 0);

  // Send parset
  LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] connected -- sending parset");
  itsParset.write(&stream);
  LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] sent parset");

  // Receive final meta data
  itsFinalMetaData.read(stream);
  LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] obtained final meta data");

  // Notify clients
  itsFinalMetaDataAvailable.up(itsStorageProcesses.size());

  // Wait for or end the remote process
  sshconn.wait();
}

}
}

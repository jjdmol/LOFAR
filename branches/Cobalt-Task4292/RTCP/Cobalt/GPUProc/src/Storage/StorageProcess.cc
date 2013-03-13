#include "lofar_config.h"
#include <Storage/StorageProcess.h>
#include <Storage/SSH.h>
#include <CoInterface/Stream.h>
#include <sys/time.h>
#include <unistd.h>
#include <Common/Thread/Thread.h>
#include <Stream/PortBroker.h>
#include <boost/format.hpp>

namespace LOFAR
{
  namespace Cobalt
  {

    using namespace std;
    using boost::format;


    StorageProcess::StorageProcess( const Parset &parset, const string &logPrefix, int rank, const string &hostname, FinalMetaData &finalMetaData, Trigger &finalMetaDataAvailable )
      :
      itsParset(parset),
      itsLogPrefix(str(boost::format("%s [StorageWriter rank %2d host %s] ") % logPrefix % rank % hostname)),
      itsRank(rank),
      itsHostname(hostname),
      itsFinalMetaData(finalMetaData),
      itsFinalMetaDataAvailable(finalMetaDataAvailable)
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
      std::string userName = itsParset.getString("OLAP.Storage.userName");
      std::string pubKey = itsParset.getString("OLAP.Storage.sshPublicKey");
      std::string privKey = itsParset.getString("OLAP.Storage.sshPrivateKey");
      std::string executable = itsParset.getString("OLAP.Storage.msWriter");

      if (userName == "") {
        // No username given -- use $USER
        const char *USER = getenv("USER");

        ASSERTSTR(USER, "$USER not set.");

        userName = USER;
      }

      if (pubKey == "" && privKey == "") {
        // No SSH keys given -- try to discover them

        char discover_pubkey[1024];
        char discover_privkey[1024];

        if (discover_ssh_keys(discover_pubkey, sizeof discover_pubkey, discover_privkey, sizeof discover_privkey)) {
          pubKey = discover_pubkey;
          privKey = discover_privkey;
        }
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

      SSHconnection sshconn(itsLogPrefix, itsHostname, commandLine, userName, pubKey, privKey, 0);
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
      itsFinalMetaDataAvailable.wait();

      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sending final meta data");
      itsFinalMetaData.write(stream);
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sent final meta data");

      // Wait for Storage to finish properly
      sshconn.wait();
    }


  }
}

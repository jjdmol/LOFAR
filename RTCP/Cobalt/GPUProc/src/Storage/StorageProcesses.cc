//# StorageProcesses.cc
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

#include <lofar_config.h>

#include "StorageProcesses.h"

#include <set>
#include <algorithm>
#include <sys/time.h>
#include <unistd.h>
#include <boost/format.hpp>

#include <Stream/PortBroker.h>
#include <Stream/SocketStream.h>
#include <CoInterface/Stream.h>

#include "SSH.h"

namespace LOFAR
{
  namespace Cobalt
  {

    using namespace std;
    using boost::format;


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


    ParameterSet StorageProcesses::feedbackLTA() const
    {
      return itsFeedbackLTA;
    }


    void StorageProcesses::start()
    {
      const vector<string> &hostnames = itsParset.settings.outputProcHosts;

      itsStorageProcesses.resize(hostnames.size());

      LOG_DEBUG_STR(itsLogPrefix << "Starting " << itsStorageProcesses.size() << " Storage processes");

      // Start all processes
      for (unsigned rank = 0; rank < itsStorageProcesses.size(); rank++) {
        itsStorageProcesses[rank] = new StorageProcess(itsParset, itsLogPrefix, rank, hostnames[rank], itsFinalMetaData, itsFinalMetaDataAvailable);
        itsStorageProcesses[rank]->start();
      }
    }


    void StorageProcesses::stop( time_t deadline )
    {
      LOG_DEBUG_STR(itsLogPrefix << "Stopping storage processes");

      struct timespec deadline_ts = { deadline, 0 };

      // Stop all processes
      for (unsigned rank = 0; rank < itsStorageProcesses.size(); rank++) {
        // stop storage process
        itsStorageProcesses[rank]->stop(deadline_ts);

        // obtain feedback for LTA
        itsFeedbackLTA.adoptCollection(itsStorageProcesses[rank]->feedbackLTA());

        // free the StorageProcess object
        itsStorageProcesses[rank] = 0;
      }

      itsStorageProcesses.clear();

      LOG_DEBUG_STR(itsLogPrefix << "Storage processes are stopped");
    }


    void StorageProcesses::forwardFinalMetaData( time_t deadline )
    {
      struct timespec deadline_ts = { deadline, 0 };

      Thread thread(this, &StorageProcesses::finalMetaDataThread, itsLogPrefix + "[FinalMetaDataThread] ", 65536);

      LOG_DEBUG("forwardFinalMetaData(): cancelling FinalMetaDataThread");
      thread.cancel(deadline_ts);
      thread.wait();

      // Notify clients
      itsFinalMetaDataAvailable.trigger();
    }


    void StorageProcesses::finalMetaDataThread()
    {
      // Note that some parset keys are overriden as test case's .run files append keys.
      // And runObservation.sh adds defaults by prepending keys to the parset.
      std::string hostName = itsParset.getString("Cobalt.FinalMetaDataGatherer.host", "localhost");
      std::string userName = itsParset.getString("Cobalt.FinalMetaDataGatherer.userName", "");
      std::string pubKey = itsParset.getString("Cobalt.FinalMetaDataGatherer.sshPublicKey", "");
      std::string privKey = itsParset.getString("Cobalt.FinalMetaDataGatherer.sshPrivateKey", "");
      std::string executable = itsParset.getString("Cobalt.FinalMetaDataGatherer.executable", "FinalMetaDataGatherer");

      if (userName == "") {
        // No username given -- use $USER
        const char *USER = getenv("USER");

        if (USER)
          userName = USER;
        else {
          LOG_WARN("[FinalMetaData] no userName given in parset and $USER not set. Using 'lofarsys'.");
          userName = "lofarsys";
        }
      }

      if (pubKey == "" && privKey == "") {
        LOG_DEBUG(itsLogPrefix + "[FinalMetaData] no SSH keys given. Try to discover them...");

        char discover_pubkey[1024];
        char discover_privkey[1024];

        if (discover_ssh_keys(discover_pubkey, sizeof discover_pubkey, discover_privkey, sizeof discover_privkey)) {
          pubKey = discover_pubkey;
          privKey = discover_privkey;
        } else {
          LOG_ERROR(itsLogPrefix + "[FinalMetaData] no SSH keys given and discovery failed: failed to obtain final meta data");
          return;
        }
      }

      std::string commandLine = str(boost::format("%s %d")
                                    % executable
                                    % itsParset.observationID()
                                    );

      // Start the remote process
      LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] SSHing to " <<
                    userName << '@' << hostName << " using key pair filenames " <<
                    pubKey << ", " << privKey << " to execute command '" << commandLine << '\'');
      SSHconnection sshconn(itsLogPrefix + "[FinalMetaData] ", hostName, commandLine, userName, pubKey, privKey);
      sshconn.start();

      // Connect
      LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] connecting...");

      const std::string resource = getStorageControlDescription(itsParset.observationID(), -1);
      SmartPtr<Stream> stream;

      // FinalMetaDataGatherer has 5 seconds to start and listen on the
      // PortBroker connection. We need this because SSH.cc/libssh2 sometimes
      // keeps connections open even though FinalMetaDataGatherer cannot even
      // be started.
      //
      // TODO: For now, we also need a deadline for non-real-time observations,
      // because FinalMetaDataGather is not necessarily present.
      const time_t deadline = time(0) + 5;

      // Keep trying to connect to the FinalMetaDataGatherer, but only
      // while the SSH connection is alive. If not, there's no point in waiting
      // for a connection that can never be established.
      while(!stream) {
        if (deadline > 0 && time(0) < deadline)
          return;

        if (sshconn.isDone())
          return;

        try {
          stream = new PortBroker::ClientStream(hostName, storageBrokerPort(itsParset.observationID()), resource, time(0) + 1);
        } catch (SocketStream::TimeOutException &) {
        }
      }

      // Send parset
      LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] connected -- sending parset");
      itsParset.write(stream);
      LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] sent parset");

      // Receive final meta data
      itsFinalMetaData.read(*stream);
      LOG_DEBUG_STR(itsLogPrefix << "[FinalMetaData] [ControlThread] obtained final meta data");

      // Wait for or end the remote process
      sshconn.wait();
    }

  }
}


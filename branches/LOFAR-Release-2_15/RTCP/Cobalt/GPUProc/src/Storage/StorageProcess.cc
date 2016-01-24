//# StorageProcess.cc
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

#include "StorageProcess.h"

#include <sys/time.h>
#include <unistd.h>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Common/Thread/Thread.h>
#include <MessageBus/ToBus.h>
#include <MessageBus/Protocols/TaskFeedbackDataproducts.h>
#include <Stream/PortBroker.h>
#include <CoInterface/Stream.h>
#include <CoInterface/LTAFeedback.h>

namespace LOFAR
{
  namespace Cobalt
  {

    using namespace std;
    using boost::format;


    StorageProcess::StorageProcess( const Parset &parset, const string &logPrefix, int rank, const string &hostname )
      :
      itsParset(parset),
      itsLogPrefix(str(boost::format("%s [StorageWriter rank %2d host %s] ") % logPrefix % rank % hostname)),
      itsRank(rank),
      itsHostname(hostname),
      itsSentFeedback(false),
      itsSuccessful(false)
    {
    }


    StorageProcess::~StorageProcess()
    {
      ScopedDelayCancellation dc; // stop() is a cancellation point

      // stop immediately
      struct timespec immediately = { 0, 0 };
      stop(immediately);

      if (!itsSentFeedback) {
        // send default LTA feedback for this host
        ToBus bus("lofar.task.feedback.dataproducts");

        const std::string myName = "Cobalt/GPUProc/Storage/StorageProcess";

        LTAFeedback feedback(itsParset.settings);

        if (itsParset.settings.correlator.enabled)
          for (size_t i = 0; i < itsParset.settings.correlator.files.size(); ++i) {
            LOG_INFO_STR(itsParset.settings.correlator.files[i].location.host << " == " << itsHostname);

            if (itsParset.settings.correlator.files[i].location.host == itsHostname) {
              Protocols::TaskFeedbackDataproducts msg(
                myName,
                "",
                str(boost::format("Feedback for Correlated Data, subband %s") % i),
                str(format("%s") % itsParset.settings.momID),
                str(format("%s") % itsParset.settings.observationID),
                feedback.correlatedFeedback(i));

              bus.send(msg);
            }
        }

        if (itsParset.settings.beamFormer.enabled)
          for (size_t i = 0; i < itsParset.settings.beamFormer.files.size(); ++i)
            if (itsParset.settings.beamFormer.files[i].location.host == itsHostname) {
              Protocols::TaskFeedbackDataproducts msg(
                myName,
                "",
                str(boost::format("Feedback for Beamformed Data, file nr %s") % i),
                str(format("%s") % itsParset.settings.momID),
                str(format("%s") % itsParset.settings.observationID),
                feedback.beamFormedFeedback(i));

              bus.send(msg);
            }
      }
    }


    void StorageProcess::start()
    {
      ASSERTSTR(!itsThread, "StorageProcess has already been started");

      itsThread = new Thread(this, &StorageProcess::controlThread, str(boost::format("%s ctrl") % itsHostname), itsLogPrefix + "[ControlThread] ", 65535);
    }


    void StorageProcess::stop(struct timespec deadline)
    {
      if (!itsThread) {
        // not started
        return;
      }

      if (!itsThread->wait(deadline)) {
        itsThread->cancel();
        itsThread->wait();
      }
    }


    bool StorageProcess::isSuccesful() const
    {
      return itsSuccessful;
    }


    bool StorageProcess::isDone() const
    {
      return itsThread->isDone();
    }


    void StorageProcess::setFinalMetaData( const FinalMetaData &finalMetaData )
    {
      itsFinalMetaData = finalMetaData;
      itsFinalMetaDataAvailable.up();
    }


    void StorageProcess::controlThread()
    {
      // Connect control stream
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] connecting...");
      std::string resource = getStorageControlDescription(itsParset.settings.observationID, itsRank);
      PortBroker::ClientStream stream(itsHostname, storageBrokerPort(itsParset.settings.observationID), resource, 0);

      // Send parset
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] connected -- sending parset");
      itsParset.write(&stream);
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sent parset");

      // Send final meta data once it is available
      itsFinalMetaDataAvailable.down();

      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sending final meta data");
      itsFinalMetaData.write(stream);
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] sent final meta data");

      // Wait for OutputProc to finish
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] waiting to finish");
      stream.read(&itsSentFeedback, sizeof itsSentFeedback);
      stream.read(&itsSuccessful, sizeof itsSuccessful);
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] finished");
    }

  }
}


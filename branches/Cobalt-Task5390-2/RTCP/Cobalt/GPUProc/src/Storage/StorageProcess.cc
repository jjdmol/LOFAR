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
#include <Stream/PortBroker.h>
#include <CoInterface/Stream.h>

#include "SSH.h"

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

      if (!itsThread->wait(deadline)) {
        itsThread->cancel();
        itsThread->wait();
      }
    }


    bool StorageProcess::isDone() const
    {
      return itsThread->isDone();
    }


    ParameterSet StorageProcess::feedbackLTA() const
    {
      // Prevent read/write conflicts
      ASSERT(isDone());

      return itsFeedbackLTA;
    }


    void StorageProcess::controlThread()
    {
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

      // Wait for LTA feedback
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] reading LTA feedback");
      Parset feedbackLTA(&stream);
      itsFeedbackLTA.adoptCollection(feedbackLTA);
      LOG_DEBUG_STR(itsLogPrefix << "[ControlThread] read LTA feedback");
    }

  }
}


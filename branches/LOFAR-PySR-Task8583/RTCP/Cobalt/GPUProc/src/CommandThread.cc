//# CommandThread.cc: Listen to commands from a Stream, and distribute them
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

#include "CommandThread.h"

#include <Common/LofarLogger.h>
#include <Stream/StreamFactory.h>
#include <CoInterface/MultiDimArray.h>
#include <InputProc/Transpose/MPIProtocol.h>
#include <InputProc/Transpose/MPIUtil.h>

using namespace std;

namespace LOFAR {
  namespace Cobalt {
    CommandThread::CommandThread(const std::string &streamdesc)
    :
      streamdesc(streamdesc)
    {
    }

    CommandThread::~CommandThread()
    {
      stop();
    }

    std::string CommandThread::pop() {
      if (streamdesc == "null:")
        return "";

      try {
        OMPThreadSet::ScopedRun sr(readerThread);

        LOG_INFO_STR("[CommandThread] Reading from " << streamdesc);

        SmartPtr<Stream> commandStream = createStream(streamdesc, true);

        const string command = commandStream->readLine();

        LOG_INFO_STR("[CommandThread] Received command: '" << command << "'");

        return command;
      } catch(EndOfStreamException &) {
        LOG_INFO("[CommandThread] Connection reset by peer");
      } catch(OMPThreadSet::CannotStartException &) {
        /* stop() was called */
        LOG_INFO("[CommandThread] Stopped");
      } catch(SystemCallException &ex) {
        if (ex.error == EINTR)
          /* False positive: stop() was called */
          LOG_INFO("[CommandThread] Stopped");
        else
          LOG_ERROR_STR("[CommandThread] Caught exception: " << ex.what());
      } catch(Exception &ex) {
        LOG_ERROR_STR("[CommandThread] Caught exception: " << ex.what());
      }

      return "";
    }

    void CommandThread::stop() {
      readerThread.killAll();
    }

    std::string CommandThread::broadcast(const std::string &str)
    {
      MPIProtocol::tag_t tag;
      tag.bits.type = MPIProtocol::CONTROL;

      vector<MPI_Request> requests;

      Vector<char> buf(1024, 1, mpiAllocator);

      strncpy(&buf[0], str.c_str(), buf.size() - 1);
      buf[buf.size() - 1] = 0;

      {
        ScopedLock sl(MPIMutex);

        requests = Guarded_MPI_Ibcast(&buf[0], buf.size(), 0, tag.value);
      }

      RequestSet rs(requests, true, "command bcast");
      rs.waitAll();

      buf[buf.size() - 1] = 0;
      return &buf[0];
    }
  }
}


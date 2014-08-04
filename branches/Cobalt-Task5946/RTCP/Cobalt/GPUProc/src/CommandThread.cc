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
#include <CoInterface/Stream.h>
#include <InputProc/Transpose/MPIProtocol.h>
#include <InputProc/Transpose/MPIUtil.h>

using namespace std;

namespace LOFAR {
  namespace Cobalt {
    CommandThread::CommandThread(const std::string &streamdesc)
    :
      streamdesc(streamdesc),
      receivedCommands("receivedCommands", false),

      // We need a separate thread to be able to cancel the syscalls used in Stream.
      // SIGHUP (which we'd otherwise use) does not interrupt all syscalls.
      thread(this, &CommandThread::mainLoop)
    {
    }

    CommandThread::~CommandThread()
    {
      stop();
    }

    void CommandThread::readOneCommand() {
      try {
        SmartPtr<Stream> commandStream = createStream(streamdesc, true);

        const string command = commandStream->readLine();

        LOG_INFO_STR("[CommandThread] Received command: '" << command << "'");

        receivedCommands.append(command);
      } catch(Stream::EndOfStreamException &) {
        LOG_WARN("[CommandThread] Connection reset by peer");
      } catch(Exception &ex) {
        LOG_ERROR_STR("[CommandThread] Caught exception: " << ex.what());
      }
    }

    void CommandThread::mainLoop() {
      if (streamdesc == "null:")
        return;

      while(true) {
        readOneCommand();
      }
    }

    void CommandThread::stop() {
      thread.cancel();
      thread.wait();

      // Send EOS
      receivedCommands.append("");
    }

    std::string CommandThread::pop() {
      return receivedCommands.remove(TimeSpec::universe_heat_death, "");
    }

    void CommandThread::broadcast_send(const std::string &str, int mpiSize)
    {
      MPIProtocol::tag_t tag;
      tag.bits.type = MPIProtocol::CONTROL;

      vector<MPI_Request> requests;

      char buf[1024];
      strncpy(&buf[0], str.c_str(), sizeof buf - 1);
      buf[sizeof buf - 1] = 0;

      for (int r = 1; r < mpiSize; r++)
        requests.push_back(Guarded_MPI_Isend(&buf[0], sizeof buf, r, tag.value));

      RequestSet rs(requests, true, "commandThread");
      rs.waitAll();
    }

    std::string CommandThread::broadcast_receive()
    {
      MPIProtocol::tag_t tag;
      tag.bits.type = MPIProtocol::CONTROL;

      vector<MPI_Request> requests;

      char buf[1024];
      requests.push_back(Guarded_MPI_Irecv(&buf[0], sizeof buf, 0, tag.value));

      RequestSet rs(requests, true, "commandThread");
      rs.waitAll();

      return buf;
    }
  }
}


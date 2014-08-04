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

#ifndef LOFAR_GPUPROC_COMMANDTHREAD_H
#define LOFAR_GPUPROC_COMMANDTHREAD_H

#include <Common/Thread/Thread.h>
#include <CoInterface/Queue.h>

#include <string>

namespace LOFAR {
  namespace Cobalt {
    class CommandThread {
    public:
      CommandThread(const std::string &streamdesc);
      ~CommandThread();

      // Stop accepting commands, and append the "" command.
      void stop();

      // Pop the oldest command
      std::string pop();

      // Broadcast a command from rank 0 to [1, mpiSize)
      static void broadcast_send(const std::string &str, int mpiSize);

      // Receive a broadcasted command from rank 0
      static std::string broadcast_receive();

    private:
      const std::string streamdesc;

      Queue<std::string> receivedCommands;

      Thread thread;

      void mainLoop();

    protected:
      void readOneCommand();
    };
  }
}

#endif



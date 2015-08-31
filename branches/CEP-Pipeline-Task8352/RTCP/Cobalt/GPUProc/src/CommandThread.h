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

#include <CoInterface/Queue.h>
#include <CoInterface/OMPThread.h>

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
      // Rank 0 should provide the string to broadcast,
      // and all ranks return the broadcasted string.
      static std::string broadcast(const std::string &str);

    private:
      const std::string streamdesc;

      OMPThreadSet readerThread;
    };
  }
}

#endif



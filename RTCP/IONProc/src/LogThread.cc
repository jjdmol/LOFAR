//#  LogThread.cc:
//#
//#  Copyright (C) 2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <LogThread.h>
#include <Scheduling.h>
#include <Interface/PrintVector.h>
#include <IONProc/Lock.h>

#include <algorithm>

#include <unistd.h>


namespace LOFAR {
namespace RTCP {


LogThread::LogThread(unsigned nrRspBoards)
:
  itsCounters(nrRspBoards),
  itsShouldStop(false)
{
  if (pthread_create(&thread, 0, logThreadStub, this) != 0) {
    cerr_logger("could not create input thread");
    exit(1);
  }
}


LogThread::~LogThread()
{
  itsShouldStop = true;

  if (pthread_join(thread, 0) != 0) {
    cerr_logger("could not join input thread");
    exit(1);
  }

  clog_logger("LogThread stopped");
}


// log from separate thread, since printing from a signal handler causes deadlocks

void *LogThread::logThreadStub(void *arg)
{
  static_cast<LogThread *>(arg)->logThread();
  return 0;
}


void LogThread::logThread()
{
#if defined HAVE_BGP_ION
  runOnCore0();
#endif

  clog_logger("LogThread running");

  // non-atomic updates from other threads cause race conditions, but who cares

  while (!itsShouldStop) {
    std::stringstream logStr;
    bool somethingRejected = false;

    for (unsigned rsp = 0; rsp < itsCounters.size(); rsp ++) {
      logStr << (rsp == 0 ? "received [" : ",") << itsCounters[rsp].nrPacketsReceived;
      itsCounters[rsp].nrPacketsReceived = 0;

      if (itsCounters[rsp].nrPacketsRejected > 0)
	somethingRejected = true;
    }

    if (somethingRejected)
      for (unsigned rsp = 0; rsp < itsCounters.size(); rsp ++) {
	logStr << (rsp == 0 ? "] packets, rejected [" : ",") << itsCounters[rsp].nrPacketsRejected;
	itsCounters[rsp].nrPacketsRejected = 0;
      }

    logStr << "] packets";
    clog_logger(logStr.str());
    sleep(1);
  }
}

} // namespace RTCP
} // namespace LOFAR

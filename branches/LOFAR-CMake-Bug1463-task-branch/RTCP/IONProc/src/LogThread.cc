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
#include <Common/LofarLogger.h>

#include <algorithm>
#include <cstdio>
#include <sstream>

#include <unistd.h>


namespace LOFAR {
namespace RTCP {


// log from separate thread, since printing from a signal handler causes deadlocks

LogThread::LogThread(unsigned nrRspBoards)
:
  itsCounters(nrRspBoards),
  //thread(((void (LogThread::*)(void)) &LOFAR::RTCP::LogThread::logThread), this)
  thread(this, &LogThread::mainLoop, "LogThread", 65536)
{
  thread.start();
}


LogThread::~LogThread()
{
  thread.abort();

  LOG_DEBUG("LogThread stopped");
}


#if defined HAVE_BGP_ION

bool LogThread::readCPUstats(struct CPUload &load)
{
  FILE *file = fopen("/proc/stat", "r");
  int  retval;

  if (file == 0)
    return false;

  do
    retval = fscanf(file, "cpu %llu %*u %llu %llu %*u %*u %llu %*u\n", &load.user, &load.system, &load.idle, &load.interrupt);
  while (retval != 4 && retval != EOF);

  do
    retval = fscanf(file, "cpu0 %*u %*u %*u %llu %*u %*u %*u %*u\n", &load.idle0);
  while (retval != 1 && retval != EOF);

#if 0
  for (unsigned cpu = 0; cpu < 4 && retval != EOF;)
    if ((retval = fscanf(file, "cpu%*d %*u %*u %*u %llu %*u %*u %*u %*u\n", &load.idlePerCore[cpu])) == 1)
      ++ cpu;
#endif

  fclose(file);
  return retval != EOF;
}


void LogThread::writeCPUstats(std::stringstream &str)
{
  struct CPUload load;

  if (readCPUstats(load)) {
    //str << ", us/sy/in/id: ["
    str << ", us/sy/in/id(0): ["
	<< (unsigned(load.user	    - previousLoad.user)      + 2) / 4 << '/'
	<< (unsigned(load.system    - previousLoad.system)    + 2) / 4 << '/'
	<< (unsigned(load.interrupt - previousLoad.interrupt) + 2) / 4 << '/'
	<< (unsigned(load.idle	    - previousLoad.idle)      + 2) / 4 << '('
	<< (unsigned(load.idle0	    - previousLoad.idle0)) << ")]";
#if 0
	<< "], id: ["
	<< (unsigned(load.idlePerCore[0] - previousLoad.idlePerCore[0]) << '/'

    for (unsigned cpu = 0; cpu < 4; cpu ++)
      str << unsigned(load.idle[cpu] - previousLoad.idle[cpu])
	  << (cpu == 3 ? ']' : ',');
#endif

    previousLoad = load;
  } else {
    str << ", no CPU load info";
  }
}

#endif


void LogThread::mainLoop()
{
#if defined HAVE_BGP_ION
  runOnCore0();
  readCPUstats(previousLoad);
#endif

  LOG_DEBUG("LogThread running");

  // non-atomic updates from other threads cause race conditions, but who cares

  while (!thread.stop) {
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

#if defined HAVE_BGP_ION
    writeCPUstats(logStr);
#endif

    LOG_INFO_STR(logStr.str());
    sleep(1);
  }
}

} // namespace RTCP
} // namespace LOFAR

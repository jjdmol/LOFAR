//#  LogThread.h: log from separate thread, since printing from a signal
//#  handler causes deadlocks
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_IONPROC_LOG_THREAD_H
#define LOFAR_IONPROC_LOG_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!


#include <Interface/Thread.h>

#include <vector>

namespace LOFAR {
namespace RTCP {

class LogThread
{
  public:
    LogThread(unsigned nrRspBoards);
    ~LogThread();

    struct Counters {
      unsigned nrPacketsReceived, nrPacketsRejected;
      unsigned pad[6]; // pad to cache line size to avoid false sharing 
    };

    std::vector<Counters> itsCounters;

  private:
    void	  mainLoop();
  
    volatile bool itsShouldStop;
    Thread	  itsThread;

#if defined HAVE_BGP_ION
    struct CPUload {
      //unsigned long long user, system, interrupt, idle, idlePerCore[4];
      unsigned long long user, system, interrupt, idle, idle0;
    } previousLoad;

    bool readCPUstats(struct CPUload &load);
    void writeCPUstats(std::stringstream &str);
#endif
  };

  // @}

} // namespace RTCP
} // namespace LOFAR

#endif

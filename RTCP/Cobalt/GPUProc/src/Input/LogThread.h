/* LogThread.h: log from separate thread, since printing from a signal
 * handler causes deadlocks
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#ifndef LOFAR_GPUPROC_LOG_THREAD_H
#define LOFAR_GPUPROC_LOG_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <sys/time.h>
#include <string>
#include <vector>
#include <sstream>

#include <Common/Thread/Thread.h>
#include <CoInterface/SmartPtr.h>

namespace LOFAR
{
  namespace Cobalt
  {

    class LogThread
    {
    public:
      LogThread(unsigned nrRspBoards, std::string stationName);
      ~LogThread();

      void start();

      struct Counters {
        unsigned received, badTimeStamp, badSize;
        unsigned pad[5]; // pad to cache line size to avoid false sharing
      };

      std::vector<Counters> itsCounters;

    private:
      void        mainLoop();

      std::string itsStationName;

      SmartPtr<Thread>    itsThread;

#if defined HAVE_BGP_ION
      struct CPUload {
        //unsigned long long user, system, interrupt, idle, idlePerCore[4];
        unsigned long long user, system, interrupt, idle, idle0;
      } previousLoad;

      struct timeval previousTimeval;

      bool readCPUstats(struct CPUload &load);
      void writeCPUstats(std::stringstream &str);
#endif
    };

    // @}

  } // namespace Cobalt
} // namespace LOFAR

#endif


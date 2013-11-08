//# SysInfoLogger.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
#include "SysInfoLogger.h"

#include <fstream>
#include <cstdio>
#include <unistd.h>

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {

    SysInfoLogger::SysInfoLogger()
    :
      thread(this, &SysInfoLogger::mainLoop)
    {
    }

    SysInfoLogger::~SysInfoLogger()
    {
      thread.cancel();
    }

    void SysInfoLogger::mainLoop()
    {
      /*
       * We fetch system statistics, calculate the changes over time,
       * and log warnings if suspicious behaviour is detected.
       */

      struct stats {
        // nr of UDP packets received but discared because nobody was listening
        int noport;

        // nr of UDP packets received but triggered an error (likely, we failed
        // to fetch it from the kernel in time)
        int inerror;
      };

      struct stats old_stats;
      bool first = true;

      for(;;) {
        // Default to previous statistics
        struct stats new_stats = old_stats;

#ifdef __linux__
        // Fetch UDP receive errors
        ifstream snmp("/proc/net/snmp");
        string line;

        while(std::getline(snmp, line))
          if (sscanf(line.c_str(), "Udp: %*d %d %d %*d %*d %*d", &new_stats.noport, &new_stats.inerror) == 2)
            break;
#endif

        if (!first) {
          // Parse differences with previous statistics
          int inerror = new_stats.inerror - old_stats.inerror;

          if (inerror > 0)
            LOG_WARN_STR("Kernel dropped " << inerror << " UDP packets");
        }

        old_stats = new_stats;
        first = false;

        // Don't spam the logs..
        sleep(10);
      }
    }

  }
}


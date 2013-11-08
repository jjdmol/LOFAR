//# SysInfoLogger.h: Periodically analyses system information.
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
//# $Id: cpu_utils.h 25651 2013-07-12 12:19:11Z mol $

// \file
// Include for processor optimalizetion functionality

#ifndef LOFAR_GPUPROC_SYSINFOLOGGER_H
#define LOFAR_GPUPROC_SYSINFOLOGGER_H

#include <Common/Thread/Thread.h>

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * The SysInfoLogger will periodically analyse
     * the system's performance, and log warnings
     * in case of suspicious behaviour.
     *
     * Logging will stop when the destructor is called.
     */
    class SysInfoLogger {
    public:
      SysInfoLogger();
      ~SysInfoLogger();

    private:
      Thread thread;

      void mainLoop();
    };
  }
}
#endif

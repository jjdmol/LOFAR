//# ProcessId.cc: Unique process identifier (a combination of the name of the
//# host where the process is running and the process' PID).
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
#include <BBSControl/ProcessId.h>
#include <Common/LofarLogger.h>
#include <unistd.h>

namespace LOFAR
{
namespace BBS
{

ProcessId::ProcessId()
    :   pid(-1)
{
}

ProcessId::ProcessId(const string &hostname, int64 pid)
    :   hostname(hostname),
        pid(pid)
{
}

ProcessId ProcessId::id()
{
  char hostname[512];
  int status = gethostname(hostname, 512);
  ASSERT(status == 0);
  return ProcessId(string(hostname), getpid());
}

bool operator<(const ProcessId &lhs, const ProcessId &rhs)
{
    // ProcessIds are sorted on pid first as this is a faster comparison.
    return lhs.pid < rhs.pid || (lhs.pid == rhs.pid
      && lhs.hostname < rhs.hostname);
}

bool operator==(const ProcessId &lhs, const ProcessId &rhs)
{
    return lhs.pid == rhs.pid && lhs.hostname == rhs.hostname;
}

ostream& operator<<(ostream& os, const ProcessId &obj)
{
    return os << obj.hostname << ":" << obj.pid;
}

} //# namespace BBS
} //# namespace LOFAR

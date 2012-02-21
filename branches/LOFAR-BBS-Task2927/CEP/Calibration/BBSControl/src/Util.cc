//# Util.cc: Miscellaneous utility functions.
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
#include <BBSControl/Util.h>

namespace LOFAR
{
namespace BBS
{

#ifdef HAVE_PQXX
ProcessGroup makeProcessGroup(const CalSession &session)
{
  ProcessGroup group;

  for(size_t i = 0, end = session.getWorkerCount(CalSession::KERNEL); i < end;
    ++i)
  {
    ProcessId id = session.getWorkerByIndex(CalSession::KERNEL, i);
    Process process;
    process.id = id;
    process.port = 0;
    process.freqRange = session.getFreqRange(id);
    process.timeRange = session.getTimeRange(id);
    group.push_back(ProcessGroup::KERNEL, process);
  }

  for(size_t i = 0, end = session.getWorkerCount(CalSession::SOLVER); i < end;
    ++i)
  {
    ProcessId id = session.getWorkerByIndex(CalSession::SOLVER, i);
    Process process;
    process.id = id;
    process.port = session.getPort(id);
    group.push_back(ProcessGroup::SOLVER, process);
  }

  return group;
}
#endif

pair<unsigned int, unsigned int> parseRange(const string &in)
{
  static const string errorMessage = "Invalid range: ";

  const size_t pos = in.find(':');
  if(pos == string::npos || pos == 0 || in.size() <= (pos + 1))
  {
    THROW(BBSControlException, errorMessage << in);
  }

  pair<unsigned int, unsigned int> out(as<unsigned int>(in.substr(0, pos)),
    as<unsigned int>(in.substr(pos + 1)));
  if(out.second < out.first)
  {
    THROW(BBSControlException, errorMessage << in);
  }

  return out;
}

} //# namespace BBS
} //# namespace LOFAR

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
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>
#include <casa/BasicMath/Math.h>

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
    const ProcessId &id = session.getWorkerByIndex(CalSession::KERNEL, i);
    group.appendReducerProcess(id, session.getFreqRange(id),
      session.getTimeRange(id));
  }

  for(size_t i = 0, end = session.getWorkerCount(CalSession::SOLVER); i < end;
    ++i)
  {
    const ProcessId &id = session.getWorkerByIndex(CalSession::SOLVER, i);
    group.appendSharedEstimatorProcess(id, session.getPort(id));
  }

  return group;
}
#endif

pair<unsigned int, unsigned int> parseRange(const string &in)
{
  static const string errorMessage = "Invalid range specifcation: ";

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

pair<size_t, size_t> parseTimeRange(const Axis::ShPtr &axis,
  const vector<string> &range)
{
  static const string errorMessage = "Invalid time specification: ";

  double start = axis->start(), end = axis->end();
  if(range.size() > 0)
  {
    casa::Quantity time;
    if(!casa::MVTime::read(time, range[0]))
    {
      THROW(BBSControlException, errorMessage << range[0]);
    }

    start = time.getValue("s");
  }

  if(range.size() > 1)
  {
    casa::Quantity time;
    if(!casa::MVTime::read(time, range[1]))
    {
      THROW(BBSControlException, errorMessage << range[1]);
    }

    end = time.getValue("s");
  }

  if(axis->size() == 0 || start > axis->end() || casa::near(start, axis->end())
    || end < axis->start() || casa::near(end, axis->start()))
  {
    // Axis and range do not overlap.
    return pair<size_t, size_t>(1, 0);
  }

  pair<size_t, bool> status;
  pair<size_t, size_t> selection(0, axis->size() - 1);

  status = axis->find(start);
  if(status.second) {
    selection.first = status.first;
  }

  status = axis->find(end);
  if(status.second) {
    selection.second = status.first;
  }

  return selection;
}

} //# namespace BBS
} //# namespace LOFAR

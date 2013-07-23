//# ProcessGroup.cc: List of the processes co-operating in a distributed
//# calibration run.
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
#include <BBSControl/ProcessGroup.h>

namespace LOFAR
{
namespace BBS
{

ProcessGroup::ProcessGroup()
{
}

ProcessGroup::ProcessGroup(const ProcessGroup &other)
{
    initFrom(other);
}

ProcessGroup &ProcessGroup::operator=(const ProcessGroup &other)
{
    if(&other != this)
    {
        clear();
        initFrom(other);
    }

    return *this;
}

size_t ProcessGroup::nProcesses() const
{
  return itsProcessMap.size();
}

size_t ProcessGroup::nProcesses(ProcessType type) const
{
  ASSERT(type < N_ProcessType);
  return itsProcesses[type].size();
}

ProcessGroup::ProcessType ProcessGroup::type(const ProcessId &id) const
{
  ProcessMap::const_iterator it = findProcessByID(id);
  return it->second.first.type;
}

size_t ProcessGroup::index(const ProcessId &id) const
{
  ProcessMap::const_iterator it = findProcessByID(id);
  return it->second.second;
}

const ProcessId &ProcessGroup::id(ProcessGroup::ProcessType type, size_t index)
  const
{
  return itsProcesses[type][index]->first;
}

const Interval<double> &ProcessGroup::range(size_t index, AxisType axis) const
{
  return itsProcesses[REDUCER][index]->second.first.range[axis];
}

unsigned int ProcessGroup::port(size_t index) const
{
  return itsProcesses[SHARED_ESTIMATOR][index]->second.first.port;
}

void ProcessGroup::appendReducerProcess(const ProcessId &id,
  const Interval<double> &freqRange, const Interval<double> &timeRange)
{
  ProcessDescriptor descriptor = {REDUCER, 0, {freqRange, timeRange}};
  appendProcess(REDUCER, id, descriptor);
}

void ProcessGroup::appendSharedEstimatorProcess(const ProcessId &id,
  unsigned int port)
{
  ProcessDescriptor descriptor = {SHARED_ESTIMATOR, port,
    {Interval<double>(), Interval<double>()}};
  appendProcess(SHARED_ESTIMATOR, id, descriptor);
}

void ProcessGroup::clear()
{
  for(size_t i = 0; i < N_ProcessType; ++i)
  {
    itsProcesses[i].clear();
  }

  itsProcessMap.clear();
}

void ProcessGroup::initFrom(const ProcessGroup &other)
{
    for(size_t i = 0; i < other.nProcesses(REDUCER); ++i)
    {
      appendReducerProcess(other.id(REDUCER, i), other.range(i, FREQ),
        other.range(i, TIME));
    }

    for(size_t i = 0; i < other.nProcesses(SHARED_ESTIMATOR); ++i)
    {
      appendSharedEstimatorProcess(other.id(SHARED_ESTIMATOR, i),
        other.port(i));
    }
}

void ProcessGroup::appendProcess(ProcessGroup::ProcessType type,
  const ProcessId &id, const ProcessGroup::ProcessDescriptor &descriptor)
{
  pair<ProcessMap::const_iterator, bool> status =
    itsProcessMap.insert(make_pair(id, make_pair(descriptor,
    nProcesses(type))));

  if(!status.second)
  {
    THROW(BBSControlException, "Process already defined: " << id);
  }

  itsProcesses[type].push_back(status.first);
}

ProcessGroup::ProcessMap::const_iterator
ProcessGroup::findProcessByID(const ProcessId &id) const
{
  ProcessMap::const_iterator it = itsProcessMap.find(id);
  if(it == itsProcessMap.end())
  {
    THROW(BBSControlException, "Unknown process: " << id);
  }

  return it;
}

} //# namespace BBS
} //# namespace LOFAR

//# ProcessGroup.h: List of the processes co-operating in a distributed
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

#ifndef LOFAR_BBSCONTROL_PROCESSGROUP_H
#define LOFAR_BBSCONTROL_PROCESSGROUP_H

// \file
// List of the processes co-operating in a distributed calibration run.

#include <BBSControl/ProcessId.h>
#include <BBSKernel/Types.h>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

struct Process
{
    ProcessId         id;
    size_t            port;
    Interval<double>  freqRange;
    Interval<double>  timeRange;
};

class ProcessGroup
{
public:
    enum ProcessType
    {
        KERNEL,
        SOLVER,
        N_ProcessType
    };

    // Querying the worker register.
    // @{
//    bool slotsAvailable() const;
    size_t getProcessCount() const
    {
      size_t total = 0;
      for(size_t i = 0; i < N_ProcessType; ++i)
      {
        total += itsWorkers[i].size();
      }
      return total;
    }

    size_t getProcessCount(ProcessType type) const
    {
      if(type < N_ProcessType)
      {
        return itsWorkers[type].size();
      }
      return 0;
    }

    size_t getIndex(const ProcessId &id = ProcessId::id()) const
    {
      map<ProcessId, pair<ProcessType, size_t> >::const_iterator it =
        itsProcessMap.find(id);
      ASSERT(it != itsProcessMap.end());
      return it->second.second;
    }

//    ProcessId id(ProcessType type, size_t index) const
//    {
//      return itsWorkers[type][index].id;
//    }

//    bool isWorker(const ProcessId &id) const;
//    bool isKernel(const ProcessId &id) const;
//    bool isSolver(const ProcessId &id) const;

//    size_t getIndex() const;
//    size_t getIndex(const ProcessId &id) const;
//    size_t getPort(const ProcessId &id) const;
////    string getFilesys(const ProcessId &id) const;
////    string getPath(const ProcessId &id) const;
//    Interval<double> getFreqRange(const ProcessId &id) const;
//    Interval<double> getTimeRange(const ProcessId &id) const;
////    Axis::ShPtr getFreqAxis(const ProcessId &id) const;
////    Axis::ShPtr getTimeAxis(const ProcessId &id) const;

//    vector<ProcessId> getWorkersByType(WorkerType type) const;
//    ProcessId getWorkerByIndex(WorkerType type, size_t index) const;
    // @}

    void push_back(ProcessType type, const Process &process = Process())
    {
      pair<map<ProcessId, pair<ProcessType, size_t> >::const_iterator, bool>
        status = itsProcessMap.insert(make_pair(process.id, make_pair(type,
        itsWorkers[type].size())));
      ASSERT(status.second);
      itsWorkers[type].push_back(process);
    }

//    void insert(const Worker &worker,

    Process &process(ProcessType type, size_t i)
    {
      return itsWorkers[type][i];
    }

    const Process &process(ProcessType type, size_t i) const
    {
      return itsWorkers[type][i];
    }

    bool is(const ProcessId &id, ProcessType type) const
    {
      map<ProcessId, pair<ProcessType, size_t> >::const_iterator it =
        itsProcessMap.find(id);
      ASSERT(it != itsProcessMap.end());
      return it->second.first == type;
    }

private:
//    const Worker &getWorkerById(const ProcessId &id) const;

    map<ProcessId, pair<ProcessType, size_t> >  itsProcessMap;
    vector<Process>                             itsWorkers[N_ProcessType];

//    vector<size_t>          itsSlotCount;
//    vector<Worker>          itsWorkers;
//    map<ProcessId, size_t>  itsWorkerMap;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif

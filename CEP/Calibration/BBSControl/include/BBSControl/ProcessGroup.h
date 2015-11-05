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
#include <BBSControl/Exceptions.h>
#include <BBSKernel/Types.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

class ProcessGroup
{
public:
    enum ProcessType
    {
        REDUCER,
        SHARED_ESTIMATOR,
        N_ProcessType
    };

private:
    struct  ProcessDescriptor
    {
      ProcessType       type;
      unsigned int      port;
      Interval<double>  range[N_AxisType];
    };

    typedef map<ProcessId, pair<ProcessDescriptor, size_t> > ProcessMap;

public:
    // Constructor.
    ProcessGroup();

    // Copy constructor.
    ProcessGroup(const ProcessGroup &other);

    // Assignment operator.
    ProcessGroup &operator=(const ProcessGroup &other);

    // Get total number of processes in the process group.
    size_t nProcesses() const;

    // Get number of processes of the specified \p type.
    size_t nProcesses(ProcessType type) const;

    // Find process type given its \p id.
    ProcessType type(const ProcessId &id = ProcessId::id()) const;

    // Find process index given its \p id. Each process type has its own index,
    // which starts at 0 for the first process of a type and increases with 1.
    size_t index(const ProcessId &id = ProcessId::id()) const;

    // Get process id given a process' \p type and \p index.
    const ProcessId &id(ProcessType type, size_t index) const;

    // Get range on FREQ or TIME axis covered by the reducer process \p index.
    const Interval<double> &range(size_t index, AxisType axis) const;

    // Get port on which the shared estimator process \p index is listening.
    unsigned int port(size_t index) const;

    // Append a reducer process to the group.
    void appendReducerProcess(const ProcessId &id,
      const Interval<double> &freqRange, const Interval<double> &timeRange);

    // Append a shared estimator process to the group.
    void appendSharedEstimatorProcess(const ProcessId &id, unsigned int port);

    // Remove all processes.
    void clear();

private:
    void initFrom(const ProcessGroup &other);
    void appendProcess(ProcessType type, const ProcessId &id,
      const ProcessDescriptor &descriptor);
    ProcessMap::const_iterator findProcessByID(const ProcessId &id) const;

    ProcessMap                          itsProcessMap;
    vector<ProcessMap::const_iterator>  itsProcesses[N_ProcessType];
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif

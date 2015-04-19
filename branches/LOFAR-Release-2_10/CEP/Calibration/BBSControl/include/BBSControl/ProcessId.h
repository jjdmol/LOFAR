//# ProcessId.h: Unique process identifier (a combination of the name of the
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

#ifndef LOFAR_BBSCONTROL_PROCESSID_H
#define LOFAR_BBSCONTROL_PROCESSID_H

// \file
// Unique process identifier (a combination of the name of the host where the
// process is running and the process' PID).

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

// Unique process identifier (a combination of the name of the host where the
// process is running and the process' PID).
class ProcessId
{
public:
    ProcessId();
    ProcessId(const string &hostname, int64 pid);

    // Return the ProcessId of the current process.
    static ProcessId id();

    string  hostname;
    int64   pid;
};

// Comparison operators.
bool operator<(const ProcessId &lhs, const ProcessId &rhs);
bool operator==(const ProcessId &lhs, const ProcessId &rhs);

// Output ProcessId in human-readable form.
ostream& operator<<(ostream& os, const ProcessId &obj);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif

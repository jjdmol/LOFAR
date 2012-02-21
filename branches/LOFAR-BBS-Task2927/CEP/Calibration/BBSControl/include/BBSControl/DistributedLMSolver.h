//# DistributedLMSolver.h: Manages connections to multiple (groups of) worker
//# processes. Normal equations received from the worker processes are merged
//# and new parameter estimates are computed by solving the normal equations.
//# The parameter estimates are sent back to the worker processes and the cycle
//# repeats until convergence is reached. Each group of workers (calibration
//# group) is processed independently (although within a single thread).
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

#ifndef LOFAR_BBSCONTROL_DISTRIBUTEDLMSOLVER_H
#define LOFAR_BBSCONTROL_DISTRIBUTEDLMSOLVER_H

// \file
// Manages connections to multiple (groups of) worker processes. Normal
// equations received from the worker processes are merged and new parameter
// estimates are computed by solving the normal equations. The parameter
// estimates are sent back to the worker processes and the cycle repeats until
// convergence is reached. Each group of workers (calibration group) is
// processed independently (although within a single thread).

#include <BBSControl/KernelConnection.h>
#include <Common/Net/Socket.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{
class ProcessGroup;
class SolverOptions;

// \addtogroup BBSControl
// @{

class DistributedLMSolver
{
public:
    typedef shared_ptr<DistributedLMSolver>       Ptr;
    typedef shared_ptr<const DistributedLMSolver> ConstPtr;

    DistributedLMSolver(unsigned int port, unsigned int backlog,
      unsigned int range = 1);

    unsigned int port() const;

    void init(const ProcessGroup &group);
    void run(const vector<unsigned int> &partition,
      const SolverOptions &options);

private:
    Socket                    itsSocket;
    unsigned int              itsPort;
    vector<KernelConnection>  itsConnections;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif

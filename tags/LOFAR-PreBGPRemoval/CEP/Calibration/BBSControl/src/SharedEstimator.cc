//# SharedEstimator.cc: Manages connections to multiple (groups of) worker
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

#include <lofar_config.h>
#include <BBSControl/SharedEstimator.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Messages.h>
#include <BBSControl/ProcessGroup.h>
#include <BBSControl/SolveTask.h>
#include <BBSKernel/Solver.h>
#include <Common/lofar_numeric.h>

namespace LOFAR
{
namespace BBS
{

SharedEstimator::SharedEstimator(unsigned int port,
  unsigned int backlog, unsigned int portRange)
  : itsPort(0)
{
  itsSocket.setName("bbs-shared-estimator");

  // Try to bind to a port from the specified range.
  int status = Socket::BIND;
  unsigned int first = port, last = port + portRange;
  while(first < last)
  {
    // Socket::initServer takes port as a string.
    ostringstream oss;
    oss << first;

    // Try to bind socket to port.
    status = itsSocket.initServer(oss.str(), Socket::TCP, backlog);
    if(status != Socket::BIND)
    {
      break;
    }

    ++first;
  }

  if(status != Socket::SK_OK)
  {
    THROW(IOException, "Unable to initialize server socket using a port from"
      " the range [" << port << "," << port + portRange << ")");
  }

  LOG_DEBUG_STR("Listening on port: " << first);
  itsPort = first;
}

unsigned int SharedEstimator::port() const
{
  return itsPort;
}

void SharedEstimator::init(const ProcessGroup &group)
{
  const size_t nKernels = group.nProcesses(ProcessGroup::REDUCER);

  // Close any existing connections and resize the vector of connections to the
  // appropiate size.
  itsConnections.clear();
  itsConnections.resize(nKernels);

  // Wait for all kernels to connect.
  LOG_DEBUG_STR("Waiting for " << nKernels << " reducers(s) to connect...");

  // Create a TCP socket accepted by the listening socket.
  shared_ptr<BlobStreamableConnection> connection;

  for(size_t i = 0; i < nKernels; ++i)
  {
    connection.reset(new BlobStreamableConnection(itsSocket.accept()));
    if(!connection->connect())
    {
      THROW(IOException, "Connection failed.");
    }

    scoped_ptr<const ProcessIdMsg>
      msg(dynamic_cast<ProcessIdMsg*>(connection->recvObject()));
    if(!msg)
    {
      THROW(ProtocolException, "Expected a ProcessIdMsg.");
    }

    ASSERT(group.type(msg->getProcessId()) == ProcessGroup::REDUCER);
    KernelIndex index = group.index(msg->getProcessId());
    ASSERT(index >= 0 && static_cast<size_t>(index) < itsConnections.size());
    ASSERT(itsConnections[index].index() == -1);

    itsConnections[index] = KernelConnection(connection, index);

    LOG_DEBUG_STR("Reducer " << i+1 << " of " << nKernels << " connected"
      " (index=" << index << ")");
  }
}

void SharedEstimator::run(const vector<unsigned int> &partition,
  const SolverOptions &options)
{
  // Sanity check
  ASSERT(itsConnections.size() == accumulate(partition.begin(), partition.end(),
    0U));

  // Create calibration groups, passing the correct subrange of kernel
  // connections to each group.
  vector<SolveTask> tasks;
  vector<KernelConnection>::const_iterator it = itsConnections.begin();
  for(unsigned int i = 0; i < partition.size(); ++i)
  {
    tasks.push_back(SolveTask(vector<KernelConnection>(it, it + partition[i]),
      options));
    it += partition[i];
  }

  // Call the run() method on each calibration group. In the current
  // implementation, this is a serialized operation. Once we run each
  // calibration group in its own thread, we can parallellize things. Threads
  // will also make it possible to return swiftly from the current method.
  //
  // [Q] Should we let run() return a bool or do we handle error conditions with
  // exceptions. I think the former (bools) is a better choice, since we're
  // planning on running each task in a separate thread, and it is usually a bad
  // thing to handle an exception in a different thread than in which it was
  // thrown.
  bool done = false;
  while(!done)
  {
    done = true;
    for(unsigned int i = 0; i < tasks.size(); ++i)
    {
      done = tasks[i].run() && done;
    }
  }
}

} //# namespace BBS
} //# namespace LOFAR

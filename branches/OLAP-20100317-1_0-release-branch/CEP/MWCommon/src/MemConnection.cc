//# MemConnection.cc: Memory connection to a worker
//#
//# Copyright (c) 2007
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

#include <MWCommon/MemConnection.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>

namespace LOFAR { namespace CEP {

  MemConnection::MemConnection (const WorkerProxy::ShPtr& worker)
    : itsWorker (worker)
  {}

  MemConnection::~MemConnection()
  {}

  int MemConnection::getMessageLength()
  {
    ASSERTSTR (itsResult.size() > 0,
		 "MemConnection: no result has been received");
    return itsResult.size();
  }

  void MemConnection::receive (void* buf, unsigned size)
  {
    ASSERT (itsResult.size() == size);
    memcpy (buf, itsResult.data(), size);
    // Clear buffer to make sure data cannot be read twice.
    itsResult.resize (0);
  }

  void MemConnection::write (const LOFAR::BlobString& data)
  {
    // Internal buffer must be empty, otherwise no read was done.
    ASSERTSTR (itsResult.size() == 0,
		 "MemConnection: received result has not been read");
    // Let the worker process the data and keep its result.
    itsWorker->handleMessage (data, itsResult);
  }

  void MemConnection::send (const void*, unsigned)
  {
    THROW (MWError, "MemConnection::send should not be called");
  }

}} // end namespaces

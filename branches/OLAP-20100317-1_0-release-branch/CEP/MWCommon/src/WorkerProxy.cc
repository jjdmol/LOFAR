//# WorkerProxy.cc: Base class for the proxy of a worker
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

#include <MWCommon/WorkerProxy.h>
#include <MWCommon/WorkerInfo.h>
#include <MWCommon/MasterControl.h>
#include <MWCommon/MWBlobIO.h>
#include <MWCommon/SocketConnection.h>
#include <MWCommon/MWError.h>
#include <Blob/BlobArray.h>


namespace LOFAR { namespace CEP {

  WorkerProxy::WorkerProxy()
    : itsWorkerId (-1)
  {}

  WorkerProxy::~WorkerProxy()
  {}

  void WorkerProxy::putWorkerInfo (LOFAR::BlobString& out)
  {
    // Write the work types and the host name of the worker.
    MWBlobOut bout(out, 0, 0);
    WorkerInfo info(SocketConnection::getHostName(), getWorkTypes());
    bout.blobStream() << info;
    bout.finish();
  }

  WorkerInfo WorkerProxy::getWorkerInfo (LOFAR::BlobString& in)
  {
    // Read back from blob string.
    MWBlobIn bin(in);
    ASSERT (bin.getOperation() == 0);
    WorkerInfo info;
    bin.blobStream() >> info;
    return info;
  }

  bool WorkerProxy::handleMessage (const LOFAR::BlobString& in,
                                   LOFAR::BlobString& out)
  {
    MWBlobIn bin(in);
    int operation = bin.getOperation();
    if (operation < 0) {
      quit();
    } else {
      // Set the (unique) worker id when initializing.
      if (operation == MasterControl::Init) {
        itsWorkerId = bin.getWorkerId();
      }
      // Create the output blob using the operation of the input.
      // The process function can reset the operation.
      // Do timings of the process functions and put them into the blob.
      MWBlobOut bout(out, operation, bin.getStreamId(), itsWorkerId);
      casa::Timer timer;
      LOFAR::NSTimer precTimer;
      precTimer.start();
      int oper = operation;
      if (operation == MasterControl::Init) {
	ParameterSet parset;
	string dataPartName;
	///	bin.blobStream() >> parset >> dataPartName;
	bin.blobStream() >> parset >> dataPartName;
	setInitInfo (parset, dataPartName);
      } else {
	oper = process (operation, bin.getStreamId(),
			bin.blobStream(), bout.blobStream());
      }
      // Set the timings.
      precTimer.stop();
      if (oper < 0) {
        // Do not send a reply.
        out.resize (0);
      } else {
        bout.setTimes (timer, precTimer);
        // Reset operation if changed.
        if (oper != bin.getOperation()) {
          bout.setOperation (oper);
        }
        bout.finish();
      }
    }
    bin.finish();
    return operation >= 0;
  }

  void WorkerProxy::quit()
    {}

}} // end namespaces

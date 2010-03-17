//# WorkerControl.cc: Worker connection of distributed VDS processing
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

#include <MWCommon/WorkerControl.h>
#include <Blob/BlobString.h>

using namespace std;


namespace LOFAR { namespace CEP {

  WorkerControl::WorkerControl (const WorkerProxy::ShPtr& proxy)
    : itsProxy (proxy)
  {}

  void WorkerControl::init (const MWConnection::ShPtr& connection)
  {
    itsConnection = connection;
  }

  void WorkerControl::run()
  {
    LOFAR::BlobString bufIn, bufOut;
    // Start with sending the work types.
    itsProxy->putWorkerInfo (bufOut);
    itsConnection->write (bufOut);
    // Read data until an end command is received.
    while (true) {
      bufIn.resize (0);
      bufOut.resize (0);
      itsConnection->read (bufIn);
      if (! itsProxy->handleMessage (bufIn, bufOut)) {
	break;
      }
      if (bufOut.size() > 0) {
	itsConnection->write (bufOut);
      }
    }
  }

}} // end namespaces

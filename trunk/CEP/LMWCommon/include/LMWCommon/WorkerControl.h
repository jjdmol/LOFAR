//# WorkerControl.h: High level worker control
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_LMWCOMMON_WORKERCONTROL_H
#define LOFAR_LMWCOMMON_WORKERCONTROL_H

// @file
// @brief High level worker control.
// @author Ger van Diepen (diepen AT astron nl)

#include <LMWCommon/WorkerProxy.h>
#include <LMWCommon/MWConnection.h>


namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief High level worker control.

  // This class if the high level control of a proxy worker.
  // The \a init function sets up the connection and does the initialisation.
  // The \a run function receives commands from the master
  // control, lets the proxy execute them, and sends replies back.
  // When the quit command is received, the \a run function will end.

  class WorkerControl
  {
  public:
    // Construct with the given proxy, that will execute the commands.
    WorkerControl (const WorkerProxy::ShPtr& proxy);

    // Initialise the connection and send an init message to the master.
    void init (const MWConnection::ShPtr& connection);

    // Receive and execute messages until an end message is received.
    void run();

  private:
    MWConnection::ShPtr itsConnection;
    WorkerProxy::ShPtr  itsProxy;
  };

}} //# end namespaces

#endif

//# SocketListener.cc: Class that creates a socket and accepts connections
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

#include <MWCommon/SocketListener.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>


namespace LOFAR { namespace CEP {

  SocketListener::SocketListener (const std::string& port)
    : itsConnSocket (new LOFAR::Socket("mwsck", port))
  {}

  SocketConnection::ShPtr SocketListener::accept()
  {
    LOFAR::Socket* socket = itsConnSocket->accept();
    SocketConnection::ShPtr dataConn(new SocketConnection(socket));
    int status = itsConnSocket->errcode();
    ASSERTSTR (socket  &&  status == LOFAR::Socket::SK_OK,
                 "SocketConnection server did not accept on host "
                 << itsConnSocket->host() << ", port " << itsConnSocket->port()
                 << ", LOFAR::Socket status " << status << ' '
                 << itsConnSocket->errstr());
    ASSERT (dataConn->isConnected());
    return dataConn;
  }

}} // end namespaces

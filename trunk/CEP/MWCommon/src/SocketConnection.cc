//# SocketConnection.cc: Connection to workers based on a socket
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

#include <MWCommon/SocketConnection.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>
#include <unistd.h>             // for gethostname


namespace LOFAR { namespace CEP {

  SocketConnection::SocketConnection (const std::string& hostName,
                                      const std::string& port)
    : itsConnSocket ("mwsck", hostName, port),
      itsDataSocket (0)
  {}

  SocketConnection::SocketConnection (LOFAR::Socket* conn)
    : itsDataSocket (conn)
  {}

  SocketConnection::~SocketConnection()
  {
    if (itsDataSocket != &itsConnSocket) {
      delete itsDataSocket;
    }
  }

  bool SocketConnection::isConnected() const
  {
    return itsDataSocket  &&  itsDataSocket->isConnected();
  }

  int SocketConnection::getMessageLength()
  {
    return -1;
  }

  void SocketConnection::receive (void* buf, unsigned size)
  {
    if (!itsDataSocket) {
      init();
    }
    char* cbuf = static_cast<char*>(buf);
    while (size > 0) {
      int sz = itsDataSocket->read (cbuf, size);
      ASSERTSTR (sz>=0,
                   "Read on socket failed: " << itsDataSocket->errstr());
      cbuf += sz;
      size -= sz;
    }
  }

  void SocketConnection::send (const void* buf, unsigned size)
  {
    if (!itsDataSocket) {
      init();
    }
    itsDataSocket->writeBlocking (buf, size);
  }

  void SocketConnection::init()
  {
    // Create a client socket.
    // Try to connect: may fail if no listener started yet.
    // So retry during one minute.
    int status;
    for (int i=0; i<60; ++i) {
      status = itsConnSocket.connect();
      if (status == LOFAR::Socket::SK_OK) {
        // Connected, so socket can be used to send/receive data.
        itsDataSocket = &itsConnSocket;
        break;
      }
      sleep (1);
    }
    ASSERTSTR (status == LOFAR::Socket::SK_OK,
                 "SocketConnection client could not connect to host "
                 << itsConnSocket.host() << ", port "
                 << itsConnSocket.port()
                 << ", LOFAR::Socket status " << status << ' '
                 << itsConnSocket.errstr());
    ASSERT (isConnected());
  }

  std::string SocketConnection::getHostName()
  {
    char nm[256];
    ::gethostname(nm, sizeof(nm));
    return std::string(nm);
  }

}} // end namespaces

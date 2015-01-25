//# SocketConnection.h: Connection to workers based on a socket
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

#ifndef LOFAR_LMWCOMMON_SOCKETCONNECTION_H
#define LOFAR_LMWCOMMON_SOCKETCONNECTION_H

// @file
// @brief Connection to workers based on a socket.
// @author Ger van Diepen (diepen AT astron nl)

#include <LMWCommon/MWConnection.h>
#include <Common/Net/Socket.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief Connection to workers based on a socket.

  // This class handles the socket connection between two processes.
  // For a client it can set up the connection to a server on a given
  // host and port.
  // For a server it can hold the connection created by SocketListener.
  //
  // It is meant to send and receive blobs. The length of a message to
  // receive is read (by base class MWConnection) from the blob header.

  class SocketConnection: public MWConnection
  {
  public:
    // Define a shared pointer to this object.
    typedef boost::shared_ptr<SocketConnection> ShPtr;

    // Set up the client side of a connection.
    // Upon the first send or receive it connects to the server
    // on the given host and port.
    // If making the connection fails, it will sleep one second and try
    // again for up to 60 attempts. In this way the case is handled
    // where a server is started a bit later than a client.
    SocketConnection (const std::string& hostName, const std::string& port);

    // Add a socket from the server when it accepted a connection
    // (used by SocketListener).
    // It takes over the ownership of the pointer.
    explicit SocketConnection (LOFAR::Socket*);

    virtual ~SocketConnection();

    // Check the state of the connection.
    virtual bool isConnected() const;

    // Get the length of the message.
    // Always returns -1 indicating that the length has to be read
    // from the header.
    virtual int getMessageLength();

    // Receive the data sent by the destination
    // and wait until data has been received into buf.
    virtual void receive (void* buf, unsigned size);

    // Send the data to the destination
    // and wait until the data has been sent.
    virtual void send (const void* buf, unsigned size);

    // Get the name of the host this process is running on.
    // If sockets are not supported (e.g. Cray), it returns an empty string.
    static std::string getHostName();

  private:
    // Initialize the connection.
    void init();

    LOFAR::Socket  itsConnSocket;
    LOFAR::Socket* itsDataSocket;
  };

}} //# end namespaces

#endif

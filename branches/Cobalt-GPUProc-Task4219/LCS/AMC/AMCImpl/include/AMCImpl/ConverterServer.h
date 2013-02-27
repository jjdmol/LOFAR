//# ConverterServer.h: server side of the AMC client/server implementation.
//#
//# Copyright (C) 2002-2004
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

#ifndef LOFAR_AMCIMPL_CONVERTERSERVER_H
#define LOFAR_AMCIMPL_CONVERTERSERVER_H

// \file
// Server side of the AMC client/server implementation

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/LofarTypes.h>
#include <Common/Net/Socket.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCImpl
    // @{

    // This class represents the server side of the client/server
    // implementation of the AMC. Its main purpose is to handle incoming
    // connection requests from ConverterClient objects. It does \e not
    // implement the Converter interface.
    //
    // Whenever the server accepts a connection, it immediately creates a new
    // ConverterProcess object and spawns the current process. The newly
    // created process will then handle all client requests until the client
    // disconnects. The server will continue listening for incoming
    // connections requests.
    class ConverterServer
    {
    public:
      // Constructor. The server will by default use port 31337 to listen for
      // client connection requests.
      explicit ConverterServer(uint16 port = 31337);

      // Destructor.
      ~ConverterServer();

      // Start running the event-loop. The event loop will continuously call
      // the handleConnections() method, which blocks until it receives a
      // connection request.
      void run();

    private:
      //@{
      // Make this class non-copyable.
      ConverterServer(const ConverterServer&);
      ConverterServer& operator=(const ConverterServer&);
      //@}

      // This method handles incoming connection requests. It blocks until it
      // receives a connection request. For each connection a new process is
      // spawned that will further handle any coordinate conversion requests,
      // until the client side closes the connection.
      void handleConnections();

      // On this socket we will be listening for incoming connection requests.
      Socket itsListenSocket;

    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif

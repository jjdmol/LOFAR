//#  ConverterServer.h: one line description
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_AMCIMPL_AMCSERVER_CONVERTERSERVER_H
#define LOFAR_AMCIMPL_AMCSERVER_CONVERTERSERVER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/LofarTypes.h>
#include <Common/Net/Socket.h>

namespace LOFAR
{
  namespace AMC
  {
    // This class represents the server side of the client/server
    // implementation of the AMC. Its main purpose is to handle the
    // communication with the ConverterClient. It does \e not implement the
    // Converter interface. It uses ConverterImpl to handle the conversion
    // requests from the clients.
    class ConverterServer
    {
    public:
      // Constructor. The server will by default use port 31337 to listen for
      // client connection requests.
      explicit ConverterServer(uint16 port = 31337);

      // Destructor.
      ~ConverterServer();

      // Start running the event-loop that will continuously monitor for
      // incoming connections. When a connection request is received the
      // handleConnections() method is called to further handle the client
      // request.
      void run();

    private:
      //@{
      // Make this class non-copyable.
      ConverterServer(const ConverterServer&);
      ConverterServer operator=(const ConverterServer&);
      //@}

      // This method handles incoming connection requests. For each
      // connection a new process is spawned that will further handle any
      // coordinate conversion requests, until the client side closes the
      // connection.
      void handleConnections();

      // On this socket we will be listening for incoming connection requests.
      Socket itsListenSocket;

    };


  } // namespace AMC

} // namespace LOFAR

#endif

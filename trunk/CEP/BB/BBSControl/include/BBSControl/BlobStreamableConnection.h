//#  BlobStreamableConnection.h: Connection class linking global and local control.
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

#ifndef LOFAR_BBSCONTROL_BLOBSTREAMABLECONNECTION_H
#define LOFAR_BBSCONTROL_BLOBSTREAMABLECONNECTION_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Connection class linking global and local control.

//# Includes
#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBS
    // @{

    // This class provides a client-server connection to be used within
    // BBS. It encapsulates a CSConnection class for connecting client and
    // server, a TH_Socket class for creating the socket and a
    // DH_BlobStreamable class for exchanging data.
    class BlobStreamableConnection
    {
    public:
      // Create a server connection end-point.
      BlobStreamableConnection(const string& port, 
                               int32 protocol = Socket::TCP,
                               bool blocking = true);

      // Create a client connection end-point.
      BlobStreamableConnection(const string& server, 
                               const string& port, 
                               int32 protocol = Socket::TCP,
                               bool blocking = true);

      // Create a connection using an existing data socket.
      BlobStreamableConnection(Socket* socket);

      // Try to connection to the other side.
      bool connect();

      // Send the BlobStreamable object \a obj.
      bool sendObject(const BlobStreamable& obj);

      // Receive a BlobStreamable object. A new object will be constructed on
      // the heap.
      BlobStreamable* recvObject();

    private:
      // DataHolder for exchanging data between global (BBS) and local
      // (BBSKernel) process control.
      DH_BlobStreamable itsDataHolder;

      // TransportHolder used to exchange DataHolders. The global (BBS)
      // controller will open a server connection, waiting for local
      // (BBSKernel) client controllers to connect.
      TH_Socket itsTransportHolder;

      // Connection between the global (BBS) process control and the local
      // (BBSKernel) process control.
      scoped_ptr<CSConnection> itsConnection;

      // Flag indicating whether we should try to reconnect when a read or
      // write on \c itsConnection returns an error. By default, a server will
      // not try to reconnect, but simply assume that the client has
      // disconnected. A client, however, will try to reconnect to the server.
      bool itsDoReconnect;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif

//#  BlobStreamableConnection.cc: Connection class linking global and local control.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/BlobStreamableConnection.h>
#include <Common/Exceptions.h>

namespace LOFAR
{
  namespace BBS
  {

    BlobStreamableConnection::BlobStreamableConnection(const string& port, 
                                                       int32 protocol,
                                                       bool blocking) : 
      itsTransportHolder(port, blocking, protocol, 5, true),
      itsDoReconnect(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    BlobStreamableConnection::BlobStreamableConnection(const string& server,
                                                       const string& port,
                                                       int32 protocol,
                                                       bool blocking) :
      itsTransportHolder(server, port, blocking, protocol, true),
      itsDoReconnect(true)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    BlobStreamableConnection::BlobStreamableConnection(Socket* socket) :
      itsTransportHolder(socket),
      itsDoReconnect(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }

    bool BlobStreamableConnection::connect()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      if(!itsTransportHolder.init())
        return false;
      
      // Create a new CSConnection object.
      itsConnection.reset(new CSConnection("BlobStreamableConnection", 
        &itsDataHolder, 
        &itsDataHolder, 
        &itsTransportHolder));

      return (itsConnection && itsConnection->isConnected());
    }

    
    bool BlobStreamableConnection::sendObject(const BlobStreamable& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      if(!itsConnection)
      {
        LOG_ERROR("sendObject() called while not connected.");
        return false;
      } 
      
      // Serialize the object
      itsDataHolder.serialize(bs);
      
      // Do a blocking send. If the write fails, we may have lost the
      // connection. 
      LOG_DEBUG_STR("Sending a " << itsDataHolder.classType() << " object");
      if (itsConnection->write() == CSConnection::Error)
      {
        LOG_WARN("BlobStreamableConnection::sendObject() - Connection error");

        // If \c itsDoReconnect is \c true, try to reconnect.
        if (itsDoReconnect)
        {
          LOG_DEBUG("Trying to reconnect ...");
          // Try to reconnect
          if (!itsTransportHolder.init()) 
            THROW (IOException, "Failed to reconnect");
        }
        return false;
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsConnection->waitForWrite();

      // When we get here, everything went well.
      return true;
    }


    BlobStreamable* BlobStreamableConnection::recvObject()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      if(!itsConnection)
      {
        LOG_ERROR("recvObject() called while not connected.");
        return false;
      }
      
      BlobStreamable* bs(0);

      // Do a blocking receive. If the read fails, we may have lost the
      // connection.
      if (itsConnection->read() == CSConnection::Error)
      {
        LOG_WARN("BlobStreamableConnection::recvObject() - Connection error");

        // If \c itsDoReconnect is \c true, try to reconnect.
        if (itsDoReconnect) 
        {
          LOG_DEBUG("Trying to reconnect ...");
          // Try to reconnect
          if (!itsTransportHolder.init())
            THROW (IOException, "Failed to reconnect");
        }
        return 0;
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsConnection->waitForRead();

      // Deserialize the object
      LOG_DEBUG_STR("Received a " << itsDataHolder.classType() << " object");
      if (!(bs = itsDataHolder.deserialize())) 
      {
        LOG_ERROR_STR("BlobStreamableConnection::recvObject() - "
                      "Error deserializing object " << 
                      itsDataHolder.classType());
        return 0;
      }
      
      // When we get here, everything went well. Return the object.
      return bs;
    }
   

  } // namespace BBS

} // namespace LOFAR

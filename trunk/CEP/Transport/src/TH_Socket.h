//# TH_Socket.h: Socket based TransportHolder
//#              Based on the Socket wrapper class 
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef TRANSPORT_TH_SOCKET_H
#define TRANSPORT_TH_SOCKET_H

#include <lofar_config.h>
#include <Transport/TransportHolder.h>
#include <Common/Net/Socket.h>

namespace LOFAR
{
  class TH_Socket: public TransportHolder
  {
  public:
    // Create the socket transport holder.
    // Argument serverAtSender determines which peer opens the connection
    // and is completely independent of the direction of the transfer
    // (i.e. the sender can either open or accept a connection).
    // The server is said to be the peer that accepts the connection.
    TH_Socket (const std::string& sendhost, 
	       const std::string& recvhost, 
	       int portno,
	       bool serverAtSender=true);
    
    virtual ~TH_Socket();
    
    virtual TH_Socket* make() const;
     
    /// Get the type of transport.
    virtual string getType() const;

    /// Read the data.
    virtual bool recvBlocking (void* buf, int nbytes, int tag);
    virtual bool recvVarBlocking (int tag);
    virtual bool recvNonBlocking (void* buf, int nbytes, int tag);
    virtual bool recvVarNonBlocking (int tag);
    /// Wait for the data to be received
    virtual bool waitForReceived (void* buf, int nbytes, int tag);

    /// Write the data.
    virtual bool sendBlocking (void* buf, int nbytes, int tag);
    virtual bool sendNonBlocking (void* buf, int nbytes, int tag);
    /// Wait for the data to be sent
    virtual bool waitForSent (void* buf, int nbytes, int tag);

    virtual bool connectionPossible (int srcRank, int dstRank) const;

    virtual bool isBlocking() const
      { return true; }
    
    virtual bool init();
    
  private:
    bool connectToServer();
    bool connectToClient();

    std::string itsSendingHostName;
    std::string itsReceivingHostName;
    int         itsPortNo;
    bool        itsIsConnected;
    bool        itsServerAtSender;
    Socket*     itsServerSocket;
    Socket*     itsDataSocket;
  };
  
}

#endif

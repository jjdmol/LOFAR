//# TH_Socket.h: Socket based TransportHolder
//#              Based on the Socket wrapper class from Common
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

// \file TH_Socket.h
// Socket based TransportHolder. Based on the Socket wrapper class from
// Common.

#include <lofar_config.h>
#include <Common/lofar_string.h>
#include <Transport/TransportHolder.h>
#include <Common/Net/Socket.h>

namespace LOFAR
{
  // \addtogroup Transport
  // @{

class TH_Socket: public TransportHolder
{
public:
    // Create the socket transport holder.
	// The sourceName and destinationName correspond with the machines on which
	// the endpoints (dataholders) are situated. 
	// The flag listenerAtDestination determines which peer starts the listener.
	// When sAD is true the dataholder TO which the connection is made starts
	// the listener, this is the most common situation.
	// When sAD is false the dataholder FROM which the connection originates
	// will start the listener.
	//
	// E.g.: DH1.connectTo(DH2, TH_Socket([host_DH1], host_DH2, port, true))
	//
	//    DH1 ---------> DH2
	//    Clnt           Srvr
	//
	// E.g.: DH1.connectTo(DH2, TH_Socket(host_DH1, [host_DH2], port, false))
	//
	//    DH1 ---------> DH2
	//    Srvr           Clnt
	//
	// Note: one of the hostnames is always obsolete, but this makes the API
	//       more confirm to other TH API's.
	//
    TH_Socket (const string&	sourceName,
			   const string&	destinationName, 
			   int32			portno,
			   const bool		listenerAtDestination = true,
			   const bool		blocking = true);
    
    virtual ~TH_Socket();
    
	// Make an instance of the transportholder
    virtual TH_Socket* make() const;

	// Sets up the connection(s) between server and client
	virtual bool	init();
     
    // Get the type of transport.
    virtual string getType() const;

    // \name Read the data.
    // @{
    virtual bool recvBlocking 		(void* buf, int32 nbytes, int32 tag);
    virtual bool recvVarBlocking 	(int32 tag);
    virtual bool recvNonBlocking 	(void* buf, int32 nbytes, int32 tag);
    virtual bool recvVarNonBlocking (int32 tag);
    // @}

    // Wait for the data to be received
    virtual bool waitForReceived 	(void* buf, int32 nbytes, int32 tag);

    // \name Write the data.
    // @{
    virtual bool sendBlocking 		(void* buf, int32 nbytes, int32 tag);
    virtual bool sendNonBlocking	(void* buf, int32 nbytes, int32 tag);
    // @}

    // Wait for the data to be sent
    virtual bool waitForSent 		(void* buf, int32 nbytes, int32 tag);

    virtual bool connectionPossible (int32 srcRank, int32 dstRank) const;
	virtual bool isBidirectional() const
		{ return(true); }

    virtual bool isBlocking() const
      { return (itsDataSocket && itsDataSocket->isBlocking()); }
   
	// Specialties for TH_Socket.
	bool			setDataSocket   (Socket*	aDataSocket);
	inline Socket*	getDataSocket   () const; 
	bool			setListenSocket (Socket*	aListenSocket);
	inline Socket*	getListenSocket () const; 
    bool			connectToServer (int32	waitMs = -1);
    bool			connectToClient (int32	waitMs = -1);

private:
	typedef enum {
		CmdNone = 0,
		CmdRecvNonBlock,
		CmdRecvVarNonBlock,
	} CmdTypes;

    string		itsServerHostname;
    int32		itsPort;
    bool		itsIsConnected;
    bool		itsSyncComm;
    bool		itsDestHasListener;
    Socket*		itsServerSocket;
    Socket*		itsDataSocket;
	int32		itsReadOffset;

	// Administration for non-blocking receiving. In the recv-call
	// these fields are filled so that waitForRecv knows what to do.
	int16		itsLastCmd;
};
  
//
// getDataSocket
//
inline Socket* TH_Socket::getDataSocket() const
{
	return (itsDataSocket);
}

//
// getListenSocket
//
inline Socket* TH_Socket::getListenSocket() const
{
	return (itsServerSocket);
}

  // @} // Doxygen endgroup Transport

} // namespace LOFAR

#endif

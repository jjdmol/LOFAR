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

#ifndef LOFAR_TRANSPORT_TH_SOCKET_H
#define LOFAR_TRANSPORT_TH_SOCKET_H

// \file
// Socket based TransportHolder. Based on the Socket wrapper class from
// Common.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/lofar_string.h>
#include <Transport/TransportHolder.h>
#include <Common/Net/Socket.h>

namespace LOFAR {

// \addtogroup Transport
// @{

class TH_Socket: public TransportHolder
{
public:
	// Create a TH_Socket with a server socket (starts a listener).
	// Note: this is a little bit strange form of a TH_Socket since it owns
	//       a listener-socket and a datasocket whereas the other constructors
	//       only contain a datasocket.
    TH_Socket (const string& 	service,
	       const bool		sync = true,
	       int32			protocol = Socket::TCP,
	       int32			backlog = 5,
	       const bool openSocketNow = true,
	       const int recvBufferSize = -1,
	       const int serdBufferSize = -1);

	// Create a TH_Socket with a client socket.
    TH_Socket (const string&	hostName,
	       const string& 	service,
	       const bool		sync = true,
	       int32			protocol = Socket::TCP,
	       const bool openSocketNow = true,
	       const int recvBufferSize = -1,   
	       const int sendBufferSize = -1);

    // Create a TH_Socket based on an existing data socket.
    TH_Socket (Socket*		aDataSocket);
    
    virtual ~TH_Socket();

    /// Get the type of transport.
    virtual string getType() const;

    /// Is TH_Socket clonable?
    virtual bool isClonable() const;

    // Resets all members which are source or destination specific.
    virtual void reset();    

    /// Read the data.
    virtual bool recvBlocking 		(void* buf, int32 nbytes, int32 tag,
									 int offset = 0, DataHolder* dh = 0);
    virtual int32 recvNonBlocking 	(void* buf, int32 nbytes, int32 tag,
									 int offset = 0, DataHolder* dh = 0);
    /// Wait for the data to be received
    virtual void waitForReceived 	(void* buf, int32 nbytes, int32 tag);

    /// Write the data.
    virtual bool sendBlocking 		(void* buf, int32 nbytes, int32 tag,
									 DataHolder* dh = 0);
    virtual bool sendNonBlocking	(void* buf, int32 nbytes, int32 tag,
									 DataHolder* dh = 0);
    /// Wait for the data to be sent
    virtual void waitForSent 		(void* buf, int32 nbytes, int32 tag);

	// new
	virtual bool isConnected() const
		{ return (itsDataSocket && itsDataSocket->isConnected()); }

	// make the connection
	bool		init();
	// init kernel level socket buffer sizes
	bool initBuffers(int recvBufferSize = -1, int sendBufferSize = -1);


private:
	bool            openSocket();
	bool		connectToServer();
	bool		connectToClient();

	// Close socket \a aSocket; delete it when we \e own it.
	void		close(Socket*& aSocket);

	typedef enum {
		CmdNone = 0,
		CmdRecvNonBlock,
	} CmdTypes;

	bool            itsIsServer;
    Socket*		itsServerSocket;		// Listener socket (server only)
    Socket*		itsDataSocket;			// The transport channel.
	bool		itsIsOwner;			// Owner of socket(s).
	bool		itsIsClosed;			// Has close been called?
	int32		itsReadOffset;			// For partial reads.

	string itsHostName;
	string itsService;
	int32 itsProtocol;
	int32 itsBacklog;
	bool itsIsBlocking;

	// Administration for non-blocking receiving. In the recv-call
	// these fields are filled so that waitForRecv knows what to do.
	int16		itsLastCmd;

	/// when set, the Socket transport holder will adjust the
	/// kernel level buffer size to the value provided when the
	/// socket is initialised.
	int itsRecvBufferSize;
	int itsSendBufferSize;

};

inline bool TH_Socket::isClonable() const
  { return false; }  

inline void TH_Socket::reset()
  { }

  // @} // Doxygen endgroup Transport

} // namespace LOFAR

#endif

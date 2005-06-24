//  TH_Socket.cc: POSIX Socket based Transport Holder
//
//  Copyright (C) 2000-2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <unistd.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>

namespace LOFAR
{

//
// TH_Socket(host, port [, sync, protocol, backlog])
//
// Create TH_Socket with underlying listener.
// At return the listener is always started.
TH_Socket::TH_Socket (const string& service,
					  bool			sync,
					  int32			protocol,
					  int32			backlog) :
    itsServerSocket(new Socket(service+"_server", service, protocol, backlog)),
    itsDataSocket  (0),
	itsIsOwner     (true),
	itsReadOffset  (0),
	itsLastCmd     (CmdNone)
{
	LOG_TRACE_FLOW("TH_Socket<server>");

	ASSERTSTR(itsServerSocket->ok(), "Cannot start listener");

	itsServerSocket->setBlocking (sync);
}

//
// TH_Socket(host, port [, sync, protocol])
//
// Create a TH_Socket with a client socket.
TH_Socket::TH_Socket (const string&	hostName,
					  const string& service,
					  bool			sync,
					  int32			protocol) :
    itsServerSocket(0),
    itsDataSocket  (new Socket(hostName+"_client", hostName, service,protocol)),
	itsIsOwner     (true),
	itsReadOffset  (0),
	itsLastCmd     (CmdNone)
{
	LOG_TRACE_FLOW("TH_Socket<client>");

	ASSERTSTR(itsDataSocket, "Cannot allocate client socket");

	itsDataSocket->setBlocking (sync);		// Set correct mode.

}

//
// TH_Socket (Socket*)
//
// Create a TH_Socket based on an existing data socket.
TH_Socket::TH_Socket (Socket*		aDataSocket) :
    itsServerSocket(0),
    itsDataSocket  (0),
	itsIsOwner     (false),
	itsReadOffset  (0),
	itsLastCmd     (CmdNone)
{
	ASSERTSTR(aDataSocket && aDataSocket->ok(), 
		      "Invalid dataSocket in constructor");

	itsDataSocket = aDataSocket;
}
    
//
// ~TH_Socket()
//
TH_Socket::~TH_Socket()
{
	LOG_TRACE_OBJ("~TH_Socket");

	// Only delete the sockets if we created them
	if (itsIsOwner) {
		LOG_TRACE_LOOP("TH_Socket:shutdown datasocket");
		shutdown(itsDataSocket);

		LOG_TRACE_LOOP("TH_Socket:shutdown listensocket");
		shutdown(itsServerSocket);
	}
}

//
// getType
//
string TH_Socket::getType() const
{
	return ("TH_Socket");
}
  

//
// recvBlocking (buf, nrBytes)
//
bool TH_Socket::recvBlocking (void*	buf, int32	nrBytes, int32	/*tag*/,
							  int32 /* nrBytesRead */, DataHolder* /* dh */)
{ 
	LOG_TRACE_OBJ("TH_Socket::recvBlocking");

	if (!init()) {								// be sure we are connected
		return (false);
	}
	// Now we should have a connection

	itsReadOffset = 0;
	int32 btsRead = itsDataSocket->readBlocking (buf, nrBytes);
	if (btsRead == nrBytes) {	// all bytes received? Ok!
		return (true);
	}

	if (btsRead == Socket::PEERCLOSED) {	// peer closed connection.
		LOG_DEBUG("TH_Socket:shutdown datasocket after read-error");
		if (itsServerSocket) {			// server role?
			shutdown(itsDataSocket);	// completely delete datasocket
		}
		else {							// client role
			itsDataSocket->shutdown();	// just close connection
		}
		itsLastCmd    = CmdNone;
		itsReadOffset = 0;						// it's a total mess
		return (false);
	}

	THROW(AssertError, "TH_Socket: data not succesfully received");
}
  
//
// recvNonBlocking
//
// Returns nrBytes when data is fully received, otherwise 0
//
// Note: +---------------------------------------+
//       ^  <- offset -> ^     <- nrbytes ->     ^
//		 |               |                       |
//       |               |                       +-- size of dataHolder
//       |               +-- buf variable
//       +-- begin of dataHolder
//
int32 TH_Socket::recvNonBlocking(void*	buf, int32	nrBytes, int32 /*tag*/,
							  int32  offset , DataHolder* /* dh */)
{
	// Note: buf, offset and nrBytes are outerWorld view on databuffer contents
	// itsReadOffset is innerWorld view on databuffer contents

	LOG_TRACE_OBJ(formatString("TH_Socket::recvNonBlocking(%d), offset=%d", 
														nrBytes, offset));

	if (itsReadOffset >= offset+nrBytes) {		// already in buffer?
		itsReadOffset = 0;						// reset internal offset
		itsLastCmd = CmdNone;					// async admin.
		return (nrBytes);
	}

	if (itsLastCmd == CmdNone) {				// adopt offset on fresh entry
		itsReadOffset = offset;
	}
	itsLastCmd  = CmdRecvNonBlock;				// remember where to wait for.

	if (!init()) {								// be sure we are connected
		return (0);
	}

	// read remaining bytes
	int32	bytesRead = itsDataSocket->read((char*)buf-offset+itsReadOffset, 
											(offset+nrBytes)-itsReadOffset);
	LOG_TRACE_VAR_STR("Read " << bytesRead << " bytes from socket");

	// Errors are reported with negative numbers
	if (bytesRead < 0) {						// serious error?
		if (bytesRead == Socket::INCOMPLETE) {	// interrrupt occured
			return (0);							// rest of data may come
		}
		// It's a total mess, anything could have happend. Bail out.
		LOG_DEBUG_STR("TH_Socket: serious read-error, result=" << bytesRead);
		if (itsServerSocket) {			// server role?
			shutdown(itsDataSocket);	// completely delete datasocket
		}
		else {							// client role
			itsDataSocket->shutdown();	// just close connection
		}
		itsLastCmd    = CmdNone;
		itsReadOffset = 0;						// it's a total mess
		return (false);
	}

	// Some data was read
	if (bytesRead+itsReadOffset < offset+nrBytes) {	// everthing read?
		itsReadOffset += bytesRead;				// No, update readoffset.
		return(0);
	}

	itsReadOffset = 0;							// message complete, reset
	itsLastCmd    = CmdNone;					// async admin.

	return (nrBytes);
}

//
// waitForReceived(buf, maxBytes, tag)
//
void TH_Socket::waitForReceived(void*	buf, int32 	maxBytes, int32  tag)
{
	switch (itsLastCmd) {
	case CmdNone:
		return;
	case CmdRecvNonBlock:
		recvBlocking(buf, maxBytes, tag);
		return;
	}
	ASSERTSTR(false, "TH_Socket:LastCmd is unknown: " << itsLastCmd);
}

//
// sendBlocking (buf, nrBytes, tag)
//
bool TH_Socket::sendBlocking (void*	buf, int32	nrBytes, int32 /*tag*/,
								DataHolder* /* dh */)
{
	if (!init()) {
		return (false);
	}

	LOG_TRACE_OBJ("TH_Socket::sendBlocking");

	// Now we should have a connection
	int32 sent_len = itsDataSocket->writeBlocking (buf, nrBytes);

	return (sent_len == nrBytes);
}  
   
//
// sendNonBlocking(buf, nrBytes, tag)
//
bool TH_Socket::sendNonBlocking(void*	buf, int32	nrBytes, int32	 /*tag*/,
								DataHolder* /* dh */)
{
	if (!init()) {
		return (false);
	}

	LOG_TRACE_OBJ("TH_Socket::sendNonBlocking");

	//int32 sent_len = itsDataSocket->write (buf, nrBytes);
	// TODO: For now implemented as blocking!
	int32 sent_len = itsDataSocket->writeBlocking (buf, nrBytes);

	return (sent_len == nrBytes);
}
  
//
// waitforSent(buf, nrBytes, tag)
//
void TH_Socket::waitForSent(void*, int, int)
{
	// TODO when sendNonBlocking is modified
	return;
}

// -------------------- Private methods --------------------
//
// init()
//
// Make sure the connection is open(ed).
//
bool TH_Socket::init()
{
	LOG_TRACE_FLOW("TH_Socket::init()");

	if (isConnected()) {
		return (true);
	}

	itsReadOffset = 0;
	itsLastCmd    = CmdNone;
	
	if (itsServerSocket) {
		return(connectToClient());
	}

	return (connectToServer());
}

//
// connectToServer
//
bool TH_Socket::connectToServer ()
{
	// create a client socket
	LOG_TRACE_OBJ("TH_Socket::connectToServer");

	if (isConnected()) {
		return (true);
	}

	// try to connect: may time out, succes of get error.
	int32 status = itsDataSocket->connect(itsDataSocket->isBlocking() ? -1 : 0);
	LOG_TRACE_COND_STR("connect() returned: " << status);

	if ((status == Socket::SK_OK) || (status == Socket::INPROGRESS)) {
		LOG_TRACE_FLOW(formatString("Connection to server %s", 
							isConnected() ? "succesful" : "timed out"));
		return (isConnected());
	}
			
	// when errno == ECONNREFUSED the server might not be on the air yet
	int32 	sysErrno = itsDataSocket->errnoSys();
	ASSERTSTR (((status == Socket::CONNECT) || (sysErrno == ECONNREFUSED)),
					"TH_Socket: could not connect to server(" << 
					itsDataSocket->host() << "," << 
					itsDataSocket->port() << "), status =" 
					<< status);

	return (false);
}

//
// connectToClient
//
bool TH_Socket::connectToClient ()
{
	LOG_TRACE_OBJ("TH_Socket::connectToClient");

	if (isConnected()) {
		return (true);
	}

	// wait for incoming connections
	itsDataSocket = itsServerSocket->accept(itsServerSocket->isBlocking() ? 
																		-1 : 0);
	if (itsDataSocket) {
		LOG_TRACE_FLOW("Connection with client succesful");
		return (true);
	}

	int32 status = itsServerSocket->errcode();
	ASSERTSTR (((status == Socket::SK_OK) || (status == Socket::INPROGRESS)), 
				"TH_Socket: no connection from client, status = " << status);

	LOG_TRACE_FLOW("ConnectToClient timed out");

	return (false);
}

void TH_Socket::shutdown(Socket*& aSocket)
{
	LOG_TRACE_OBJ("TH_Socket::shutdown");

	if (aSocket) {
		aSocket->shutdown();
		delete aSocket;
		aSocket = 0;
	}
}

} // namespace LOFAR

//# TH_Socket.cc: POSIX Socket based Transport Holder
//#
//# Copyright (C) 2000-2004
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

#include <unistd.h>
#include <sys/socket.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Transport/TH_Socket.h>

#if defined(__linux__) && !defined(USE_NOSOCKETS)
#include <sys/sysctl.h>
#endif


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
		      int32			backlog,
		      const bool openSocketNow,
		      const int recvBufferSize, 
		      const int sendBufferSize):
    itsIsServer    (true),
    itsServerSocket(0),
    itsDataSocket  (0),
    itsIsOwner     (true),
    itsIsClosed    (false),
    itsReadOffset  (0),
    itsHostName    (),
    itsService     (service),
    itsProtocol    (protocol),
    itsBacklog     (backlog),
    itsIsBlocking  (sync),
    itsLastCmd     (CmdNone),
    itsRecvBufferSize(recvBufferSize),
    itsSendBufferSize(sendBufferSize)
{
        LOG_TRACE_FLOW("TH_Socket<server>");
  
	if (openSocketNow) {
	  ASSERTSTR(openSocket(), "Could not open server socket");
	}
}

//
// TH_Socket(host, port [, sync, protocol])
//
// Create a TH_Socket with a client socket.
TH_Socket::TH_Socket (const string&	hostName,
		      const string& service,
		      bool			sync,
		      int32			protocol,
		      const bool openSocketNow,
		      const int recvBufferSize,
		      const int sendBufferSize) :
    itsIsServer    (false),
    itsServerSocket(0),
    itsDataSocket  (0),
    itsIsOwner     (true),
    itsIsClosed    (false),
    itsReadOffset  (0),
    itsHostName    (hostName),
    itsService     (service),
    itsProtocol    (protocol),
    itsBacklog     (0),
    itsIsBlocking  (sync),
    itsLastCmd     (CmdNone),
    itsRecvBufferSize(recvBufferSize),
    itsSendBufferSize(sendBufferSize)
{
	LOG_TRACE_FLOW("TH_Socket<client>");

	if (openSocketNow) {
	  ASSERTSTR(openSocket(), "Could not start client socket");
	}
}

//
// TH_Socket (Socket*)
//
// Create a TH_Socket based on an existing data socket.
TH_Socket::TH_Socket (Socket*		aDataSocket) :
    itsIsServer(false),
    itsServerSocket(0),
    itsDataSocket  (0),
    itsIsOwner     (false),
    itsIsClosed    (false),
    itsReadOffset  (0),
    itsHostName    (),
    itsService     (),
    itsProtocol    (0),
    itsBacklog     (0),
    itsIsBlocking  (true),
    itsLastCmd     (CmdNone)
{
	ASSERTSTR(aDataSocket && aDataSocket->ok(), 
		      "Invalid dataSocket in constructor");

	itsDataSocket = aDataSocket;
	itsHostName = itsDataSocket->host();
	itsService = itsDataSocket->port();
	itsProtocol = itsDataSocket->getType();
	itsIsBlocking = itsDataSocket->isBlocking();
}
    
//
// ~TH_Socket()
//
TH_Socket::~TH_Socket()
{
	LOG_TRACE_OBJ("~TH_Socket");

	bool isSameSocket = itsDataSocket == itsServerSocket;

	// Close sockets.
	LOG_TRACE_OBJ("TH_Socket:close datasocket");
	close(itsDataSocket);

	if (!isSameSocket) {
		LOG_TRACE_OBJ("TH_Socket:close listensocket");
		close(itsServerSocket);
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
		LOG_DEBUG("TH_Socket:close datasocket after read-error");
		close(itsDataSocket);
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
		perror("Socket");
		close(itsDataSocket);
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
        bool result;

	LOG_TRACE_RTTI("TH_Socket::init()");

	if (itsIsClosed) {
		LOG_WARN("TH_Socket::init() called after socket close");
		return false;
	}

	if (!openSocket()) {
	  return false;
	}

	if (isConnected()) {
		return (true);
	}

	itsReadOffset = 0;
	itsLastCmd    = CmdNone;
	
	if (itsIsServer) {
		result = connectToClient();
	} else {
	        result = connectToServer();
	}
	initBuffers(itsRecvBufferSize, itsSendBufferSize);
	return result;
}


bool TH_Socket::initBuffers(int recvBufferSize, int sendBufferSize) {
  // set the size of the kernel level socket buffer
  // use -1 in the constructor (default) to leave it untouched.
  
  // BG/L doesn't implement setsockopt; also ignore if no sockets are used.
#if !defined(HAVE_BGL) && !defined(USE_NOSOCKETS)
  int socketFD; 
  if (itsIsServer && itsServerSocket != NULL) socketFD = itsServerSocket->getSid();
  else socketFD = itsDataSocket->getSid();
  
  if (recvBufferSize != -1) {
#if defined __linux__
    int name[] = { CTL_NET, NET_CORE, NET_CORE_RMEM_MAX };
    int value;
    size_t valueSize = sizeof(value);
    // check the max buffer size of the kernel
    sysctl(name, sizeof(name)/sizeof(int), &value, &valueSize, 0, 0);
    if (recvBufferSize > value) {
      // if the max size is not large enough increase it
      if (sysctl(name, sizeof(name)/sizeof(int), 0, 0, &recvBufferSize, sizeof(recvBufferSize)) < 0){
	LOG_WARN("TH_Socket: could not increase max socket receive buffer");
      }
    }
#endif
    // now set the buffer for our socket
    if (setsockopt(socketFD, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, sizeof(recvBufferSize)) < 0)
    {
      LOG_WARN("TH_Socket: receive buffer size could not be set, default size will be used.");
    }

  }    
  if (sendBufferSize != -1) {
#if defined __linux__
    int name[] = { CTL_NET, NET_CORE, NET_CORE_WMEM_MAX };
    int value;
    size_t valueSize = sizeof(value);
    // check the max buffer size of the kernel
    sysctl(name, sizeof(name)/sizeof(int), &value, &valueSize, 0, 0);
    if (sendBufferSize > value) {
      // if the max size is not large enough increase it
      if (sysctl(name, sizeof(name)/sizeof(int), 0, 0, &sendBufferSize, sizeof(sendBufferSize)) < 0){ 
	LOG_WARN("TH_Socket: could not increase max socket send buffer");
      }
    }
#endif
    if (setsockopt(socketFD, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(sendBufferSize)) < 0)
    {
      LOG_WARN("TH_Socket: send buffer size could not be set, default size will be used.");
    }

  }    
#endif /* HAVE_BGL */
  return true;
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

//
// openSocket
//
// open the socket, this method can be called if it is already open
//
bool TH_Socket::openSocket()
{
  if (itsIsServer) {
    if (itsServerSocket == 0) {
      LOG_TRACE_OBJ("TH_Socket::openSocket opening serverSocket");
      itsServerSocket = new Socket(itsService+"_server", itsService, itsProtocol, itsBacklog);
      ASSERTSTR(itsServerSocket && itsServerSocket->ok(), "Cannot start listener");
      itsServerSocket->setBlocking (itsIsBlocking);
    }
  } else {
    if (itsDataSocket == 0) {
      LOG_TRACE_OBJ("TH_Socket::openSocket opening dataSocket");
      itsDataSocket = new Socket(itsHostName+"_client", itsHostName, itsService, itsProtocol);
      ASSERTSTR(itsDataSocket && itsDataSocket->ok(), "Cannot allocate client socket");
      itsDataSocket->setBlocking (itsIsBlocking);		// Set correct mode.
    }
  }
  return true; // right now we return true, because if the assert fails we won't get here
}

  

void TH_Socket::close(Socket*& aSocket)
{
	if (aSocket && aSocket->getSid() >= 0) {
		LOG_TRACE_OBJ("TH_Socket::close: closing open socket");
		aSocket->close();
		itsIsClosed = true;
		if (itsIsOwner) {
			LOG_TRACE_OBJ("TH_Socket::close: deleting owned socket object");
			delete aSocket;
			aSocket = 0;
		}
	}
}

} // namespace LOFAR

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

#include <Transport/TH_Socket.h>
#include <Transport/Transporter.h>
#include <Transport/DataHolder.h>
#include <Common/LofarLogger.h>
#include <unistd.h>

namespace LOFAR
{

//
// TH_Socket (sourceName, destinationName, portno, listenerAtDestination)
//
TH_Socket::TH_Socket (const std::string&	sourceName, 
					  const std::string&	destinationName,
					  int32					portno,
					  const bool			listenerAtDestination,
					  const bool			blocking) :
	itsServerHostname	(""), 
	itsPort				(portno),
	itsIsConnected		(false),
	itsSyncComm			(blocking),
	itsDestHasListener	(listenerAtDestination), 
	itsServerSocket		(0),
	itsDataSocket		(0),
	itsReadOffset		(0),
	itsLastCmd			(CmdNone)
{
	if (listenerAtDestination) {
		itsServerHostname = destinationName;
	}
	else {
		itsServerHostname = sourceName;
	}
}

//
// ~TH_Socket()
//
TH_Socket::~TH_Socket()
{
	LOG_TRACE_OBJ("~TH_Socket");

	if (itsIsConnected) {
		LOG_TRACE_LOOP("TH_Socket:shutdown datasocket");
		itsDataSocket->shutdown();
		delete itsDataSocket;
	}

	if (itsServerSocket) {
		LOG_TRACE_LOOP("TH_Socket:shutdown listensocket");
		itsServerSocket->shutdown();
		delete itsServerSocket;
	}
}

//
// make()
//
TH_Socket* TH_Socket::make() const
{
	LOG_TRACE_OBJ(formatString("make TH_Socket(%s,%d,%sHasListener)", 
								itsServerHostname.c_str(), itsPort, 
								itsDestHasListener ? "Dest": "Source"));

	if (itsDestHasListener) {
		return (new TH_Socket("", itsServerHostname, itsPort, 
									itsDestHasListener, itsSyncComm));
	}
	else {
		return (new TH_Socket(itsServerHostname, "", itsPort, 
									itsDestHasListener, itsSyncComm));
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
// connectionPossible(src, dest)
//
bool TH_Socket::connectionPossible (int32 srcRank, int32 dstRank) const
{
	return (srcRank == dstRank);
}
  
//
// recvBlocking (buf, nrBytes)
//
bool TH_Socket::recvBlocking (void*	buf, int32	nrBytes, int32	/*tag*/)
{ 
	LOG_TRACE_OBJ("TH_Socket::recvBlocking");

	if (!init()) {								// be sure we are connected
		return (false);
	}
	// Now we should have a connection

	itsReadOffset = 0;
	if (itsDataSocket->readBlocking (buf, nrBytes) != nrBytes) {
		THROW(AssertError, "TH_Socket: data not succesfully received");
	}

	return (true);
}
  
//
// recvVarBlocking(tag)
//
bool TH_Socket::recvVarBlocking (int32 tag)
{
#if 1
	LOG_TRACE_OBJ("TH_Socket::recvVarBlocking");

	if (!init()) {								// be sure we are connected
		return (false);
	}

	// Read the blob header.
	itsReadOffset = 0;
	DataHolder* target = getTransporter()->getDataHolder();
	void*	buf   = target->getDataPtr();
	int32	hdrsz = target->getHeaderSize();

	if (!recvBlocking (buf, hdrsz, tag)) {
		return (false);
	}

	// Extract the length and resize the buffer.
	int32 size = DataHolder::getDataLength (buf);
	target->resizeBuffer (size);
	buf = target->getDataPtr();
	// Read the remainder.
	bool result = recvBlocking (static_cast<char*>(buf)+hdrsz, size-hdrsz, tag);

    return (result);
#else
	return(true);
#endif
}

//
// recvNonBlocking
//
bool TH_Socket::recvNonBlocking(void*	buf, int32	nrBytes, int32 /*tag*/)
{
	LOG_TRACE_OBJ(formatString("TH_Socket::recvNonBlocking(%d), offset=%d", 
													nrBytes, itsReadOffset));
	
	if (nrBytes <= itsReadOffset) {				// already in buffer?
		return (true);
	}

	itsLastCmd  = CmdRecvNonBlock;				// remember where to wait for.

	if (!init()) {								// be sure we are connected
		return (false);
	}

	// read remaining bytes
	int32	bytesRead = itsDataSocket->read ((char*)buf+itsReadOffset, 
												nrBytes-itsReadOffset);
	LOG_TRACE_VAR_STR("Read " << bytesRead << " bytes from socket");

	// Errors are reported with negative numbers
	if (bytesRead < 0) {						// serious error?
		if (bytesRead == Socket::INCOMPLETE) {	// interrrupt occured
			return (false);						// rest of data may come
		}
		// It's a total mess, anything could have happend. Close line.
		// At next read or write it will be reopened.
		LOG_DEBUG("TH_Socket:shutdown datasocket after read-error");
		itsDataSocket->shutdown();
		delete itsDataSocket;
		itsIsConnected = false;
		itsLastCmd     = CmdNone;
		itsReadOffset  = 0;						// it's a total mess
		return (false);
	}

	// Some data was read
	if (bytesRead+itsReadOffset < nrBytes) {	// everthing read?
		itsReadOffset += bytesRead;				// No, update readoffset.
		return(false);
	}

	itsReadOffset = 0;							// message complete, reset
	itsLastCmd    = CmdNone;					// async asmin.

	return (true);
}

//
// recvVarNonBlocking(tag)
//
bool TH_Socket::recvVarNonBlocking(int32	tag)
{
#if 1
	LOG_TRACE_OBJ("TH_Socket::recvVarNonBlocking");

	itsLastCmd  = CmdRecvVarNonBlock;			// remember where to wait for.

	if (!init()) {								// be sure we are connected
		return (false);
	}

	// Read the blob header.
	DataHolder* target = getTransporter()->getDataHolder();
	void*	buf   = target->getDataPtr();
	int32	hdrsz = target->getHeaderSize();

	if (!recvNonBlocking(buf, hdrsz, tag)) {
		itsLastCmd  = CmdRecvVarNonBlock;		// remember where to wait for.
		return (false);
	}

	// Extract the length and resize the buffer.
	int32 size = DataHolder::getDataLength (buf);
	target->resizeBuffer (size);
	buf = target->getDataPtr();
	// Read the remainder.
	bool result = recvNonBlocking (static_cast<char*>(buf)+hdrsz, 
														size-hdrsz, tag);
	if (!result) {
		itsLastCmd  = CmdRecvVarNonBlock;		// remember where to wait for.
	}

    return (result);
#else
	return(true);
#endif
}

//
// waitForReceived(buf, maxBytes, tag)
//
// NOTE: THIS CALL IS BLOCKING, WHY RETURN A BOOL???
//
bool TH_Socket::waitForReceived(void*	buf, int32 	maxBytes, int32  tag)
{
	switch (itsLastCmd) {
	case CmdNone:
		return (true);
	case CmdRecvNonBlock:
		return(recvBlocking(buf, maxBytes, tag));
	case CmdRecvVarNonBlock:
		while (!recvVarNonBlocking(tag)) {
			;
		}
		return (true);
	}
	ASSERTSTR(false, "TH_Socket:LastCmd is unknown: " << itsLastCmd);
}

//
// sendBlocking (buf, nrBytes, tag)
//
bool TH_Socket::sendBlocking (void*	buf, int32	nrBytes, int32 /*tag*/)
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
bool TH_Socket::sendNonBlocking(void*	buf, int32	nrBytes, int32	 /*tag*/)
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
bool TH_Socket::waitForSent(void*, int, int)
{
	// TODO when sendNonBlocking is modified
	return (true);
}


//
// init()
//
// Code to open the connection
bool TH_Socket::init () 
{
	if (itsIsConnected) {
		return (true);
	}

	itsReadOffset = 0;
	itsLastCmd    = CmdNone;

	// On a destination DH the field sourceDH is filled (with a pointer to
	// the source DH), on a sourceDH this field is 0. This way we can figure
	// out how the 'arrow' is drawn between the DH objects.
	// Together with the DHL flag we know whether we should behave like a client
	// or a server.
	DataHolder*		sourceDH = getTransporter()->getSourceDataHolder();
	bool			blocking = getTransporter()->isBlocking();
	if ((!sourceDH && (itsDestHasListener)) || (sourceDH && !itsDestHasListener)) {
		// client role
		return(connectToServer(blocking ? -1 : 10000));
	}
	else {
		// server role
		return(connectToClient(blocking ? -1 : 10000));
	}
}

//
// connectToServer
//
bool TH_Socket::connectToServer (const int32	waitMs)
{
	// TODO:The port number of the connection must be based on the tag after
	// succesful testing?
	
	// create a client socket
	LOG_TRACE_OBJ(formatString("TH_Socket::connectToServer(%d)", waitMs));

	if (itsIsConnected) {
		return (true);
	}

	// create new data socket
	std::stringstream 	service;
	service << itsPort;
	itsDataSocket  = new Socket("TH_Socket", itsServerHostname, service.str());
	ASSERTSTR (itsDataSocket->ok(), "TH_Socket::connectToServer (" <<
											itsServerHostname << "," <<
											itsPort << ") failed: " <<
											itsDataSocket->errstr());
	itsDataSocket->setBlocking(itsSyncComm);

	// try to connect to the server
	int32		status;
	int16		maxRetry, tryCnt = 0;
	int32		sleepTime;
	do {
		// try to connect: may time out, succes of get error.
		status = itsDataSocket->connect(waitMs);

		if ((status == Socket::SK_OK) || (status == Socket::INPROGRESS)) {
			itsIsConnected = (status == Socket::SK_OK);
			LOG_TRACE_FLOW(formatString("Connection to server %s", 
								itsIsConnected ? "succesful" : "timed out"));
			return (itsIsConnected);
		}
			
		// when errno == ECONNREFUSED the server might not be on the air yet
		int32 	sysErrno = itsDataSocket->errnoSys();
		ASSERTSTR (((status == Socket::CONNECT) || (sysErrno == ECONNREFUSED)),
					"TH_Socket: could not connect to server(" << 
					itsServerHostname << "," << itsPort << "), status =" 
					<< status);

		// The server is probably not on the air. Do a couple of retries.
		if (waitMs < 0) {
			// blocking mode, try max 60 times with interval 1 sec.
			maxRetry = 60;
			sleepTime = 1000000;
		}
		else {
			// non blocking mode, do max 10 retries
			maxRetry = 10;
			sleepTime = (1000*waitMs) / 10;
		}
		usleep(sleepTime);
		tryCnt++;
	} while (tryCnt < maxRetry);

	THROW(AssertError, "Connection to server failed! Is the Server on the air?");
	return (false);
}

//
// connectToClient
//
bool TH_Socket::connectToClient (const int32	waitMs)
{
	LOG_TRACE_OBJ(formatString("TH_Socket::connectToClient(%d)", waitMs));

	if (itsIsConnected) {
		return (true);
	}

	// create a server socket (and start listener)
	if (!itsServerSocket) {
		std::stringstream 	service;
		service << itsPort;
		itsServerSocket = new Socket("TH_Socket", service.str());
		ASSERTSTR (itsServerSocket->ok(), "TH_Socket::starting listener (" <<
							itsPort << ") failed: " << itsServerSocket->errstr());
	}

	// wait for incoming connections
	itsDataSocket = itsServerSocket->accept(waitMs);
	if (itsDataSocket) {
		LOG_TRACE_FLOW("Connection with client succesful");
		itsIsConnected = true;
		itsDataSocket->setBlocking(itsSyncComm);
		return (true);
	}

	int32 status = itsServerSocket->errcode();
	ASSERTSTR (((status == Socket::SK_OK) || (status == Socket::INPROGRESS)), 
				"TH_Socket: no connection from client, status = " << status);

	LOG_TRACE_FLOW("ConnectToClient timed out");

	return (false);
}

void TH_Socket::setDataSocket (Socket*	aDataSocket)
{
	if (aDataSocket) {
		itsDataSocket  = aDataSocket;
		itsIsConnected = aDataSocket->isConnected();
		itsSyncComm    = aDataSocket->isBlocking();
		itsReadOffset  = 0;
		itsLastCmd     = CmdNone;
	}
}

} // namespace LOFAR

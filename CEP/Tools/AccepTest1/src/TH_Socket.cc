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
TH_Socket::TH_Socket (const std::string& sourceName, 
		      const std::string& destinationName,
		      int32		 portno,
		      const bool	 listenerAtDestination,
		      const bool	 blocking) :
	itsServerHostname	(""), 
	itsPort			(portno),
	itsIsConnected		(false),
	itsIsBlocking		(blocking),
	itsDestHasListener	(listenerAtDestination), 
	itsServerSocket		(0),
	itsDataSocket		(0)
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
			  itsDestHasListener, itsIsBlocking));
  }
  else {
    return (new TH_Socket(itsServerHostname, "", itsPort, 
			  itsDestHasListener, itsIsBlocking));
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
bool TH_Socket::recvBlocking (void*	buf, int32	nrBytes, int32	tag)
{ 
	if (!init()) {
		return (false);
	}

	// Now we should have a connection
// 	if (itsDataSocket->readBlocking (buf, nrBytes) != nrBytes) {
// 		THROW(AssertError, "TH_Socket: data not succesfully received");
// 	}

	return (true);
}
  
//
// recvVarBlocking(tag)
//
bool TH_Socket::recvVarBlocking (int32 tag)
{
	if (!init()) {
		return (false);
	}

	// Read the blob header.
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
// 	bool result = recvBlocking (static_cast<char*>(buf)+hdrsz, size-hdrsz, tag);

    return (false);
}

//
// recvNonBlocking
//
bool TH_Socket::recvNonBlocking(void*	buf, int32	nrBytes, int32 tag)
{
	if (!init()) {
		return (false);
	}

	// Now we should have a connection
	if (itsDataSocket->read (buf, nrBytes) != nrBytes) {
		THROW(AssertError, "TH_Socket: data not succesfully received");
	}
	return (true);
}

//
// recvVarNonBlocking(tag)
//
bool TH_Socket::recvVarNonBlocking(int32	tag)
{
	if (!init()) {
		return (false);
	}

	// Read the blob header.
	DataHolder* target = getTransporter()->getDataHolder();
	void*	buf   = target->getDataPtr();
	int32	hdrsz = target->getHeaderSize();

	if (!recvNonBlocking(buf, hdrsz, tag)) {
		return (false);
	}

	// Extract the length and resize the buffer.
	int32 size = DataHolder::getDataLength (buf);
	target->resizeBuffer (size);
	buf = target->getDataPtr();
	// Read the remainder.
	bool result = recvNonBlocking (static_cast<char*>(buf)+hdrsz, size-hdrsz, tag);

    return (result);
}

//
// waitForReceived(buf, maxBytes, tag)
//
bool TH_Socket::waitForReceived(void*	buf, int32 	maxBytes, int32  tag)
{
	// TODO
	return (true);
}

//
// sendBlocking (buf, nrBytes, tag)
//
bool TH_Socket::sendBlocking (void*	buf, int32	nrBytes, int32 tag)
{
	if (!init()) {
		return (false);
	}

	// Now we should have a connection
// 	int32 sent_len = itsDataSocket->writeBlocking (buf, nrBytes);
	int32 sent_len = 0;

	return (sent_len == nrBytes);
}  
   
//
// sendNonBlocking(buf, nrBytes, tag)
//
bool TH_Socket::sendNonBlocking(void*	buf, int32	nrBytes, int32	 tag)
{
	if (!init()) {
		return (false);
	}

	// Now we should have a connection
	int32 sent_len = itsDataSocket->write (buf, nrBytes);

	return (sent_len == nrBytes);
}
  
//
// waitforSent(buf, nrBytes, tag)
//
bool TH_Socket::waitForSent(void*, int, int)
{
	// TODO
	return (true);
}


//
// init()
//
// Code to open the connection
bool TH_Socket::init () {
  if (itsIsConnected) {
    return (true);
  }
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
  itsDataSocket  = new Socket("TH_Socket");
  itsDataSocket->setBlocking(itsIsBlocking);
  
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
    ASSERTSTR (((status == Socket::CONNECT) && (sysErrno == ECONNREFUSED)),
	       "TH_Socket: could not connect to server, status =" << status);
    
    
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
	std::stringstream 	service;
	service << itsPort;
	itsServerSocket = new Socket("TH_Socket", service.str());

	// wait for incoming connections
	itsDataSocket = itsServerSocket->accept(waitMs);
	if (itsDataSocket) {
		LOG_TRACE_FLOW("Connection with client succesful");
		itsIsConnected = true;
		itsDataSocket->setBlocking(itsIsBlocking);
		return (true);
	}

	int32 status = itsServerSocket->errcode();
	ASSERTSTR (((status == Socket::SK_OK) || (status == Socket::INPROGRESS)), 
				"TH_Socket: no connection from client, status = " << status);

	LOG_TRACE_FLOW("ConnectToClient timed out");

	return (false);
}

} // namespace LOFAR

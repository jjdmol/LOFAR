//# Socket.cc: Class for connections over TCP/IP or UDP
//#
//# Copyright (C) 2002-2004
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

//#include <Common/Net/Socket.h>
#include <Common/Net/Socket.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

namespace LOFAR
{

//# NOTE: EAGAIN = EWOULDBLOCK so errors are only checked for EWOULDBLOCK

int32 Socket::defaultSigpipeCounter;

//
// Socket(optionalname)
//
// Create trivial Socket object, not connected to anything
//
Socket::Socket (const string&	socketname) :
	itsSocketname	(socketname),
	itsErrno		(Socket::NOINIT),
	itsSocketID		(-1),
	itsType			(LOCAL),
	itsIsServer		(false),
	itsIsConnected	(false),
	itsAllowIntr	(false),
	itsIsInitialized(false),
	itsIsBlocking	(false)
{
	LOG_TRACE_OBJ(formatString("Socket(%s)", socketname.c_str()));

	sigpipeCounter = &defaultSigpipeCounter;
}

//
// Socket(socketname, service, optional protocol, optional backlog)
//
// Create server socket and start listener
//
Socket::Socket (const string&	socketname, 
				const string&	service,
				int32 			protocol, 
				int32 			backlog) : 
	itsSocketname	(socketname),
	itsErrno		(Socket::NOINIT),
	itsSocketID		(-1),
	itsType			(LOCAL),
	itsIsServer		(true),
	itsIsConnected	(false),
	itsAllowIntr	(false),
	itsIsInitialized(false),
	itsIsBlocking	(false)
{
	sigpipeCounter = &defaultSigpipeCounter;
	initServer (service, protocol, backlog);
}

//
// Socket(socketname, host, service, optional protocol)
//
// Create client socket
//
Socket::Socket (const string&	socketname, 
				const string&	hostname,
				const string&	service,
				int32 			protocol) :
	itsSocketname	(socketname),
	itsErrno		(Socket::NOINIT),
	itsSocketID		(-1),
	itsType			(LOCAL),
	itsIsServer		(false),
	itsIsConnected	(false),
	itsHost			(hostname),
	itsPort			(service),
	itsAllowIntr	(false),
	itsIsInitialized(false),
	itsIsBlocking	(false)
{
	sigpipeCounter = &defaultSigpipeCounter;
	initClient (hostname, service, protocol);
}

//
// ~Socket()
//
Socket::~Socket()
{
	if (!itsIsInitialized) {
		return;
	}

	LOG_TRACE_OBJ ("~Socket");

	if (itsSocketID >=0) {
		shutdown ();
	}

	if ((itsType == Socket::UNIX) && itsUnixAddr.sun_path[0]) {
		int32 result = unlink(itsUnixAddr.sun_path);
		LOG_TRACE_FLOW(formatString("unlink(%s) = %d (%s)", itsUnixAddr.sun_path,
						(result < 0) ? errno : result, 
						(result < 0) ? strerror(errno) :" OK"));
	}

}


//
// initServer (service, protocol, backlog)
//
// Makes the socket a server socket by starting a listener.
int32 Socket::initServer (const string& service, int32 protocol, int32 backlog)
{
	if (itsIsInitialized) {
		return (SK_OK);
	}

	LOG_TRACE_FLOW(formatString("Socket::initServer(%s,%d,%d)", service.c_str(),
														protocol, backlog));

	itsErrno 	= SK_OK;
	itsSysErrno = 0;
	itsIsServer = true;
	itsType 	= protocol;
    itsPort		= service;
  
	struct sockaddr*	addrPtr;
	if (itsType == UNIX) {
		itsHost = "unix";
		if(initUnixSocket(itsIsServer) < 0) {
			return(itsErrno);
		}
		addrPtr = (struct sockaddr*) &itsUnixAddr;
	} 
	else  { 		// networked socket (type TCP or UDP)
    	itsHost		= "localhost";
		if(initTCPSocket(itsIsServer) < 0) {
			return(itsErrno);
		}
		addrPtr = (struct sockaddr*) &itsTCPAddr;
	}
		
	// bind the socket (always blocking)
	if ((::bind (itsSocketID, addrPtr, sizeof(*addrPtr))) < 0) {
		LOG_DEBUG(formatString("Socket:Bind error for port %s, err = %d", 
													itsPort.c_str(), errno));
		close(itsSocketID);
		itsSocketID = -1;
		return (setErrno(BIND));
	}
	itsIsInitialized = true;

	// start listening for connections on UNIX and TCP sockets
	if (itsType == UNIX || itsType == TCP) {
		if ((::listen (itsSocketID, backlog)) < 0) {	// always blocking
			LOG_DEBUG(formatString("Socket:Listen error for port %s, err = %d(%s)", 
									itsPort.c_str(), errno, strerror(errno)));
			close(itsSocketID);
			itsSocketID = -1;
			return (setErrno(LISTEN));
		}
		LOG_DEBUG(formatString("Socket:Listener started at port %s", itsPort.c_str()));
	}
	else {	// UDP socket
		memset(addrPtr, 0, sizeof(*addrPtr));
    }
  
	return (SK_OK);
}


//
// initClient (host, service, optional protocol)
//
// Initializes the socket as client socket.
//
int32 Socket::initClient (const string&	hostname, 
						const string&	service, 
						int32 			protocol)
{
	if (itsIsInitialized) {
		return (SK_OK);
	}

	LOG_TRACE_FLOW(formatString("Socket::initClient(%s,%s,%d)", hostname.c_str(),
							service.c_str(), protocol));

	itsErrno 	= SK_OK;
	itsSysErrno = 0;
	itsIsServer = false;
	itsType 	= protocol;
	itsHost		= hostname;
	itsPort		= service;

	struct sockaddr*	addrPtr;
	if (itsType == UNIX) {
		if(initUnixSocket(itsIsServer) < 0) {
			return(itsErrno);
		}
		addrPtr = (struct sockaddr*) &itsUnixAddr;
	} 
	else  { 		// networked socket (type TCP or UDP)
		if(initTCPSocket(itsIsServer) < 0) {
			return(itsErrno);
		}
		addrPtr = (struct sockaddr*) &itsTCPAddr;
	}
		
	itsIsInitialized = true;

	return (SK_OK);
}

//
// initUnixSocket(asServer)
//
// Tries to setup the socket by resolving hostname, service and protocol
// and finally allocating the real socket.
//
int32 Socket::initUnixSocket(bool		asServer)
{
	string 		path;
	if (asServer) {
		path = itsPort;
	}
	else {
		path = itsHost + ":" + itsPort;
	}

    // setup socket address
    itsUnixAddr.sun_family = AF_UNIX;
    ASSERTSTR (path.length() < sizeof(itsUnixAddr.sun_path), 
													"socket name too long");

    if (path[0] == '=')  { // abstract socket name
		memset (itsUnixAddr.sun_path, 0, sizeof(itsUnixAddr.sun_path));
		path.substr(1).copy(itsUnixAddr.sun_path+1, sizeof(itsUnixAddr.sun_path)-1);
    }
    else  { // socket in filesystem
		path.copy (itsUnixAddr.sun_path, sizeof(itsUnixAddr.sun_path));
	}

    // create socket
    itsSocketID = ::socket (PF_UNIX, SOCK_STREAM, 0);
    if (itsSocketID < 0) {
      return (setErrno(SOCKET));
	}
    LOG_TRACE_FLOW(formatString("creating unix socket %s", path.c_str()));

    if (setDefaults() < 0) {
		close (itsSocketID);
		itsSocketID = -1;
		return (itsErrno);
    }

	return (SK_OK);
}

//
// initUnixSocket(asServer)
//
// Tries to setup the socket by resolving hostname, service and protocol
// and finally allocating the real socket.
//
int32 Socket::initTCPSocket(bool	asServer)
{
	// Preinit part of TCP address structure
	memset (&itsTCPAddr, 0, sizeof(itsTCPAddr));
	itsTCPAddr.sin_family = AF_INET;
	itsTCPAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// as Client we must resolve the hostname to connect to.
	if (!asServer) {
		struct hostent*		hostEnt;		// server host entry
		uint32				IPbytes;
		// try if hostname is hard ip address
		if ((IPbytes = inet_addr(itsHost.c_str())) == INADDR_NONE) {
			// No, try to resolve the name
			if (!(hostEnt = gethostbyname(itsHost.c_str()))) {
				LOG_DEBUG("Socket:Hostname can not be resolved");
				return (itsErrno = BADHOST);
			}
			// Check type
			if (hostEnt->h_addrtype != AF_INET) {
				LOG_DEBUG("Socket:Hostname is of wrong protocoltype");
				return (itsErrno = BADADDRTYPE);
			}
			memcpy (&IPbytes, hostEnt->h_addr, sizeof (IPbytes));
		}
		memcpy ((char*) &itsTCPAddr.sin_addr.s_addr, (char*) &IPbytes, 
															sizeof(IPbytes));
	}
			
	// try to resolve the service
	const char*			protocol = (itsType == UDP ? "udp" : "tcp");
	struct servent*		servEnt;		// service info entry
	if ((servEnt = getservbyname(itsPort.c_str(), protocol))) {
		itsTCPAddr.sin_port = servEnt->s_port;
	}
	else {
		if (!(itsTCPAddr.sin_port = htons((uint16)atoi(itsPort.c_str())))) {
			LOG_DEBUG("Socket:Portnr/service can not be resolved");
			return (itsErrno = PORT);
		}
	}

	// Try to map protocol name to protocol number
	struct protoent*	protoEnt;
	if (!(protoEnt = getprotobyname(protocol))) {
	  return (itsErrno = PROTOCOL);
	}

	// Finally time to open the real socket
	int32	socketType = (itsType == TCP) ? SOCK_STREAM : SOCK_DGRAM;
	if ((itsSocketID = ::socket(PF_INET, socketType, protoEnt->p_proto)) < 0) {
		LOG_DEBUG(formatString("Socket:Can not get a socket, err = %d", errno));
		return (setErrno(SOCKET));
	}

    LOG_DEBUG(formatString("Socket:Created %s socket, port %d, protocol %d",
				asServer ? "server" : "client",
                ntohs((ushort)itsTCPAddr.sin_port), (int)protoEnt->p_proto));

    // set default options
    if (setDefaults() < 0) {
		close(itsSocketID);
		itsSocketID = -1;
		return (itsErrno);
	}

	return (SK_OK);
}

//
// connect(waitMs = -1)
//
// Tries to connect to the server
//
// waitMs < 0: wait Blocking
// 		  >=0: wait max waitMs milliseconds
//
// Return values:
//	SK_OK		Successful connected
//	INPROGRESS	Not connected within given timelimit, still in progress
//	CONNECT		Some error occured, errnoSys() gives more info
//
int32 Socket::connect (int32 waitMs)
{
	if (itsIsServer) {							// only clients can connect
		return (itsErrno = INVOP);
	}

	bool	blockingMode = itsIsBlocking;
	setBlocking(waitMs < 0 ? true : false);		// switch temp to non-blocking?

	struct sockaddr*	addrPtr;				// get pointer to result struct
	if (itsType == UNIX) {
		addrPtr = (struct sockaddr*) &itsUnixAddr;
	}
	else {
		addrPtr = (struct sockaddr*) &itsTCPAddr;
	}

	if (::connect(itsSocketID, addrPtr, sizeof(*addrPtr)) >= 0) {
		LOG_DEBUG("Socket:connect() successful");
		itsIsConnected = true;
		setBlocking (blockingMode);
		return (itsErrno = SK_OK);
	}

	LOG_TRACE_FLOW(formatString("connect() failed: errno=%d (%s)", errno,
														strerror(errno)));

	if (errno != EINPROGRESS && errno != EALREADY) {// real error
		setBlocking (blockingMode);					// reinstall blocking mode
		return (setErrno(CONNECT));
	}

	// connecton request is still in progress, wait waitMs for a result
	struct pollfd	pollInfo;
	pollInfo.fd     = itsSocketID;
	pollInfo.events = POLLWRNORM;
	LOG_TRACE_FLOW(formatString("going into a poll for %d ms", waitMs));
	int32 pollRes = poll (&pollInfo, 1, waitMs);	// poll max waitMs time
	setBlocking (blockingMode);						// restore blocking mode

	switch (pollRes) {
	case -1:							// error on poll
		return (setErrno(CONNECT));
	case 0:								// timeout on poll let user do the rest
		return (itsErrno = INPROGRESS);
	}

	int32		connRes;				// check for sys errors
	socklen_t	resLen = sizeof(connRes);
	if ((getsockopt(itsSocketID, SOL_SOCKET, SO_ERROR, &connRes, &resLen))) {
		return (setErrno(CONNECT));		// getsockopt failed, assume conn failed
	}
	errno = connRes;					// put it were it belongs

	if (connRes != 0) {					// not yet connected
		LOG_TRACE_FLOW(formatString("delayed connect failed also, err=%d(%s)",
													errno, strerror(errno)));
		if ((errno == EINPROGRESS) || (errno == EALREADY)) {
			return (setErrno(INPROGRESS));
		}

		return (setErrno(CONNECT));
	}

	LOG_DEBUG("Socket:delayed connect() succesful");
	itsIsConnected = true;
	setBlocking (blockingMode);
	return (itsErrno = SK_OK);
}

//
// accept(waitMS = -1)
//
// waitMs < 0: wait Blocking
// 		  >=0: wait max waitMs milliseconds
//
// Returns: 0 on error otherwise pointer to new socket
//
Socket* Socket::accept(int32	waitMs)
{
	// Only possible on server sockets
	if (!itsIsServer) {
		itsErrno = INVOP; 
		return (0); 
	}

	// Initialisatie must have been done already
	if (itsSocketID < 0) { 
		itsErrno = NOINIT; 
		return (0); 
	}

	if (itsType == UDP)  {
		return (this);
	}

	bool	blockingMode = itsIsBlocking;
	setBlocking(waitMs < 0 ? true : false);		// switch temp to non-blocking?

	struct sockaddr*	addrPtr;
	if (itsType == UNIX) {
		addrPtr = (struct sockaddr*) &itsUnixAddr;
	}
	else {
		addrPtr = (struct sockaddr*) &itsTCPAddr;
	}
	socklen_t		addrLen = sizeof(*addrPtr);

	itsErrno = SK_OK;
	errno 	 = 0;
	int32 	newSocketID = ::accept(itsSocketID, addrPtr, &addrLen);
	if (newSocketID > 0) {
	    LOG_DEBUG("Socket:accept() successful");
		setBlocking(blockingMode);
		Socket*		newSocket = (itsType == UNIX) ?
								new Socket(newSocketID, itsUnixAddr) :
							  	new Socket(newSocketID, itsTCPAddr);
		newSocket->setBlocking(blockingMode);
		return (newSocket);
	}

	LOG_TRACE_FLOW(formatString("accept() failed: errno=%d (%s)", errno,
														strerror(errno)));

	if ((errno != EWOULDBLOCK) && (errno != EALREADY)) {
		// real error
		setBlocking(blockingMode);
		setErrno(ACCEPT);
		return(0);
	}

	// accept request is still in progress, wait waitMs for a result
	struct pollfd	pollInfo;
	pollInfo.fd     = itsSocketID;
	pollInfo.events = POLLIN;
	LOG_TRACE_FLOW(formatString("going into a poll for %d ms", waitMs));
	int32 pollRes = poll (&pollInfo, 1, waitMs);	// poll max waitMs time
	setBlocking (blockingMode);					// restore blocking mode

	switch (pollRes) {
	case -1:							// error on poll
		setErrno(ACCEPT);
		return (0);
	case 0:								// timeout on poll let user do the rest
		setErrno(INPROGRESS);
		return (0);
	}

	int32		connRes;				// check for errors
	socklen_t	resLen = sizeof(connRes);
	if ((getsockopt(itsSocketID, SOL_SOCKET, SO_ERROR, &connRes, &resLen))) {
		setErrno(ACCEPT);				// getsockopt failed, assume conn failed
		return (0);
	}

	if (connRes != 0) {			// not yet connected
		setErrno(INPROGRESS);
		return (0);
	}

	newSocketID = ::accept(itsSocketID, addrPtr, &addrLen);
	ASSERT (newSocketID > 0);
    LOG_DEBUG("Socket:accept() successful after delay");
	itsErrno = 0;
	setBlocking(blockingMode);
	Socket*		newSocket = (itsType == UNIX) ?
							new Socket(newSocketID, itsUnixAddr) :
						  	new Socket(newSocketID, itsTCPAddr);
	newSocket->setBlocking(blockingMode);
	return (newSocket);
}


//
// shutdown (receive = true, send = true)
//
int32 Socket::shutdown (bool receive, bool send)
{
	ASSERTSTR (receive || send, "neither receive nor send specified");

	itsErrno = SK_OK;					// assume no failure

	if (itsSocketID < 0) { 
		return (itsErrno = NOINIT); 
	}

	if (itsIsConnected) {				// realy shutdown the socket.
		itsErrno     = SK_OK;
		int32 how    = receive ? (send ? SHUT_RDWR : SHUT_RD) : SHUT_WR;
		int32 result = ::shutdown (itsSocketID, how);
		LOG_DEBUG(formatString("Socket:shutdown(%d)=%d", how, result));
		if (result < 0) {
			setErrno(SHUTDOWN);
		}
	}

	if (send && receive) {				// update administration
		close(itsSocketID);
		itsSocketID    = -1;
		itsIsConnected = false;
	}

	return (itsErrno);
}


//
// setBlocking (blockit)
//
int32 Socket::setBlocking (bool block)
{
	if (itsIsBlocking == block) {			// no mode change? ready!
		return (SK_OK);
	}

	itsIsBlocking = block;					// register user wish

	if (itsSocketID >= 0) {					// already a socket?
		if (fcntl (itsSocketID, F_SETFL, block ? 0 : O_NONBLOCK) < 0) {
			return (setErrno(SOCKOPT));
		}
	}

	LOG_TRACE_FLOW(formatString("setBlocking(%s)", block ? "true" : "false"));

	return (SK_OK);
}


//
// read(buf, maxBytes)
//
// Returnvalue: < 0						error occured
//				>= 0 != maxBytes		intr. occured or socket is nonblocking
//				>= 0 == maxBytes		everthing went OK.
int32 Socket::read (void	*buf, int32	maxBytes)
{
	if (itsSocketID < 0)  {
		return (itsErrno = NOINIT); 
	}

	ASSERTSTR (buf, "read():null buffer");

	itsErrno = SK_OK;
	if (!maxBytes) {
		return (SK_OK);
	}

	bool sigpipe = false;
  
	int32	bytesRead = 0;
	int32	bytesLeft = maxBytes;
	if (itsType == UDP) {
		// ----- UDP sockets -----
	    errno = 0;
		socklen_t alen = sizeof(itsTCPAddr);
		if (((bytesRead = recvfrom(itsSocketID, (char*)buf, maxBytes, 0,
						(struct sockaddr*)&itsTCPAddr, &alen)) <= 0) ||
																errno) {
			return (setErrno(READERR));
		}

		bytesLeft = 0;
		itsIsConnected = true;
	}
	else { 
		// ----- UNIX and TCP sockets -----
		while (bytesLeft > 0 && !itsErrno && !sigpipe) {
			errno = 0;								// reset system errno
			int32 oldCounter = *sigpipeCounter;

			// try to read something
			LOG_TRACE_FLOW(formatString("read for %d bytes", bytesLeft));
			bytesRead = ::recv (itsSocketID, buf, bytesLeft, 0);
			sigpipe = (oldCounter != *sigpipeCounter); 	// check for SIGPIPE
			LOG_TRACE_FLOW(formatString("read(%d)=%d%s, errno=%d (%s)", 
						bytesLeft, bytesRead, sigpipe ? " SIGPIPE" : "", 
						errno, strerror(errno)));

			// allow interrupting threads
			if (itsIsBlocking && itsAllowIntr)
				return (setErrno(INCOMPLETE));

#ifdef ENABLE_TRACER
			// trace databytes
			if (bytesRead > 0) {
				string	hdump;
				hexdump (hdump, buf, bytesRead);
				LOG_TRACE_VAR_STR ("data:" << endl << hdump);
			}
#endif

			if (bytesRead < 0) { 					// error?
				if (errno == EWOULDBLOCK) { 
					// if refuses to block, that's OK, return 0
					itsErrno = INCOMPLETE;
					return (maxBytes - bytesLeft);
				}
				else {								// else a real error
					return (setErrno(READERR));
				}
			}

			if (bytesRead == 0) {					// conn reset by peer?
				shutdown();
				return (setErrno(PEERCLOSED));
			}

			buf = bytesRead + (char*)buf;
			bytesLeft -= bytesRead;

		} // while
	}
  
	if (sigpipe) {
		shutdown();
		return (setErrno(PEERCLOSED));
	}

	return (maxBytes - bytesLeft);
}


//
// write(buf, nrBytes)
//
// Returnvalue: < 0					error occured
//				>= 0 != nrBytes		intr. occured or socket is nonblocking
//				>= 0 == nrBytes		everthing went OK.
int32 Socket::write (const void*	buf, int32	nrBytes)
{
	if (itsSocketID < 0)  {
		return (itsErrno = NOINIT); 
	}

	ASSERTSTR (buf, "write():null buffer");

	itsErrno = SK_OK;
	if (!nrBytes)  {
		return (SK_OK);
	}

	bool sigpipe = false;

	int32	bytesLeft = nrBytes;
	int32	bytesWritten = 0;
	if (itsType == UDP) {
		errno = 0;							// reset system errno

		if (!itsIsConnected) {
			return (itsErrno = WRITERR);
		}

		if ((bytesWritten = sendto (itsSocketID, (char*) buf, nrBytes, 0,
									(struct sockaddr*)&itsTCPAddr,
									sizeof (itsTCPAddr)) <= 0) || errno) {
			return (setErrno(WRITERR));
		}

		bytesLeft = 0;
	}
	else { // UNIX or TCP socket
		while (bytesLeft > 0 && !itsErrno && !sigpipe) {
			errno = 0;								// reset system error
			int32 oldCounter = *sigpipeCounter;
			bytesWritten = ::write (itsSocketID, buf, bytesLeft);
			sigpipe = (oldCounter != *sigpipeCounter); // check for SIGPIPE
			LOG_DEBUG(formatString("Socket:write(%d)=%d%s, errno=%d (%s)", bytesLeft,
							bytesWritten, sigpipe ? " SIGPIPE" : "",
							errno, strerror(errno)));

			// allow interrupting threads
			if (itsIsBlocking && itsAllowIntr)
				return (setErrno(INCOMPLETE));

#ifdef ENABLE_TRACER
			// trace databytes
			string	hdump;
			hexdump (hdump, buf, bytesWritten);
			LOG_TRACE_VAR_STR ("data:" << endl << hdump);
#endif

			if (bytesWritten < 0) {
				if (errno == EWOULDBLOCK) { // if refuses to block, that's OK, return 0
					itsErrno = INCOMPLETE;
					return (nrBytes - bytesLeft);
				}
				else  {// else a real error
					return (setErrno(WRITERR));
				} 
			}

			if (bytesWritten == 0) {
				shutdown();
				return (itsErrno = PEERCLOSED);
			}

			// bytesWritten > 0
			buf = bytesWritten + (char*)buf;
			bytesLeft -= bytesWritten;
		} // while
	}
  
	if (sigpipe) {
		shutdown();
		return (itsErrno = PEERCLOSED);
	}
  
	return (nrBytes - bytesLeft);
}

//
// readBlocking (buf, maxBytes)
//
// reads blocking independant of socket mode
//
int32 Socket::readBlocking (void *buf, int32 maxBytes)
{
	ASSERTSTR (buf, "readBlocking():null buffer");

	bool	blockingMode = itsIsBlocking;
	setBlocking(true);

	LOG_TRACE_FLOW("readBlocking()");
	int32 result = read(buf, maxBytes);

	setBlocking (blockingMode);

	return (result);
}

//
// writeBlocking(buf, nrBytes)
//
int32 Socket::writeBlocking (const void *buf, int32	nrBytes)
{
	ASSERTSTR (buf, "writeBlocking():null buffer");

	bool	blockingMode = itsIsBlocking;
	setBlocking(true);

	LOG_TRACE_FLOW("writeBlocking()");
	int32 result = write(buf, nrBytes);

	setBlocking (blockingMode);
  
	return (result);
}

//
// errstr()
//
string Socket::errstr () const
{
	static char const *socketErrStr[] = {
		"OK",
		"Can't create socket (%d: %s)",
		"Can't bind local address (%d: %s)",
		"Can't connect to server (%d: %s)",
		"Can't accept client socket (%d: %s)",
		"Bad server host name given",
		"Bad address type",
		"Read error (%d: %s)",
		"Write error (%d: %s)",
		"Remote client closed connection (%d: %s)",
		"Couldn't read/write whole message (%d: %s)",
		"Invalid operation",
		"setsockopt() or getsockopt() failure (%d: %s)",
		"wrong port/service specified (%d: %s)",
		"invalid protocol (%d: %s)",
		"listen() error (%d: %s)",
		"timeout (%d: %s)",
		"connect in progress (%d: %s)",
		"No more clients (%d: %s)",
		"General failure",
		"Uninitialized socket" 
	};  

	if (itsErrno < NOINIT || itsErrno > 0) {
    	return ("");
	}

	return (formatString(socketErrStr[-itsErrno], errno, strerror(errno)));
}


// --------------- Protected mode ---------------

//
// Socket (socketID, inetAddress)
//
// Used by 'accept' after succesful accepted a connection
//
Socket::Socket (int32	aSocketID, struct sockaddr_in &inetAddr) : 
	itsSocketname	("data"),
	itsSocketID		(aSocketID),
	itsType			(TCP),
	itsIsServer		(false),
	itsIsConnected	(false),
	itsHost			(">tcp"),
	itsPort			("0"),
	itsAllowIntr	(false),
	itsIsInitialized(false),
	itsIsBlocking	(false)
{
	sigpipeCounter = &defaultSigpipeCounter;
	LOG_TRACE_FLOW("creating connected socket");

	itsTCPAddr = inetAddr;
	if (setDefaults() < 0) {
		shutdown();
		return;
	}

	// successfully created connected socket
	itsIsConnected = true;

}

//
// Socket(socketID, unixAddr)
//
// Used by 'accept' after succesful accepting a connection
//
Socket::Socket (int32	aSocketID, struct sockaddr_un &unixAddr) : 
	itsSocketname	("client"),
	itsSocketID		(aSocketID),
	itsType			(UNIX),
	itsIsServer		(false),
	itsIsConnected	(false),
	itsHost			(">unix"),
	itsPort			("0"),
	itsAllowIntr	(false),
	itsIsInitialized(false),
	itsIsBlocking	(false)
{
	sigpipeCounter = &defaultSigpipeCounter;
	itsUnixAddr = unixAddr;

	if (setDefaults() < 0) {
		shutdown();
		return;
	}

	// successfully created connected socket
	itsIsConnected = true;
}

//
// setDefaults()
//
int32 Socket::setDefaults ()
{
	if (itsSocketID < 0)  {
		return (itsErrno = NOINIT);
	}

	setBlocking(itsIsBlocking);				// be sure blocking mode is right.

	uint32 			val = 1;
	struct linger 	lin = { 1, 1 };

	if (setsockopt(itsSocketID, SOL_SOCKET, SO_REUSEADDR, (char*)&val,
															sizeof(val)) < 0) {
		return (setErrno(SOCKOPT));
	}

	// no more defaults for UNIX sockets
	if (itsType == UNIX) {
		return (SK_OK);
	}

	if (setsockopt(itsSocketID, SOL_SOCKET, SO_KEEPALIVE, (char*)&val,
															sizeof(val)) < 0) {
		return (setErrno(SOCKOPT));
	}

	if (setsockopt(itsSocketID, SOL_SOCKET, SO_LINGER, (const char*)&lin, 
															sizeof(lin)) < 0) {
		return (setErrno(SOCKOPT));
	}

	return (SK_OK);
}

} // namespace LOFAR

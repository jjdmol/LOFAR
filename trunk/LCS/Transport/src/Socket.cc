//#  Socket.cc: Creates an object used to access the sys/socket
//#             system calls. Extra checks and administration is 
//#             added to these access methods for  ease of use.
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


#include <Transport/Socket.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>

#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace LOFAR
{

Socket::Socket()
  : itsHostname    (""),
    itsPortnr      (0),
    itsSocketID    (0),
    itsBufferSize  (0),
    itsRptr        (0),
    itsWptr        (0),
    itsData        (0),
    itsConnected   (false),
    itsIsMyBuffer  (false),
    itsIsAllocated (false)
{}

Socket::~Socket()
{
  if (itsIsMyBuffer && itsIsAllocated) {
    //! delete [] itsData;
  }
}

Socket::Socket(const Socket&	that) :
	itsHostname		(that.itsHostname),
	itsPortnr		(that.itsPortnr),
	itsSocketID		(that.itsSocketID),
	itsRptr			(that.itsRptr),
	itsWptr			(that.itsWptr),
	itsData			(0),
	itsConnected            (that.itsConnected),
        itsIsMyBuffer           (false),
	itsIsAllocated          (that.itsIsAllocated)
{ 
	//# alloc databuffer and copy data
	allocBuffer(that.itsBufferSize);
	memcpy(that.itsData, itsData, itsBufferSize);
}

//#
//# operator= copying
//#
Socket& Socket::operator=(const Socket& that)
{
	if (this != &that) {
		itsHostname 	= that.itsHostname;
		itsPortnr	= that.itsPortnr;
		itsSocketID 	= that.itsSocketID;
		itsRptr 	= that.itsRptr;
		itsWptr		= that.itsWptr;
		itsConnected 	= that.itsConnected;
		itsIsAllocated  = that.itsIsAllocated;

		//# alloc databuffer and copy data
		allocBuffer(that.itsBufferSize);
		memcpy(that.itsData, itsData, itsBufferSize);
	}
	return (*this);
}

//#
//# Socket::connect (hostname, portnr)
//#
//#	Tries to connect to the given host and service. The host parameter
//#	may contain a hostname (resolved by DNS) or a dotted IP adres.
//#
//#	In:		host		Name of IP-number of the host to connect to
//#			service		Service name or portnumber to connect to.
//#	Out:	--
//#	Return:	bool indicating the result of the connection request.
bool Socket::connect (const string&	hostname,
		      const int16	portnr) 
{
	//# Try to resolve the hostname, it may be a quartet of digits but
	//# also a name to be resolved by /etc/hosts or NIS.
	in_addr_t 			IPaddr;
	struct hostent		*host_ent;
	if ((IPaddr = inet_addr (hostname.c_str())) == (in_addr_t)-1) {	//# try digit quartet
		if (!(host_ent = gethostbyname (hostname.c_str()))) {	//# try hostname
		  //LOG_DEBUG(formatString("Unknown hostname (%s)\n", hostname.c_str()));
		  return (false);
		}
		//# copy ip_address to host-entity struct.
		memcpy (&IPaddr, host_ent->h_addr, sizeof (IPaddr));
	}

	//# put the bytes in the right network-order
	int16	portno = htons (portnr);

	//# Time to try to open the socket.
	int		sock;
	if ((sock = ::socket (AF_INET, SOCK_STREAM, 0)) <0) {
		//LOG_DEBUG(formatString("Can't connect to %s at %d, err=%d\n", 
	  //				hostname.c_str(), portnr, errno));
		return (false);
	}

	//# Initialize socket address data structure
	struct sockaddr_in		sockAddr;
	memset ((char *) &sockAddr, 0, sizeof (sockAddr));	//# clear it
	memcpy ((char *) &sockAddr.sin_addr.s_addr, 
		(char *) &IPaddr,
		sizeof (IPaddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port   = portno;

	//# Open socket connection to server (connect does the bind for us)
	if ((::connect (sock, (struct sockaddr *) &sockAddr,
			sizeof(sockAddr))) < 0) {
		//LOG_DEBUG(formatString("Connection error [%d]\n", errno));
		close (sock);
		return (false);
	}

	//# OK, the socket is opened, allocate a databuffer if not already done.
	//allocBuffer(1024);
	itsConnected = true;
	itsHostname  = hostname;
	itsPortnr	 = portnr;
	itsSocketID	 = sock;

	//LOG_DEBUG(formatString("Connection to %s at %d succesfull\n",
	//		  	  itsHostname.c_str(), itsPortnr));

	return (itsConnected);
}

//#
//# Socket::openListener(portnr)
//#
//#	Starts a listener-service on a TCP socket so the client may 
//#	connect to us.
//#
//#	In:		service	Servicename of number to start listener for
//#	Out:	--
//#	Return:	result of open action
bool Socket::openListener (const int16		portnr)
{
	//# put the bytes in the right network-order
	int16	portno = htons (portnr);

	//# Time to try to open the socket.
	int16		sock;
	if ((sock = ::socket (AF_INET, SOCK_STREAM, 0)) <0) {
		//LOG_DEBUG(formatString("Can't open listener at %d, err=%d\n", 
	  //		 		  portnr, errno));
		return (itsConnected);
	}

	//# Turn on reuse-address for more comfort during bind
	//# Also turn keep-alive on against suddenly disappearing hosts
	//# (most of the times behind firewalls).
	//# Ignore errors, it works also without these options.
	long	optval = 1;
	setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
	setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof (optval));

	//# Initialize socket address data structure
	struct sockaddr_in		sockAddr;
	sockAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	sockAddr.sin_family      = AF_INET;
	sockAddr.sin_port		 = portno;

	//# Bind socket to port number
	if ((::bind (sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr)))< 0) {
		//LOG_DEBUG(formatString("Bind error for port %d, err=%d\n", portnr,  errno));
		return (itsConnected);
	}

	//# OK, the socket is opened, we don't need a databuffer, just do
	//# the admin tasks.
	itsSocketID  = sock;
	itsConnected = true;
	itsPortnr	 = portnr;

	::listen (sock, 5);		//# startup the listener.

	//LOG_DEBUG(formatString("Started listener at port %d\n", itsPortnr));

	return (itsConnected);
}

//#
//#	Socket::accept()
//#
//#	Waits for an incomming message at the given socket. The socket must
//#	have been opened with Socket::listen before.
//#	Returns the new socket to continue with, the user should check the
//# connectionstate of the returned socket because it is set to not-conn.
//# when a serious error occured.
//#
//#	In:		--
//#	Out:	--
//#	Return:				Socket of the new connection

Socket	 Socket::accept()
{
	struct sockaddr_in  sockAddr;
	socklen_t	    len = sizeof (sockAddr);
	int16		    hostSock;
	do {
	  //# wait for an incomming connection request
	  //		LOG_TRACE_LOOP_STR("accept-call on sid:" << itsSocketID);
		hostSock = ::accept (itsSocketID, 
				     (struct sockaddr *) &sockAddr, 
				     &len);

		//# if new sock < 0 then serious error or just an interrrupt
		if ((hostSock < 0) && (errno != EINTR)) {
			//LOG_TRACE_COND(formatString("Accept error %d on port %d\n", 
		  //					errno, itsPortnr));
			Socket dummySocket;
			return (dummySocket);
		}
	} while (hostSock < 0L);			//# while not succesfull

	//# create the new data socket
	Socket		dataSocket;
	dataSocket.itsSocketID 	= hostSock;
	dataSocket.itsConnected = true;

	//	LOG_TRACE_LOOP_STR("successfull connect at sid:" << itsSocketID);

	return (dataSocket);
}

//#
//# TBW
//#
//#
void Socket::setBuffer( int   buffersize, 
			char* bufferptr) {
 	// set or create output buffer 
  if (bufferptr != NULL) {
    // external puffer ptr specified; we will use that buffer
    // address and available length are copied into our own variables
    //cout << "accept: reuse buffer  " << endl;
    itsData = bufferptr;
    itsBufferSize = buffersize;
    itsIsMyBuffer = true;
  } else {
    // no external puffer specified, so we will
    // allocate our own buffer
    //cout << "accept: create new buffer" << endl;
    allocBuffer(buffersize);
  }
}

//#
//# Socket::disconnect()
//#
//#	Closes the connection in a nice way and releases the memory of the
//#	connection.
//#
//#	In:		--
//#	Out:	--
//#	Return:	--
void Socket::disconnect()
{
	//LOG_DEBUG(formatString("Closing connection at port %d\n", itsSocketID));
	
	::shutdown (itsSocketID, 2);		//# notify other side
	::close    (itsSocketID);			//# close TCP socket

	freeBuffer();						//# release memory
	itsConnected = false;
	itsSocketID  = 0;
}


//--------------------------- memory management -----------------------------
//#
//# allocBuffer(size)
//#
//#	Allocates a databuffer of the socket
//#	
//#	In:		bufferSize	Size of receivebuffer to allocate.
//#	Out:	--
//#	Return:	--
void Socket::allocBuffer (int32	bufferSize)
{
	freeBuffer();
	itsIsAllocated=true;
	itsData = new char[bufferSize];
	itsIsMyBuffer = true;
	itsBufferSize = bufferSize;

}

//#
//# freeBuffer()
//#
//#	Release the memory of this socket.
//#	
//#	In:		--
//#	Out:	--
//#	Return:	--
void Socket::freeBuffer() 
{
  if (itsIsMyBuffer) {
    delete [] itsData;
  } 
  itsData = 0;
}

//--------------------------- exchanging data -------------------------------
//#
//# Socket::send(message, messageLength)
//#
//#	Write the given message to the TCP/IP port of the given connection after
//#	formatting the message according the used protocol.
//#
//#	In:		msg		Pointer to the start of the data to send
//#			msglen	Number of bytes to send
//#	Out:	--
//#	Return:	Number of bytes written on success, or 0 on failure.
int32 Socket::send(const char*		message,
		   const int32		messageLength)
{
	//# Anything to send?
	if (!messageLength) {
		return (0);
	}

	if (!itsConnected && !connect(itsHostname, itsPortnr)) {
		//LOG_DEBUG(formatString("Write:no connection on %s:%d\n",
		//							itsHostname.c_str(), itsPortnr));
	}

	//	LOG_DEBUG(formatString("Writing %ld bytes to port %d\n", 
	//											messageLength, itsPortnr));

	//# write the data to the socket.
	int32		wLen;
	int flags = 0;
	if ((wLen = ::send (itsSocketID, 
			    message, 
			    messageLength, 
			    flags)	     ) != messageLength) {
	  //LOG_DEBUG(formatString(
	  //			"Write of message fails on port %d, wlen = %ld, err = %d\n", 
	  //			itsPortnr, wLen, errno));
	  return (0);
	}

	//cout << "Socket::send  sent " << wLen << "/" << messageLength <<endl;
   return (messageLength);
}


//#
//#	Socket::recv()
//# 
//#
//#	Pulls all the available bytes from the TCPIP socket and stores them
//#	in the connection buffer. Returns the number of bytes read.
//#
//#	In:	buffer -- the memory address to store the data
//#             len    -- message length to be read
//#	Out:	--
//#	Return:	Number of bytes read from the socket
int32 Socket::recv (char*		buf,
		    const int32		len)
{
  // DbgAssert(len > 0)

  if (!itsConnected && !connect(itsHostname, itsPortnr)) {
    cout << "no connection" << endl;
    //LOG_DEBUG(formatString("Write:no connection on %s:%d\n",
    //							itsHostname.c_str(), itsPortnr));
  }
  //# flush buffer first
  itsRptr = 0;
  itsWptr = 0;
  if (itsData == 0) {
    // LOG_ERROR("itsData buffer not initialised");
    cout << "itsData buffer not initialised" << endl;
    return 0;
  }
  buf[0] = '\0';
  
  //# Setup variables for the read
  if ((buf != NULL) && (len>0)) {
    // probably new values are supplied for the buffer address and length
    itsData = buf;
    //DbgAssertStr(len <= itsBufferSize,"Socket initialised with too small buffer size");
    itsBufferSize = len;
  } 
  // 
  int32	newBytes = 0;
  int flags = MSG_WAITALL;
  errno  = 0;
  //LOG_TRACE_FLOW(formatString("Read on %d:length = %ld\n", itsPortnr, itsBufferSize));
   if (len && itsConnected) {
      //# Try to read from the socket

     while (newBytes < len) {

       newBytes += ::recv (itsSocketID,          // socket
			   itsData + newBytes,   // buffer ptr (with offset)
			   len     - newBytes,   // remaining length
			   flags);               // flags
       //LOG_DEBUG(formatString("Read %ld bytes, errno = %d\n", newBytes, errno));
     }      

     //# Check for errors
     // this should never be reached.
     if (!errno && (newBytes > 0)) {
       //DbgAssertStr(len == newBytes,"did not read all data");
       if (len != newBytes) cout << "did not read all data" << endl;
       //itsData[itsWptr] = '\0';	//# always terminate buf
     }
     else {
       // LOG_ERROR("error during read");
       itsConnected = false;
       newBytes = 0L;
     }
   }
   return (newBytes);
}


//#
//# Socket::poll(newMsg, msgLen, timer)
//#
//#	Wait for timeout millisecs for incomming data.
//# If timeout < 0 then the call is blocking
//#
//#	In:		timer	Max time to wait for data (msec).
//#	Out:	msg		Handle to start of messagedata when msg if available
//#			msglen	Pointer to long containing the length of the received msg
//#	Return:	>=0 	Number of bytes read from the socket
//#			-1		Connection closed by peer.
   int32 Socket::poll (char			**message,
		       int32			*messageLength,
		       const int32		timeout)
{
	//# reset write-pointer
	itsWptr = 0;

	//# perform a poll on this socket
	struct 			pollfd	fdset [1];
	fdset[0].fd     = itsSocketID;
	fdset[0].events = POLLRDNORM;
	int32	presult = ::poll (fdset, 1, timeout);

	//# any data waiting?
	if (!presult) {
		return (0);
	}

	//# serious error or signal received?
	if (presult < 0) {
		if (errno != EINTR) {
			//LOG_DEBUG(formatString("ERROR %d during poll at port %d\n", errno, itsPortnr));
		}
		return (0);
	}

	//# any bytes received?
	int32	newBytes = recv();

	//# disconnected by peer?
	if (!itsConnected) {
		return (-1L);
	}

	//# no new data?
	if (!newBytes)	{
		return (0L);
	}

	//# return the raw data
	*message       = itsData;
	*messageLength = itsWptr;
	return (itsWptr);
}

//--------------------------- showing on a stream ---------------------------
//#
//# operator<<
//#
std::ostream&	operator<< (std::ostream& os, const Socket&	theSocket)
{
	os << "Connect: " << theSocket.itsHostname << "@" << theSocket.itsPortnr << std::endl;
	os << "Socket : " << theSocket.itsSocketID << ":" << theSocket.itsConnected << std::endl;
	os << "Buffer : " << theSocket.itsBufferSize << ", R=" << theSocket.itsRptr << 
										  ", W=" << theSocket.itsWptr << std::endl;
	return os;
}

} //# namespace LOFAR

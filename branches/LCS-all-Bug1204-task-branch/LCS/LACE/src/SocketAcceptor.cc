//#  SocketAcceptor.cc: Class for a point of IO.
//#
//#  Copyright (C) 2008
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

#include <Common/hexdump.h>

//# Includes
#include <sys/poll.h>
#include <Common/LofarLogger.h>
#include <LACE/SocketAcceptor.h>
#include <LACE/SocketSAP.h>

namespace LOFAR {
  namespace LACE {

//
// SocketAcceptor()
//
SocketAcceptor::SocketAcceptor()
{}


//
// ~SocketAcceptor()
//
SocketAcceptor::~SocketAcceptor()
{
	close();
}

//
// open(Address)
//
// return value:
// real error : SK_ALLOC, SK_BIND, SK_LISTEN, SK_SYS_ERROR
//  0 : OK, listener is opened
//
int SocketAcceptor::open(const Address&		anAddress, int	backlog)
{
	// first check arguments
	ASSERTSTR(anAddress.getType() == INET_TYPE, 
						"Address must be of the type InetAddress");
	const InetAddress*	theAddr = dynamic_cast<const InetAddress*>(&anAddress);

	int	error;
	if ((error = doOpen(anAddress)) != SK_OK) {
		LOG_DEBUG("allocating listen-socket failed");
		return(error);
	}

	// bind the address and the socket
	if (::bind(getHandle(), 
				  reinterpret_cast<const sockaddr *> (anAddress.getAddress()) ,
				  anAddress.getAddressSize()) < 0) {
		LOG_DEBUG(formatString("SocketAcceptor:bind(%d) failed: errno=%d(%s)", 
								getHandle(), errno, strerror(errno)));
		close();
		return (SK_BIND);
	}
	LOG_DEBUG(formatString("Socket(%d) bound to address %s", 
				getHandle(), anAddress.deviceName().c_str()));

	if (theAddr->protocolNr() == IPPROTO_TCP) { // TODO or UNIX
		if ((::listen(getHandle(), backlog)) < 0) {
			LOG_DEBUG(formatString("Socket(%d):Listen error for port %d, err = %d(%s)",
					  getHandle(), theAddr->portNr(), 
					  errno, strerror(errno)));
			close();
			return (SK_LISTEN);
		}
		LOG_DEBUG(formatString("Socket(%d):Listener started at port %d",
					getHandle(), theAddr->portNr()));
	}
	itsAddress = *(dynamic_cast<const InetAddress*>(&anAddress));
	return (SK_OK);	
}


//
// accept(SocketStream,	waitMs)
//
// Return value:
//  SK_SYS_ERROR : real error
//  SK_OK		 : OK, connection is made
//  SK_TIMEOUT	 : call complete again to complete the 'connect'
//
int SocketAcceptor::accept(SocketStream&		aStream,
						   int					waitMs)
{
	ASSERTSTR(isOpen(), "Listener is not opened yet, call 'open' first'");

	bool			acceptBlockMode = (waitMs < 0) ? true : false;
	bool			orgBlockMode    = getBlocking();
	int				acceptErrno		= 0;
	struct sockaddr	sockAddr;
	socklen_t		sockAddrLen 	= itsAddress.getAddressSize();
	setBlocking(acceptBlockMode);
	do {
		errno = 0;
		int	deviceID = ::accept(getHandle(),
							  &sockAddr,
							  &sockAddrLen);
		acceptErrno = errno;
		if (deviceID > 0) {
			aStream.setHandle(deviceID);
			setBlocking(acceptBlockMode);
			LOG_DEBUG(formatString("StreamSocket(%d) successful connected",
						aStream.getHandle()));
			aStream.itsAddress.set((struct sockaddr_in *)(&sockAddr), sockAddrLen);
			aStream.itsIsConnected = true;
			return (SK_OK);
		}
	} while ((acceptErrno == EINTR) && acceptBlockMode);

	// Show why we left the accept-call
	LOG_TRACE_COND(formatString("Socket(%d):accept() failed: errno=%d(%s)",
						getHandle(), acceptErrno, strerror(acceptErrno)));

    // In non-blocking mode the errno's:EWOULDBLOCK, EALREADY and EINTR are
    // legal errorcode. In blocking mode we should never come here at all!
    if ((acceptErrno != EWOULDBLOCK) && (acceptErrno != EALREADY)
                                     && (acceptErrno != EINTR)) {
        // real error
		setBlocking(orgBlockMode);
        LOG_TRACE_COND(formatString("Socket(%d):accept():REAL ERROR!",
													 getHandle()));
        return(SK_SYS_ERROR);
    }
	
	// wait 'waitMs' seconds for the accept to complete
	struct pollfd	pollInfo;
	pollInfo.fd		= aStream.getHandle();
	pollInfo.events = POLLIN;
	LOG_TRACE_CALC(formatString("Socket(%d):going into a poll for %d ms",
								aStream.getHandle(), waitMs));
	setBlocking (true);			// we want to wait!
	int	pollRes = poll(&pollInfo, 1, waitMs);
	setBlocking (orgBlockMode);
	
	// proces the result of the poll
	switch (pollRes) {
	case -1:	
		// Note: when an interrupt occured we just return that there was no
		// connection yet. The user will sure try it again some time later.
		// No effort is taken to do a re-poll for the remaining time.
		return ((errno == EINTR) ? SK_TIMEOUT : SK_SYS_ERROR);	// timeout or error on poll
	case  0:	return (SK_TIMEOUT);		// timeout on poll
	}

	// after the poll the error is somewhere deep down under in the socket.
	int32		connRes;
	socklen_t	resLen = sizeof(connRes);
	if (aStream.getOption(SO_ERROR, &connRes, &resLen)) {
		return (SK_SYS_ERROR);	// getOption call failed, assume connection failed
	}
	errno = connRes;			// put error where it belongs

	if (errno != 0) {			// not yet connected
		LOG_DEBUG(formatString("Socket(%d):delayed accept failed also, err=%d(%s)",
									getHandle(), errno, strerror(errno)));
		if (errno == EINPROGRESS || errno == EALREADY) {
			return (SK_TIMEOUT);// timeout on poll
		}
		return (SK_SYS_ERROR);	// real problem
	}

	aStream.setHandle(::accept(getHandle(),
							  &sockAddr,
							  &sockAddrLen));
	ASSERT(aStream.getHandle() > 0);
	LOG_DEBUG(formatString("StreamSocket(%d) successful connected after delayed accept()",
							aStream.getHandle()));
	aStream.itsAddress.set((struct sockaddr_in *)(&sockAddr), sockAddrLen);
	aStream.itsIsConnected = true;

	return (SK_OK);					// no errors
}

  } // namespace LACE
} // namespace LOFAR

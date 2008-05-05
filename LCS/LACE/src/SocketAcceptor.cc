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
	itsListener.close();
}

//
// open(Address)
//
// return value:
// -1 : real error
//  0 : OK, listener is opened
//
int SocketAcceptor::open(const Address&		anAddress, int	backlog)
{
	// first check arguments
	ASSERTSTR(anAddress.getType() == INET_TYPE, 
						"Address must be of the type InetAddress");
	const InetAddress*	theAddr = dynamic_cast<const InetAddress*>(&anAddress);

	if (itsListener.doOpen(anAddress) != 0) {
		LOG_DEBUG("allocating listen-socket failed");
		return(-1);
	}

	// bind the address and the socket
	if (::bind(itsListener.getHandle(), 
				  reinterpret_cast<const sockaddr *> (anAddress.getAddress()) ,
				  anAddress.getAddressSize()) < 0) {
		LOG_DEBUG(formatString("SocketAcceptor:bind(%d) failed: errno=%d(%s)", 
								itsListener.getHandle(), errno, strerror(errno)));
		itsListener.close();
		return (-1);
	}
	LOG_DEBUG(formatString("Socket(%d) bound to address %s", 
				itsListener.getHandle(), anAddress.deviceName().c_str()));

	if (theAddr->protocolNr() == IPPROTO_TCP) { // TODO or UNIX
		if ((::listen(itsListener.getHandle(), backlog)) < 0) {
			LOG_DEBUG(formatString("Socket(%d):Listen error for port %d, err = %d(%s)",
					  itsListener.getHandle(), theAddr->portNr(), 
					  errno, strerror(errno)));
			itsListener.close();
			return (-1);
		}
		LOG_DEBUG(formatString("Socket(%d):Listener started at port %d",
					itsListener.getHandle(), theAddr->portNr()));
	}
	itsAddress = *(dynamic_cast<const InetAddress*>(&anAddress));

	return (0);	
}


//
// accept(SocketStream,	waitMs)
//
// Return value:
// -1 : real error
//  0 : OK, connection is made
//  1 : call complete again to complete the 'connect'
//
int SocketAcceptor::accept(SocketStream&		aStream,
						   int					waitMs)
{
	ASSERTSTR(itsListener.isOpen(), "Listener is not opened yet, call 'open' first'");

	bool			acceptBlockMode = (waitMs < 0) ? true : false;
	bool			orgBlockMode    = itsListener.getBlocking();
	int				acceptErrno		= 0;
	struct sockaddr	sockAddr;
	socklen_t		sockAddrLen 	= itsAddress.getAddressSize();
	itsListener.setBlocking(acceptBlockMode);
	do {
		errno = 0;
		int	deviceID = ::accept(itsListener.getHandle(),
							  &sockAddr,
							  &sockAddrLen);
		acceptErrno = errno;
		if (deviceID > 0) {
			aStream.setHandle(deviceID);
			itsListener.setBlocking(acceptBlockMode);
			LOG_DEBUG(formatString("StreamSocket(%d) successful connected",
						aStream.getHandle()));
			aStream.itsAddress.set((struct sockaddr_in *)(&sockAddr), sockAddrLen);
			aStream.itsIsConnected = true;
			return (0);
		}
	} while ((acceptErrno == EINTR) && acceptBlockMode);

	// Show why we left the accept-call
	LOG_TRACE_COND(formatString("Socket(%d):accept() failed: errno=%d(%s)",
						itsListener.getHandle(), acceptErrno, strerror(acceptErrno)));

    // In non-blocking mode the errno's:EWOULDBLOCK, EALREADY and EINTR are
    // legal errorcode. In blocking mode we should never come here at all!
    if ((acceptErrno != EWOULDBLOCK) && (acceptErrno != EALREADY)
                                     && (acceptErrno != EINTR)) {
        // real error
		itsListener.setBlocking(orgBlockMode);
        LOG_TRACE_COND(formatString("Socket(%d):accept():REAL ERROR!",
													 itsListener.getHandle()));
        return(-1);
    }
	
	// wait 'waitMs' seconds for the accept to complete
	struct pollfd	pollInfo;
	pollInfo.fd		= aStream.getHandle();
	pollInfo.events = POLLWRNORM;
	LOG_TRACE_CALC(formatString("Socket(%d):going into a poll for %d ms",
								aStream.getHandle(), waitMs));
	itsListener.setBlocking (true);			// we want to wait!
	int	pollRes = poll(&pollInfo, 1, waitMs);
	itsListener.setBlocking (orgBlockMode);
	
	// proces the result of the poll
	switch (pollRes) {
	case -1:	
		// Note: when an interrupt occured we just return that there was no
		// connection yet. The user will sure try it again some time later.
		// No effort is taken to do a re-poll for the remaining time.
		return ((errno == EINTR) ? 1 : -1);	// timeout or error on poll
	case  0:	return (1);		// timeout on poll
	}

	// after the poll the error is somewhere deep down under in the socket.
	int32		connRes;
	socklen_t	resLen = sizeof(connRes);
	if (aStream.getOption(SO_ERROR, &connRes, &resLen)) {
		return (-1);			// getOption call failed, assume connection failed
	}
	errno = connRes;			// put error where it belongs

	if (errno != 0) {			// not yet connected
		LOG_DEBUG(formatString("Socket(%d):delayed accept failed also, err=%d(%s)",
									itsListener.getHandle(), errno, strerror(errno)));
		if (errno == EINPROGRESS || errno == EALREADY) {
			return (1);			// timeout on poll
		}
		return (-1);			// real problem
	}

	aStream.setHandle(::accept(itsListener.getHandle(),
							  &sockAddr,
							  &sockAddrLen));
	ASSERT(aStream.getHandle() > 0);
	LOG_DEBUG(formatString("StreamSocket(%d) successful connected after delayed accept()",
							aStream.getHandle()));
	aStream.itsAddress.set((struct sockaddr_in *)(&sockAddr), sockAddrLen);
	aStream.itsIsConnected = true;

	return (0);					// no errors
}

  } // namespace LACE
} // namespace LOFAR

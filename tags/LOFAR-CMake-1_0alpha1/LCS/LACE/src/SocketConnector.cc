//#  SocketConnector.cc: Class for a point of IO.
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

//# Includes
#include <sys/poll.h>
#include <Common/LofarLogger.h>
#include <LACE/SocketConnector.h>
#include <LACE/SocketSAP.h>

namespace LOFAR {
  namespace LACE {

//
// SocketConnector()
//
SocketConnector::SocketConnector()
{}


//
// ~SocketConnector()
//
SocketConnector::~SocketConnector()
{}

//
// connect(blocking)
//
// return value:
// SK_NOT_OPENED, SK_SYS_ERROR : real error
// SK_OK		 : OK, connection is made
// SK_TIMEOUT	 : call complete to complete the connect
//
int SocketConnector::connect(SocketStream&		aStream,
					   		 const Address&		anAddress,
							 int				waitMs)
{
	// first check arguments
//	ASSERTSTR(aStream, "ServiceAccessPoint may not be null");
	ASSERTSTR(anAddress.getType() == INET_TYPE, 
						"Address must be of the type InetAddress");

	// allocate the SAP
	if (aStream.open(dynamic_cast<const InetAddress&>(anAddress), true) < 0) {
		return (SocketSAP::SK_NOT_OPENED);
	}

	// do the connect.
	if (::connect(aStream.getHandle(), 
				  reinterpret_cast<const sockaddr *> (anAddress.getAddress()) ,
				  anAddress.getAddressSize()) >= 0) {
		LOG_DEBUG(formatString("SocketConnector:connect(%d) successful", aStream.getHandle()));
		aStream.itsIsConnected = true;
		return (SocketSAP::SK_OK);
	}
	LOG_DEBUG(formatString("SocketConnector:connect(%d) failed: errno=%d(%s)", 
							aStream.getHandle(), errno, strerror(errno)));

	// When failure is not due to a timeout, close the socket
	if (errno != EINPROGRESS && errno != EALREADY) {	// real error
		aStream.close();
		return (SocketSAP::SK_SYS_ERROR);
	}

	// connect failed wit a timeout, give it a retry.
	bool	orgBlockMode  = aStream.getBlocking();
	bool	pollBlockMode = (waitMs < 0) ? true : false;
	
	// wait 'waitMs' seconds for the connect to complete
	struct pollfd	pollInfo;
	pollInfo.fd		= aStream.getHandle();
	pollInfo.events = POLLWRNORM;
	LOG_TRACE_CALC(formatString("Socket(%d):going into a poll for %d ms",
								aStream.getHandle(), waitMs));
	aStream.setBlocking (pollBlockMode);
	int	pollRes = poll(&pollInfo, 1, waitMs);
	aStream.setBlocking (orgBlockMode);
	
	// proces the result of the poll
	switch (pollRes) {
	case -1:	return (SocketSAP::SK_SYS_ERROR);	// error on poll
	case  0:	return (SocketSAP::SK_TIMEOUT);	// timeout on poll
	}

	// after the poll the error is somewhere deep down under in the socket.
	int32		connRes;
	socklen_t	resLen = sizeof(connRes);
	if (aStream.getOption(SO_ERROR, &connRes, &resLen)) {
		return (SocketSAP::SK_SYS_ERROR);	// getOption call failed, assume connection failed
	}
	errno = connRes;			// put error where it belongs

	if (errno != 0) {			// not yet connected
		LOG_DEBUG(formatString("Socket(%d):delayed connect failed also, err=%d(%s)",
									aStream.getHandle(), errno, strerror(errno)));
		if (errno == EINPROGRESS || errno == EALREADY) {
			return (SocketSAP::SK_TIMEOUT);// timeout on poll
		}
		return (SocketSAP::SK_SYS_ERROR);			// real problem
	}

	LOG_DEBUG(formatString("Socket(%d):delayed connect() succesful", 
														aStream.getHandle()));
	aStream.itsIsConnected = true;

	return (SocketSAP::SK_OK);					// no errors
}

  } // namespace LACE
} // namespace LOFAR

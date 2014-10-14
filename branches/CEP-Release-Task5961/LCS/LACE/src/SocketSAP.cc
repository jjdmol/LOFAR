//# SocketSAP.cc: Abstract base class for several kinds of sockets
//#
//# Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <unistd.h>					// file stuff
#include <netinet/in.h>				// IPPROTO
//#include <netdb.h>					// fileStat
#include <errno.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <LACE/SocketSAP.h>

namespace LOFAR {
  namespace LACE {

//
// SocketSAP()
//
SocketSAP::SocketSAP()
{}


//
// ~SocketSAP()
//
SocketSAP::~SocketSAP()
{
	this->close();
}


//
// close()
//
void SocketSAP::close()
{
	if (getHandle() != 0) {
		LOG_DEBUG_STR("Shutdown and close of socket: " << getHandle());
		::shutdown(getHandle(), 2);
		::close(getHandle());
		setHandle(SK_OK);
	}
}


//
// open(address, reuseAddress)
//
// Returns: SK_OK, SK_ALLOC, SK_SYS_ERROR
//
int SocketSAP::doOpen(const Address&	anAddress,
					  bool				reuseAddress)
{
	LOG_DEBUG_STR("SocketSAP::doOpen(" << anAddress.deviceName() << ")");

	ASSERTSTR(anAddress.getType() == INET_TYPE, 
			  "Only addresses of the type Inet are allowed");
	// check for (non fatal) programming errors
	if (getHandle()) {
		LOG_WARN(formatString("Socket(%d) is already open!!", getHandle()));
		return (SK_OK);		// it wrong, but not a failure.
	}

	// check protocol type
	int	protocolNr = dynamic_cast<const InetAddress&>(anAddress).protocolNr();
	ASSERTSTR(protocolNr == IPPROTO_UDP || protocolNr == IPPROTO_TCP, 
				"SocketSAP only supports UDP and TCP connections");

	// first create the underlaying socket
	int	socketType = (protocolNr == IPPROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;
	setHandle(::socket(PF_INET, socketType, protocolNr));
	if (!getHandle()) {
		LOG_ERROR(formatString("SocketSAP:Can not allocate a socket(%d,%d), err=%d(%s)", 
					socketType, protocolNr, errno, strerror(errno)));
		return (SK_ALLOC);
	}

	LOG_DEBUG(formatString("SocketSAP:Created socket(%d) for protocol %d",
				getHandle(), protocolNr));

	// set the socket options we consider default
	socklen_t		optVal 	= reuseAddress ? 1 : 0;
	int				one		= 1;
	struct linger	lin 	= { 1, 1 };
	if ((setOption(SO_REUSEADDR, &optVal,sizeof(optVal)) < 0) ||
		(setOption(SO_KEEPALIVE, &one, 	 sizeof(one)) < 0) ||
		(setOption(SO_LINGER,    &lin, 	 sizeof(lin)) < 0)) {
		LOG_ERROR("Setting socket options failed, closing just opened socket");
		this->close();
		return (SK_SYS_ERROR);
	}

	itsAddress = dynamic_cast<const InetAddress&>(anAddress);

	return (SK_OK);
}

//
// read (buffer, nrBytes)
//
// Returns: SK_NOT_OPENED, SK_PEER_CLOSED, SK_SYS_ERROR
// >0 = number of bytes received
//
int SocketSAP::read(void*	buffer, size_t	nrBytes)
{
	// socket must be open ofcourse
	if (!getHandle()) {
		LOG_ERROR("SocketSAP::read: socket not opened yet.");
		return (SK_NOT_OPENED);
	}

	// check arguments
	ASSERTSTR (buffer, "SocketSAP::read:null buffer");
	if (!nrBytes)  {
		return (nrBytes);
	}

	// lets try to write something
	int32	bytesLeft = nrBytes;
	int32	bytesRead = 0;
	if (itsAddress.protocolNr() == IPPROTO_UDP) {
	    errno = 0;
		socklen_t	alen = itsAddress.getAddressSize();
		if ((bytesRead = recvfrom (getHandle(), (char*) buffer, nrBytes, 0,
									(sockaddr*) itsAddress.getAddress(),
									&alen) <= 0) || errno) {
			LOG_DEBUG(formatString("SocketSAP(%d):read(%d)=%d, errno=%d(%s)", 
						getHandle(), bytesLeft, bytesRead, 
						errno, strerror(errno)));
			if (bytesRead < 0) { 					// error?
				return (errno == EWOULDBLOCK ? 0 : SK_SYS_ERROR);
			}

			if (bytesRead == 0) {					// conn reset by peer?
				this->close();
				return (SK_PEER_CLOSED);
			}
		}

		return (bytesRead);
	}

	// ----- UNIX and TCP sockets -----
	errno = 0;								// reset system errno
	while (bytesLeft > 0 && !errno) {
		bytesRead = ::recv (getHandle(), (char*)buffer, bytesLeft, 0);
		LOG_DEBUG(formatString("SocketSAP(%d):read(%d)=%d, errno=%d(%s)", 
					getHandle(), bytesLeft, bytesRead, 
					errno, strerror(errno)));

		if (bytesRead < 0) { 					// error?
			return ((errno == EWOULDBLOCK) ? ((int)nrBytes - bytesLeft) : SK_SYS_ERROR);
		}

		if (bytesRead == 0) {					// conn reset by peer?
			this->close();
			return (SK_PEER_CLOSED);
		}

		buffer = bytesRead + (char*)buffer;
		bytesLeft -= bytesRead;
		errno = 0;	

	} // while

	return (nrBytes - bytesLeft);
}

//
// write (buffer, nrBytes)
//
// Returnvalue: < 0					SK_NOT_OPENED, SK_PEER_CLOSED, SK_SYS_ERROR
//				>= 0 != nrBytes		intr. occured or socket is nonblocking
//				>= 0 == nrBytes		everthing went OK.
//
int SocketSAP::write(const void*	buffer, size_t	nrBytes)
{
	// socket must be open ofcourse
	if (!getHandle()) {
		LOG_ERROR("SocketSAP::write: socket not opened yet.");
		return (SK_NOT_OPENED);
	}

	// check arguments
	ASSERTSTR (buffer, "SocketSAP::write:null buffer");
	if (!nrBytes)  {
		return (nrBytes);
	}

	LOG_DEBUG_STR("SocketSAP::Write(...)");
	// lets try to write something
	int32	bytesLeft = nrBytes;
	int32	bytesWritten = 0;
	if (itsAddress.protocolNr() == IPPROTO_UDP) {
		errno = 0;
		if ((bytesWritten = sendto (getHandle(), (char*) buffer, nrBytes, 0,
									(sockaddr*) itsAddress.getAddress(),
									itsAddress.getAddressSize()) <= 0) || errno) {
			LOG_ERROR_STR(formatString("SocketSAP::write: Error during write, errno=%d(%s)",
							errno, strerror(errno)));
			if (errno == EWOULDBLOCK) {
				return (0);		// nr of bytes written
			}

			if (errno == ECONNRESET) {					// conn reset by peer?
				this->close();
				return (SK_PEER_CLOSED);
			}
			return (SK_SYS_ERROR);
		}
		return (nrBytes);
	}

	// UNIX or TCP socket
	errno = 0;	
	while (bytesLeft > 0 && !errno) {
		bytesWritten = ::write (getHandle(), buffer, bytesLeft);

		if (bytesWritten < 0) {
			if (errno == EWOULDBLOCK) { 
				return (nrBytes - bytesLeft);
			}
			else  {// else a real error
				return (SK_SYS_ERROR);
			} 
		}

		// connection closed by peer?
		if (bytesWritten == 0) {
			this->close();
			return (SK_PEER_CLOSED);
		}

		// bytesWritten > 0
		buffer = bytesWritten + (char*)buffer;
		bytesLeft -= bytesWritten;
		errno = 0;	
	} // while

	return (nrBytes - bytesLeft);
}



  } // namespace LACE
} // namespace LOFAR

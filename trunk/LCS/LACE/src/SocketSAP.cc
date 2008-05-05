//#  SocketSAP.cc: Abstract base class for several kinds of sockets
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
#include <unistd.h>					// file stuff
#include <netinet/in.h>				// IPPROTO
//#include <netdb.h>					// fileStat
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
		setHandle(0);
	}
}


//
// open(address, reuseAddress)
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
		return (0);		// it wrong, but not a failure.
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
		return (-1);
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
		return (-1);
	}

	itsAddress = dynamic_cast<const InetAddress&>(anAddress);

	return (0);
}

//
// read (buffer, nrBytes)
//
int SocketSAP::read(void*	buffer, size_t	nrBytes)
{
	// socket must be open ofcourse
	if (!getHandle()) {
		LOG_ERROR("SocketSAP::read: socket not opened yet.");
		return (-1);
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
		size_t	alen = itsAddress.getAddressSize();
		if ((bytesRead = recvfrom (getHandle(), (char*) buffer, nrBytes, 0,
									(sockaddr*) itsAddress.getAddress(),
									&alen) <= 0) || errno) {
			LOG_DEBUG(formatString("SocketSAP(%d):read(%d)=%d, errno=%d(%s)", 
						getHandle(), bytesLeft, bytesRead, 
						errno, strerror(errno)));
			return (-1);
		}

		return (nrBytes);
	}

	// ----- UNIX and TCP sockets -----
	errno = 0;								// reset system errno
	while (bytesLeft > 0 && !errno) {
		bytesRead = ::recv (getHandle(), (char*)buffer, bytesLeft, 0);
		LOG_DEBUG(formatString("SocketSAP(%d):read(%d)=%d, errno=%d(%s)", 
					getHandle(), bytesLeft, bytesRead, 
					errno, strerror(errno)));

		if (bytesRead < 0) { 					// error?
			if (errno == EWOULDBLOCK) { 
				return (nrBytes - bytesLeft);
			}
			else {								// else a real error
				return (-1);
			}
		}

		if (bytesRead == 0) {					// conn reset by peer?
			this->close();
			return (0);
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
// Returnvalue: < 0					error occured
//				>= 0 != nrBytes		intr. occured or socket is nonblocking
//				>= 0 == nrBytes		everthing went OK.
//
int SocketSAP::write(const void*	buffer, size_t	nrBytes)
{
	// socket must be open ofcourse
	if (!getHandle()) {
		LOG_ERROR("SocketSAP::write: socket not opened yet.");
		return (-1);
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
			return (-1);
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
				return (-1);
			} 
		}

		// connection closed by peer?
		if (bytesWritten == 0) {
			this->close();
			return (0);
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

//#  GTM_TCPSocket.cc: base class for all sockets
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include "GTM_TCPSocket.h"
#include "GTM_FileHandler.h"
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

GTMTCPSocket::GTMTCPSocket(GCFTCPPort& port, bool useUDP) :
  GTMFile      (port),
  itsUseUDP    (useUDP),
  itsConnecting(false)
{
}

void GTMTCPSocket::doWork()
{
	LOG_TRACE_FLOW("GTMTCPSocket::doWork()");
	unsigned long bytesRead = 0;

	if (ioctl(_fd, FIONREAD, &bytesRead) > -1) {
		if (bytesRead == 0 && !itsUseUDP) {             // We need the test on UDP :-(
			GCFEvent discoEvent(F_DISCONNECTED);
			itsScheduler->queueEvent(0, discoEvent, &_port);
		}
		else {
			GCFEvent dataEvent(F_DATAIN);
			itsScheduler->queueEvent(0, dataEvent, &_port);
		}
	}
	else {
		ASSERT(_port.getTask());
		LOG_FATAL(LOFAR::formatString ("%s(%s): Error in 'ioctl' on socket fd %d: %s",
					_port.getTask()->getName().c_str(),
					_port.getName().c_str(),
					_fd,
					strerror(errno)));
	}
}

//
// send(buf, count)
//
ssize_t GTMTCPSocket::send(void* buf, size_t count)
{
	if (_fd < 0) {
		LOG_WARN("send, error: Socket not opened");
		return (-1);
	}

	ssize_t countLeft(count);
	ssize_t written(0);
	do {
		if (itsUseUDP) {
			written = sendto(_fd, ((char*)buf) + (count - countLeft), countLeft, 0,
							(struct sockaddr*) &itsTCPaddr, sizeof(itsTCPaddr));
		}
		else {
			written = ::write(_fd, ((char*)buf) + (count - countLeft), countLeft);
		}
		if (written == 0) {		// is it a disconnect?
			return (0);
		}

		if (written == -1) {
			if (errno == ECONNRESET) {
				return (0);
			}
			if (errno != EINTR && errno != EAGAIN) {
				LOG_WARN(LOFAR::formatString ( "send, error(%d): %s", errno, strerror(errno)));
				return -1;
			}
		}
		else {
			countLeft -= written;
		}      
	} while (countLeft > 0);

	return count;
}

//
// recv(buf, count, raw)
//
ssize_t GTMTCPSocket::recv(void* buf, size_t count, bool raw)
{
	if (_fd < 0) {
		LOG_WARN("recv, error: Socket not opend");
		return (-1);
	}

	ssize_t countLeft(count);
	ssize_t received(0);
	socklen_t   addrLen = sizeof(itsTCPaddr);
	do {
		if (itsUseUDP) {
			return (recvfrom(_fd, (char*)buf, count, 0, (struct sockaddr*) &itsTCPaddr, &addrLen));
		}

		received = ::read(_fd, ((char*)buf) + (count - countLeft), countLeft);
		if (received == 0) {	// is it a disconnect?
			return(0);
		}

		if (received == -1) {
			if (errno != EINTR && errno != EAGAIN) {
				LOG_WARN(formatString ( "recv, error(%d): %s", errno, strerror(errno)));
				return -1;
			}
		}
		else {
			countLeft -= received;
		}      
	} while (!raw && (countLeft > 0));

	return (count - countLeft);
}

//
// open
//
bool GTMTCPSocket::open(unsigned int /*portNumber*/)
{
	LOG_TRACE_COND_STR("open:fd=" << _fd << ", port=" << _port.getName());

	if (_fd >= 0) {		// already open?
		return (true);
	}

	_fd = ::socket(AF_INET, itsUseUDP ? SOCK_DGRAM : SOCK_STREAM, 0);
	if (_fd < 0) {
		LOG_WARN(formatString ( "::socket, error: %s", strerror(errno)));
		close();
	}

    // close socket on execve
	if (::fcntl(_fd, F_SETFD, FD_CLOEXEC) != 0) {
		close();
	}

	return (_fd > -1);
}

//
// connect(portnr, host)
//
// Return: -1:error, 0:wait, 1:ok
//
int GTMTCPSocket::connect(unsigned int portNumber, const string& host)  
{
	LOG_TRACE_COND_STR(_port.getName() << ":connect(" << portNumber << "," << host << "),fd=" << _fd);

	// try to resolve hostaddress/name
	struct hostent *hostinfo;
	hostinfo = gethostbyname(host.c_str());
	ASSERTSTR(hostinfo, _port.getName() << ":hostname " << host << " could not be resolved, error = " << errno);

	// try to connect
	itsTCPaddr.sin_family = AF_INET;
	itsTCPaddr.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
	itsTCPaddr.sin_port = htons(portNumber);
	errno = 0;

	if (!itsConnecting) {
		// create a new connection
		if ((::connect(_fd, (struct sockaddr *)&itsTCPaddr, sizeof(struct sockaddr_in)) == 0)) {
			// connect succesfull, register filedescriptor
			setFD(_fd);
			return (1);
		}

		// socket should be in 'non-blocking' mode so some errors are allowed
		if (errno != EINPROGRESS) {
			// serious error
			LOG_WARN_STR(_port.getName() << ":connect(" << host << "," << portNumber << "), error: " << strerror(errno));
			close();
			return (-1);	
		}

		itsConnecting = true;

	} else {
		// poll an existing connection
		fd_set fds;
		struct timeval timeout = { 0, 0 };

		FD_ZERO(&fds);
		FD_SET(_fd, &fds);

		// if the socket is writable, then the connection is established
		switch(::select(_fd + 1, NULL, &fds, NULL, &timeout)) {
			case 0:
				// no data available
				break;

			case -1:
				// serious error
				LOG_WARN_STR(_port.getName() << ":select(" << host << "," << portNumber << "), error: " << strerror(errno));
				close();
				return (-1);	

			default:
				// data available OR connection error
				int so_error;
				socklen_t slen = sizeof so_error;
				if (getsockopt(_fd, SOL_SOCKET, SO_ERROR, &so_error, &slen) < 0) {
					// serious error
					LOG_WARN_STR(_port.getName() << ":getsockopt(" << host << "," << portNumber << "), error: " << strerror(errno));
					close();
					return (-1);	
				}
				
				if (so_error == 0) {
					// connect succesfull, register filedescriptor
					setFD(_fd);
					return 1;
				}

				// connection failure
				LOG_WARN_STR(_port.getName() << ":connect(" << host << "," << portNumber << "), error: " << strerror(errno));
				close();
				return (-1);	
		} // switch
	} // case

	LOG_DEBUG_STR(_port.getName() << ": still waiting for connection");
	return (0);
} 

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

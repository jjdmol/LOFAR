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
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

GTMTCPSocket::GTMTCPSocket(GCFTCPPort& port) :
  GTMFile(port),
  _connecting(false)
{
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
		written = ::write(_fd, ((char*)buf) + (count - countLeft), countLeft);
		if (written == 0) {		// is it a disconnect?
			return (0);
		}

		if (written == -1) {
			if (errno == ECONNRESET) {
				return (0);
			}
			if (errno != EINTR) {
				LOG_WARN(LOFAR::formatString ( "send, error: %s", strerror(errno)));
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
	do {
		received = ::read(_fd, ((char*)buf) + (count - countLeft), countLeft);
		if (received == 0) {	// is it a disconnect?
			return(0);
		}

		if (received == -1) {
			if (errno != EINTR) {
				LOG_WARN(formatString ( "recv, error: %s", strerror(errno)));
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

	_fd = ::socket(AF_INET, SOCK_STREAM, 0);
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
	struct sockaddr_in serverAddr;
	struct hostent *hostinfo;
	hostinfo = gethostbyname(host.c_str());
	ASSERTSTR(hostinfo, _port.getName() << ":hostname " << host << " could not be resolved, error = " << errno);

	// try to connect
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
	serverAddr.sin_port = htons(portNumber);
	errno = 0;

        if (!_connecting) {
		// create a new connection
		if ((::connect(_fd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in)) == 0)) {
			// connect succesfull, register filedescriptor
			ASSERT(_pHandler);
			_pHandler->registerFile(*this);
			return (1);
		}

		// socket should be in 'non-blocking' mode so some errors are allowed
		if (errno != EINPROGRESS) {
			// serious error
			LOG_WARN_STR(_port.getName() << ":connect(" << host << "," << portNumber << "), error: " << strerror(errno));
			::close(_fd);
			_fd = -1;
			return (-1);	
		}

		_connecting = true;

	} else {
		// poll an existing connection
		fd_set fds;
		struct timeval timeout = { 0, 0 };

		FD_ZERO(&fds);
		FD_SET(_fd, &fds);

		// if the socket is writable, then the connection is established
		switch(::select(_fd + 1, NULL, &fds, NULL, &timeout)) {
			case -1:
				// serious error
				LOG_WARN_STR(_port.getName() << ":select(" << host << "," << portNumber << "), error: " << strerror(errno));
				::close(_fd);
				_fd = -1;
				return (-1);	
			case 0:
				// no data available
				return 0;

			default:
                                // check whether connect was succesful or error
                                int so_error;
                                socklen_t slen = sizeof so_error;
                                if (getsockopt(_fd, SOL_SOCKET, SO_ERROR, &so_error, &slen) < 0) {
                                    // serious error
                                    LOG_WARN_STR(_port.getName() << ":getsockopt(" << host << "," << portNumber << "), error: " << strerror(errno));
                                    ::close(_fd);
                                    _fd = -1;
                                    return (-1);	
                                }

                                if (so_error == 0) {
                                    // connect succesfull, register filedescriptor
                                    ASSERT(_pHandler);
                                    _pHandler->registerFile(*this);
                                    return 1;
                                }

                                // connection failure
                                LOG_WARN_STR(_port.getName() << ":connect(" << host << "," << portNumber << "), error: " << strerror(errno));
                                ::close(_fd);
                                _fd = -1;
                                return (-1);	
		}
	}

	LOG_DEBUG_STR(_port.getName() << ": still waiting for connection");
	return(0);
} 

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

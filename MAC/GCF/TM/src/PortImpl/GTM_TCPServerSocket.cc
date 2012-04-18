//#  GTM_TCPServerSocket.cc: server socket implementation
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

#include "GTM_TCPServerSocket.h"
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Protocols.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_ACCEPT_RETRIES 100 /* maximum number of ::accept retries */

namespace LOFAR {
 namespace GCF {
  namespace TM {
    
GTMTCPServerSocket::GTMTCPServerSocket(GCFTCPPort& port, bool isProvider) : 
	GTMTCPSocket(port),
	_isProvider(isProvider),
	_pDataSocket(0)
{
}

GTMTCPServerSocket::~GTMTCPServerSocket()
{
	close();  
}

bool GTMTCPServerSocket::open(unsigned int portNumber)
{
	if (_fd != -1) {
		if (_pDataSocket != 0) {
			_pDataSocket->close();
			return(true);
		}
		return (false);
	}

	struct sockaddr_in address;
	int addrLen;
	int socketFD = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1) {
		LOG_WARN(formatString ( "::socket, error: %s", strerror(errno)));
		return (false);
	}
	unsigned int val = 1;
	struct linger lin = { 1,1 };

	if (::setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val)) < 0) {
		LOG_WARN(formatString (
					"Error on setting socket options SO_REUSEADDR: %s",
					strerror(errno)));
		return false;
	}

	if (::setsockopt(socketFD, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(lin)) < 0) {
		LOG_WARN(formatString (
					"Error on setting socket options SO_LINGER: %s",
					strerror(errno)));
		return false;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(portNumber);
	addrLen = sizeof(address);
	if (bind(socketFD, (struct sockaddr*)&address, addrLen) == -1) {
		LOG_WARN(formatString ( "::bind, error: %s", strerror(errno)));
		close();
		return (false);
	}

	if (listen(socketFD, 5) == -1) {    
		LOG_WARN(formatString ( "::listen, error: %s", strerror(errno)));
		close();
		return (false);
	}

	setFD(socketFD);
	if (fcntl(socketFD, F_SETFL, FNDELAY) != 0) {
		close();
		return (false);
	}    

	return (true);
}

//
// doWork()
//
// Called by the GTM_FileHandler when 'select' returned an event on this fd.
//
void GTMTCPServerSocket::doWork()
{
	LOG_TRACE_FLOW("GTMTCPServerSocket::doWork()");

	if (_isProvider) {
		GCFEvent acceptReqEvent(F_ACCEPT_REQ);
		forwardEvent(acceptReqEvent);
	}
	else {
		GCFTCPPort* pPort = (GCFTCPPort*)(&_port);

		ASSERT(_pDataSocket == 0);
		_pDataSocket = new GTMTCPSocket(*pPort);
		ASSERT (_pDataSocket->getFD() < 0);

		struct sockaddr_in clientAddress;
		socklen_t clAddrLen = sizeof(clientAddress);
		int newSocketFD;

		/* loop to handle transient errors */
		int retries = MAX_ACCEPT_RETRIES;
		while ((newSocketFD = ::accept(_fd, (struct sockaddr*) &clientAddress, &clAddrLen)) < 0
			&& (EINTR == errno || EWOULDBLOCK == errno || EAGAIN == errno) && --retries > 0) 
			/*noop*/;

		_pDataSocket->setFD(newSocketFD);

		if (_pDataSocket->getFD() >= 0) {
			GCFEvent connectedEvent(F_CONNECTED);
			forwardEvent(connectedEvent);
		}
		else {
			LOG_WARN(formatString ( "::accept(%d, %d)-> error:(%d)=%s", _fd, clAddrLen, errno, strerror(errno)));
		}

		// because we only accept one connection (SPP), we don't need to listen with
		// this socket anymore
		GTMFile::close();
	}
}

//
// accept(newsocket)
//
// Called by the usercode after getting an F_ACCEPT_REQ
//
bool GTMTCPServerSocket::accept(GTMFile& newSocket)
{
	bool result(false);
	if (_isProvider && _pDataSocket == 0) {
		struct sockaddr_in clientAddress;
		socklen_t clAddrLen = sizeof(clientAddress);
		int newSocketFD;

		/* loop to handle transient errors */
		int retries = MAX_ACCEPT_RETRIES;
		while ((newSocketFD = ::accept(_fd, (struct sockaddr*) &clientAddress, &clAddrLen)) < 0
			&& (EINTR == errno || EWOULDBLOCK == errno || EAGAIN == errno) && --retries > 0) 
			/*noop*/;

		result = (newSocket.setFD(newSocketFD) > 0);
		if (!result) {
			LOG_WARN(formatString ("::accept, error: %s", strerror(errno)));
		}
	}

	return result;
}

bool GTMTCPServerSocket::close()
{
	bool result(true);

	if (!_isProvider && _pDataSocket != 0) {
		result = _pDataSocket->close();
		delete _pDataSocket;
		_pDataSocket = 0;
	}
	if (result) {
		result = GTMFile::close();
	}

	return result;
}

ssize_t GTMTCPServerSocket::send(void* buf, size_t count)
{
	if (!_isProvider && _pDataSocket != 0) {
		return _pDataSocket->send(buf, count);
	}
	return 0;
}

ssize_t GTMTCPServerSocket::recv(void* buf, size_t count, bool  raw)
{
	if (!_isProvider && _pDataSocket != 0) {
		return _pDataSocket->recv(buf, count, raw);
	}
	return 0;
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

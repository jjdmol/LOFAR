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

#include "GTM_TCPServerSocket.h"
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_TCPPort.h>
#include <GCF/GCF_TMProtocols.h>
#include <GCF/GCF_PeerAddr.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

GTMTCPServerSocket::GTMTCPServerSocket(GCFTCPPort& port, bool isProvider) : 
  GTMTCPSocket(port),
  _isProvider(isProvider),
  _pTCPServerSocket(0)
{
}

GTMTCPServerSocket::~GTMTCPServerSocket()
{
  close();  
}

int GTMTCPServerSocket::open(GCFPeerAddr& localaddr)
{
  int result(-1);
  if (_socketFD == -1)
  {
    struct sockaddr_in address;
    int addrLen;
    int socketFD = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD > -1)
    {
      unsigned int val = 1;
      struct linger lin = { 1,1 };

      if (::setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val)) < 0)
        return -2;

      if (::setsockopt(socketFD, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(lin)) < 0)
        return -2;
      
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_ANY);
      address.sin_port = htons(localaddr.getPortnumber());
      addrLen = sizeof(address);
      if (bind(socketFD, (struct sockaddr*)&address, addrLen) > -1)
      {
        if (listen(socketFD, 5) > -1)
        {    
          setFD(socketFD);
          result = (fcntl(socketFD, F_SETFL, FNDELAY) < 0 ? -1 : 0);            
        }
      }
      if (result < 0)
        close();
    }    
  }
  else
  {
    if (_pTCPServerSocket != 0)
    {
      _pTCPServerSocket->close();
      result = 0;
    }
  }
    
  return result;
}

void GTMTCPServerSocket::workProc()
{
  if (_isProvider)
  {
    GCFEvent acceptReqEvent(F_ACCEPT_REQ_SIG);
    _port.dispatch(acceptReqEvent);
  }
  else
  {
    struct sockaddr_in clientAddress;
    socklen_t clAddrLen = sizeof(clientAddress);
    if (_pTCPServerSocket == 0)
    {
      GCFTCPPort* pPort = static_cast<GCFTCPPort*>(&_port);
      assert(pPort);
      _pTCPServerSocket = new GTMTCPSocket(*pPort);
    }
    if (_pTCPServerSocket->getFD() < 0)
      _pTCPServerSocket->setFD(::accept(_socketFD, 
                   (struct sockaddr*) &clientAddress, 
                   &clAddrLen));
         
    if (_pTCPServerSocket->getFD() >= 0)
    {
      GCFEvent connectedEvent(F_CONNECTED_SIG);
      _port.dispatch(connectedEvent);
    }
    // else ignore further connect requests
  }
}

int GTMTCPServerSocket::accept(GTMSocket& newSocket)
{
  int result(-2);
  if (_isProvider && _pTCPServerSocket == 0)
  {
    struct sockaddr_in clientAddress;
    socklen_t clAddrLen = sizeof(clientAddress);
    int newSocketFD;
    newSocketFD = ::accept(_socketFD, 
                       (struct sockaddr*) &clientAddress, 
                       &clAddrLen);
    
    result = newSocket.setFD(newSocketFD);
  }
  
  return result;
}

int GTMTCPServerSocket::close()
{
  int result(0);
  
  if (!_isProvider && _pTCPServerSocket != 0)
  {
    result = _pTCPServerSocket->close();
    delete _pTCPServerSocket;
    _pTCPServerSocket = 0;
  }
  if (result >= 0)
    result = GTMSocket::close();
    
  return result;
}

ssize_t GTMTCPServerSocket::send(void* buf, size_t count)
{
  if (!_isProvider && _pTCPServerSocket != 0) 
    return _pTCPServerSocket->send(buf, count);
  else
    return 0;
}

ssize_t GTMTCPServerSocket::recv(void* buf, size_t count)
{
  if (!_isProvider && _pTCPServerSocket != 0) 
    return _pTCPServerSocket->recv(buf, count);
  else
    return 0;
}

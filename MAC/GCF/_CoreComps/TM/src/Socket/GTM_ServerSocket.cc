//#  GTM_ServerSocket.cc: server socket implementation
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

#include "GTM_ServerSocket.h"
#include "GTM_SocketHandler.h"
#include <netinet/in.h>
#include <arpa/inet.h>

GTMServerSocket::GTMServerSocket(GCFRawPort& port, bool isProvider) : 
  GTMSocket(port),
  _isProvider(isProvider)
  _serverSocketFD(0)
{
}

int GTMServerSocket::open(GTMPeerAddr& addr)
{
  if (_socketFD == 0)
  {
    struct sockaddr_in address;
    int addrLen;
    
    setFD(socket(AF_INET, SOCK_STREAM, 0));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(addr.getPortnumber());
    addrLen = sizeof(address);
    bind(_socketFD, (struct sockaddr*)&address, addrLen);
    listen(_socketFD, 5);    
  }
}

void GTMServerSocket::workProc()
{
  if (_isProvider)
  {
    _port.dispatch(GCFEvent(F_ACCEPT_REQ_SIG));
  }
  else
  {
    if (_serverSocketFD == 0)
    {
      struct sockaddr_in clientAddress;
      int clAddrLen;
      _serverSocketFD = _socketFD;
      _socketFD = ::accept(_serverSocketFD, 
                         (struct sockaddr*) &clientAddress, 
                         &clAddrLen);
    }
  }
}

void GTMServerSocket::accept(GTMSocket& newSocket)
{
  if (_isProvider)
  {
    struct sockaddr_in clientAddress;
    int clAddrLen;
    int newSocketFD;
    newSocketFD = ::accept(_serverSocketFD, 
                       (struct sockaddr*) &clientAddress, 
                       &clAddrLen);
    newSocket.setFD(newSocketFD);                       
  }
}
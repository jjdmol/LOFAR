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

#include "GTM_TCPSocket.h"
#include "GTM_SocketHandler.h"
#include <GCF/GCF_TCPPort.h>
#include <GCF/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/GCF_TMProtocols.h>
#include <GCF/GCF_PeerAddr.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

GTMTCPSocket::GTMTCPSocket(GCFTCPPort& port) :
  GTMSocket(port)
{
}

GTMTCPSocket::~GTMTCPSocket()
{
  close();
}

ssize_t GTMTCPSocket::send(void* buf, size_t count)
{
  if (_socketFD > -1) 
    return ::write(_socketFD, buf, count);
  else
    return 0;
}

ssize_t GTMTCPSocket::recv(void* buf, size_t count)
{
  if (_socketFD > -1) 
    return ::read(_socketFD, buf, count);
  else
    return 0;
}

int GTMTCPSocket::open(GCFPeerAddr& /*addr*/)
{
  if (_socketFD > -1)
    return 0;
  else
  {
    _socketFD = ::socket(AF_INET, SOCK_STREAM, 0);
    return (_socketFD < 0 ? -1 : 0);
  }
}

int GTMTCPSocket::connect(GCFPeerAddr& serveraddr)  
{
  int result(-2);
  if (_socketFD >= -1)
  {
    struct sockaddr_in serverAddr;
    struct hostent *hostinfo;
    hostinfo = gethostbyname(serveraddr.getHost().c_str());
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
    serverAddr.sin_port = htons(serveraddr.getPortnumber());
    result = ::connect(_socketFD, 
              (struct sockaddr *)&serverAddr, 
              sizeof(struct sockaddr_in));
    if (result < 0)
      close();
    else
      GTMSocketHandler::instance()->registerSocket(*this);
  }
  return (result > -1 ? 0 : -1);
} 

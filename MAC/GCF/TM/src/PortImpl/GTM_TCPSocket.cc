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
#include "GTM_FileHandler.h"
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

GTMTCPSocket::GTMTCPSocket(GCFTCPPort& port) :
  GTMFile(port)
{
}

GTMTCPSocket::~GTMTCPSocket()
{
  close();
}

ssize_t GTMTCPSocket::send(void* buf, size_t count)
{
  if (_fd > -1) 
  {
    ssize_t countLeft(count);
    ssize_t written(0);
    do
    {
      written = ::write(_fd, ((char*)buf) + (count - countLeft), countLeft);
      if (written == -1)
      {
        if (errno != EINTR)
        {
          LOG_WARN(LOFAR::formatString (
              "send, error: %s",
              strerror(errno)));
          return -1;
        }
      }
      else
      {
        countLeft -= written;
      }      
    } while (countLeft > 0);
    
    return count;
  }
  else
  {
    LOG_WARN("send, error: Socket not opend");
    return -1;
  }
}

ssize_t GTMTCPSocket::recv(void* buf, size_t count)
{
  if (_fd > -1) 
  {
    ssize_t countLeft(count);
    ssize_t received(0);
    do
    {
      received = ::read(_fd, ((char*)buf) + (count - countLeft), countLeft);
      if (received == -1)
      {
        if (errno != EINTR)
        {
          LOG_WARN(formatString (
              "recv, error: %s",
              strerror(errno)));
          return -1;
        }
      }
      else
      {
        countLeft -= received;
      }      
    } while (countLeft > 0);
    
    return count;
  }
  else
  {
    LOG_WARN("recv, error: Socket not opend");
    return -1;
  }
}

bool GTMTCPSocket::open(unsigned int /*portNumber*/)
{
  assert(_fd == -1);
  _fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (_fd < 0)
  {
    LOG_WARN(formatString (
            "::socket, error: %s",
            strerror(errno)));
    close();
  }
  return (_fd > -1);
}

bool GTMTCPSocket::connect(unsigned int portNumber, const string& host)  
{
  bool result(false);
  if (_fd >= -1)
  {
    struct sockaddr_in serverAddr;
    struct hostent *hostinfo;
    hostinfo = gethostbyname(host.c_str());
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
    serverAddr.sin_port = htons(portNumber);
    result = (::connect(_fd, 
                        (struct sockaddr *)&serverAddr, 
                        sizeof(struct sockaddr_in)) 
               == 0);
    if (result)
    {
      assert(_pHandler);
      _pHandler->registerFile(*this);
    }
    else
    {
      LOG_WARN(formatString (
              "connect, error: %s",
              strerror(errno)));
      close();
    }
  }
  return result;
} 

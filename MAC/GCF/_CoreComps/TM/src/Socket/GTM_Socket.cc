//#  GTM_Socket.cc: base class for all sockets
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

#include "GTM_Socket.h"
#include "GTM_SocketHandler.h"
#include "GCF_TCPPort.h"
#include <GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF_TMProtocols.h>
#include <PortInterface/GCF_PeerAddr.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

GTMSocket::GTMSocket(GCFTCPPort& port) :
  _port(port), _socketFD(-1)
{
}

GTMSocket::~GTMSocket()
{
  close();
}

ssize_t GTMSocket::send(void* buf, size_t count)
{
  if (_socketFD > -1) 
    return ::write(_socketFD, buf, count);
  else
    return 0;
}

ssize_t GTMSocket::recv(void* buf, size_t count)
{
  if (_socketFD > -1) 
    return ::read(_socketFD, buf, count);
  else
    return 0;
}

int GTMSocket::close()
{
  int result(0);
  
  if (_socketFD > -1)
  { 
    GTMSocketHandler::instance()->deregisterSocket(*this);
    result = ::close(_socketFD);
    _socketFD = -1;
  }
  return result;
}

int GTMSocket::setFD(int fd)
{
  if (fd >= 0)
  {
    if (_socketFD > -1)
    {
      close();
    }
    _socketFD = fd;
    GTMSocketHandler::instance()->registerSocket(*this);
  }
  return (fd);    
}

int GTMSocket::open(GCFPeerAddr& /*addr*/)
{
  if (_socketFD > -1)
    return 0;
  else
  {
    _socketFD = ::socket(AF_INET, SOCK_STREAM, 0);
    return (_socketFD < 0 ? -1 : 0);
  }
}

int GTMSocket::connect(GCFPeerAddr& serveraddr)  
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

void GTMSocket::workProc()
{
  GCFEvent e(F_DISCONNECTED_SIG);
  GCFEvent::TResult status;
  
  ssize_t bytesRead = read(_socketFD, &e, sizeof(e));
  if (bytesRead == 0)
  {
    status = _port.dispatch(e);    
  }
  else if (bytesRead == sizeof(e))
  {
    status = eventReceived(e);
    if (status != GCFEvent::HANDLED)
    {
      LOFAR_LOG_INFO(TM_STDOUT_LOGGER, (
        "Event %s for task %s on port %s not handled or an error occured",
        _port.getTask()->evtstr(e),
        _port.getTask()->getName().c_str(), 
        _port.getName().c_str()
        ));
    }
  }
}

GCFEvent::TResult GTMSocket::eventReceived(const GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  char*   event_buf  = 0;
  GCFEvent* full_event = 0;

  event_buf = (char*)malloc(e.length);
  full_event = (GCFEvent*)event_buf;

  memcpy(event_buf, &e, sizeof(e));
  if (e.length - sizeof(e) > 0)
  {
    // recv the rest of the message (payload)
    ssize_t payloadLength = e.length - sizeof(e);
    
    ssize_t count = recv(event_buf + sizeof(e),
                          payloadLength);
    
    if (payloadLength != count)
    {
      LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, (
          "truncated recv on event %s (missing %d bytes)",
          _port.getTask()->evtstr(e),
          payloadLength - count
          ));
    }
    if (payloadLength != count) // retry to read the rest
    {
      //TODO: Make this retry more secure
      usleep(10);
      count += recv(event_buf + sizeof(e) + count ,
                           payloadLength - count);
      assert(payloadLength == count);
    }
  }

  status = _port.dispatch(*full_event);

  free(event_buf);
  return status;
}

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
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>


GTMSocket::GTMSocket(GCFRawPort& port) :
  _socketFD(-1),
  _port(port),
  _pHandler(0)
{
  _pHandler = GTMSocketHandler::instance();
  assert(_pHandler);
}

GTMSocket::~GTMSocket()
{
  close();
  GTMSocketHandler::release();
  _pHandler = 0;
}

int GTMSocket::close()
{
  int result(0);
  
  if (_socketFD > -1)
  { 
    assert(_pHandler);
    _pHandler->deregisterSocket(*this);
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
    assert(_pHandler);
    _pHandler->registerSocket(*this);
  }
  return (fd);    
}

void GTMSocket::workProc()
{
  unsigned long bytesRead = 0;
  
  if (ioctl(_socketFD, FIONREAD, &bytesRead) > -1)
  {
    if (bytesRead == 0)
    {
      GCFEvent e(F_DISCONNECTED);
      _port.dispatch(e);    
    }
    else 
    {
      GCFEvent e(F_DATAIN);
      _port.dispatch(e);
    }
  }
  else
  {
    assert(_port.getTask());
    LOG_FATAL(LOFAR::formatString (
        "%s(%s): Error in 'ioctl' on socket fd %d: %s",
        _port.getTask()->getName().c_str(), 
        _port.getName().c_str(), 
        _socketFD, 
        strerror(errno)));        
  }
}

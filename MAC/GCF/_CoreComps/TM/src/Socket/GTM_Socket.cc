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

GTMSocket::GTMSocket(GCFRawPort& port) :
  _port(port), _socketFD(0)
{
}

ssize_t GTMSocket::send(void* buf, size_t count)
{
  if (_socketFD != 0) 
    return ::write(_socketFD, buf, count);
  else
    return 0;
}

ssize_t GTMSocket::recv(void* buf, size_t count)
{
  if (_socketFD != 0) 
    return ::read(_socketFD, buf, count);
  else
    return 0;
}

int GTMSocket::close()
{
  int result(0);
  
  if (_socketFD != 0)
  { 
    result = ::close(_socketFD);
    _socketFD = 0;
  }
  return result;
}

void GTMSocket::setFD(int fd)
{
  if (_socketFD != 0)
  {
    GTMSocketHandler::instance()->deregisterSocket(*this);
    GTMSocket::close();
  }
  _socketFD = fd;
  GTMSocketHandler::instance()->registerSocket(*this);
}

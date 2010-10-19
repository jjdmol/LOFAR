//#  GTM_SocketHandler.cc: the specific handler for socket message production
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

#include "GTM_SocketHandler.h"
#include "GTM_Socket.h"
#include <GCF/GCF_Task.h>

GTMSocketHandler* GTMSocketHandler::_pInstance = 0;

GTMSocketHandler* GTMSocketHandler::instance()
{
  if (0 == _pInstance)
  {
    _pInstance = new GTMSocketHandler();
  }

  return _pInstance;
}

GTMSocketHandler::GTMSocketHandler() : _running(true)
{
  FD_ZERO(&_readFDs);  
  GCFTask::registerHandler(*this);
}

void GTMSocketHandler::registerSocket(GTMSocket& socket)
{
  FD_SET(socket.getFD(), &_readFDs);
  _sockets[socket.getFD()] = &socket;
}

void GTMSocketHandler::deregisterSocket(GTMSocket& socket)
{
  FD_CLR(socket.getFD(), &_readFDs);
  _sockets.erase(socket.getFD());  
}

void GTMSocketHandler::workProc()
{
  int result;
  int fd;
  map<int, GTMSocket*> testSockets;

  struct timeval select_timeout;

  //
  // because select call changes the timeout value to
  // contain the remaining time we need to set it to 10ms
  // on every call to workProc
  // 
  select_timeout.tv_sec  = 0;
  select_timeout.tv_usec = 10000;
    
  _running = true;
  fd_set testFDs;
  testFDs = _readFDs;
  testSockets.insert(_sockets.begin(), _sockets.end());
  result = ::select(FD_SETSIZE, &testFDs, (fd_set *) 0, (fd_set *) 0, &select_timeout);

  if (_sockets.empty()) return;
  
  if (result >= 0)
  {
    for (fd = 0; fd < FD_SETSIZE && _running; fd++)
    {
      if (FD_ISSET(fd, &testFDs))
      {
        testSockets[fd]->workProc();
      }
    }
  }
}

void GTMSocketHandler::stop()
{
  _running = false;
}

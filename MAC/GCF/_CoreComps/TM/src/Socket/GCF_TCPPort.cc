//#  GCF_TCPPort.cc: connection to a remote process
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


#include "GCF_TCPPort.h"
#include <GCF_Task.h>
#include <GCF_TMProtocols.h>
#include <PortInterface/GTM_NameService.h>
#include <PortInterface/GTM_TopologyService.h>
#include <Socket/GTM_ClientSocket.h>
#include <Socket/GTM_ServerSocket.h>

GCFTCPPort::GCFTCPPort(GCFTask& task, string name, TPortType type, int protocol) 
  : GCFRawPort(task, name, type, protocol),
    _addrIsSet(false),
    _pSocket(0)
{
  if (SPP == getType() || MSPP == getType())
  {
    _pSocket = new GTMServerSocket(*this, (MSPP == type));
  }
  else if (SAP == getType())
  {
    _pSocket = new GTMClientSocket(*this);
  }
}

GCFTCPPort::GCFTCPPort()
    : GCFRawPort(),
    _addrIsSet(false),
    _pSocket(0)
{
}

GCFTCPPort::~GCFTCPPort()
{
}


int GCFTCPPort::open()
{
  GCFPeerAddr fwaddr;
  int result(0);

  if (isConnected())
  {
    LOG_ERROR(( "ERROR: Port %s already open.\n",
	       getName()));
  }
  else if (!_pSocket)
  {
    LOG_ERROR(( "ERROR: Port %s not initialised.\n",
         getName()));
  }
  else if (!_addrIsSet && !isSlave())
  {
    if (findAddr(fwaddr))
    {
      setAddr(fwaddr);
      result = 1;
    }
    else
    {
      LOG_ERROR(("Could not get address info for port '%s' of task '%s'\n",
		     _name, _pTask->getName()));
    }
  }
  if (!result) return result;
  
  if (_pSocket->open(_addr) < 0)
  {
    _isConnected = false;
    if (SAP == getType())
    {
      schedule_disconnected();
    }    
  }
  else
  {    
    schedule_connected();
    result = 1;
  }
  return result;
}

ssize_t GCFTCPPort::send(const GCFEvent& e, void* buf, size_t count)
{
  size_t written = 0;
  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port

  iovec* newbufs(0);
  if (F_RAW_SIG != e.signal)
  { 
    iovec buffers[2];
    buffers[0].iov_base = (void*)&e;
    buffers[0].iov_len = e.length - count;
    buffers[1].iov_base  = buf;
    buffers[1].iov_len = count;
    newbufs = buffers;
    sendv(GCFEvent(F_RAW_SIG), buffers, (count > 0 ? 2 : 1));
  }
  else
  {
    if ((written = _pSocket->send(buf, count)) != count)
    {
      cerr << "GCFTCPPort::sendv(data)\n";
      LOG_DEBUG(("truncated send\n"));
    }
  }

  return written;
}

ssize_t GCFTCPPort::sendv(const GCFEvent& e, const iovec buffers[], int n)
{
  size_t written = 0;
  size_t count = 0;
  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port
  
  if (F_RAW_SIG != e.signal)
  { 
    count = e.length;
    if ((written = _pSocket->send((void*)&e, e.length)) != count)
    {
      cerr << "GCFTCPPort::sendv(data)\n";
      LOG_DEBUG(("truncated send\n"));
    }
  }

  for (int i = 0; i < n; i++)
  {
    count += buffers[i].iov_len;
    if ((written += _pSocket->send(buffers[i].iov_base, buffers[i].iov_len)) != count)
    {
      cerr << "GCFTCPPort::sendv(data)\n";
      LOG_DEBUG(("truncated send\n"));
    }
  }

  return written;
}

ssize_t GCFTCPPort::recv(void* buf, size_t count)
{
  return _pSocket->recv(buf, count);
}

ssize_t GCFTCPPort::recvv(iovec buffers[], int n)
{
  return 0;//_pSocket->recvv_n(buffers, n);
}

int GCFTCPPort::close()
{
  if (isConnected()) 
  {
    _pSocket->close();
    schedule_close();
  }
  // return success when port is still connected
  // scheduled close will only occur later
  return (isConnected() ? 0 : -1);
}

void GCFTCPPort::setAddr(const GCFPeerAddr& addr)
{
  _addr = addr;
  _addrIsSet = true;
}

//# GCF_ETHRawPort.cc: connection to a remote process
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include "GCF_ETHRawPort.h"
#include <GCF/GCF_Port.h>
#include <GCF/GCF_RawPort.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_TMProtocols.h>
#include <GTM_Defines.h>
#include <PortInterface/GTM_NameService.h>
#include <PortInterface/GTM_TopologyService.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>

GCFETHRawPort::GCFETHRawPort(GCFTask& task,
			 string name,
			 TPortType type) 
    : GCFRawPort(task, name, type, 0), _ifname(0)
{
}

GCFETHRawPort::GCFETHRawPort()
    : GCFRawPort(), _ifname(0)
{
}

GCFETHRawPort::~GCFETHRawPort()
{
  if (_ifname) free(_ifname);
}

int GCFETHRawPort::close()
{
  LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, ( 
      "close -> schedule_close"));

  schedule_close();

  return 0;
}

int GCFETHRawPort::open()
{
  int retval = 0;
  char destMac[ETH_ALEN];

  if (isConnected())
  {
    LOFAR_LOG_WARN(TM_STDOUT_LOGGER, ( 
        "already connected"));
   
    return -1;
  }

  // check for if name
  if (0 == _ifname)
  {
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, ( 
        "no interface name specified"));
    return -1;
  }

  if (open())
  {
  }
  return retval; // RETURN
}

ssize_t GCFETHRawPort::send(const GCFEvent& e, void* buf, size_t count)
{
  size_t written = 0;

  assert(_pSocket);

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
      LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
          "truncated send: %s",
          strerror(errno)));
          
      schedule_disconnected();
      
      written = -1;
    }
  }

  return written;
}

ssize_t GCFETHRawPort::sendv(const GCFEvent& e, const iovec buffers[], int n)
{
  size_t written = 0;
  size_t count = 0;
  assert(_pSocket);
  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port
  
  if (F_RAW_SIG != e.signal)
  { 
    count = e.length;
    if ((written = _pSocket->send((void*)&e, e.length)) != count)
    {
      LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
          "truncated send: %s",
          strerror(errno)));
    }
  }

  for (int i = 0; i < n; i++)
  {
    count += buffers[i].iov_len;
    if ((written += _pSocket->send(buffers[i].iov_base, buffers[i].iov_len)) != count)
    {
      LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
          "truncated send: %s",
          strerror(errno)));
    }
  }

  return written;
}

ssize_t GCFETHRawPort::recv(void* buf, size_t count)
{
  assert(_pSocket);
  return _pSocket->recv(buf, count);
}

ssize_t GCFETHRawPort::recvv(iovec /*buffers*/[], int /*n*/)
{
  LOFAR_LOG_FATAL(TM_STDOUT_LOGGER, ( 
     "recvv: not implemented"));
  return -1;
}

void GCFETHRawPort::setAddr(const char* ifname,
			  const char* destMac)
{
  // store for use in open
  _ifname       = strdup(ifname);
  _destMacStr = strdup(destMac);
}

void GCFETHRawPort::convertCcp2sllAddr(const char* destMacStr,
				       char destMac[ETH_ALEN])
{
  unsigned int hx[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  sscanf(destMacStr, "%x:%x:%x:%x:%x:%x",
	 &hx[0], &hx[1], &hx[2], &hx[3], &hx[4], &hx[5]);
	 
  for (int i = 0; i < ETH_ALEN; i++)
  {
      destMac[i] = (char)hx[i];
  }
}

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
#include "GTM_ETHSocket.h"
#include <GTM_Defines.h>
#include <errno.h>


GCFETHRawPort::GCFETHRawPort(GCFTask& task,
                          	 string name,
                          	 TPortType type, 
                             bool transportRawData) : 
   GCFRawPort(task, name, type, 0, transportRawData), 
   _pSocket(0), _ethertype(0x0000)
{
  assert(MSPP != getType());

  _pSocket = new GTMETHSocket(*this);
}

GCFETHRawPort::GCFETHRawPort() : 
  GCFRawPort(),       
  _pSocket(0), _ethertype(0x0000)
{
  assert(MSPP != getType());
}

GCFETHRawPort::~GCFETHRawPort()
{
  if (_pSocket) delete _pSocket;
  
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

  if (isConnected())
  {
    LOFAR_LOG_WARN(TM_STDOUT_LOGGER, ( 
        "already connected"));
   
    return -1;
  }

  // check for if name
  if (_ifname == "")
  {
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, ( 
        "no interface name specified"));
    return -1;
  }

  if (!_pSocket && isSlave())
  {
    LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
        "ERROR: Port %s is not initialised.",
        _name.c_str()));
    return -1;
  } 
  else if (!_pSocket && !isSlave())
  {
    _pSocket = new GTMETHSocket(*this);
  }
   
  if (_pSocket->open(_ifname.c_str(), _destMacStr.c_str(), _ethertype) < 0)
  {
    _isConnected = false;
    if (SAP == getType())
    {
      schedule_disconnected();
    }
    else
    {
      retval = 0;
    }
  }
  else
  { 
    schedule_connected();
  }
  return retval;
}

ssize_t GCFETHRawPort::send(GCFEvent& e)
{
  size_t written = 0;

  assert(_pSocket);

  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port

  unsigned int packsize;
  void* buf = e.pack(packsize);

  if ((written = _pSocket->send(buf, packsize)) != packsize)
  {
    LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
        "truncated send: %s",
        strerror(errno)));
        
    schedule_disconnected();
    
    written = 0;
  }

  return written;
}

ssize_t GCFETHRawPort::recv(void* buf, size_t count)
{
  return _pSocket->recv(buf, count);
}

void GCFETHRawPort::setAddr(const char* ifname,
			    const char* destMac)
{
  // store for use in open
  _ifname     = string(ifname);
  _destMacStr = string(destMac);
}

void GCFETHRawPort::setEtherType(unsigned short type)
{
  _ethertype = type;
}


//#  GCF_DevicePort.cc: connection to a remote process
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


#include <GCF/TM/GCF_DevicePort.h>
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <PortInterface/GTM_NameService.h>
#include <PortInterface/GTM_TopologyService.h>
#include <Socket/GTM_Device.h>
#include <errno.h>

GCFDevicePort::GCFDevicePort(GCFTask& task, 
                       string name, 
                       TPortType type, 
                       int protocol, 
                       bool transportRawData) 
  : GCFRawPort(task, name, type, protocol, transportRawData),
    _addrIsSet(false),
    _pDevice(0)
{
    _pDevice = new GTMDevice(*this);
}

GCFDevicePort::GCFDevicePort()
    : GCFRawPort(),
    _addrIsSet(false),
    _pDevice(0)
{
}

GCFDevicePort::~GCFDevicePort()
{
  if (_pDevice)
    delete _pDevice;
}


int GCFDevicePort::open()
{
  GCFPeerAddr fwaddr;
  int result(1);

  if (isConnected())
  {
    LOG_ERROR(LOFAR::formatString ( 
        "ERROR: Port %s already open.",
	      _name.c_str()));
    result = 0;
  }
  else if (!_pDevice && isSlave())
  {
    LOG_ERROR(LOFAR::formatString ( 
        "ERROR: Port %s not initialised.",
        _name.c_str()));
    result = 0;
  }
  else if (!_addrIsSet && !isSlave())
  {
    _pDevice = new GTMDevice(*this);
    
    if (findAddr(fwaddr))
    {
      setAddr(fwaddr);
    }
    else
    {
      LOG_ERROR(LOFAR::formatString (
          "Could not get address info for port '%s' of task '%s'",
		      _name.c_str(), _pTask->getName().c_str()));
      result = 0;
    }
  }
  if (result == 0) return result;
  
  if (_pDevice->open(_addr) < 0)
  {
    _isConnected = false;
    if (SAP == getType())
    {
      schedule_disconnected();
    }
    else
    {
      result = 0;
    }
  }
  else
  { 
    schedule_connected();
  }
  return result;
}

ssize_t GCFDevicePort::send(GCFEvent& e)
{
  size_t written = 0;

  assert(_pDevice);

  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port

 
  unsigned int packsize;
  void* buf = e.pack(packsize);

  if (!isSlave())
  {
    LOG_DEBUG(LOFAR::formatString (
      "Sending event '%s' for task %s on port '%s'",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getName().c_str()));
  }
  if ((written = _pDevice->send(buf, packsize)) != packsize)
  {
    LOG_DEBUG(LOFAR::formatString (
        "truncated send: %s",
        strerror(errno)));
        
    schedule_disconnected();
    
    written = 0;
  }
 
  return written;
}

ssize_t GCFDevicePort::recv(void* buf, size_t count)
{
  assert(_pDevice);
  return _pDevice->recv(buf, count);
}

int GCFDevicePort::close()
{
  _pDevice->close();
  schedule_close();

  // return success when port is still connected
  // scheduled close will only occur later
  return (isConnected() ? 0 : -1);
}

void GCFDevicePort::setAddr(const GCFPeerAddr& addr)
{
  _addr = addr;
  _addrIsSet = true;
}

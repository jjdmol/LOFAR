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
#include <Socket/GTM_Device.h>
#include <GCF/ParameterSet.h>
#include <errno.h>

using namespace LOFAR;
using namespace GCF;

GCFDevicePort::GCFDevicePort(GCFTask& task, 
                       string name, 
                       TPortType type, 
                       int protocol,
                       const string& deviceName, 
                       bool transportRawData) 
  : GCFRawPort(task, name, type, protocol, transportRawData),
    _devNameIsSet(false),
    _pDevice(0),
    _deviceName(deviceName)
{
  _pDevice = new GTMDevice(*this);
}

GCFDevicePort::GCFDevicePort(const string& deviceName)
    : GCFRawPort(),
    _devNameIsSet(false),
    _pDevice(0),
    _deviceName(deviceName)
{
}

GCFDevicePort::GCFDevicePort()
    : GCFRawPort(),
    _devNameIsSet(false),
    _pDevice(0),
    _deviceName("")
{
}

GCFDevicePort::~GCFDevicePort()
{
  if (_pDevice)
    delete _pDevice;
}


bool GCFDevicePort::open()
{
  if (isConnected())
  {
    LOG_ERROR(LOFAR::formatString ( 
        "ERROR: Port %s already open.",
	      getRealName().c_str()));
    return false;
  }
  else if (!_pDevice)
  {
    if (isSlave())
    {
      LOG_ERROR(LOFAR::formatString ( 
          "ERROR: Port %s not initialised.",
          getRealName().c_str()));
      return false;
    }
    else
    {
      _pDevice = new GTMDevice(*this);
    }
  }

  try
  {
    setDeviceName(ParameterSet::instance()->getString(formatString(
      "mac.ns.%s.%s.deviceName",
      _pTask->getName().c_str(),
      getRealName().c_str())));    
  }
  catch (...)
  {
    if (!_devNameIsSet)
    {
      LOG_ERROR(LOFAR::formatString (
          "Could not get address info for port '%s' of task '%s'",
         getRealName().c_str(), _pTask->getName().c_str()));
      return false;
    }
  }

  if (_pDevice->open(_deviceName) < 0)
  {
    setState(S_DISCONNECTING);
    if (SAP == getType())
    {
      schedule_disconnected();
    }
    else
    {
      return false;
    }
  }
  else
  { 
    setState(S_CONNECTING);
    schedule_connected();
  }
  return true;
}

ssize_t GCFDevicePort::send(GCFEvent& e)
{
  size_t written = 0;

  assert(_pDevice);

  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port

 
  unsigned int packsize;
  void* buf = e.pack(packsize);

  LOG_DEBUG(formatString (
      "Sending event '%s' for task '%s' on port '%s'",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getRealName().c_str()));

  if ((written = _pDevice->send(buf, packsize)) != packsize)
  {
    LOG_DEBUG(LOFAR::formatString (
        "truncated send: %s",
        strerror(errno)));
      
    setState(S_DISCONNECTING);    
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

bool GCFDevicePort::close()
{
  _pDevice->close();
  setState(S_CLOSING);
  schedule_close();

  // return success when port is still connected
  // scheduled close will only occur later
  return isConnected();
}

void GCFDevicePort::setDeviceName(const string& deviceName)
{
  _deviceName = deviceName;
  _devNameIsSet = true;
}

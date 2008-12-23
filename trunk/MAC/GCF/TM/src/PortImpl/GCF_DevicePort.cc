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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/TM/GCF_DevicePort.h>
#include "GTM_Device.h"
#include <GTM_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <Common/ParameterSet.h>
#include <errno.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

GCFDevicePort::GCFDevicePort(GCFTask& 		task, 
							 const string&	name,                        
							 int 			protocol,
							 const string& 	deviceName, 
							 bool 			transportRawData) 
  : GCFRawPort(task, name, SAP, protocol, transportRawData),
    _devNameIsSet((deviceName.length() > 0)),
    _pDevice(0),
    _deviceName(deviceName)
{
  _pDevice = new GTMDevice(*this);
}

GCFDevicePort::GCFDevicePort(const string& deviceName)
    : GCFRawPort(),
    _devNameIsSet((deviceName.length() > 0)),
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
    LOG_ERROR(formatString ( 
        "Port %s already open.",
	      getRealName().c_str()));
    return false;
  }
  else if (SAP != getType())
  {
    LOG_ERROR(formatString ( 
        "Device ports only can act as a SAP (%s).",
        getRealName().c_str()));
    return false;
  }
  else if (!_pDevice)
  {
    if (isSlave())
    {
      LOG_ERROR(formatString ( 
          "Port %s not initialised.",
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
    if (!_devNameIsSet)
    {
      // retrieve the device name from the parameter set
      setDeviceName(
          globalParameterSet()->getString(
              formatString(
                  "mac.ns.%s.%s.deviceName",
                  _pTask->getName().c_str(),
                  getRealName().c_str())
              )
          );    
    }
  }
  catch (...)
  {
    if (!_devNameIsSet)
    {
      LOG_ERROR(formatString (
          "Could not get address info for port '%s' of task '%s'",
          getRealName().c_str(), _pTask->getName().c_str()));
      return false;
    }
  }

  if (_pDevice->open(_deviceName))
  { 
    setState(S_CONNECTING);
    schedule_connected();
  }
  else
  {
    setState(S_DISCONNECTING);
    schedule_disconnected();
  }
  return true;
}

ssize_t GCFDevicePort::send(GCFEvent& e)
{
  size_t written = 0;
  
  if (!isConnected()) 
  {
    LOG_ERROR(formatString (
        "Port '%s' on task '%s' not connected! Event not sent!",
        getRealName().c_str(),
        getTask()->getName().c_str()));
    return 0;
  }
  
  ASSERT(_pDevice);

  unsigned int packsize;
  void* buf = e.pack(packsize);

  LOG_DEBUG(formatString (
      "Sending event '%s' for task '%s' on port '%s'",
//      getTask()->eventName(e).c_str(),
      eventName(e).c_str(),
      getTask()->getName().c_str(), 
      getRealName().c_str()));

  if ((written = _pDevice->send(buf, packsize)) != packsize)
  {
    LOG_DEBUG(formatString (
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
  if (!isConnected()) return 0;
  ASSERT(_pDevice);
  return _pDevice->recv(buf, count);
}

bool GCFDevicePort::close()
{
  _pDevice->close();
  setState(S_CLOSING);
  schedule_close();

  // return success when port is still connected
  // scheduled close event will occur after the context switch
  return isConnected();
}

void GCFDevicePort::setDeviceName(const string& deviceName)
{
  _deviceName = deviceName;
  _devNameIsSet = true;
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

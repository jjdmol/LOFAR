//#  GCF_PVSSPort.cc: connection to a remote process
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


#include <GCF/PAL/GCF_PVSSPort.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GSA_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>

#include <GSA_PortService.h>

static GCFEvent disconnectedEvent(F_DISCONNECTED);
static GCFEvent connectedEvent   (F_CONNECTED);
static GCFEvent closedEvent      (F_CLOSED);

GCFPVSSPort::GCFPVSSPort(GCFTask& task, 
                       string name, 
                       TPortType type, 
                       int protocol, 
                       bool transportRawData) 
  : GCFRawPort(task, name, type, protocol, transportRawData),
    _pPortService(0)
{
  _pPortService = new GSAPortService(*this);
}

GCFPVSSPort::GCFPVSSPort()
    : GCFRawPort(),
    _pPortService(0)
{
}

GCFPVSSPort::~GCFPVSSPort()
{
  if (_pPortService)
  {
    _pPortService->stop();
    delete _pPortService;
    _pPortService = 0;    
  }
}


bool GCFPVSSPort::open()
{
  if (getState() != S_DISCONNECTED)
  {
    LOG_ERROR(formatString ( 
        "ERROR: Port %s already open.",
	      _name.c_str()));
    return false;
  }
  else if (!_pPortService)
  {
    if (isSlave())
    {
      LOG_ERROR(formatString ( 
          "ERROR: Port %s not initialised.",
          _name.c_str()));
      return false;
    }
    else
    {
      _pPortService = new GSAPortService(*this);
    }
  }
  
  setState(S_CONNECTING);
  _pPortService->start();
  return true;
}

void GCFPVSSPort::serviceStarted(bool successfull)
{
  if (successfull)
    schedule_connected();
  else
    schedule_disconnected();
}

ssize_t GCFPVSSPort::send(GCFEvent& e)
{
  ssize_t written = 0;

  assert(_pPortService);

  if (MSPP == getType())  
    return -1; // no messages can be send by this type of port

 
  unsigned int packsize;
  void* buf = e.pack(packsize);

  LOG_DEBUG(formatString (
      "Sending event '%s' for task '%s' on port '%s'",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getRealName().c_str()));
      
  if ((written = _pPortService->send(buf, packsize, _destDpName)) != (ssize_t) packsize)
  {  
    setState(S_DISCONNECTING);     
    schedule_disconnected();
    
    written = -1;
  }
 
  return written;
}

ssize_t GCFPVSSPort::recv(void* buf, size_t count)
{
  assert(_pPortService);
  return _pPortService->recv(buf, count);
}

bool GCFPVSSPort::close()
{
  setState(S_CLOSING);  
  assert(_pPortService);
  _pPortService->stop();
  schedule_close();

  // return success when port is still connected
  // scheduled close will only occur later
  return isConnected();
}

void GCFPVSSPort::setDestAddr(const string& destDpName)
{
  _destDpName = destDpName;
}

const string GCFPVSSPort::getOwnAddr() const 
{
  if (isConnected())
  {
    assert(_pPortService);
    return GCFPVSSInfo::getLocalSystemName() + ":" + getRealName();
  }
  else
  {
    return "";
  }
}

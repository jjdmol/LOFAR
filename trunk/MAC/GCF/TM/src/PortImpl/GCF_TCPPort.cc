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


#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/ParameterSet.h>
#include "GTM_TCPServerSocket.h"
#include <ServiceBroker/GTM_ServiceBroker.h>
#include <ServiceBroker/GSB_Defines.h>
#include <GTM_Defines.h>
#include <errno.h>

GCFTCPPort::GCFTCPPort(GCFTask& task, 
                       string name, 
                       TPortType type, 
                       int protocol, 
                       bool transportRawData) 
  : GCFRawPort(task, name, type, protocol, transportRawData),
    _pSocket(0),
    _addrIsSet(false),
    _portNumber(0),
    _broker(0)
{
  if (SPP == getType() || MSPP == getType())
  {
    _pSocket = new GTMTCPServerSocket(*this, (MSPP == type));
  }
  else if (SAP == getType())
  {
    _pSocket = new GTMTCPSocket(*this);
  }  
}

GCFTCPPort::GCFTCPPort()
    : GCFRawPort(),
    _pSocket(0),
    _addrIsSet(false),
    _portNumber(0),
    _broker(0)
{
}

GCFTCPPort::~GCFTCPPort()
{
  if (_pSocket)
  {
    delete _pSocket;
    _pSocket = 0;    
  }

  if (_broker)
  {
    _broker->deletePort(*this);
    GTMServiceBroker::release();
    _broker = 0;
  }
}

bool GCFTCPPort::open()
{
  if (isConnected())
  {
    LOG_ERROR(formatString ( 
        "Port %s already open.",
	      getRealName().c_str()));
    return false;
  }
  else if (!_pSocket)
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
      if (SPP == getType() || MSPP == getType())
      {
        _pSocket = new GTMTCPServerSocket(*this, (MSPP == getType()));
      }
      else if (SAP == getType())
      {
        _pSocket = new GTMTCPSocket(*this);
      }
    }
  }
  
  setState(S_CONNECTING);
  if (!_broker)
  {
    _broker = GTMServiceBroker::instance();
  }
  if (SAP == getType()) 
  {
    TPeerAddr fwaddr;
    if (findAddr(fwaddr))
    {
      setAddr(fwaddr);
    }
    else
    {
      if (!_addrIsSet)
      {
        LOG_ERROR(formatString (
            "No remote address info is set for port '%s' of task '%s'.",
            getRealName().c_str(), _pTask->getName().c_str()));
        LOG_INFO("See the last log of ParameterSet.cc file or use setAddr method.");
        setState(S_DISCONNECTED);
        return false;
      }
    }
    string portNumParam = formatString(PARAM_TCP_PORTNR, 
        _addr.taskname.c_str(),
        _addr.portname.c_str());
    if (ParameterSet::instance()->isDefined(portNumParam))
    {
      _portNumber = ParameterSet::instance()->getInt(portNumParam);
      string hostParam = formatString(PARAM_TCP_HOST, 
          _addr.taskname.c_str(),
          _addr.portname.c_str());
      if (ParameterSet::instance()->isDefined(hostParam))
      {
        _host = ParameterSet::instance()->getString(hostParam);
      }
      else
      {
        _host = "localhost";
      }
    }
    if (_host != "" && _portNumber > 0)
    {
      serviceInfo(SB_NO_ERROR, _portNumber, _host);
    }      
    else
    {
      string remoteServiceName = formatString("%s:%s", 
          _addr.taskname.c_str(), 
          _addr.portname.c_str());
      assert(_broker);
      _broker->getServiceinfo(*this, remoteServiceName);
    }
  }
  else 
  {
    string portNumParam = formatString(PARAM_TCP_PORTNR, 
        getTask()->getName().c_str(),
        getRealName().c_str());
    if (ParameterSet::instance()->isDefined(portNumParam))
    {
      _portNumber = ParameterSet::instance()->getInt(portNumParam);
    }
    if (_portNumber > 0)
    {
      serviceRegistered(SB_NO_ERROR, _portNumber);
    }
    else
    {
      assert(_broker);
      _broker->registerService(*this);
    }
  }
  return true;
}

void GCFTCPPort::serviceRegistered(unsigned int result, unsigned int portNumber)
{
  assert(MSPP == getType() || SPP == getType());
  if (result == SB_NO_ERROR)
  {
    LOG_DEBUG(formatString (
        "(M)SPP port '%s' in task '%s' listens on portnumber %d.",
        getRealName().c_str(),
        _pTask->getName().c_str(),
        portNumber));
    _portNumber = portNumber;
    if (!_pSocket->open(portNumber))
    {
      schedule_disconnected();
    }
    else if (MSPP == getType())
    {
      schedule_connected();
    }
  }
  else
  {
    schedule_disconnected();
  }
}

void GCFTCPPort::serviceInfo(unsigned int result, unsigned int portNumber, const string& host)
{
  assert(SAP == getType());
  if (result == SB_UNKNOWN_SERVICE)
  {
    LOG_DEBUG(formatString (
        "Cannot connect the local SAP [%s:%s] to remote (M)SPP [%s:%s]. Try again!!!",
        _pTask->getName().c_str(),
        getRealName().c_str(),
        _addr.taskname.c_str(),
        _addr.portname.c_str()));
        
    schedule_disconnected();
  }
  else if (result == SB_NO_ERROR)
  {
    _portNumber = portNumber;
    _host = host;
    LOG_DEBUG(formatString (
        "Can now connect the local SAP [%s:%s] to remote (M)SPP [%s:%s@%s:%d].",
        _pTask->getName().c_str(),
        getRealName().c_str(),
        _addr.taskname.c_str(),
        _addr.portname.c_str(),
        host.c_str(),
        portNumber));
        
    if (_pSocket->open(portNumber))
    { 
      if (_pSocket->connect(portNumber, host))
      {
        schedule_connected();
      }
      else
      {
        schedule_disconnected();
      }
    }
    else
    {
      schedule_disconnected();
    }
  }
}

void GCFTCPPort::serviceGone()
{
  _host = "";
  _portNumber = 0;
}

ssize_t GCFTCPPort::send(GCFEvent& e)
{
  ssize_t written = 0;

  assert(_pSocket);

  if (!isConnected()) 
  {
    LOG_ERROR(formatString (
        "Port '%s' on task '%s' not connected! Event not sent!",
        getRealName().c_str(),
        getTask()->getName().c_str()));
    return 0;
  }

  if (MSPP == getType())  
    return 0; // no messages can be send by this type of port

 
  unsigned int packsize;
  void* buf = e.pack(packsize);

  LOG_DEBUG(formatString (
      "Sending event '%s' for task '%s' on port '%s'",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getRealName().c_str()));
      
  if ((written = _pSocket->send(buf, packsize)) != (ssize_t) packsize)
  {  
    setState(S_DISCONNECTING);     
    schedule_disconnected();
    
    written = 0;
  }
 
  return written;
}

ssize_t GCFTCPPort::recv(void* buf, size_t count)
{
  assert(_pSocket);
  return _pSocket->recv(buf, count);
}

bool GCFTCPPort::close()
{
  setState(S_CLOSING);  
  _pSocket->close();
  schedule_close();

  // return success when port is still connected
  // scheduled close will only occur later
  return isConnected();
}

void GCFTCPPort::setAddr(const TPeerAddr& addr)
{
  if (_addr.taskname != addr.taskname || _addr.portname != addr.portname)
  {
    _host = "";
    _portNumber = 0;
  }
  _addr = addr;
  if (_addr.taskname != "" && _addr.portname != "")
  {
    _addrIsSet = true;
  }
}

bool GCFTCPPort::accept(GCFTCPPort& port)
{
  bool result(false);
  if (MSPP == getType() && SPP == port.getType())
  {
    GTMTCPServerSocket* pProvider = (GTMTCPServerSocket*)_pSocket;
    if (port._pSocket == 0)
    {
      port._pSocket = new GTMTCPSocket(port);
    }
    if (pProvider->accept(*port._pSocket))
    {
      setState(S_CONNECTING);        
      port.schedule_connected();
      result = true;
    }
  }
  return result;
}

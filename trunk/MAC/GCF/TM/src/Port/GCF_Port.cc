//#  GCF_Port.cc: connection to a remote process
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

#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/ParameterSet.h>
#include <GTM_Defines.h>

#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

GCFPort::GCFPort(GCFTask& task, 
                 string& name, 
                 TPortType type, 
                 int protocol, 
                 bool transportRawData) : 
    GCFPortInterface(&task, name, type, protocol, transportRawData), _pSlave(0)
{
}

GCFPort::GCFPort() :
    GCFPortInterface(0, "", SAP, 0, false), _pSlave(0)
{
}

GCFPort::~GCFPort()
{
  //
  // if the open method on the port has been called, there will be
  // a slave port which needs to be cleaned up.
  //
  if (_pSlave) delete _pSlave;
  _pSlave = 0;
}

void GCFPort::init(GCFTask& task,
		 string name,
		 TPortType type,
		 int protocol, 
     bool transportRawData)
{
  GCFPortInterface::init(task, name, type, protocol, transportRawData);
  if (_pSlave) delete _pSlave;
  _pSlave = 0;
}

bool GCFPort::open()
{
  if (_state != S_DISCONNECTED && _state != S_CONNECTED)
  {
    return false;
  }
  if (MSPP == getType())
  {
    LOG_ERROR("Ports of type MSPP should not be initialised directly via this common port");
    return false;
  }
  _state = S_CONNECTING;
  
  //
  // If the port has been openend before then the _pSlave port has already
  // been connected, and we don't need to connect it again. We can simply
  // call the ::open() method on the slave and return.
  //
  if (_pSlave) return _pSlave->open();
  
  //
  // This is the first call to open.
  // Determine what kind of slave port to create and open it.
  //

  string typeParam = formatString(
      PARAM_PORT_PROT_TYPE,
      _pTask->getName().c_str(),
      _name.c_str());
  string protType;
  TPeerAddr addr;
  try
  {
    protType = ParameterSet::instance()->getString(typeParam);
  }
  catch (...)
  {
    if (SAP == getType())
    {
      // try to retrive the protType via the remote server name
      string remoteServiceNameParam = formatString(
          PARAM_SERVER_SERVICE_NAME,
          _pTask->getName().c_str(),
          _name.c_str());
      try 
      {
        // remote service is set by user?
        if (_remotetask.length() > 0 && _remoteport.length() > 0)
        {
          addr.taskname = _remotetask;
          addr.portname = _remoteport;
        }
        else
        {
          // retrieve remote service addr from parameter set
          string remoteAddr;
          remoteAddr += ParameterSet::instance()->getString(remoteServiceNameParam);
          const char* pRemoteTaskName = remoteAddr.c_str();
          // format is: "<taskname>:<portname>"
          char* colon = strchr(pRemoteTaskName, ':');
          if (colon) *colon = '\0';
          addr.taskname = pRemoteTaskName;
          addr.portname = colon + 1;
        }
        
        // now the protType of the remote service port can be retrieved from the
        // parameter set
        typeParam = formatString(
            PARAM_PORT_PROT_TYPE,
            addr.taskname.c_str(),
            addr.portname.c_str());
        protType = ParameterSet::instance()->getString(typeParam);
      }
      catch (...)
      {
        LOG_ERROR(formatString (
            "Could not find port info for port '%s' of task '%s via (%s:%s)'.", 
            _name.c_str(), _pTask->getName().c_str(),
            addr.taskname.c_str(),
            addr.portname.c_str()));
        
        return false;        
      }
    }
    else
    {
      LOG_ERROR(formatString (
        "Could not find port info for port '%s' of task '%s'.", 
        _name.c_str(), _pTask->getName().c_str()));
        
      return false;        
    }      
  }

  // Check for the various port types
  if (protType == "TCP")
  {
    GCFTCPPort* pNewPort(0);
    string pseudoName = _name + "_TCP";
    pNewPort = new GCFTCPPort(*_pTask, pseudoName, _type, _protocol, _transportRawData);
    if (_remotetask.length() > 0 && _remoteport.length() > 0)
    {
      addr.taskname = _remotetask;
      addr.portname = _remoteport;
      pNewPort->setAddr(addr);    
    }
    pNewPort->setMaster(this);    

    _pSlave = pNewPort;
  }
  else if (protType == "ETH")
  {
    GCFETHRawPort* pNewPort(0);
    string pseudoName = _name + "_ETH";
    pNewPort = new GCFETHRawPort(*_pTask, pseudoName, _type, _transportRawData);
    pNewPort->setMaster(this);    

    _pSlave = pNewPort;
  }
  else if (protType == "DEV")
  {
    GCFDevicePort* pNewPort(0);
    string pseudoName = _name + "_DEV";
    pNewPort = new GCFDevicePort(*_pTask, pseudoName, 0, "", _transportRawData);
    pNewPort->setMaster(this);    

    _pSlave = pNewPort;
  }
  else  
  {
    LOG_ERROR(formatString (
        "No implementation found for port protocol type '%s'. At this moment only TCP, DEV and ETH is supported!",
	      protType.c_str()));
    
    return false;
  }

  return _pSlave->open();
}

bool GCFPort::close()
{
  _state = S_CLOSING;
  if (!_pSlave)
  {
    LOG_WARN("Was not opened!!!");
    
    return false;
  }

  return _pSlave->close();
}

bool GCFPort::setRemoteAddr(const string& remotetask, const string& remoteport)
{
  if (_type == SAP)
  {
    _remotetask = remotetask;
    _remoteport = remoteport;
    return (_remotetask.length() > 0 && _remoteport.length() > 0);
  }
  return false;
}

ssize_t GCFPort::send(GCFEvent& e)
{
  if (SPP == _type)
  {
    if (!(F_EVT_INOUT(e) & F_OUT))
    {
      LOG_ERROR(formatString (
          "Trying to send IN event '%s' on SPP "
		      "port '%s'; discarding this event.",
		      getTask()->evtstr(e), _name.c_str()));
         
      return -1;
    }
  }
  else if (SAP == _type)
  {
    if (!(F_EVT_INOUT(e) & F_IN))
    {
      LOG_ERROR(formatString (
          "Trying to send OUT event '%s' on SAP "
		      "port '%s'; discarding this event.",
		      getTask()->evtstr(e), _name.c_str()));

      return -1;
    }
  }
  else if (MSPP == _type)
  {
     LOG_ERROR(formatString (
        "Trying to send event '%s' by means of the portprovider: %s (MSPP). "
        "Not supported yet",
         getTask()->evtstr(e), _name.c_str()));

      return -1;
  }
  
  return _pSlave->send(e);

}

ssize_t GCFPort::recv(void* buf, size_t count)
{
  if (!_pSlave) return -1;
  return _pSlave->recv(buf, count);
}

long GCFPort::setTimer(long delay_sec,    long delay_usec,
		     long interval_sec, long interval_usec,
		     void* arg)
{
  if (!_pSlave) return -1;
  return _pSlave->setTimer(delay_sec, delay_usec, 
			  interval_sec, interval_usec,
			  arg);
}

long GCFPort::setTimer(double delay_seconds, 
		     double interval_seconds,
		     void* arg)
{
  if (!_pSlave) return -1;
  return _pSlave->setTimer(delay_seconds, interval_seconds, arg);
}

int GCFPort::cancelTimer(long timerid, void **arg)
{
  if (!_pSlave) return -1;
  return _pSlave->cancelTimer(timerid, arg);
}

int GCFPort::cancelAllTimers()
{
  if (!_pSlave) return -1;
  return _pSlave->cancelAllTimers();
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

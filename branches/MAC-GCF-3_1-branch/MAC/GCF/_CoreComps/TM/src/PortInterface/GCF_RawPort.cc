//#  GCF_RawPort.cc: Raw connection to a remote process
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

#include <GCF/GCF_RawPort.h>
#include <GCF/GCF_Port.h>
#include <GCF/GCF_PortInterface.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_TMProtocols.h>
#include <PortInterface/GTM_NameService.h>
#include <PortInterface/GTM_TopologyService.h>
#include <GTM_Defines.h>
#include <Timer/GTM_TimerHandler.h>

static GCFEvent disconnected_event(F_DISCONNECTED);
static GCFEvent connected_event   (F_CONNECTED);
static GCFEvent closed_event      (F_CLOSED);

GCFRawPort::GCFRawPort(GCFTask& task, 
                       string& name, 
                       TPortType type,  
                       int protocol,
                       bool transportRawData) : 
    GCFPortInterface(&task, name, type, protocol, transportRawData),   
    _pMaster(0)
{
}

GCFRawPort::GCFRawPort() :
    GCFPortInterface(0, "", SAP, 0, false),   
    _pMaster(0)
{
}

void GCFRawPort::init(GCFTask& task, 
                      string name, 
                      TPortType type, 
                      int protocol,
                      bool transportRawData)
{
    GCFPortInterface::init(task, name, type, protocol, transportRawData);
    _pMaster = 0;
}

GCFRawPort::~GCFRawPort()
{
  cancelAllTimers();
}

void GCFRawPort::setMaster(GCFPort* pMaster)
{
  _pMaster = pMaster;
}

GCFEvent::TResult GCFRawPort::dispatch(GCFEvent& event)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  assert(_pTask);
  if ((F_DATAIN != event.signal) && 
      (F_DATAOUT != event.signal) &&
      (F_EVT_PROTOCOL(event) != F_FSM_PROTOCOL) &&
      (F_EVT_PROTOCOL(event) != F_PORT_PROTOCOL))
  {
    LOFAR_LOG_INFO(TM_STDOUT_LOGGER, (
        "%s receives '%s' on port '%s'",
        _pTask->getName().c_str(), 
        _pTask->evtstr(event), 
        (isSlave() ? _pMaster->getName().c_str() : _name.c_str()))); 
  }

  switch (event.signal)
  {
    case F_CONNECTED:
      LOFAR_LOG_INFO(TM_STDOUT_LOGGER, (
          "port '%s' of task %s is connected!",
          _name.c_str(), _pTask->getName().c_str()));    
      _isConnected = true;
      break;
    case F_DISCONNECTED: 
    case F_CLOSED:
      LOFAR_LOG_INFO(TM_STDOUT_LOGGER, (
          "port '%s' of task %s is %s!",
          _name.c_str(), _pTask->getName().c_str(),
          (event.signal == F_CLOSED ? "closed" : "disconnected")));    
      _isConnected = false;
      break;
    case F_TIMER:
    {
      GCFTimerEvent* pTE = static_cast<GCFTimerEvent*>(&event);
      if (&disconnected_event == pTE->arg || 
          &connected_event == pTE->arg ||
          &closed_event == pTE->arg)
      {    
        return dispatch(*((GCFEvent*) pTE->arg));
      }
      break;
    }
    case F_DATAIN:
    {
      if (!isTransportRawData())
      {
        status = recvEvent();
        return status;
      }
      break;
    }
    default:        
      if (SPP == getType() && (F_EVT_INOUT(event) == F_OUT)) 
      {    
        LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
            "Developer error in %s (port %s): received an OUT event (%s) in a SPP",
            _pTask->getName().c_str(), 
            _name.c_str(), 
            _pTask->evtstr(event)
            ));    
        return status;
      }
      else if (SAP == getType() && (F_EVT_INOUT(event) == F_IN)) 
      {
        LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
            "Developer error in %s (port %s): received an IN event (%s) in a SAP",
            _pTask->getName().c_str(), 
            _name.c_str(), 
            _pTask->evtstr(event)
            ));    
        return status;
      }
      break;
  }

  if (isSlave())
  {
    _pMaster->setIsConnected(_isConnected);
    status = _pTask->dispatch(event, *_pMaster);      
  }
  else
  {
    status = _pTask->dispatch(event, *this);
  }

  return status;
}

long GCFRawPort::setTimer(long delay_sec, long delay_usec,
			  long interval_sec, long interval_usec,
			  void* arg)
{
  return GTMTimerHandler::instance()->setTimer(*this, 
          (unsigned long) (delay_sec * 1000000 + delay_usec), 
          (unsigned long) (interval_sec * 1000000 + interval_usec),
          arg);  
}

long GCFRawPort::setTimer(double delay_seconds, 
			  double interval_seconds,
			  void* arg)
{
  return GTMTimerHandler::instance()->setTimer(*this, 
         (unsigned long) (delay_seconds * 1000000.0), 
         (unsigned long) (interval_seconds * 1000000.0),
         arg);
}

int GCFRawPort::cancelTimer(long timerid, void **arg)
{
  return GTMTimerHandler::instance()->cancelTimer(timerid, arg);
}

int GCFRawPort::cancelAllTimers()
{
  return GTMTimerHandler::instance()->cancelAllTimers(*this);
}

int GCFRawPort::resetTimerInterval(long timerid,
				 long interval_sec,
				 long interval_usec)
{
  return 1;
}

/**
 * ::findAddr
 */
bool GCFRawPort::findAddr(GCFPeerAddr& addr)
{
  GCFPeerAddr theaddr;
  
  if (SAP == _type)
  {  
    // find local and remote address
    if (GTMTopologyService::instance()->getPeerAddr(_pTask->getName(),
				      _name, theaddr) < 0)
    {
      LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
          "No remote address found for port '%s' of task '%s'",
          _name.c_str(), _pTask->getName().c_str()));
      
      return false;
    }

    LOFAR_LOG_DEBUG(TM_STDOUT_LOGGER, (
        "Connecting local SAP [%s:%s] "
        "to remote SPP [%s:%s]@(%s:%d).",
        _pTask->getName().c_str(),
        _name.c_str(),
        theaddr.getTaskname().c_str(),
        theaddr.getPortname().c_str(),
        theaddr.getHost().c_str(),
        theaddr.getPortnumber()));
  }
  else if (SPP == _type || MSPP == _type)
  {
    // find my own address in the nameservice
    if (GTMNameService::instance()->query(_pTask->getName(),
    		    theaddr) < 0)
    {
     
      LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
          "Could not find own address for "
          "task '%s'.", 
          _pTask->getName().c_str()));
      return false;
    }
    if (GTMNameService::instance()->queryPort(_pTask->getName(),
    			_name,
    			theaddr))
    {
      LOFAR_LOG_ERROR(TM_STDOUT_LOGGER, (
          "Could not find port info for port '%s' of "
          "task '%s'.", _name.c_str(), 
          _pTask->getName().c_str()));
     
      return false;
    }
    
    LOFAR_LOG_INFO(TM_STDOUT_LOGGER, ("Listening on port %d for clients.",
                theaddr.getPortnumber()));
  }
  else 
    return false;

  addr = theaddr;

  return true;
}

void GCFRawPort::schedule_disconnected()
{
  setTimer(0, 0, 0, 0, (void*)&disconnected_event);
}

void GCFRawPort::schedule_close()
{
  setTimer(0, 0, 0, 0, (void*)&closed_event);
}

void GCFRawPort::schedule_connected()
{
  setTimer(0, 0, 0, 0, (void*)&connected_event);
}

GCFEvent::TResult GCFRawPort::recvEvent()
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  GCFEvent e;
  ssize_t bytesRead = recv(&e.signal, sizeof(e.signal));
  assert(bytesRead == sizeof(e.signal));
  bytesRead = recv(&e.length, sizeof(e.length));
  assert(bytesRead == sizeof(e.length));
  
  if (e.length > 0)
  {
    GCFEvent* full_event = 0;
    char* event_buf = new char[sizeof(e) + e.length];
    full_event = (GCFEvent*)event_buf;
    memcpy(event_buf, &e, sizeof(e));
    
    event_buf += sizeof(e);
    bytesRead = 0;    
    ssize_t payloadLength = e.length;
    do 
    {
      bytesRead += recv(event_buf + bytesRead, payloadLength);
      payloadLength = e.length - bytesRead;
                          
    } while (payloadLength > 0);
        
    status = dispatch(*full_event);
    
    event_buf -= sizeof(e);
    delete [] event_buf;
  }
  else
  {
    status = dispatch(e);
  }

  if (status != GCFEvent::HANDLED)
  {
    assert(getTask());
    LOFAR_LOG_INFO(TM_STDOUT_LOGGER, (
      "Event %s for task %s on port %s not handled or an error occured",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getName().c_str()
      ));
  }

  return status;
}

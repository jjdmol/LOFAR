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

#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GTM_Defines.h>
#include <Timer/GTM_TimerHandler.h>
#include <GCF/ParameterSet.h>

static GCFEvent disconnectedEvent(F_DISCONNECTED);
static GCFEvent connectedEvent   (F_CONNECTED);
static GCFEvent closedEvent      (F_CLOSED);

using namespace GCF;

GCFRawPort::GCFRawPort(GCFTask& task, 
                       string& name, 
                       TPortType type,  
                       int protocol,
                       bool transportRawData) : 
    GCFPortInterface(&task, name, type, protocol, transportRawData),   
    _pMaster(0)
{
  _pTimerHandler = GTMTimerHandler::instance(); 
  assert(_pTimerHandler);
}

GCFRawPort::GCFRawPort() :
    GCFPortInterface(0, "", SAP, 0, false),   
    _pMaster(0)
{
  _pTimerHandler = GTMTimerHandler::instance(); 
  assert(_pTimerHandler);
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
  assert(_pTimerHandler);
  GTMTimerHandler::release();
  _pTimerHandler = 0;
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
    LOG_INFO(formatString (
        "%s was received on port '%s' in task '%s'",
        _pTask->evtstr(event), 
        getRealName().c_str(), 
        _pTask->getName().c_str())); 
  }

  switch (event.signal)
  {
    case F_CONNECTED:
      LOG_INFO(formatString (
          "Port '%s' in task '%s' is connected!",
          getRealName().c_str(), _pTask->getName().c_str()));    
      _state = S_CONNECTED;
      break;
    case F_DISCONNECTED: 
    case F_CLOSED:
      LOG_INFO(formatString (
          "Port '%s' in task '%s' is %s!",
          getRealName().c_str(), _pTask->getName().c_str(),
          (event.signal == F_CLOSED ? "closed" : "disconnected")));    
      _state = S_DISCONNECTED;
      break;
    case F_TIMER:
    {
      GCFTimerEvent* pTE = static_cast<GCFTimerEvent*>(&event);
      if (&disconnectedEvent == pTE->arg || 
          &connectedEvent == pTE->arg ||
          &closedEvent == pTE->arg)
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
        LOG_ERROR(formatString (
            "Developer error in %s (port %s): received an OUT event (%s) in a SPP",
            _pTask->getName().c_str(), 
            getRealName().c_str(), 
            _pTask->evtstr(event)
            ));    
        return status;
      }
      else if (SAP == getType() && (F_EVT_INOUT(event) == F_IN)) 
      {
        LOG_ERROR(formatString (
            "Developer error in %s (port %s): received an IN event (%s) in a SAP",
            _pTask->getName().c_str(), 
            getRealName().c_str(), 
            _pTask->evtstr(event)
            ));    
        return status;
      }
      break;
  }

  if (isSlave())
  {
    _pMaster->setState(_state);
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
  assert(_pTimerHandler);
  return _pTimerHandler->setTimer(*this, 
          (unsigned long) (delay_sec * 1000000 + delay_usec), 
          (unsigned long) (interval_sec * 1000000 + interval_usec),
          arg);  
}

long GCFRawPort::setTimer(double delay_seconds, 
			  double interval_seconds,
			  void* arg)
{
  assert(_pTimerHandler);
  return _pTimerHandler->setTimer(*this, 
         (unsigned long) (delay_seconds * 1000000.0), 
         (unsigned long) (interval_seconds * 1000000.0),
         arg);
}

int GCFRawPort::cancelTimer(long timerid, void **arg)
{
  assert(_pTimerHandler);
  return _pTimerHandler->cancelTimer(timerid, arg);
}

int GCFRawPort::cancelAllTimers()
{
  assert(_pTimerHandler);
  return _pTimerHandler->cancelAllTimers(*this);
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
bool GCFRawPort::findAddr(TPeerAddr& addr)
{
  // find remote address
  string remoteAddrParam = formatString(
      PARAM_SERVER_SERVICE_NAME, 
      _pTask->getName().c_str(),
      getRealName().c_str());
  
  addr.taskname = "";
  addr.portname = "";
  
  if (SAP == _type)
  {  
    string remoteAddr;
    try
    {
      remoteAddr += ParameterSet::instance()->getString(remoteAddrParam);
    }
    catch (...)
    {
      LOG_DEBUG(formatString (
          "No remote address found for port '%s' of task '%s'",
          getRealName().c_str(), _pTask->getName().c_str()));
      return false;
    }
    
    const char* pRemoteTaskName = remoteAddr.c_str();
    char* colon = strchr(pRemoteTaskName, ':');
    if (colon) *colon = '\0';
    
    addr.taskname = pRemoteTaskName;
    addr.portname = colon + 1;
  }
  else 
    return false;

  return true;
}

void GCFRawPort::schedule_disconnected()
{
  setTimer(0, 0, 0, 0, (void*)&disconnectedEvent);
}

void GCFRawPort::schedule_close()
{
  setTimer(0, 0, 0, 0, (void*)&closedEvent);
}

void GCFRawPort::schedule_connected()
{
  setTimer(0, 0, 0, 0, (void*)&connectedEvent);
}

GCFEvent::TResult GCFRawPort::recvEvent()
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  GCFEvent e;
  // expects and reads signal
  if (recv(&e.signal, sizeof(e.signal)) != sizeof(e.signal)) 
  {
    // don't continue with receiving
  }
  // expects and reads length
  else if (recv(&e.length, sizeof(e.length)) != sizeof(e.length))
  {
    // don't continue with receiving
  }  
  // reads payload if specified
  else if (e.length > 0)
  {
    GCFEvent* full_event = 0;
    char* event_buf = new char[sizeof(e) + e.length];
    full_event = (GCFEvent*)event_buf;
    memcpy(event_buf, &e, sizeof(e));

    // read the payload right behind the just memcopied basic event structure
    if (recv(event_buf + sizeof(e), e.length) > 0)
    {          
    // dispatchs an event with just received params
      status = dispatch(*full_event);
    }    
    delete [] event_buf;
  }
  // dispatchs an event without params
  else
  {
    status = dispatch(e);
  }

  if (status != GCFEvent::HANDLED)
  {
    assert(getTask());
    LOG_INFO(formatString (
      "%s for port '%s' in task '%s' not handled or an error occured",
      getTask()->evtstr(e),
      getTask()->getName().c_str(), 
      getRealName().c_str()
      ));
  }

  return status;
}

const string& GCFRawPort::getRealName() const
{
  return (isSlave() ? _pMaster->getName() : _name);
}

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

#include "GCF_RawPort.h"
#include "GCF_Port.h"
#include "GCF_PortInterface.h"
#include "GCF_Task.h"
#include "GCF_TMProtocols.h"
#include "GTM_NameService.h"
#include "GTM_TopologyService.h"
#include <Timer/GTM_Timer.h>

static GCFEvent disconnected_event(F_DISCONNECTED_SIG);
static GCFEvent connected_event   (F_CONNECTED_SIG);
static GCFEvent closed_event      (F_CLOSED_SIG);

GCFRawPort::GCFRawPort(GCFTask& task, string& name, TPortType type,  
                                                int protocol) : 
    GCFPortInterface(&task, name, type, protocol),   
    _pMaster(0)
{
}

GCFRawPort::GCFRawPort() :
    GCFPortInterface(0, "", SAP, 0),   
    _pMaster(0)
{
}

void GCFRawPort::init(GCFTask& task, string& name, TPortType type, int protocol)
{
    GCFPortInterface::init(task, name, type, protocol);
    _pMaster = 0;
}

GCFRawPort::~GCFRawPort()
{
}

void GCFRawPort::setMaster(GCFPort* pMaster)
{
  _pMaster = pMaster;
}

int GCFRawPort::dispatch(GCFEvent& event)
{
  int status = GCFEvent::NOT_HANDLED;

  if (F_TIMER_SIG == event.signal)
  {
  }
  if (isSlave())
  {
    status = _pTask->dispatch(event, *_pMaster);
  }
  else
  {
    // this dispatch function is only called when the port
    // is used in raw mode => master_ must be 0
    if ((F_DATAIN_SIG != event.signal)
	&& (F_DATAOUT_SIG != event.signal))
    {
      cout << getTask()->getName() << ": RAW RECVD: "
	   << " on port " << getName()
	   << endl;
    }

    status = _pTask->dispatch(event, *this);
  }

  return status;
}

void GCFRawPort::handleTimeout(const GTMTimer& timer)
{
  if (&disconnected_event == timer.getTimerArg() || 
      &connected_event == timer.getTimerArg() ||
      &closed_event == timer.getTimerArg())
  {
    _isConnected = (&connected_event == timer.getTimerArg());

    if (isSlave()) _pMaster->setIsConnected(_isConnected);
    
    dispatch(*((GCFEvent*) timer.getTimerArg()));
  }
  else
  {
    GCFTimerEvent te;
    te.sec = timer.getTime() / 1000000;
    te.usec = timer.getTime() - (te.sec * 1000000);
    te.arg = timer.getTimerArg();
    if (!timer.hasInterval())
    {
      GTMTimer* pCurTimer(0);
      TTimerIter iter = _timers.find(timer.getID());
      pCurTimer = iter->second;
      if (pCurTimer)
        delete pCurTimer;
      _timers.erase(iter);    
    }
    dispatch(te);
  }
}

long GCFRawPort::setTimer(long delay_sec, long delay_usec,
			long interval_sec, long interval_usec,
			const void* arg)
{
  GTMTimer* pNewTimer = new GTMTimer(*this, 
                                 delay_sec * 1000000 + delay_usec, 
                                 interval_sec * 1000000 + interval_usec,
                                 arg);
  _timers.insert(make_pair(pNewTimer->getID(),pNewTimer));
  return pNewTimer->getID();
}

long GCFRawPort::setTimer(double delay_seconds, 
			double interval_seconds,
			const void* arg)
{
  GTMTimer* pNewTimer = new GTMTimer(*this, 
                                 (unsigned long) (delay_seconds * 1000000.0), 
                                 (unsigned long) (interval_seconds * 1000000.0),
                                 arg);
  _timers.insert(make_pair(pNewTimer->getID(),pNewTimer));
  return pNewTimer->getID();
}

int GCFRawPort::cancelTimer(long timerid, const void **arg)
{
  int result(-1);
  GTMTimer* pCurTimer(0);
  TTimerIter iter = _timers.find(timerid);
  pCurTimer = iter->second; //second is of type GTMTimer*
  if (pCurTimer)
  {
    *arg = pCurTimer->getTimerArg();
    result = pCurTimer->stop();
    delete pCurTimer;
  }
  _timers.erase(iter);
    
  return result;
}

int GCFRawPort::cancelAllTimers()
{
  int result(1);
  GTMTimer* pCurTimer(0);
  
  for (TTimerIter iter = _timers.begin(); iter != _timers.end(); ++iter)
  {
    pCurTimer = iter->second;
    if (pCurTimer)
    {
      result = pCurTimer->stop();
      delete pCurTimer;
    }
  }
  _timers.clear();
  
  return result;
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
      LOG_DEBUG(("No remote address found for port '%s' of task '%s'\n",
                 _name, _pTask->getName()));
      
      return false;
    }

    LOG_DEBUG((
            "Connecting local SAP [%s:%s] "
            "to remote SPP [%s:%s]@(%s:%d).\n",
            _pTask->getName(),
            _name,
            theaddr.getTaskname(),
            theaddr.getPortname(),
            theaddr.getHost(),
            theaddr.getPortnumber()));
  }
  else if (SPP == _type)
  {
    // find my own address in the nameservice
    if (GTMNameService::instance()->query(_pTask->getName(),
    		    theaddr) < 0)
    {
     
        LOG_ERROR(("Could not find own address for "
                    "task '%s'.\n", _pTask->getName()));
        return false;
    }
    if (GTMNameService::instance()->queryPort(_pTask->getName(),
    			_name,
    			theaddr))
    {
        LOG_ERROR(("Could not find port info for port '%s' of "
                    "task '%s'.\n", getName(), task_->getName()));
     
        return false;
    }
    
    LOG_DEBUG(("Listening on port %d for clients.\n",
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

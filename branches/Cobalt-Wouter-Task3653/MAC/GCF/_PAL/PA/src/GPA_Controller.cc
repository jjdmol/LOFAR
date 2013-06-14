//#  GPA_Controller.cc: 
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

#include <GPA_Controller.h>
#include <GPA_PropSetSession.h>
#include <stdio.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <GCF/PAL/GCF_PVSSInfo.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace TM;
using namespace Common;
  namespace PAL 
  {
   
GPAController::GPAController() : 
  GCFTask((State)&GPAController::startup_state, PA_TASK_NAME),
  _garbageTimerId(0),
  _queueTimerId(0)
{
  // register the protocol for debugging purposes
  TM::registerProtocol(PA_PROTOCOL, PA_PROTOCOL_STRINGS);

  // startup_stateize the TCP port provider
  _pmlPortProvider.init(*this, "provider", GCFPortInterface::MSPP, PA_PROTOCOL);
  // startup_stateize the PVSS port for distributed communication between CCU and LCU
  _distPmlPortProvider.setConverter(_converter);
  _distPmlPortProvider.init(*this, "DPAserver", GCFPortInterface::MSPP, PA_PROTOCOL);
}

GPAController::~GPAController()
{
  LOG_INFO("Deleting PropertyAgent");
  emptyGarbage();
}

GCFEvent::TResult GPAController::startup_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
    {
//      LOG_INFO("Prepare PVSS DB for proper use by PA");
//      system("chmod 777 preparePVSS-DB");
//      system("./preparePVSS-DB");
      break;
    }
    case F_ENTRY:
      _pmlPortProvider.open();
      _distPmlPortProvider.open();      
      break;
      
    case F_TIMER:
      p.open();
      break;

    case F_CONNECTED:
      if (_pmlPortProvider.isConnected() &&  _distPmlPortProvider.isConnected())
      {
        TRAN(GPAController::waiting_state);
      }
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

//
// wait_state(event, port)
//
GCFEvent::TResult GPAController::waiting_state(GCFEvent& e, GCFPortInterface& p)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	GPAPropSetSession* pPropSetSession(0);
	GCFEvent* pEvent(0);

	switch (e.signal) {
	case F_ENTRY:
		LOG_INFO("Property Agent is ready!!!");
		_garbageTimerId = _distPmlPortProvider.setTimer(2.0, 2.0);
		break;

	case F_CONNECTED:   
		_pmlPorts.push_back(&p);
		break;

	case F_ACCEPT_REQ:
		acceptConnectRequest(p);
		break;

	case F_DISCONNECTED:      
		if (&p != &_pmlPortProvider && &p != &_distPmlPortProvider) {
			p.close();
		}
		// else //TODO: find out whether this can realy happend
		break;

	case F_CLOSED:
		for (TPropSetSessions::iterator iter = _propSetSessions.begin();
										iter != _propSetSessions.end(); ++iter) {
			_pmlPortGarbage[&p].push_back(iter->second);

			LOG_DEBUG(formatString("Distributes 'closed' event to session '%s' (port %p is now in use by %d sessions)...",
						iter->second->getName().c_str(), &p, _pmlPortGarbage[&p].size()));

			iter->second->GCFFsm::dispatch(e, p);
		}
		if (_propSetSessions.empty()) {
			_pmlPortGarbage[&p];
		}
		_pmlPorts.remove(&p);
		break;

	case PA_LOAD_PROP_SET:
	{
		PALoadPropSetEvent* pRequest = new PALoadPropSetEvent(e);
		LOG_DEBUG(formatString("Try to load prop. set '%s'...", pRequest->scope.c_str()));
		pEvent = pRequest;
		pPropSetSession = findPropSet(pRequest->scope);
		if (!pPropSetSession) {
			LOG_DEBUG(formatString("Prop. set '%s' not exists. Not loaded!!!",
			pRequest->scope.c_str()));
			PAPropSetLoadedEvent response;
			response.seqnr = pRequest->seqnr;
			response.result = PA_PROP_SET_NOT_EXISTS;
			p.send(response);
		}
		break;
	}
	case PA_UNLOAD_PROP_SET:
	{      
		PAUnloadPropSetEvent* pRequest = new PAUnloadPropSetEvent(e);
		LOG_DEBUG(formatString("Try to unload prop. set '%s'...", pRequest->scope.c_str()));
		pEvent = pRequest;
		pPropSetSession = findPropSet(pRequest->scope);
		if (!pPropSetSession) {
			LOG_DEBUG(formatString("Prop. set '%s' not exists. Not unloaded!!!",
			pRequest->scope.c_str()));
			PAPropSetUnloadedEvent response;
			response.seqnr = pRequest->seqnr;
			response.result = PA_PROP_SET_NOT_EXISTS;
			p.send(response);
		}
		break;
	}
	case PA_CONF_PROP_SET:
	{      
		PAConfPropSetEvent* pRequest = new PAConfPropSetEvent(e);
		LOG_DEBUG(formatString("Try to configure prop. set '%s'...", pRequest->scope.c_str()));
		pEvent = pRequest;
		pPropSetSession = findPropSet(pRequest->scope);
		if (!pPropSetSession) {
			LOG_DEBUG(formatString("Prop. set '%s' not exists. Not configured!!!",
			pRequest->scope.c_str()));
			PAPropSetConfEvent response;
			response.seqnr = pRequest->seqnr;
			response.apcName = pRequest->apcName;
			response.result = PA_PROP_SET_NOT_EXISTS;
			p.send(response);
		}
		break;
	}
	case PA_REGISTER_SCOPE:
	{
		PARegisterScopeEvent* pRequest = new PARegisterScopeEvent(e);
		LOG_DEBUG(formatString("Try to enable prop. set '%s'...", pRequest->scope.c_str()));
		pPropSetSession = findPropSet(pRequest->scope);
		if (pPropSetSession) {
			LOG_DEBUG(formatString("Prop. set '%s' already exists. Not enabled again!!!",
			pRequest->scope.c_str()));
			PAScopeRegisteredEvent response;
			response.seqnr = pRequest->seqnr;
			response.result = PA_PROP_SET_ALREADY_EXISTS;
			p.send(response);
		}
		else {
			LOG_DEBUG(formatString("Create the prop. set session: '%s_session'.",
			pRequest->scope.c_str()));
			pEvent = pRequest;
			pPropSetSession = new GPAPropSetSession(*this, p);
			ASSERT(pPropSetSession);
			_propSetSessions[pRequest->scope] = pPropSetSession;
		}
		break;
	}
	case PA_UNREGISTER_SCOPE:
	{      
		PAUnregisterScopeEvent* pRequest = new PAUnregisterScopeEvent(e);
		LOG_DEBUG(formatString("Try to disable prop. set '%s'...",
		pRequest->scope.c_str()));
		pEvent = pRequest;
		pPropSetSession = findPropSet(pRequest->scope);
		if (!pPropSetSession) {
			LOG_DEBUG(formatString("Prop. set '%s' not exists. Not disable!!!",
			pRequest->scope.c_str()));
			PAScopeUnregisteredEvent response;
			response.seqnr = pRequest->seqnr;
			response.result = PA_PROP_SET_NOT_EXISTS;
			p.send(response);
		}
		break;
	}
	case PA_PROP_SET_LINKED:
	{
		PAPropSetLinkedEvent* pResponse = new PAPropSetLinkedEvent(e);
		pEvent = pResponse;
		pPropSetSession = findPropSet(pResponse->scope);
		break;
	}
	case PA_PROP_SET_UNLINKED:
	{
		PAPropSetUnlinkedEvent* pResponse = new PAPropSetUnlinkedEvent(e);
		pEvent = pResponse;
		pPropSetSession = findPropSet(pResponse->scope);
		break;
	}
	case F_TIMER:
	{
		GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
		//      LOG_DEBUG(formatString("Timer %d expired.", pTimer->id));
		if (&p == &_distPmlPortProvider) {        
			if (pTimer->id == _garbageTimerId) {
				emptyGarbage();
			}
			else {
				if (pTimer->arg) {
					pPropSetSession = (GPAPropSetSession*)pTimer->arg;
					pPropSetSession->doNextRequest();
				}          
			}
		}      
		break;
	}

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	if (pEvent) {
		if (pPropSetSession) {    
			if (pPropSetSession->GCFFsm::dispatch(*pEvent, p) == GCFEvent::HANDLED) {
				// message is handled event pointer can be deleted
				delete pEvent;
			}
			//else: message is queued, so keep the pointer a live until this message is handled, 
			//      will be destroyed by RequestManager
		}
		else {
			delete pEvent;
		}
	}

	return status;
}

void GPAController::mayDeleted(GPAPropSetSession& session)
{
  LOG_DEBUG(formatString("Prop. set session '%s' will not be used anymore. Move to garbage...",
      session.getName().c_str())); 
  _propSetSessionsGarbage.push_back(&session);
  string propSetName = session.getName(); // task name 
  propSetName.erase(propSetName.length() - PA_PS_SESSION_TASK_NAME_EXT.length()); // remove session extension to get the prop set name (scope)
  _propSetSessions.erase(propSetName);
  
  // There is a possibility that the session has received more than 1 closed port events to handle.
  // The first closed port event to handle can result in deleting the session. All following closed port 
  // events than will be ignored by the session. Normally each session has to report to the controller that the closed port event 
  // is handled, which results in deleting the entry in the controllers administration by the closingPortFinished method.
  // So each closed port event, which is send to a session, must be released by the session 1 by 1. But in this case 
  // not all pending closed port events will never be handled and therefore not released. That's why here the closed port(event)s 
  // will be collected for this session and the closingPortFinished method will be called for these ports.
  LOG_DEBUG(formatString("Also release closed ports for the current session (%s) to be deleted...",
      session.getName().c_str())); 
  list<GCFPortInterface*> releasedPorts; // closed ports, which needs no closing sequence in the session to be deleted here
  for (TClosedPorts::iterator giter = _pmlPortGarbage.begin();
       giter != _pmlPortGarbage.end(); ++giter)
  {
    for (list<GPAPropSetSession*>::iterator pssiter = giter->second.begin();
         pssiter != giter->second.end(); ++pssiter)
    {
      if (&session == *pssiter)
      {
        releasedPorts.push_back(giter->first);
      }
    }
  }
  for (list<GCFPortInterface*>::iterator iter = releasedPorts.begin();
       iter != releasedPorts.end(); ++iter)
  {
    closingPortFinished(session, **iter);
  }
}

void GPAController::closingPortFinished(GPAPropSetSession &pss, GCFPortInterface& p)
{
  TClosedPorts::iterator iter = _pmlPortGarbage.find(&p);
  ASSERT(iter != _pmlPortGarbage.end());
  ASSERT(!iter->second.empty());
  iter->second.remove(&pss);
  LOG_DEBUG(formatString("Closed port is not used anymore by the prop. set session '%s'. Port (=%p) (still) used by %d sessions!",
     pss.getName().c_str(), &p, iter->second.size())); 
}

GPAPropSetSession* GPAController::findPropSet(const string& scope) const
{
	LOG_DEBUG_STR("findPropSet(" << scope << ")");

  TPropSetSessions::const_iterator iter = _propSetSessions.find(scope);  
  return (iter != _propSetSessions.end() ? iter->second : 0);
}

void GPAController::acceptConnectRequest(GCFPortInterface& p)
{
  if (&p == &_pmlPortProvider)
  {
    GCFTCPPort* pNewPMLPort = new GCFTCPPort();
    LOG_INFO(formatString("New property set provider accepted (%p)!",
        pNewPMLPort));
    ASSERT(pNewPMLPort);
    pNewPMLPort->init(*this, "tcp-pa-client", GCFPortInterface::SPP, PA_PROTOCOL);
    _pmlPortProvider.accept(*pNewPMLPort);
  }
  else
  {
    GCFPVSSPort* pNewPVSSClientPort = new GCFPVSSPort();
    LOG_INFO(formatString("New property set user accepted (%p)!",
        pNewPVSSClientPort));
    ASSERT(pNewPVSSClientPort);
    pNewPVSSClientPort->init(*this, "pvss-pa-client", GCFPortInterface::SPP, PA_PROTOCOL);
    _distPmlPortProvider.accept(*pNewPVSSClientPort);
  }
}

void GPAController::emptyGarbage()
{
  
  // starts with emptying the garbages 
  GCFPortInterface* pPort(0);
  list<GCFPortInterface*> portsToBeDeletedFromGarbage;
  for (TClosedPorts::iterator iter = _pmlPortGarbage.begin();
       iter != _pmlPortGarbage.end(); ++iter)
  {
    if (iter->second.empty())
    {
      pPort = iter->first;
      portsToBeDeletedFromGarbage.push_back(pPort);
    }
  }  
  
  if (portsToBeDeletedFromGarbage.size() > 0 || _propSetSessionsGarbage.size() > 0)
  {
    LOG_DEBUG("Try to empty the garbages.");
  }
  for (list<GCFPortInterface*>::iterator iter = portsToBeDeletedFromGarbage.begin();
       iter != portsToBeDeletedFromGarbage.end(); ++iter)
  {
    pPort = *iter;
    LOG_DEBUG(formatString("Port '%s' (%p) will now be destroyed!!!",
       pPort->getName().c_str(), pPort));       
    delete pPort;
    _pmlPortGarbage.erase(pPort);
  }
  
  GPAPropSetSession* pPropSetSession(0);
  for (list<GPAPropSetSession*>::iterator iter = _propSetSessionsGarbage.begin();
       iter != _propSetSessionsGarbage.end(); ++iter)
  {
    pPropSetSession = *iter;
    LOG_DEBUG(formatString("Prop. set session '%s' will now be destroyed!!!",
       pPropSetSession->getName().c_str())); 
    delete pPropSetSession;
  }
  _propSetSessionsGarbage.clear();
}

void GPAController::propSetIdle(GPAPropSetSession& idlePropSet)
{
  // If this method is called the prop. set session (idlePropSet) is ready to handle queued messages (if there some).
  // To leave the current prop. set session event handling context this NULL timer will be started.
  // If the timer expires the doNextRequest of this session will be called.
  long timerId = _distPmlPortProvider.setTimer(0.0, 0.0, &idlePropSet); 
  LOG_DEBUG(formatString("Session %s is now IDLE. So start a 0 timer (%d) to enable handling a (possible) queued message in a new context.",
      idlePropSet.getName().c_str(),
      timerId));
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

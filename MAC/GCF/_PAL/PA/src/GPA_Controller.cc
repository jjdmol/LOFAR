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

#include <GPA_Controller.h>
#include <GPA_PropertySet.h>
#include <stdio.h>
#include <PA_Protocol.ph>

#define QUEUE_REQUEST(p, e)  \
  if (!mayContinue(e, p)) break;

static string sPATaskName("PA");

GPAController::GPAController() : 
  GCFTask((State)&GPAController::initial, sPATaskName),
  _isBusy(false),
  _isRegistered(false),
  _counter(0),
  _pCurPropSet(0)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port
  _pmlPortProvider.init(*this, "server", GCFPortInterface::MSPP, PA_PROTOCOL);

  // To force a connection with the PVSS system at start-up of the PA 
  // a dummy property set will be created temporary. 
  // The GPAPropertySet class inherits from the GSAService class which initiates
  // the connection with the PVSS system on construction. Because it will managed
  // by a singleton the property set only needs to be temporary.
  GPAPropertySet dummy(*this, _pmlPortProvider);  
}

GPAController::~GPAController()
{
}

GCFEvent::TResult GPAController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _pmlPortProvider.open();
      break;

    case F_CONNECTED:
      TRAN(GPAController::operational);
      break;

    case F_DISCONNECTED:
      if (&p == &_pmlPortProvider)
        _pmlPortProvider.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPAController::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_CONNECTED:   
      _pmlPorts.push_back(&p);
      break;

    case F_ACCEPT_REQ:
      acceptConnectRequest();
      break;

    case F_DISCONNECTED:      
      if (&p != &_pmlPortProvider) p.close();
      // else //TODO: find out this can realy happend
      break;

    case F_CLOSED:
    {
      QUEUE_REQUEST(p, e);
      closeConnection(p);
      break;
    }

    case PA_LOAD_PROP_SET:
    {
      QUEUE_REQUEST(p, e);
      PALoadPropSetEvent request(e);
      GPAPropertySet* pPropSet = findPropSet(request.scope);
      if (pPropSet)
      {
        pPropSet->load(request, p);
      }
      else
      {
        PAPropSetLoadedEvent response;
        response.seqnr = request.seqnr;
        response.result = PA_PROP_SET_NOT_EXISTS;
        sendAndNext(response);
      }
      break;
    }
    case PA_UNLOAD_PROP_SET:
    {
      QUEUE_REQUEST(p, e);
      PAUnloadPropSetEvent request(e);
      GPAPropertySet* pPropSet = findPropSet(request.scope);
      if (pPropSet)
      {
        pPropSet->unload(request, p);
      }
      else
      {
        PAPropSetUnloadedEvent response;
        response.seqnr = request.seqnr;
        response.result = PA_PROP_SET_NOT_EXISTS;
        sendAndNext(response);
      }
      break;
    }
    case PA_CONF_PROP_SET:
    {
      QUEUE_REQUEST(p, e)
      PAConfPropSetEvent request(e);
      GPAPropertySet* pPropSet = findPropSet(request.scope);
      if (pPropSet)
      {
        pPropSet->configure(request);
      }
      else
      {
        PAPropSetConfEvent response;
        response.seqnr = request.seqnr;
        response.apcName = request.apcName;
        response.result = PA_PROP_SET_NOT_EXISTS;
        sendAndNext(response);
      }
      break;
    }
    case PA_REGISTER_SCOPE:
    {
      QUEUE_REQUEST(p, e)
      PARegisterScopeEvent request(e);
      GPAPropertySet* pPropSet = findPropSet(request.scope);
      if (pPropSet)
      {
        PAScopeRegisteredEvent response;
        response.seqnr = request.seqnr;
        response.result = PA_PROP_SET_ALLREADY_EXISTS;
        sendAndNext(response);        
      }
      else
      {
        pPropSet = new GPAPropertySet(*this, p);
        _propertySets[request.scope] = pPropSet;
        if (!pPropSet->enable(request))
        {
          delete pPropSet;
          _propertySets.erase(request.scope);
        }
      }
      break;
    }
    case PA_UNREGISTER_SCOPE:
    {
      QUEUE_REQUEST(p, e)
      PAUnregisterScopeEvent request(e);
      GPAPropertySet* pPropSet = findPropSet(request.scope);
      if (pPropSet)
      {
        pPropSet->disable(request);
      }
      else
      {
        PAScopeUnregisteredEvent response;
        response.seqnr = request.seqnr;
        response.result = PA_PROP_SET_NOT_EXISTS;
        sendAndNext(response);
      }
      break;
    }
    /*case PA_PROP_SET_LINKED:
    {
      PAPropSetLinkedEvent response(e);
      GPAPropertySet* pPropSet = findPropSet(response.scope);
      assert(pPropSet);
      pPropSet->linked(response); 
      break;
    }
    case PA_PROP_SET_UNLINKED:
    {
      PAPropSetUnlinkedEvent response(e);
      GPAPropertySet* pPropSet = findPropSet(response.scope);
      assert(pPropSet);
      pPropSet->unlinked(response); 
      break;
    }*/
    case PA_LINK_PROP_SET:
    {
      PALinkPropSetEvent* pRequest = (PALinkPropSetEvent*) (&e);
      _pCurPropSet = findPropSet(pRequest->scope);
      assert(_pCurPropSet);
      _pmlPortProvider.setTimer(0, 0, 0, 0, &p); // pass the server port
      p.send(e);
      TRAN(GPAController::linking);
      break;
    }
    case PA_UNLINK_PROP_SET:
    {
      PALinkPropSetEvent* pRequest = (PALinkPropSetEvent*) (&e);
      _pCurPropSet = findPropSet(pRequest->scope);
      assert(_pCurPropSet);
      _pmlPortProvider.setTimer(0, 0, 0, 0, &p); // pass the server port
      p.send(e);
      TRAN(GPAController::unlinking);
      break;
    }
    case F_TIMER:
    {
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
      if (&p == &_pmlPortProvider)
      {
        if (_deletePortTimId == pTimer->id)
        {
          GCFPortInterface* pPort = (GCFPortInterface*)(pTimer->arg);
          if (pPort)
          {
            _pmlPorts.remove(pPort);
            delete pPort;
          }
        }
        else
        {
          GPAPropertySet* pPropSet = (GPAPropertySet*)(pTimer->arg);
          if (pPropSet)
          {
            delete pPropSet;
          }
        }
        _counter--;
        if (_counter == 0)
        {
          GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
          GCFEvent* pEvent = _requestManager.getOldestRequest();
          _isBusy = false;
      
          if (pPort) // new request available?
          {
            _isRegistered = true;
            dispatch(*pEvent, *pPort);
          }
          
        }
      }
      else
      {
        GCFEvent* pEvent = _requestManager.getOldestRequest();
        PAUnregisterScopeEvent request(*pEvent);
        GPAPropertySet* pPropSet = findPropSet(request.scope);
        if (pPropSet)
        { 
          delete pPropSet;
        }
        _propertySets.erase(request.scope);
        PAScopeUnregisteredEvent response;
        response.seqnr = request.seqnr;
        response.result = (TPAResult) (unsigned int) (pTimer->arg);
        if (p.isConnected())
        {
          p.send(response);
        }
        doNextRequest();
      }
      
      break;
    }
 
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPAController::linking(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static GCFPortInterface* pPort = 0;
  
  switch (e.signal)
  {
    case F_ENTRY:
      pPort = 0;
      break;
      
    case F_CONNECTED:   
      _pmlPorts.push_back(&p);
      break;

    case F_ACCEPT_REQ:
      acceptConnectRequest();
      break;

    case F_DISCONNECTED:      
      if (&p != &_pmlPortProvider) p.close();
      // else //TODO: find out this can realy happend
      break;
    
    case F_CLOSED:
      if (&p == pPort)
      {
        // this closed event came from the port wherefrom a link response were expected
        // the response will not come anymore, so we can continue with an error
        // it also registers this event for further handling (closeConnection)
        TRAN(GPAController::operational);
        PAPropSetLinkedEvent response;
        response.result = PA_SERVER_GONE;
        _requestManager.registerRequest(p, e);
        assert(_pCurPropSet);
        _pCurPropSet->linked(response);
        break;
      }
      // intentional fall through      
    case PA_LOAD_PROP_SET:
    case PA_UNLOAD_PROP_SET:
    case PA_CONF_PROP_SET:
    case PA_REGISTER_SCOPE:
    case PA_UNREGISTER_SCOPE:
    {
      QUEUE_REQUEST(p, e);
      // should not passes the check because in this state it is always busy
      assert(0); 
      break;
    }
    case PA_PROP_SET_LINKED:
    {
      PAPropSetLinkedEvent response(e);
      _pmlPortProvider.cancelAllTimers();
      TRAN(GPAController::operational);
      assert(_pCurPropSet);
      _pCurPropSet->linked(response); 
      break;
    }
    case F_TIMER:
    {
      assert(&p == &_pmlPortProvider);

      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
      
      pPort = (GCFPortInterface*)(pTimer->arg);
      break;
    }
 
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPAController::unlinking(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static GCFPortInterface* pPort = 0;

  switch (e.signal)
  {
    case F_ENTRY:
      pPort = 0;
      break;
      
    case F_CONNECTED:   
      _pmlPorts.push_back(&p);
      break;

    case F_ACCEPT_REQ:
      acceptConnectRequest();
      break;

    case F_DISCONNECTED:      
      if (&p != &_pmlPortProvider) p.close();
      // else //TODO: find out this can realy happend
      break;
    
    case F_CLOSED:
      if (&p == pPort)
      {
        // this closed event came from the port wherefrom a link response is expected
        // the response will not come anymore, so we can continue with an error
        // it also registers this event for further handling (closeConnection)
        TRAN(GPAController::operational);
        PAPropSetUnlinkedEvent response;
        response.result = PA_SERVER_GONE;
        _requestManager.registerRequest(p, e);
        assert(_pCurPropSet);
        _pCurPropSet->unlinked(response);
        break;
      }
      // intentional fall through      
    case PA_LOAD_PROP_SET:
    case PA_UNLOAD_PROP_SET:
    case PA_CONF_PROP_SET:
    case PA_REGISTER_SCOPE:
    case PA_UNREGISTER_SCOPE:
    {
      QUEUE_REQUEST(p, e);
      // should always be queued because in this state PA is always busy
      assert(0); 
      break;
    }
    case PA_PROP_SET_UNLINKED:
    {
      PAPropSetUnlinkedEvent response(e);
      _pmlPortProvider.cancelAllTimers();
      TRAN(GPAController::operational);
      assert(_pCurPropSet);
      _pCurPropSet->unlinked(response); 
      break;
    }
    case F_TIMER:
    {
      assert(&p == &_pmlPortProvider);

      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
            
      pPort = (GCFPortInterface*)(pTimer->arg);
      break;
    }
 
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

bool GPAController::mayContinue(GCFEvent& e, GCFPortInterface& p)
{
  bool result(false);
  if (!_isRegistered)
    _requestManager.registerRequest(p, e);
  
  _isRegistered = false;
  if (!_isBusy)
  { 
    _isBusy = true; 
    result = true;
  }
  return result;
}

void GPAController::sendAndNext(GCFEvent& e)
{
  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  assert(pPort);
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  assert(pEvent);
  if (pEvent->signal == F_CLOSED)
  {
    // all responses, which are the result of a closed port will handled here
    assert (e.signal == PA_PROP_SET_UNLOADED || e.signal == PA_SCOPE_UNREGISTERED);
    _counter--; // counter was set in operational method (signal F_CLOSED)
    if (_counter == 0)
    {
      // all responses are received now
      // the port and ass. prop. sets will be deleted from administration here
      // and must then be destructed in a different context. 
      // So 0 timers will be used to force a context switch
      list<string> propSetsToDelete;
      GPAPropertySet* pPropSet(0);
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        pPropSet = iter->second;
        assert(pPropSet);
        if (pPropSet->mayDelete())
        {
          // reuses the counter for counting started timers
          _counter++;
          // add the scope of the prop. set to be deleted
          propSetsToDelete.push_back(iter->first); 
          _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) (unsigned int) pPropSet);
        }
      }
      for (list<string>::iterator iter = propSetsToDelete.begin();
           iter != propSetsToDelete.end(); ++iter)
      {
        _propertySets.erase(*iter);
      }
      // id's of the above started timers always will be different to the following 
      // received id because they are started in the same context
      _requestManager.deleteRequestsOfPort(*pPort);
      _deletePortTimId = _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) pPort);
      _pmlPorts.remove(pPort);
      // reuses the counter for counting started timers
      _counter++;
    }
  }
  else if (e.signal == PA_SCOPE_UNREGISTERED)
  {
    // prop. set may be deleted now
    // this is only possible after a context switch
    // this can be forced by means of the 0 timer
    PAScopeUnregisteredEvent* pE = (PAScopeUnregisteredEvent*) &e;    
    pPort->setTimer(0,0,0,0, (void*) pE->result);
  }
  else
  {
    // normal use
    if (pPort->isConnected())
    {
      pPort->send(e);
    }
    doNextRequest();
  }
}

void GPAController::doNextRequest()
{
  _requestManager.deleteOldestRequest();
  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  _isBusy = false;
  if (pPort) // new action available ?
  {
    _isRegistered = true;
    dispatch(*pEvent, *pPort);
  }
}

GPAPropertySet* GPAController::findPropSet(const string& scope) const
{
  TPropertySets::const_iterator iter = _propertySets.find(scope);  
  return (iter != _propertySets.end() ? iter->second : 0);
}

void GPAController::acceptConnectRequest()
{
  GCFTCPPort* pNewPMLPort = new GCFTCPPort();
  pNewPMLPort->init(*this, "pa", GCFPortInterface::SPP, PA_PROTOCOL);
  _pmlPortProvider.accept(*pNewPMLPort);
}

void GPAController::closeConnection(GCFPortInterface& p)
{
  GPAPropertySet* pPropSet(0);
  _counter = 0;
  for (TPropertySets::iterator iter = _propertySets.begin();
       iter != _propertySets.end(); ++iter)
  {
    pPropSet = iter->second;
    assert(pPropSet);
    if (pPropSet->isOwner(p) || pPropSet->knowsClient(p))
    {
      // these property sets will be deleted after they will be unregistered
      // correctly
      _counter++;
    }
    pPropSet->deleteClient(p);        
  }
  if (_counter == 0)
  {
    // nothing needed to be done for any prop. set 
    // so the port can be deleted
    // this is only possible after a context switch
    // this can be forced by means of the 0 timer
    _counter = 1;
    _requestManager.deleteRequestsOfPort(p);
    _deletePortTimId = _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) &p);
  }
}

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
  LOG_INFO("Deleting PropertyAgent");
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
      // this starts the close port sequence
      clientGone(p);
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
        assert(pPropSet);
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
        // timeout of 0 timers used in the close port sequence (see F_CLOSED in 
        // sendAndNext method)
        GCFPortInterface* pPort(0);
        if (_deletePortTimId == pTimer->id)
        {
          // timer which has the only port reference to be deleted as argument
          pPort = (GCFPortInterface*)(pTimer->arg);
          if (pPort)
          {
            delete pPort;
            // already removed from the port list
          }          
          // now the close port sequence is finished so the next request can be handled
          _requestManager.deleteRequestsOfPort(*pPort);
          pPort = _requestManager.getOldestRequestPort();
          GCFEvent* pEvent = _requestManager.getOldestRequest();
          _isBusy = false;
      
          LOG_INFO("Closing sequence step 3: Port deleted");
          LOG_INFO("----End of closing sequence");  
          if (pPort) // new request available?
          {
            _isRegistered = true;
            dispatch(*pEvent, *pPort);
          }
        }
        else
        {
          // timer which has a prop. set reference to be deleted as argument                   
          GPAPropertySet* pPropSet = (GPAPropertySet*)(pTimer->arg);
          if (pPropSet)
          {
            delete pPropSet;
            // already removed from the prop. set list
          }
          // counter set on F_CLOSED event in sendAndNext method
          _counter--;
          if (_counter == 0)
          {
            LOG_INFO("Closing sequence step 1: Involved prop. sets deleted. End of this step!");
            // now the firt part of the close port sequence is finsihed
            // move to the second part, which informs the involved prop. sets 
            // about the gone client 
            pPort = _requestManager.getOldestRequestPort();
            propSetClientGone(*pPort);
          }
        }
      }
      else
      {
        // 0 timer used in disabling procedure
        GCFEvent* pEvent = _requestManager.getOldestRequest();
        PAUnregisterScopeEvent request(*pEvent);
        GPAPropertySet* pPropSet = findPropSet(request.scope);
        if (pPropSet)
        { 
          delete pPropSet;
        }
        _propertySets.erase(request.scope);
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
        // it also registers this event for further handling (close port sequence)
        TRAN(GPAController::operational);
        _requestManager.registerRequest(p, e);
        PAPropSetLinkedEvent response;
        response.result = PA_SERVER_GONE;
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
      // should always be queued because in this state the PA is always busy
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
        // it also registers this event for further handling (close port sequence)
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
      // should always be queued because in this state the PA is always busy
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
  {    
    _requestManager.registerRequest(p, e);
    if (_isBusy) LOG_INFO("PA busy! Request is queued!");
  }
  
  _isRegistered = false;
  if (!_isBusy)
  { 
    _isBusy = true; 
    result = true;
  }
  return result;
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

void GPAController::sendAndNext(GCFEvent& e)
{
  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  assert(pPort);
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  assert(pEvent);
  if (pEvent->signal == F_CLOSED)
  {
    // this is part of the close port sequence
    // all responses, which are the result of a closed port will handled here
    if (e.signal == PA_SCOPE_UNREGISTERED)
    {    
      // responses of the first step of the close port sequence
      _counter--; // counter was set in closeConnection method
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
            // it is safe now to reuses the counter for counting started 0 timers
            _counter++;
            // add the scope of the prop. set to be deleted
            propSetsToDelete.push_back(iter->first); 
            _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) pPropSet);
          }
        }
        for (list<string>::iterator iter = propSetsToDelete.begin();
             iter != propSetsToDelete.end(); ++iter)
        {
          _propertySets.erase(*iter);
        }
        LOG_INFO("Closing sequence step 1: Involved prop. sets are disabled. Delete them now.");
      }
    }
    else if (e.signal == PA_PROP_SET_UNLOADED)
    {
      // responses of the second step of the close port sequence
      PAPropSetUnloadedEvent* pEvent = (PAPropSetUnloadedEvent*)(&e);
      assert(pEvent->seqnr == 0);
      propSetClientGone(*pPort);      
    }
    else
    {
      assert(0);
    }
  }
  else if (e.signal == PA_SCOPE_UNREGISTERED)
  {
    // prop. set is now disabled in a normal way and may be deleted
    // this is only possible after a context switch
    // this can be forced by means of the 0 timer
    pPort->setTimer(0.0);
    if (pPort->isConnected())
    {
      pPort->send(e);
    }
  }
  else
  {
    // normal responses
    if (pPort->isConnected())
    {
      pPort->send(e);
    }
    doNextRequest();
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
  assert(pNewPMLPort);
  pNewPMLPort->init(*this, "pa", GCFPortInterface::SPP, PA_PROTOCOL);
  _pmlPortProvider.accept(*pNewPMLPort);
}

void GPAController::clientGone(GCFPortInterface& p)
{
  // first step of what must be done before the closed port can be deleted too
  // - search all prop. sets, which are managed by the remote application 
  //   connected via the closed port port (owner)
  // - pretend that these prop. sets receives a unregister scope event from the
  //   remote application
  LOG_INFO("----A PA client is gone, so start the closing sequence!");  
  GPAPropertySet* pPropSet(0);
  _counter = 0;
  _deletePortTimId = 0xFFFFFFFF;
  list<string> propSetsToDelete;
  for (TPropertySets::iterator iter = _propertySets.begin();
       iter != _propertySets.end(); ++iter)
  {
    pPropSet = iter->second;
    assert(pPropSet);
    if (pPropSet->isOwner(p))
    {
      // these property sets will be deleted after they are unregistered/disabled
      // correctly
      _counter++;
      propSetsToDelete.push_back(iter->first); 
    }
  }
  PAUnregisterScopeEvent request;  
  request.seqnr = 0;
  LOG_INFO("Closing sequence step 1: Start disabling involved prop. sets");  
  
  for (list<string>::iterator iter = propSetsToDelete.begin();
       iter != propSetsToDelete.end(); ++iter)
  {
    pPropSet = findPropSet(*iter);
    if (pPropSet)
    {              
      request.scope = *iter;
      pPropSet->disable(request);
      // response will be handled in the sendAndNext method (F_CLOSED event)
    }   
  }
  if (_counter == 0)
  {
    LOG_INFO("Closing sequence step 1: Nothing to do. End of this step!");
    // no prop. set needed to be disabled
    // move to second step, which deletes the prop. set client (represented by 
    // the port 'p') from the prop. set
    propSetClientGone(p);
  }
}

void GPAController::propSetClientGone(GCFPortInterface& p)
{
  // second step of what must be done before the closed port can be deleted too
  // - find the first prop. set who knows the client port as prop. set client
  // - delete the client port from prop. set, which results in an unload request
  //   with sequence 0

  LOG_INFO("Closing sequence step 2: Find (next) prop. set who knows port as client");
  string nextPropSet;
  GPAPropertySet* pPropSet(0);

  for (TPropertySets::iterator iter = _propertySets.begin(); 
       iter != _propertySets.end(); ++iter)
  {
    pPropSet = iter->second;
    assert(pPropSet);            
    if (pPropSet->knowsClient(p))
    {
      nextPropSet = iter->first;
      break; 
    }
  }
  if (nextPropSet.length() > 0)
  {
    pPropSet = findPropSet(nextPropSet);
    if (pPropSet)
    {              
      pPropSet->clientGone(p);
    }   
  }
  else 
  {
    LOG_INFO("Closing sequence step 2: No more prop. sets needed to be informed.");
    LOG_INFO("Closing sequence step 3: Delete port");
    // no more prop. set were involved in this action
    // so the port 'p' can be deleted
    // this is only possible after a context switch
    // this can be forced by means of the 0 timer
    _deletePortTimId = _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) &p);   
    _pmlPorts.remove(&p);
  }
}

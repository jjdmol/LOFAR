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
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/ParameterSet.h>

using namespace GCF;

#define QUEUE_REQUEST(p, e)  \
  if (!mayContinue(e, p)) break;

static string sPATaskName("GCF-PA");

GPAController::GPAController() : 
  GCFTask((State)&GPAController::initial, sPATaskName),
  //_distClientManager(*this),
  _isBusy(false),
  _isRegistered(false),
  _counter(0),
  _pCurPropSet(0),
  _synchronous(false)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the TCP port provider
  _pmlPortProvider.init(*this, "provider", GCFPortInterface::MSPP, PA_PROTOCOL);
  // initialize the PVSS port for distributed communication between CCU and LCU
  _distPmlPortProvider.setConverter(_converter);
  _distPmlPortProvider.init(*this, "DPA_server", GCFPortInterface::MSPP, PA_PROTOCOL);
}

GPAController::~GPAController()
{
  LOG_INFO("Deleting PropertyAgent");
  emptyGarbage();
}

GCFEvent::TResult GPAController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
    {
      LOG_INFO("Prepare PVSS DB for proper use by PA");
      system("chmod 777 preparePVSS-DB");
      system("preparePVSS-DB");
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
        TRAN(GPAController::operational);
      }
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    case F_EXIT:
      LOG_INFO("Property Agent is ready!!!");
      _garbageTimerId = _distPmlPortProvider.setTimer(2.0, 2.0);
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
      acceptConnectRequest(p);
      break;

    case F_DISCONNECTED:      
      if (&p != &_pmlPortProvider && &p != &_distPmlPortProvider) p.close();
      // else //TODO: find out this can realy happend
      break;

    case F_CLOSED:
    {
      QUEUE_REQUEST(p, e);
      // this starts the close port sequence
      clientPortGone(p);
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
      if (&p == &_distPmlPortProvider)
      {        
        if (_garbageTimerId == pTimer->id)
        {
          emptyGarbage();
        }
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
      acceptConnectRequest(p);
      break;

    case F_DISCONNECTED:      
      if (&p != &_pmlPortProvider && &p != &_distPmlPortProvider) p.close();
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
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
      if (&p == &_pmlPortProvider)
      {
        pPort = (GCFPortInterface*)(pTimer->arg);
      }
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
      acceptConnectRequest(p);
      break;

    case F_DISCONNECTED:      
      if (&p != &_pmlPortProvider && &p != &_distPmlPortProvider) p.close();
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
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
      if (&p == &_pmlPortProvider)
      {
        pPort = (GCFPortInterface*)(pTimer->arg);
      }
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
    // all responses, which are the result of a closed port will be handled here
    if (e.signal == PA_SCOPE_UNREGISTERED)
    {    
      // responses of the first step of the close port sequence
      _counter--; // counter was set in clientPortGone method
      if (_counter == 0)
      {
        // all responses are received now
        // the port and ass. prop. sets will be deleted from administration here
        // and must then be destructed by moving it to the garbage collector. 
        list<string> propSetsToDelete;
        GPAPropertySet* pPropSet(0);
        for (TPropertySets::iterator iter = _propertySets.begin();
             iter != _propertySets.end(); ++iter)
        {
          pPropSet = iter->second;
          assert(pPropSet);
          if (pPropSet->mayDelete())
          {
            // add the scope of the prop. set to be deleted
            propSetsToDelete.push_back(iter->first); 
            _propertySetGarbage.push_back(iter->second);
          }
        }
        for (list<string>::iterator iter = propSetsToDelete.begin();
             iter != propSetsToDelete.end(); ++iter)
        {
          _propertySets.erase(*iter);
        }
        LOG_INFO("Closing sequence step 2: Involved prop. sets are disabled. Add to garbage.");
        if (!_synchronous)
        {
          deletePort(*pPort);
        }
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
  else
  {
    // normal responses
    if (pPort->isConnected())
    {     
      pPort->send(e);
    }
    if (e.signal == PA_SCOPE_UNREGISTERED)
    {
      // prop. set is now disabled in a normal way and may be deleted
      // this is only possible by moving it to the garbage collector
      PAUnregisterScopeEvent request(*pEvent);
      GPAPropertySet* pPropSet = findPropSet(request.scope);
      _propertySets.erase(request.scope);
      _propertySetGarbage.push_back(pPropSet);
    }
    doNextRequest();
  }
}

GPAPropertySet* GPAController::findPropSet(const string& scope) const
{
  TPropertySets::const_iterator iter = _propertySets.find(scope);  
  return (iter != _propertySets.end() ? iter->second : 0);
}

void GPAController::acceptConnectRequest(GCFPortInterface& p)
{
  if (&p == &_pmlPortProvider)
  {
    GCFTCPPort* pNewPMLPort = new GCFTCPPort();
    assert(pNewPMLPort);
    pNewPMLPort->init(*this, "tcp-pa-client", GCFPortInterface::SPP, PA_PROTOCOL);
    _pmlPortProvider.accept(*pNewPMLPort);
  }
  else
  {
    GCFPVSSPort* pNewPVSSClientPort = new GCFPVSSPort();
    assert(pNewPVSSClientPort);
    pNewPVSSClientPort->init(*this, "pvss-pa-client", GCFPortInterface::SPP, PA_PROTOCOL);
    _distPmlPortProvider.accept(*pNewPVSSClientPort);
  }
}

void GPAController::clientPortGone(GCFPortInterface& p)
{
  // there are a number of steps of what must be done before the closed port can 
  // be deleted too
  // - since the server MCA communicates with the PA by means of TCP/IP and 
  //   the client MCA by means of PVSS DP's the closing sequence can be split up
  //   into 2 parts:
  //   1) closing conn. with server MCA:
  //      - pretend that these prop. sets receives an unregister scope event from the
  //      remote application
  //      - search all prop. sets, which are managed by the remote application 
  //      connected via the closed port (server MCA/owner)
  //   2) closing conn. with client MCA:
  //      predent that the PA receives an unload request from the remote applictaion
  //      for all prop. sets which are loaded by this app.
  //     
  LOG_INFO("----A PA client is gone, so start the closing sequence!");  
  LOG_INFO("Closing sequence step 1: Server MCA or client MCA?");  
  if (p.getName() == "tcp-pa-client")
  {
    LOG_INFO("Closing sequence step 1: It is a server MCA!!!");  
    GPAPropertySet* pPropSet(0);
    _counter = 0;
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
    LOG_INFO("Closing sequence step 2: Start disabling involved prop. sets");  
    
    // as long as this member is true the sendAndNext method may not finish the 
    // the closing sequence with 'deletePort'
    _synchronous = true;    
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
    _synchronous = false;
    if (_counter == 0)
    {
      LOG_INFO("Closing sequence step 2: No (more) prop. sets needed to be disabled!");
      // no prop. set needed to be disabled
      // move to last step, which deletes the port
      deletePort(p);
    }
  }
  else
  {
    LOG_INFO("Closing sequence step 1: It is a client MCA!!!");  
    propSetClientGone(p);
  }
}

void GPAController::propSetClientGone(GCFPortInterface& p)
{
  // in case the port points to a client MCA this is what must be done before 
  // the closed port can be deleted too
  // - find the first prop. set who knows the client port as prop. set client
  // - delete the client port from prop. set, which results in an unload request
  //   with sequence 0 (see GPA_PropertySet.cc::clientGone)

  LOG_INFO("Closing sequence step 2: Find (next) prop. set who knows port as client");
  string nextPropSet("");
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
    // no more prop. set were involved in this action
    // move to last step, which deletes the port
    deletePort(p);
  }
}

void GPAController::deletePort(GCFPortInterface& p)
{
  LOG_INFO("Closing sequence step 3: Add port to garbage");  
  // In this second step the closed port will be moved to the garbage collector
  // and removes all events, which are received from this port and still in the request 
  // manager
  // and continues with the next event if exist
  _pmlPortGarbage.push_back(&p);
  _pmlPorts.remove(&p);
  _requestManager.deleteRequestsOfPort(p);
  LOG_INFO("----End of closing sequence");  

  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  _isBusy = false;

  if (pPort) // new request available?
  {
    _isRegistered = true;
    dispatch(*pEvent, *pPort);
  }
}

void GPAController::emptyGarbage()
{
  // starts with emptiing the garbages 
  GCFPortInterface* pPort(0);
  for (list<GCFPortInterface*>::iterator iter = _pmlPortGarbage.begin();
       iter != _pmlPortGarbage.end(); ++iter)
  {
    pPort = *iter;
    delete pPort;
  }
  _pmlPortGarbage.clear();
  
  GPAPropertySet* pPropSet(0);
  for (list<GPAPropertySet*>::iterator iter = _propertySetGarbage.begin();
       iter != _propertySetGarbage.end(); ++iter)
  {
    pPropSet = *iter;
    delete pPropSet;
  }
  _propertySetGarbage.clear();
}

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

#define CHECK_REQUEST(p, e)  \
  if (!mayContinue(e, p)) break;

static string sPATaskName("PA");

GPAController::GPAController() : 
  GCFTask((State)&GPAController::initial, sPATaskName),
  _isBusy(false),
  _isRegistered(false),
  _counter(0)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port
  _pmlPortProvider.init(*this, "server", GCFPortInterface::MSPP, PA_PROTOCOL);
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
      TRAN(GPAController::connected);
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

GCFEvent::TResult GPAController::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED:      
      if (&p == &_pmlPortProvider)
      {
        //TODO: find out this can realy happend
      }
      else
      {
        p.close();
      }
      break;
    
    case F_CLOSED:
    {
      CHECK_REQUEST(p, e)
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
      _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) &p);
      break;
    }
    case F_CONNECTED:   
      _pmlPorts.push_back(&p);
      break;

    case PA_LOAD_PROP_SET:
    {
      CHECK_REQUEST(p, e)
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
      CHECK_REQUEST(p, e)
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
      CHECK_REQUEST(p, e)
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
      CHECK_REQUEST(p, e)
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
        pPropSet->enable(request);
      }
      break;
    }
    case PA_UNREGISTER_SCOPE:
    {
      CHECK_REQUEST(p, e)
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
    case PA_PROP_SET_LINKED:
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
    }
    case F_ACCEPT_REQ:
    {
      GCFTCPPort* pNewPMLPort = new GCFTCPPort();
      pNewPMLPort->init(*this, "pa", GCFPortInterface::SPP, PA_PROTOCOL);
      _pmlPortProvider.accept(*pNewPMLPort);
      break;
    }    
    case F_TIMER:
    {
      if (&p == &_pmlPortProvider)
      {
        GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);
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
    _counter--; // counter was set in connected method (signal F_CLOSED)
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
          _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) pPropSet);
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

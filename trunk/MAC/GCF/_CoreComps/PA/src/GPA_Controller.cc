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
#include <stdio.h>
#include <Utils.h>
#include <PA_Protocol.ph>

#define CHECK_REQUEST(p, e)  \
  if (!mayContinue(e, p)) break;

static string sPATaskName("PA");

GPAController::GPAController() : 
  GCFTask((State)&GPAController::initial, sPATaskName),
  _usecountManager(*this),
  _scopeManager(*this),
  _isBusy(false),
  _isRegistered(false)
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
        _scopeManager.deleteAllScopes();
        _requestManager.deleteAllRequests();
        _usecountManager.deleteAllProperties();
        if (!_usecountManager.waitForAsyncResponses())
        {
          TRAN(GPAController::initial);
        }
        //TODO: all SPP's has to be closed to
        //TODO: don't try to reconnect before the _scopeManager 
        //      and the _usecountManager busy
      }
      else
      {
        p.close();
      }
      break;
    
    case F_CLOSED:
    {
      CHECK_REQUEST(p, e)
      list<string> deletedScopes;
      list<string> subScopes;
      _scopeManager.deleteScopesByPort(p, deletedScopes);

      for (list<string>::iterator iter = deletedScopes.begin();
           iter != deletedScopes.end(); ++iter)
      {
        _scopeManager.getSubScopes(*iter, subScopes);
        _usecountManager.deletePropertiesByScope(*iter, subScopes);
      }
      if (deletedScopes.size() == 0 || !_usecountManager.waitForAsyncResponses())
      {
        allPropertiesDeletedByScope();
      }
      break;
    }
    case F_CONNECTED:   
      _pmlPorts.push_back(&p);
      break;

    case PA_LOAD_APC:
    {
      CHECK_REQUEST(p, e)
      loadAPC(e);
      break;
    }
    case PA_UNLOAD_APC:
    {
      CHECK_REQUEST(p, e)
      unloadAPC(e);
      break;
    }
    case PA_RELOAD_APC:
    {
      CHECK_REQUEST(p, e)
      reloadAPC(e);
      break;
    }
    case PA_REGISTER_SCOPE:
    {
      PARegisterScopeEvent request(e);
      LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
          "PML-REQ: Register scope %s",
          request.scope.c_str()));
      PAScopeRegisteredEvent response;
      response.result = _scopeManager.registerScope(request.scope, p);
      response.scope = request.scope;
      p.send(response);
      break;
    }
    case PA_UNREGISTER_SCOPE:
    {
      CHECK_REQUEST(p, e)
      unregisterScope(e);
      break;
    }
    case PA_PROPERTIES_LINKED:
      propertiesLinked(e);      
      break;
    case PA_PROPERTIES_UNLINKED:
      propertiesUnlinked(e);
      break;
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
        GCFTimerEvent* pTimer = static_cast<GCFTimerEvent*>(&e);
        GCFPortInterface* pPort = (GCFPortInterface*)(pTimer->arg);
        if (pPort)
        {
          _pmlPorts.remove(pPort);
          delete pPort;
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
    _curRequestPort = &p;
    _curResult = PA_NO_ERROR;
    result = true;
  }
  return result;
}

void GPAController::propertiesCreated(list<string>& propList)
{
  TPAResult result = _scopeManager.linkProperties(propList);
  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;        
  if (!_scopeManager.waitForAsyncResponses())
    apcLoaded(result);
}

void GPAController::propertiesDeleted(list<string>& propList)
{
  TPAResult result = _scopeManager.unlinkProperties(propList);
  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;        
  if (!_scopeManager.waitForAsyncResponses())
    apcUnloaded(result);
}

void GPAController::allPropertiesDeletedByScope()
{
  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  GCFEvent* pEvent = _requestManager.getOldestRequest();
    
  if (pPort)
  {
    if (pEvent->signal == F_CLOSED)
    {
      _requestManager.deleteRequestsOfPort(*pPort);
      _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) pPort);
      pPort = _requestManager.getOldestRequestPort();
      pEvent = _requestManager.getOldestRequest();
      _isBusy = false;
      
      if (pPort) // new action available?
      {
        _isRegistered = true;
        dispatch(*pEvent, *pPort);
      }
    }
    else if (pEvent->signal == PA_UNREGISTER_SCOPE)
    {      
      if (pPort->isConnected())
      {
        PAUnregisterScopeEvent request(*pEvent);

        PAScopeUnregisteredEvent response;
        response.result = _curResult;
        response.scope = request.scope;
        _isBusy = true;     
        pPort->send(response);
      }
      doNextRequest();
    }
  }
}

void GPAController::allPropertiesDeleted()
{
  TRAN(GPAController::initial);
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

void GPAController::loadAPC(GCFEvent& e)
{
  const list<TAPCProperty>* propsFromAPC;
  PALoadApcEvent request(e);
  _curApcName = request.name;
  _curScope = request.scope;
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Load APC %s with scope %s",
      request.name.c_str(), request.scope.c_str()));
  _apcFileReader.readFile(request.name, request.scope);
  propsFromAPC = &_apcFileReader.getProperties();
  _usecountManager.incrementUsecount(*propsFromAPC);
  if (!_usecountManager.waitForAsyncResponses())
    apcLoaded(_curResult);
}  

void GPAController::apcLoaded(TPAResult result)
{  
  const list<TAPCProperty>* pPropsFromAPC;
  pPropsFromAPC = &_apcFileReader.getProperties();

  GCFEvent* pRequestEvent = _requestManager.getOldestRequest();
  assert(pRequestEvent);
  PALoadApcEvent request(*pRequestEvent);

  if (request.loadDefaults && result == PA_NO_ERROR )
    result = _usecountManager.setDefaults(*pPropsFromAPC);

  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;
  
  PAApcLoadedEvent response;
  response.seqnr = request.seqnr;
  response.result = _curResult;
  sendAPCActionResponse(response);
}  

void GPAController::unloadAPC(GCFEvent& e)
{
  const list<TAPCProperty>* pPropsFromAPC;
  PAUnloadApcEvent request(e);
  _curApcName = request.name;
  _curScope = request.scope;
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Unload APC %s with scope %s",
      request.name.c_str(), request.scope.c_str()));
  _apcFileReader.readFile(request.name, request.scope);
  pPropsFromAPC = &_apcFileReader.getProperties();
  _usecountManager.decrementUsecount(*pPropsFromAPC);
  if (!_usecountManager.waitForAsyncResponses())
    apcUnloaded(_curResult);
}  

void GPAController::apcUnloaded(TPAResult result)
{  
  GCFEvent* pRequestEvent = _requestManager.getOldestRequest();
  assert(pRequestEvent);
  PAUnloadApcEvent request(*pRequestEvent);
  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;
  PAApcUnloadedEvent response;
  response.seqnr = request.seqnr;
  response.result = _curResult;
  sendAPCActionResponse(response);
}  

void GPAController::reloadAPC(GCFEvent& e)
{
  const list<TAPCProperty>* pPropsFromAPC;
  PAReloadApcEvent request(e);
  _curApcName = request.name;
  _curScope = request.scope;
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Reload APC %s with scope %s",
      request.name.c_str(), request.scope.c_str()));
  _apcFileReader.readFile(request.name, request.scope);
  pPropsFromAPC = &_apcFileReader.getProperties();

  TPAResult result = _usecountManager.setDefaults(*pPropsFromAPC);

  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;

  PAApcReloadedEvent response;
  response.seqnr = request.seqnr;
  response.result = _curResult;
  sendAPCActionResponse(response);
}  

void GPAController::unregisterScope(GCFEvent& e)
{
  PAUnregisterScopeEvent request(e);

  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Unregister scope %s",
      request.scope.c_str()));
  list<string> subScopes;
  _scopeManager.getSubScopes(request.scope, subScopes);
  _usecountManager.deletePropertiesByScope(request.scope, subScopes);
  _scopeManager.unregisterScope(request.scope);
  if (!_usecountManager.waitForAsyncResponses())
    allPropertiesDeletedByScope();
}

void GPAController::propertiesLinked(GCFEvent& e)
{
  PAPropertiesLinkedEvent response(e);

  if (response.result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = response.result;
  
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-RESP: Properties linked on scope %s",
      response.scope.c_str()));
  _scopeManager.propertiesLinked(response.scope);
}

void GPAController::propertiesUnlinked(GCFEvent& e)
{
  PAPropertiesUnlinkedEvent response(e);

  if (response.result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = response.result;
  
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-RESP: Properties unlinked on scope %s",
      response.scope.c_str()));
  _scopeManager.propertiesUnlinked(response.scope);
}

void GPAController::sendAPCActionResponse(GCFEvent& e)
{
  if (_curRequestPort)
  {
    if (_curRequestPort->isConnected())
    {
      _curRequestPort->send(e);
    }
  }
  doNextRequest();
}

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
#define DECLARE_SIGNAL_NAMES
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
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _pmlPortProvider.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(GPAController::connected);
      break;

    case F_DISCONNECTED_SIG:
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
    case F_DISCONNECTED_SIG:      
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
    
    case F_CLOSED_SIG:
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
    case F_CONNECTED_SIG:   
      _pmlPorts.push_back(&p);
      break;

    case PA_LOADAPC:
    {
      CHECK_REQUEST(p, e)
      loadAPC((char*)(&e) + sizeof(PALoadapcEvent));
      break;
    }
    case PA_UNLOADAPC:
    {
      CHECK_REQUEST(p, e)
      unloadAPC((char*)(&e) + sizeof(PAUnloadapcEvent));
      break;
    }
    case PA_RELOADAPC:
    {
      CHECK_REQUEST(p, e)
      reloadAPC((char*)(&e) + sizeof(PAReloadapcEvent));
      break;
    }
    case PA_REGISTERSCOPE:
    {
      PARegisterscopeEvent* pRequest = static_cast<PARegisterscopeEvent*>(&e);
      assert(pRequest);
      string scope;
      unsigned int scopeDataLength = Utils::unpackString((char*)(&e) + sizeof(PARegisterscopeEvent), scope);
      LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
          "PML-REQ: Register scope %s",
          scope.c_str()));
      PAScoperegisteredEvent response(pRequest->seqnr, PA_NO_ERROR);
      response.result = _scopeManager.registerScope(scope, p);
      response.length += scopeDataLength;
      p.send(response, (char*)(&e) + sizeof(PARegisterscopeEvent), scopeDataLength);
      break;
    }
    case PA_UNREGISTERSCOPE:
    {
      CHECK_REQUEST(p, e)
      unregisterScope((char*)(&e) + sizeof(PAUnregisterscopeEvent));
      break;
    }
    case PA_PROPERTIESLINKED:
    {
      PAPropertieslinkedEvent* pResponse = static_cast<PAPropertieslinkedEvent*>(&e);
      assert(pResponse);
      if (pResponse->result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
        _curResult = pResponse->result;
      propertiesLinked((char*)(&e) + sizeof(PAPropertieslinkedEvent));      
      break;
    }
    case PA_PROPERTIESUNLINKED:
    {
      PAPropertiesunlinkedEvent* pResponse = static_cast<PAPropertiesunlinkedEvent*>(&e);
      assert(pResponse);
      if (pResponse->result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
        _curResult = pResponse->result;
      propertiesUnlinked((char*)(&e) + sizeof(PAPropertiesunlinkedEvent));      
      break;
    }
    case F_ACCEPT_REQ_SIG:
    {
      GCFTCPPort* pNewPMLPort = new GCFTCPPort();
      pNewPMLPort->init(*this, "pa", GCFPortInterface::SPP, PA_PROTOCOL);
      _pmlPortProvider.accept(*pNewPMLPort);
      break;
    }
    
    case F_TIMER_SIG:
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
    if (pEvent->signal == F_CLOSED_SIG)
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
    else if (pEvent->signal == PA_UNREGISTERSCOPE)
    {      
      if (pPort->isConnected())
      {
        PAUnregisterscopeEvent* pRequest = static_cast<PAUnregisterscopeEvent*>(pEvent);
        assert(pRequest);
        PAScopeunregisteredEvent response(pRequest->seqnr, _curResult);
        // reuse the request data (scope) for the response
        unsigned short bufLength(pRequest->length - sizeof(PAUnregisterscopeEvent));
        char* buffer = ((char*) pRequest) + sizeof(PAUnregisterscopeEvent);
        _isBusy = true;     
        response.length += bufLength; 
        pPort->send(response, buffer, bufLength);     
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

void GPAController::loadAPC(char* actionData)
{
  const list<TAPCProperty>* propsFromAPC;
  unpackAPCActionData(actionData);
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Load APC %s with scope %s",
      _curApcName.c_str(), _curScope.c_str()));
  _apcFileReader.readFile(_curApcName, _curScope);
  propsFromAPC = &_apcFileReader.getProperties();
  _usecountManager.incrementUsecount(*propsFromAPC);
  if (!_usecountManager.waitForAsyncResponses())
    apcLoaded(_curResult);
}  

void GPAController::apcLoaded(TPAResult result)
{  
  const list<TAPCProperty>* pPropsFromAPC;
  pPropsFromAPC = &_apcFileReader.getProperties();

  GCFEvent* pEvent = _requestManager.getOldestRequest();
  assert(pEvent);
  PALoadapcEvent* pLoadapcE = static_cast<PALoadapcEvent*> (pEvent);
  assert(pLoadapcE);

  if (pLoadapcE->loadDefaults && result == PA_NO_ERROR )
    result = _usecountManager.setDefaults(*pPropsFromAPC);

  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;
  
  PAApcloadedEvent e(pLoadapcE->seqnr, _curResult);
  sendAPCActionResponse(e);
}  

void GPAController::unloadAPC(char* actionData)
{
  const list<TAPCProperty>* pPropsFromAPC;
  unpackAPCActionData(actionData);
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Unload APC %s with scope %s",
      _curApcName.c_str(), _curScope.c_str()));
  _apcFileReader.readFile(_curApcName, _curScope);
  pPropsFromAPC = &_apcFileReader.getProperties();
  _usecountManager.decrementUsecount(*pPropsFromAPC);
  if (!_usecountManager.waitForAsyncResponses())
    apcUnloaded(_curResult);
}  

void GPAController::apcUnloaded(TPAResult result)
{  
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  PAUnloadapcEvent* pUnloadapcE = static_cast<PAUnloadapcEvent*> (pEvent);
  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;
  PAApcunloadedEvent e(pUnloadapcE->seqnr, _curResult);
  sendAPCActionResponse(e);
}  

void GPAController::reloadAPC(char* actionData)
{
  const list<TAPCProperty>* pPropsFromAPC;
  unpackAPCActionData(actionData);
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Reload APC %s with scope %s",
      _curApcName.c_str(), _curScope.c_str()));
  _apcFileReader.readFile(_curApcName, _curScope);
  pPropsFromAPC = &_apcFileReader.getProperties();

  TPAResult result = _usecountManager.setDefaults(*pPropsFromAPC);

  if (result != PA_NO_ERROR && _curResult == PA_NO_ERROR) 
    _curResult = result;

  GCFEvent* pEvent = _requestManager.getOldestRequest();
  PAReloadapcEvent* pReloadapcE = static_cast<PAReloadapcEvent*> (pEvent);
  PAApcreloadedEvent e(pReloadapcE->seqnr, _curResult);
  sendAPCActionResponse(e);
}  

void GPAController::unregisterScope(char* pScopeData)
{
  string scope;
  Utils::unpackString(pScopeData, scope);

  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-REQ: Unregister scope %s",
      scope.c_str()));
  list<string> subScopes;
  _scopeManager.getSubScopes(scope, subScopes);
  _usecountManager.deletePropertiesByScope(scope, subScopes);
  _scopeManager.unregisterScope(scope);
  if (!_usecountManager.waitForAsyncResponses())
    allPropertiesDeletedByScope();
}

void GPAController::propertiesLinked(char* pResponseData)
{
  string scope;
  Utils::unpackString(pResponseData, scope);
  
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-RESP: Properties linked on scope %s",
      scope.c_str()));
  _scopeManager.propertiesLinked(scope);
}

void GPAController::propertiesUnlinked(char* pResponseData)
{
  string scope;
  Utils::unpackString(pResponseData, scope);
  
  LOFAR_LOG_INFO(PA_STDOUT_LOGGER, ( 
      "PML-RESP: Properties unlinked on scope %s",
      scope.c_str()));
  _scopeManager.propertiesUnlinked(scope);
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

void GPAController::unpackAPCActionData(char* pActionData)
{
  unsigned int dataLength = Utils::unpackString(pActionData, _curApcName);
  Utils::unpackString(pActionData + dataLength, _curScope);
}

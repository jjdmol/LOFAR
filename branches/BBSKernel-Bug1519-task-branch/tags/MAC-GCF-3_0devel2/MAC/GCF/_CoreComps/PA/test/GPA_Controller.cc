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

#include "GPA_Controller.h"
#include <stdio.h>
#define DECLARE_SIGNAL_NAMES
#include "PA_Protocol.ph"

#define CHECK_REQUEST(p, e) \
  _requestManager.registerRequest(p, e); \
  if (!_isBusy) \
  { \
    _isBusy = true; \
    _curRequestPort = &p; \
  } \
  else \
    break;

static string sPATaskName("PA");

GPAController::GPAController() : 
  GCFTask((State)&GPAController::initial, sPATaskName),
  _usecountManager(*this),
  _scopeManager(*this),
  _isBusy(false)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port
  _pmlPortProvider.init(*this, "server", GCFPortInterface::MSPP, PA_PROTOCOL);
}

GPAController::~GPAController()
{
}

int GPAController::initial(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _pmlPortProvider.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(&GPAController::connected);
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

int GPAController::connected(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED_SIG:      
      if (&p == &_pmlPortProvider)
      {
        _scopeManager.deleteAllScopes();
        _requestManager.deleteAllRequests();
        _usecountManager.deleteAllProperties();
        if (!_usecountManager.waitForAsyncResponses())
          TRAN(&GPAController::initial);
        //TODO: all SPP's has to be closed to
        //TODO: don't try to reconnect before the _scopeManager 
        //      and the _usecountManager busy
      }
      else
      {
        CHECK_REQUEST(p, e)
        list<string> deletedScopes;
        list<string> subScopes;
        _scopeManager.deleteScopesByPort(p, deletedScopes);

        _counter = 0;
        
        for (list<string>::iterator iter = deletedScopes.begin();
             iter != deletedScopes.end(); ++iter)
        {
          _scopeManager.getSubScopes(*iter, subScopes);
          if (_usecountManager.deletePropertiesByScope(*iter, subScopes))
            _counter++;
        }
        if (deletedScopes.size() == 0 || _counter == 0)
        {
          _requestManager.deleteRequestsOfPort(p);
        }
      }
      break;

    case F_CONNECTED_SIG:   
      _pmlPorts.push_back(&p);
      break;

    case PA_LOADAPC:
    {
      CHECK_REQUEST(p, e)
      PALoadapcEvent* request = static_cast<PALoadapcEvent*>(&e);
      loadAPC((char*)request + sizeof(PALoadapcEvent));
      break;
    }
    case PA_UNLOADAPC:
    {
      CHECK_REQUEST(p, e)
      PAUnloadapcEvent* request = static_cast<PAUnloadapcEvent*>(&e);
      unloadAPC((char*)request + sizeof(PAUnloadapcEvent));
      break;
    }
    case PA_RELOADAPC:
    {
      CHECK_REQUEST(p, e)
      PAReloadapcEvent* request = static_cast<PAReloadapcEvent*>(&e);
      reloadAPC((char*)request + sizeof(PAReloadapcEvent));
      break;
    }
    case PA_REGISTERSCOPE:
    {
      PARegisterscopeEvent* request = static_cast<PARegisterscopeEvent*>(&e);
      char* scopeData((char*)request + sizeof(PARegisterscopeEvent));
      TPAResult result = _scopeManager.registerScope(scopeData, p);
      PAScoperegisteredEvent response(request->seqnr, result);
      unsigned int scopeDataLength(0);
      scopeDataLength = request->length - sizeof(PARegisterscopeEvent);
      response.length += scopeDataLength;
      p.send(response, scopeData, scopeDataLength);
      break;
    }
    case PA_UNREGISTERSCOPE:
    {
      CHECK_REQUEST(p, e)
      PAUnregisterscopeEvent* request = static_cast<PAUnregisterscopeEvent*>(&e);
      unregisterScope((char*)request + sizeof(PAUnregisterscopeEvent));
      break;
    }
    case PA_PROPERTIESLINKED:
    {
      PAPropertieslinkedEvent* response = static_cast<PAPropertieslinkedEvent*>(&e);
      propertiesLinked((char*)response + sizeof(PAPropertieslinkedEvent));      
      break;
    }
    case PA_PROPERTIESUNLINKED:
    {
      PAPropertiesunlinkedEvent* response = static_cast<PAPropertiesunlinkedEvent*>(&e);
      propertiesUnlinked((char*)response + sizeof(PAPropertiesunlinkedEvent));      
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

void GPAController::propertiesCreated(list<string>& propList)
{
  TPAResult result = _scopeManager.linkProperties(propList);
  if (!_scopeManager.waitForAsyncResponses())
    apcLoaded(result);
}

void GPAController::propertiesDeleted(list<string>& propList)
{
  TPAResult result = _scopeManager.unlinkProperties(propList);
  if (!_scopeManager.waitForAsyncResponses())
    apcUnloaded(result);
}

void GPAController::allPropertiesDeletedByScope()
{
  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  GCFEvent* pEvent = _requestManager.getOldestRequest();
    
  if (pPort)
  {
    if (pEvent->signal == F_DISCONNECTED_SIG)
    {
      _counter--;
      if (_counter == 0)
      {
        _requestManager.deleteRequestsOfPort(*pPort);
        _pmlPortProvider.setTimer(0, 0, 0, 0, (void*) pPort);
        pPort = _requestManager.getOldestRequestPort();
        pEvent = _requestManager.getOldestRequest();
        _isBusy = false;
        if (pPort) // new action available?
        {
          dispatch(*pEvent, *pPort);
        }
      }        
    }
    else if (pEvent->signal == PA_UNREGISTERSCOPE)
    {      
      if (pPort->isConnected())
      {
        PAUnregisterscopeEvent* request = static_cast<PAUnregisterscopeEvent*>(pEvent);
        
        PAScopeunregisteredEvent response(request->seqnr, 0);
        // reuse the request data (scope) for the response
        unsigned short bufLength(request->length - sizeof(PAUnregisterscopeEvent));
        char* buffer = ((char*) request) + sizeof(PAUnregisterscopeEvent);
        _isBusy = true;      
        pPort->send(response, buffer, bufLength);     
      }
    }
  }
  doNextRequest();
}

void GPAController::allPropertiesDeleted()
{
  TRAN(&GPAController::initial);
}

void GPAController::doNextRequest()
{
  _requestManager.deleteOldestRequest();
  GCFPortInterface* pPort = _requestManager.getOldestRequestPort();
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  _isBusy = false;
  if (pPort) // new action available ?
  {
    dispatch(*pEvent, *pPort);
  }
}

void GPAController::loadAPC(char* actionData)
{
  const list<TAPCProperty>* propsFromAPC;
  unpackAPCActionData(actionData);
  _apc.load(_curApcName, _curScope);
  propsFromAPC = &_apc.getProperties();
  TPAResult result = _usecountManager.incrementUsecount(*propsFromAPC);
  if (!_usecountManager.waitForAsyncResponses())
    apcLoaded(result);
}  

void GPAController::apcLoaded(TPAResult result)
{  
  const list<TAPCProperty>* propsFromAPC;
  propsFromAPC = &_apc.getProperties();

  if (result == PA_NO_ERROR)
  {
    result = _usecountManager.setDefaults(*propsFromAPC);
  } 
#ifdef PML
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  PALoadapcEvent* pLoadapcE = static_cast<PALoadapcEvent*> (pEvent);
  assert(pLoadapcE);
  PAApcloadedEvent e(pLoadapcE->seqnr, result);
  sendAPCActionResponse(e);
#endif
}  

void GPAController::unloadAPC(char* actionData)
{
  const list<TAPCProperty>* propsFromAPC;
  unpackAPCActionData(actionData);
  _apc.load(_curApcName, _curScope);
  propsFromAPC = &_apc.getProperties();
  TPAResult result = _usecountManager.decrementUsecount(*propsFromAPC);    
  if (!_usecountManager.waitForAsyncResponses())
    apcUnloaded(result);
}  

void GPAController::apcUnloaded(TPAResult result)
{  
  GCFEvent* pEvent = _requestManager.getOldestRequest();
  assert(pEvent);
  PAUnloadapcEvent* pUnloadapcE = static_cast<PAUnloadapcEvent*> (pEvent);
  assert(pUnloadapcE);
  PAApcunloadedEvent e(pUnloadapcE->seqnr, result);
  sendAPCActionResponse(e);
}  

void GPAController::reloadAPC(char* actionData)
{
  const list<TAPCProperty>* propsFromAPC;
  unpackAPCActionData(actionData);
  _apc.load(_curApcName, _curScope);
  propsFromAPC = &_apc.getProperties();

  TPAResult result = _usecountManager.setDefaults(*propsFromAPC);

  GCFEvent* pEvent = _requestManager.getOldestRequest();
  assert(pEvent);
  PAReloadapcEvent* pReloadapcE = static_cast<PAReloadapcEvent*> (pEvent);
  assert(pReloadapcE);
  PAApcreloadedEvent e(pReloadapcE->seqnr, result);
  sendAPCActionResponse(e);
}  

void GPAController::unregisterScope(char* pScopeData)
{
  unsigned int scopeNameLength(0);
  sscanf(pScopeData, "%03x", &scopeNameLength);
  string scope(pScopeData + 3, scopeNameLength);
  list<string> subScopes;
  _scopeManager.getSubScopes(scope, subScopes);
  _usecountManager.deletePropertiesByScope(scope, subScopes);
  _scopeManager.unregisterScope(scope);
}

void GPAController::propertiesLinked(char* pResponseData)
{
  TPAResult result;
  string scope;
  Utils::unpackString(pResponseData, scope);
  //TODO: remove props from SCADA DB if returned as not linked
  if ((result = _scopeManager.propertiesLinked(scope)) != PA_NO_ERROR)
  {
    apcLoaded(result);
  }
}

void GPAController::propertiesUnlinked(char* pResponseData)
{
  TPAResult result;
  string scope;
  Utils::unpackString(pResponseData, scope);
  if ((result = _scopeManager.propertiesUnlinked(scope)) != PA_NO_ERROR)
  {
    apcUnloaded(result);
  }
}

void GPAController::sendAPCActionResponse(GCFEvent& e)
{
  if (_curRequestPort)
  {
    if (_curRequestPort->isConnected())
    {
      unsigned short bufLength(_curApcName.size() + _curScope.size() + 6);
      char* buffer = new char[bufLength + 1];
      sprintf(buffer, "%03x%s%03x%s", _curApcName.size(), _curApcName.c_str(), 
                                      _curScope.size(),   _curScope.c_str()   );
      e.length += bufLength;
      _curRequestPort->send(e, buffer, bufLength);
      delete [] buffer;      
    }
  }
  doNextRequest();
}

void GPAController::sendUnLinkActionResponse(GCFEvent& e)
{
  if (_curRequestPort)
  {
    if (_curRequestPort->isConnected())
    {
      unsigned short bufLength(_curApcName.size() + _curScope.size() + 6);
      char* buffer = new char[bufLength + 1];
      sprintf(buffer, "%03x%s%03x%s", _curApcName.size(), _curApcName.c_str(), 
                                      _curScope.size(),   _curScope.c_str()   );
      e.length += bufLength;
      _curRequestPort->send(e, buffer, bufLength);
      delete [] buffer;      
    }
  }
  doNextRequest();
}

void GPAController::unpackAPCActionData(char* pActionData)
{
  unsigned int apcNameLength(0);
  sscanf(pActionData, "%03x", &apcNameLength);
  _curApcName.assign(pActionData + 3, apcNameLength);
  char* pScopeData = pActionData + 3 + apcNameLength;
  unsigned int scopeNameLength(0);
  sscanf(pScopeData, "%03x", &scopeNameLength);
  _curScope.assign(pScopeData + 3, scopeNameLength);
}

void GPAController::loadAPCTest()
{
  const list<TAPCProperty>* propsFromAPC;
  _apc.load("/home/mueller/workspace/LOFAR/MAC/GCF/CoreComps/PA/src/apc1.xml", "receptor1");
  propsFromAPC = &_apc.getProperties();
  TPAResult result = _usecountManager.incrementUsecount(*propsFromAPC);
  if (!_usecountManager.waitForAsyncResponses())
    result = _usecountManager.setDefaults(*propsFromAPC);
}

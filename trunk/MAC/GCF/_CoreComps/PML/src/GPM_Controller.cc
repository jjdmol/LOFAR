//#  GPM_Controller.cc: 
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

#include <GPM_Controller.h>
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>
#include <stdio.h>
#include <Utils.h>
#include <PA_Protocol.ph>

static string sPMLTaskName("PML");
GPMController* GPMController::_pInstance = 0;

extern void logResult(TPAResult result, GCFApc& apc);

GPMController::GPMController() :
  GCFTask((State)&GPMController::initial, sPMLTaskName),
  _counter(0)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port
  _propertyAgent.init(*this, "client", GCFPortInterface::SAP, PA_PROTOCOL);
}

GPMController::~GPMController()
{
}

GPMController* GPMController::instance()
{
  if (0 == _pInstance)
  {
    _pInstance = new GPMController();
    _pInstance->start();
  }

  return _pInstance;
}

TPMResult GPMController::loadAPC(GCFApc& apc, bool loadDefaults)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    unsigned short seqnr = getFreeSeqnrForApcRequest();
    _apcList[seqnr] = &apc; 
    
    PALoadApcEvent request;
    request.seqnr = seqnr;
    request.loadDefaults = loadDefaults;
    request.name = apc.getName();
    request.scope = apc.getScope();
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

TPMResult GPMController::unloadAPC(GCFApc& apc)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    unsigned short seqnr = getFreeSeqnrForApcRequest();
    _apcList[seqnr] = &apc; 
  
    PAUnloadApcEvent request;
    request.seqnr = seqnr;
    request.name = apc.getName();
    request.scope = apc.getScope();
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

TPMResult GPMController::reloadAPC(GCFApc& apc)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    unsigned short seqnr = getFreeSeqnrForApcRequest();
    _apcList[seqnr] = &apc; 
    
    PAReloadApcEvent request;
    request.seqnr = seqnr;
    request.name = apc.getName();
    request.scope = apc.getScope();
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

void GPMController::unregisterAPC (const GCFApc& apc)
{
  for (TApcList::iterator iter = _apcList.begin();
       iter != _apcList.end(); ++iter)
  {
    if (iter->second == &apc)
    {
      _apcList.erase(iter);
      break;
    }
  }
}

unsigned short GPMController::getFreeSeqnrForApcRequest() const
{
  unsigned short seqnr(0);
  TApcList::const_iterator iter;
  do   
  {
    seqnr++;
    iter = _apcList.find(seqnr);
  } while (iter != _apcList.end());

  return seqnr;
}

TPMResult GPMController::registerScope(GCFMyPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  TPropertySets::iterator iter = _propertySets.find(propSet.getScope());
  if (iter != _propertySets.end())
  {
    result = PM_SCOPE_ALREADY_EXISTS;
  }
  else
  {
    _propertySets[propSet.getScope()] = &propSet;

    if (_propertyAgent.isConnected())
    {
      PARegisterScopeEvent request;
      request.scope = propSet.getScope();
      _propertyAgent.send(request);
    }
  }
  return result;
}

TPMResult GPMController::unregisterScope(GCFMyPropertySet& propSet, 
                                         bool permanent)
{
  TPMResult result(PM_NO_ERROR);
 
  // prop set could be unloaded in case of destruction of the propSet
  // calls this method
  if (propSet.isLoaded() && _propertyAgent.isConnected())
  {
    PAUnregisterScopeEvent request;
    request.scope = propSet.getScope();
    _propertyAgent.send(request);
  }
  if (permanent)
  {
    _propertySets.erase(propSet.getScope());
  }

  return result;
}

void GPMController::propertiesLinked(const string& scope, TPAResult result)
{
  _counter--;
  LOFAR_LOG_DEBUG(PML_STDOUT_LOGGER, ( 
      "Link request %d counter", _counter));
  if (_propertyAgent.isConnected())
  {
    PAPropertiesLinkedEvent response;
    response.result = result;
    response.scope = scope;
    _propertyAgent.send(response);
  }
}

void GPMController::propertiesUnlinked(const string& scope, TPAResult result)
{
  if (_propertyAgent.isConnected())
  {
    PAPropertiesUnlinkedEvent response;
    response.result = result;
    response.scope = scope;
    _propertyAgent.send(response);
  }
}

GCFEvent::TResult GPMController::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _propertyAgent.open();
      break;

    case F_CONNECTED:
      TRAN(GPMController::connected);
      break;

    case F_DISCONNECTED:
      _propertyAgent.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPMController::connected(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  TGCFResult result;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
          "Connection lost to Property Agent"));
      TRAN(GPMController::initial);
      break;

    case F_ENTRY:
    {
      PARegisterScopeEvent regRequest;
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        regRequest.scope = iter->first;
        _propertyAgent.send(regRequest);
      }
      PALoadApcEvent apcRequest;
      apcRequest.loadDefaults = false;
      GCFApc* pApc;
      for (TApcList::iterator iter = _apcList.begin();
           iter != _apcList.end(); ++iter)
      {
        pApc = iter->second;
        assert(pApc);
        apcRequest.seqnr = iter->first;
        apcRequest.loadDefaults = pApc->mustLoadDefaults();
        apcRequest.name = pApc->getName();
        apcRequest.scope = pApc->getScope();
        _propertyAgent.send(apcRequest);
      }
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {
      PAScopeRegisteredEvent response(e);
      if (response.result == PA_SCOPE_ALREADY_REGISTERED)
      {
        LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
            "A property set with scope %s already exists in the system",
            response.scope.c_str()));        
      }
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSLOAD_ERROR);      
      GCFMyPropertySet* pPropertySet = _propertySets[response.scope];
      if (result != GCF_NO_ERROR)
      {
        _propertySets.erase(response.scope);
      }
      if (pPropertySet)
      {
        pPropertySet->scopeRegistered(result);
      }
      break;
    }

    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent response(e);
      GCFMyPropertySet* pPropertySet = _propertySets[response.scope];
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSUNLOAD_ERROR);
      if (pPropertySet)
      {
        _propertySets.erase(response.scope);
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PA_LINK_PROPERTIES:
    {
      PALinkPropertiesEvent request(e);
      list<string> propertyList;
      Utils::getPropertyListFromString(propertyList, request.propList);
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PA-REQ: Link properties %s on scope %s",
        request.propList.c_str(),
        request.scope.c_str()));
        
      GCFMyPropertySet* pPropertySet = _propertySets[request.scope];
      if (pPropertySet)
      {
        if (_counter == 0)
        {
          _propertyAgent.setTimer(0, 0, 0, 0);
        }
        _counter++;        
        pPropertySet->linkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_TRACE(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropertiesLinkedEvent response;
        response.result = PA_PROP_SET_GONE;
        response.scope = request.scope;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_UNLINK_PROPERTIES:
    {
      PAUnlinkPropertiesEvent request(e);
      list<string> propertyList;
      Utils::getPropertyListFromString(propertyList, request.propList);
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PA-REQ: Unlink properties %s on scope %s",
        request.propList.c_str(), 
        request.scope.c_str()));
      GCFMyPropertySet* pPropertySet = _propertySets[request.scope];
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_TRACE(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropertiesUnlinkedEvent response;
        response.result = PA_PROP_SET_GONE;
        response.scope = request.scope;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_APC_LOADED:
    {
      PAApcLoadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_APCLOAD_ERROR);        
      GCFApc* pApc = _apcList[response.seqnr];
      if (pApc)
      {
        logResult(response.result, *pApc);
        _apcList.erase(response.seqnr);
        pApc->loaded(result);
      }
      break;
    }
    case PA_APC_UNLOADED:
    {
      PAApcUnloadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_APCUNLOAD_ERROR);        
      GCFApc* pApc = _apcList[response.seqnr];
      if (pApc)
      {
        logResult(response.result, *pApc);
        _apcList.erase(response.seqnr);
        pApc->unloaded(result);
      }
      break;
    }
    case PA_APC_RELOADED:
    {
      PAApcReloadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_APCRELOAD_ERROR);        
      GCFApc* pApc = _apcList[response.seqnr];
      if (pApc)
      {
        logResult(response.result, *pApc);
        _apcList.erase(response.seqnr);
        pApc->reloaded(result);
      }
      break;
    }
    case F_TIMER:
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        iter->second->retryLinking();
      }
      if (_counter > 0)
      {
        _propertyAgent.setTimer(0.0);
      }
      break;      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void logResult(TPAResult result, GCFApc& apc)
{
  switch (result)
  {
    case PA_NO_ERROR:
      break;
    case PA_UNKNOWN_ERROR:
      LOFAR_LOG_FATAL(PML_STDOUT_LOGGER, ( 
          "Unknown error"));      
      break;
    case PA_PROP_SET_GONE:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "One of the property sets are gone while perfom an apc action. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_MISSING_PROPS:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "One or more loaded properties are not owned by any application. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_PROP_NOT_VALID:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "One or more loaded properties could not be mapped on a registered scope. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_UNABLE_TO_LOAD_APC:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "Troubles during loading the APC file. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_NO_TYPE_SPECIFIED_IN_APC:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "APC file: On one or more of the found properties no type is specified. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_SAL_ERROR:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "APC file: Error while reading the property value. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_MACTYPE_UNKNOWN:
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "APC file: Unknown property type specified. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    default:
      break;
  }
}

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

#include "GPM_Controller.h"
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/Utils.h>
#include <PA_Protocol.ph>

#include <stdio.h>

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

TPMResult GPMController::loadPropSet(GCFPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    PALoadPropSetEvent request;
    request.seqnr = getFreeSeqnrForPSRequest(propSet);
    request.scope = propSet.getScope();
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

TPMResult GPMController::unloadPropSet(GCFPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    PAUnloadPropSetEvent request;
    request.seqnr = getFreeSeqnrForPSRequest(propSet);
    request.scope = propSet.getScope();
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

TPMResult GPMController::configurePropSet(GCFPropertySet& propSet, const string& apcName)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    PAConfPropSetEvent request;
    request.seqnr = getFreeSeqnrForPSRequest(propSet);
    request.scope = propSet.getScope();
    request.apcName = apcName;
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

void GPMController::deletePropSet(GCFPropertySet& propSet)
{
  for (TPropertySets::iterator iter = _actionSeqList.begin();
       iter != _actionSeqList.end(); ++iter)
  {
    if (iter->second == &propSet)
    {
      _actionSeqList.erase(iter);
      break;
    }
  }
}

TPMResult GPMController::registerScope(GCFPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  TPropertySets::iterator iter = _propertySets.find(propSet.getScope());
  if (iter != _propertySets.end())
  {
    result = PM_SCOPE_ALREADY_EXISTS;
  }
  else
  {
    _propertySets[propSet.getScope()] = pPropertySet;
    if (_propertyAgent.isConnected())
    {
      PARegisterScopeEvent request;
      request.seqnr = getFreeSeqnrForPSRequest(propSet);
      request.scope = propSet.getScope();
      request.type = propSet.getType();
      request.isTemporary = propSet.isTemporary();
      _propertyAgent.send(request);
    }
  }
  return result;
}

TPMResult GPMController::unregisterScope(GCFPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
 
  if (_propertyAgent.isConnected())
  {
    PAUnregisterScopeEvent request;
    request.seqnr = getFreeSeqnrForPSRequest(propSet);
    request.scope = propSet.getScope();
    _propertyAgent.send(request);
  }
  _propertySets.erase(propSet.getScope());

  return result;
}

unsigned short GPMController::getFreeSeqnrForPSRequest(GCFPropertySet& propSet) const
{
  unsigned short seqnr(0);
  TActionSeqList::const_iterator iter;
  do   
  {
    seqnr++;
    iter = _actionSeqList.find(seqnr);
  } while (iter != _actionSeqList.end());

  _actionSeqList[seqnr] = &propSet; 

  return seqnr;
}

void GPMController::propertiesLinked(TPAResult result)
{
  _counter--;
  LOG_DEBUG(LOFAR::formatString ( 
      "Link request %d counter", _counter));
  if (_propertyAgent.isConnected())
  {
    PAPropSetLinkedEvent response;
    response.result = result;
    _propertyAgent.send(response);
  }
}

void GPMController::propertiesUnlinked(TPAResult result)
{
  if (_propertyAgent.isConnected())
  {
    PAPropSetUnlinkedEvent response;
    response.result = result;
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
      LOG_WARN(LOFAR::formatString ( 
          "Connection lost to Property Agent"));
      TRAN(GPMController::initial);
      break;

    case F_ENTRY:
    {
      PARegisterScopeEvent regRequest;
      GCFPropertySet* pPropertySet(0);
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        pPropertySet = iter->second;
        assert(pPropertySet);
        regRequest.seqnr = getFreeSeqnrForPSRequest(*pPropertySet);
        regRequest.scope = iter->first;
        regRequest.type = pPropertySet->getType();
        regRequest.isTemporary = pPropertySet->isTemporary();
        _propertyAgent.send(regRequest);
      }
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {
      PAScopeRegisteredEvent response(e);
      if (response.result == PA_SCOPE_ALREADY_REGISTERED)
      {
        LOG_INFO(LOFAR::formatString ( 
            "A property set with scope %s already exists in the system",
            response.scope.c_str()));        
      }
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_ENABLE_ERROR);      
      GCFMyPropertySet* pPropertySet = (GCMyPropertySet*) _actionSeqList[response.seqnr];
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        if (result != GCF_NO_ERROR)
        {
          _propertySets.erase(pPropertySet->getScope());
        }        
        pPropertySet->scopeRegistered(result);
      }
      break;
    }

    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent response(e);
      GCFMyPropertySet* pPropertySet = (GCMyPropertySet*) _actionSeqList[response.seqnr];
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_DISABLE_ERROR);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PA_LINK_PROP_SET:
    {
      PALinkPropSetEvent request(e);
      LOG_INFO(LOFAR::formatString ( 
        "PA-REQ: Link properties of prop. set '%s'",        
        request.scope.c_str()));
        
      GCFMyPropertySet* pPropertySet = (GCMyPropertySet*) _propertySets[request.scope];
      if (pPropertySet)
      {
        if (_counter == 0)
        {
          _propertyAgent.setTimer(0, 0, 0, 0);
        }
        _counter++;        
        pPropertySet->linkProperties();
      }
      else
      {
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropertiesLinkedEvent response;
        response.result = PA_PROP_SET_GONE;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_UNLINK_PROP_SET:
    {
      PAUnlinkPropSetEvent request(e);
      LOG_INFO(LOFAR::formatString ( 
        "PA-REQ: Unlink properties of prop. set '%s'",
        request.scope.c_str()));
      GCFMyPropertySet* pPropertySet = (GCMyPropertySet*) _propertySets[request.scope];
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties();
      }
      else
      {
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropertiesUnlinkedEvent response;
        response.result = PA_PROP_SET_GONE;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_PROP_SET_LOADED:
    {
      PAPropSetLoadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_EXTPS_LOAD_ERROR);        
      GCFExtPropertySet* pPropertySet = (GCExtPropertySet*) _actionSeqList[response.seqnr];
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        logResult(response.result, *pPropertySet);
        pPropertySet->loaded(result);
      }
      break;
    }
    case PA_PROP_SET_UNLOADED:
    {
      PAPropSetUnloadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_EXTPS_UNLOAD_ERROR);        
      GCFExtPropertySet* pPropertySet = (GCExtPropertySet*) _actionSeqList[response.seqnr];
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        logResult(response.result, *pPropertySet);
        pPropertySet->unloaded(result);
      }
      break;
    }
    case PA_PROP_SET_CONF:
    {
      PAPropSetConfEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_PS_CONFIGURE_ERROR);        
      GCFPropertySet* pPropertySet = _actionSeqList[response.seqnr];
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        logResult(response.result, *pPropertySet);
        pPropertySet->configured(result, response.apcName);
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
      LOG_FATAL(LOFAR::formatString ( 
          "Unknown error"));      
      break;
    case PA_PROP_SET_GONE:
      LOG_ERROR(LOFAR::formatString ( 
          "One of the property sets are gone while perfom an apc action. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_MISSING_PROPS:
      LOG_ERROR(LOFAR::formatString ( 
          "One or more loaded properties are not owned by any application. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_PROP_NOT_VALID:
      LOG_ERROR(LOFAR::formatString ( 
          "One or more loaded properties could not be mapped on a registered scope. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_UNABLE_TO_LOAD_APC:
      LOG_ERROR(LOFAR::formatString ( 
          "Troubles during loading the APC file. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_NO_TYPE_SPECIFIED_IN_APC:
      LOG_ERROR(LOFAR::formatString ( 
          "APC file: On one or more of the found properties no type is specified. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_SAL_ERROR:
      LOG_ERROR(LOFAR::formatString ( 
          "APC file: Error while reading the property value. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    case PA_MACTYPE_UNKNOWN:
      LOG_ERROR(LOFAR::formatString ( 
          "APC file: Unknown property type specified. (%s:%s)",
          apc.getName().c_str(), apc.getScope().c_str()));
      break;
    default:
      break;
  }
}

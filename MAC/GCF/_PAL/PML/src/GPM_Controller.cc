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

extern void logResult(TPAResult result, GCFPropertySet& propSet);

GPMController::GPMController() :
  GCFTask((State)&GPMController::initial, sPMLTaskName)
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

TPMResult GPMController::loadPropSet(GCFExtPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    PALoadPropSetEvent request;
    request.seqnr = registerAction(propSet);
    request.scope = propSet.getScope();
    
    _propertyAgent.send(request);
  }
  else
  {
    result = PM_PA_NOTCONNECTED;
  }
  return result;
}

TPMResult GPMController::unloadPropSet(GCFExtPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  
  if (_propertyAgent.isConnected())
  {
    PAUnloadPropSetEvent request;
    request.seqnr = registerAction(propSet);
    request.scope = propSet.getScope();
    
    _extPropertySets.remove(&propSet);
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
    request.seqnr = registerAction(propSet);
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

void GPMController::deletePropSet(const GCFPropertySet& propSet)
{
  for (TActionSeqList::iterator iter = _actionSeqList.begin();
       iter != _actionSeqList.end(); ++iter)
  {
    if (iter->second == &propSet)
    {
      _actionSeqList.erase(iter);
      break;
    }
  }
}

TPMResult GPMController::registerScope(GCFMyPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  TMyPropertySets::iterator iter = _myPropertySets.find(propSet.getScope());
  if (iter != _myPropertySets.end())
  {
    result = PM_SCOPE_ALREADY_EXISTS;
  }
  else
  {
    _myPropertySets[propSet.getScope()] = &propSet;
    if (_propertyAgent.isConnected())
    {
      PARegisterScopeEvent request;
      request.seqnr = registerAction(propSet);
      request.scope = propSet.getScope();
      request.type = propSet.getType();
      request.isTemporary = propSet.isTemporary();
      _propertyAgent.send(request);
    }
  }
  return result;
}

TPMResult GPMController::unregisterScope(GCFMyPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
 
  if (_propertyAgent.isConnected())
  {
    PAUnregisterScopeEvent request;
    request.seqnr = registerAction(propSet);
    request.scope = propSet.getScope();
    _propertyAgent.send(request);
  }
  _myPropertySets.erase(propSet.getScope());

  return result;
}

unsigned short GPMController::registerAction(GCFPropertySet& propSet)
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

void GPMController::propertiesLinked(const string& scope, TPAResult result)
{
  _propertyAgent.cancelAllTimers();
  if (_propertyAgent.isConnected())
  {
    PAPropSetLinkedEvent response;
    response.result = result;
    response.scope = scope;
    _propertyAgent.send(response);
  }
}

void GPMController::propertiesUnlinked(const string& scope, TPAResult result)
{
  if (_propertyAgent.isConnected())
  {
    PAPropSetUnlinkedEvent response;
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
  static unsigned long linkTimer = 0;

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
      for (TMyPropertySets::iterator iter = _myPropertySets.begin();
           iter != _myPropertySets.end(); ++iter)
      {
        pPropertySet = iter->second;
        assert(pPropertySet);
        regRequest.seqnr = registerAction(*pPropertySet);
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
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_ENABLE_ERROR);      
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*) _actionSeqList[response.seqnr];
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        logResult(response.result, *pPropertySet);
        if (result != GCF_NO_ERROR)
        {
          _myPropertySets.erase(pPropertySet->getScope());
        }        
        pPropertySet->scopeRegistered(result);
      }
      break;
    }

    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent response(e);
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*) _actionSeqList[response.seqnr];
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
        
      GCFMyPropertySet* pPropertySet = _myPropertySets[request.scope];
      if (pPropertySet)
      {
        if (!pPropertySet->linkProperties())
        {
          _propertyAgent.setTimer(0, 0, 0, 0, pPropertySet);
          linkTimer = _propertyAgent.setTimer(2, 0, 0, 0, pPropertySet);
        }
      }
      else
      {
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropSetLinkedEvent response;
        response.result = PA_PS_GONE;
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
      GCFMyPropertySet* pPropertySet = _myPropertySets[request.scope];
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties();
      }
      else
      {
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropSetUnlinkedEvent response;
        response.result = PA_PS_GONE;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_PROP_SET_LOADED:
    {
      PAPropSetLoadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_EXTPS_LOAD_ERROR);        
      GCFExtPropertySet* pPropertySet = (GCFExtPropertySet*) _actionSeqList[response.seqnr];
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        if (result == GCF_NO_ERROR)
        {
          _extPropertySets.push_back(pPropertySet);
        }
        logResult(response.result, *pPropertySet);
        pPropertySet->loaded(result);
      }
      break;
    }
    case PA_PROP_SET_UNLOADED:
    {
      PAPropSetUnloadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_EXTPS_UNLOAD_ERROR);        
      GCFExtPropertySet* pPropertySet = (GCFExtPropertySet*) _actionSeqList[response.seqnr];
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
    case PA_PROP_SET_GONE:
    {
      PAPropSetGoneEvent indication(e);
      GCFExtPropertySet* pPropertySet;
      for (TExtPropertySets::iterator iter = _extPropertySets.begin();
           iter != _extPropertySets.end(); ++iter)
      {
        pPropertySet = *iter;
        assert(pPropertySet);
        if (pPropertySet->getScope() == indication.scope && pPropertySet->isLoaded())
        {
          pPropertySet->serverIsGone();
        }
      }
      break;
    }
    case F_TIMER:
    {
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);      
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*)(pTimer->arg);
      if (linkTimer == pTimer->id)
      {
        LOG_ERROR(LOFAR::formatString(
            "link request could not be handled in time by the property set server (%s)",
            pPropertySet->getScope().c_str()));
        propertiesLinked(pPropertySet->getScope(), PA_LINK_TIME_OUT);
      }
      else
      {
        if (!pPropertySet->tryLinking())
        {
          _propertyAgent.setTimer(0, 0, 0, 0, pPropertySet);
        }
        else
        {
          _propertyAgent.cancelTimer(linkTimer);
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

void logResult(TPAResult result, GCFPropertySet& propSet)
{
  switch (result)
  {
    case PA_NO_ERROR:
      break;
    case PA_UNKNOWN_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Unknown error"));      
      break;
    case PA_PS_GONE:
      LOG_ERROR(LOFAR::formatString ( 
          "The property set is gone while perfoming an action on it. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_MISSING_PROPS:
      LOG_ERROR(LOFAR::formatString ( 
          "One or more loaded properties are not owned by any application. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_WRONG_STATE:
      LOG_FATAL(LOFAR::formatString ( 
          "The my property set is in a wrong state. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_PROP_SET_NOT_EXISTS:
      LOG_INFO(LOFAR::formatString ( 
          "Prop. set does not exists. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_PROP_SET_ALLREADY_EXISTS:
      LOG_INFO(LOFAR::formatString ( 
          "Prop. set allready exists. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_DPTYPE_UNKNOWN:
      LOG_INFO(LOFAR::formatString ( 
          "Specified type not known. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_INTERNAL_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Internal error in PA. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_APC_NOT_EXISTS:
      LOG_ERROR(LOFAR::formatString ( 
          "APC not exists. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_LINK_TIME_OUT:
      LOG_ERROR(LOFAR::formatString ( 
          "Linking of the prop. set could not be completed in time (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PA_SERVER_GONE:
      LOG_INFO(LOFAR::formatString ( 
          "Server of prop. set is gone (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;      
    default:
      break;
  }
}

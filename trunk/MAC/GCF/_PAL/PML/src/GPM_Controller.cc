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
#include <GCF/ParameterSet.h>
#include <Resources.hxx>
#include <GCF/PAL/GCF_PVSSInfo.h>

#include <stdio.h>

static string sPMLTaskName("GCF-PML");
GPMHandler* GPMHandler::_pInstance = 0;

void logResult(TPAResult result, GCFPropertySet& propSet);

GPMController::GPMController() :
  GCFTask((State)&GPMController::initial, sPMLTaskName)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port
  _propertyAgent.init(*this, "client", GCFPortInterface::SAP, PA_PROTOCOL);

  _distPropertyAgent.init(*this, "__gcf_DPA_client", GCFPortInterface::SAP, PA_PROTOCOL);
  
  _distPropertyAgent.setConverter(_converter);
  
  ParameterSet::instance()->adoptFile("gcf-pml.conf");
  ParameterSet::instance()->adoptFile("PropertyAgent.conf");
}

GPMController::~GPMController()
{
}

GPMController* GPMController::instance(bool temporary)
{
  if (0 == GPMHandler::_pInstance)
  {    
    GPMHandler::_pInstance = new GPMHandler();
    assert(!GPMHandler::_pInstance->mayDeleted());
    GPMHandler::_pInstance->_controller.start();
  }
  if (!temporary) GPMHandler::_pInstance->use();
  return &GPMHandler::_pInstance->_controller;
}

void GPMController::release()
{
  assert(GPMHandler::_pInstance);
  assert(!GPMHandler::_pInstance->mayDeleted());
  GPMHandler::_pInstance->leave(); 
  if (GPMHandler::_pInstance->mayDeleted())
  {
    delete GPMHandler::_pInstance;
    assert(!GPMHandler::_pInstance);
  }
}

TPMResult GPMController::loadPropSet(GCFExtPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
  
  string destPA = determineDest(propSet.getScope());
  if (checkDestination(destPA))
  {
    PALoadPropSetEvent request;
  
    TAction action;
    action.pPropSet = &propSet;
    action.signal = request.signal;

    request.seqnr = registerAction(action);
    
    if (_distPropertyAgent.isConnected())
    {
      request.scope += propSet.getScope();
      string::size_type index = request.scope.find(':');
      if (index < request.scope.length())
      {
        request.scope.erase(0, index + 1);
      }    
      
      _distPropertyAgent.setDestAddr(destPA);
      _distPropertyAgent.send(request);
    }
  }
  else
  {
    result = PM_PA_NOT_REACHABLE;
  }
  return result;
}

TPMResult GPMController::unloadPropSet(GCFExtPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
 
  string destPA = determineDest(propSet.getScope());
  if (checkDestination(destPA))
  {
    PAUnloadPropSetEvent request;
  
    TAction action;
    action.pPropSet = &propSet;
    action.signal = request.signal;
  
    request.seqnr = registerAction(action);
  
  
    if (_distPropertyAgent.isConnected())
    {
      request.scope += propSet.getScope();
      string::size_type index = request.scope.find(':');
      if (index < request.scope.length())
      {
        request.scope.erase(0, index + 1);
      }
  
      _distPropertyAgent.setDestAddr(destPA);
      _distPropertyAgent.send(request);
  
      _extPropertySets.remove(&propSet);
    }
  }
  else
  {
    result = PM_PA_NOT_REACHABLE;
  }
  return result;
}

TPMResult GPMController::configurePropSet(GCFPropertySet& propSet, const string& apcName)
{
  TPMResult result(PM_NO_ERROR);
   
  string destPA = determineDest(propSet.getScope());
  if (checkDestination(destPA))
  {
    PAConfPropSetEvent request;
    
    TAction action;
    action.pPropSet = &propSet;
    action.signal = request.signal;
    action.apcName = apcName;
    
    request.seqnr = registerAction(action);
  
    if (_distPropertyAgent.isConnected())
    {
      request.scope += propSet.getScope();
      string::size_type index = request.scope.find(':');
      if (index < request.scope.length())
      {
        request.scope.erase(0, index + 1);
      }
      request.apcName = apcName;
  
      _distPropertyAgent.setDestAddr(destPA);
      _distPropertyAgent.send(request);
    }
  }
  else
  {
    result = PM_PA_NOT_REACHABLE;
  }
  return result;
}

void GPMController::deletePropSet(const GCFPropertySet& propSet)
{
  TAction* pAction;
  for (TActionSeqList::iterator iter = _actionSeqList.begin();
       iter != _actionSeqList.end(); ++iter)
  {
    pAction = &iter->second;
    if (pAction->pPropSet == &propSet)
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
    PARegisterScopeEvent request;
  
    TAction action;
    action.pPropSet = &propSet;
    action.signal = request.signal;
    
    request.seqnr = registerAction(action);
    
    if (_propertyAgent.isConnected())
    {
      _myPropertySets[propSet.getScope()] = &propSet;
      request.scope = propSet.getScope();
      request.type = propSet.getType();
      request.category = propSet.getCategory();
      _propertyAgent.send(request);
    }
  }
  return result;
}

TPMResult GPMController::unregisterScope(GCFMyPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
 
  PAUnregisterScopeEvent request;

  TAction action;
  action.pPropSet = &propSet;
  action.signal = request.signal;
  
  request.seqnr = registerAction(action);
    
  if (_propertyAgent.isConnected())
  {
    request.scope = propSet.getScope();
    _propertyAgent.send(request);
  }
  _myPropertySets.erase(propSet.getScope());

  return result;
}

unsigned short GPMController::registerAction(TAction& action)
{
  unsigned short seqnr(1); // 0 is reserved for internal msg. in PA
  TActionSeqList::const_iterator iter;
  do   
  {
    seqnr++;
    iter = _actionSeqList.find(seqnr);
  } while (iter != _actionSeqList.end());

  _actionSeqList[seqnr] = action; 

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

GCFEvent::TResult GPMController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      if (!_propertyAgent.isConnected())
      {
        _propertyAgent.open();
      }
      if (!_distPropertyAgent.isConnected())
      {
        _distPropertyAgent.open();
      }
      break;
    
    case F_TIMER:
      p.open();
      break;

    case F_CONNECTED:
      if (_propertyAgent.isConnected() && _distPropertyAgent.isConnected())
      {
        TRAN(GPMController::connected);
      }
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPMController::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  TGCFResult result;
  static unsigned long linkTimer = 0;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_WARN(LOFAR::formatString ( 
          "Connection lost to Property Agent"));
      p.close();
      break;
      
    case F_CLOSED:
      TRAN(GPMController::initial);
      break;
      
    case F_ENTRY:
    {
      TAction* pAction(0);
      TActionSeqList tmpSeqList(_actionSeqList);
      _actionSeqList.clear();
      for (TActionSeqList::iterator iter = tmpSeqList.begin();
           iter != tmpSeqList.end(); ++iter)
      {
        pAction = &iter->second;
        switch (pAction->signal)
        {
          case PA_REGISTER_SCOPE: registerScope(* (GCFMyPropertySet*)pAction->pPropSet); break;
          case PA_UNREGISTER_SCOPE: unregisterScope(*(GCFMyPropertySet*)pAction->pPropSet); break;
          case PA_CONF_PROP_SET: configurePropSet(*pAction->pPropSet, pAction->apcName); break;
          case PA_LOAD_PROP_SET: loadPropSet(*(GCFExtPropertySet*)pAction->pPropSet); break;
          case PA_UNLOAD_PROP_SET: unloadPropSet(*(GCFExtPropertySet*)pAction->pPropSet); break;
          default: assert(0);
        }
      }
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {
      PAScopeRegisteredEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_ENABLE_ERROR);
      TAction* pAction = &_actionSeqList[response.seqnr];
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*) pAction->pPropSet;
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
      TAction* pAction = &_actionSeqList[response.seqnr];
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*) pAction->pPropSet;
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
      TAction* pAction = &_actionSeqList[response.seqnr];
      GCFExtPropertySet* pPropertySet = (GCFExtPropertySet*) pAction->pPropSet;
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
      TAction* pAction = &_actionSeqList[response.seqnr];
      GCFExtPropertySet* pPropertySet = (GCFExtPropertySet*) pAction->pPropSet;
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
      TAction* pAction = &_actionSeqList[response.seqnr];
      GCFPropertySet* pPropertySet = pAction->pPropSet;
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
      string fullScope;
      for (TExtPropertySets::iterator iter = _extPropertySets.begin();
           iter != _extPropertySets.end(); ++iter)
      {
        pPropertySet = *iter;
        assert(pPropertySet);
        fullScope = pPropertySet->getScope();
        if (fullScope.find(':') >= fullScope.length())
        {
          fullScope = GCFPVSSInfo::getLocalSystemName() + ":" + fullScope;
        }
        if (fullScope.find(indication.scope) == 0 && 
            pPropertySet->isLoaded())
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
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_MISSING_PROPS:
      LOG_ERROR(LOFAR::formatString ( 
          "One or more loaded properties are not owned by any application. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_WRONG_STATE:
      LOG_FATAL(LOFAR::formatString ( 
          "The my property set is in a wrong state. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_PROP_SET_NOT_EXISTS:
      LOG_INFO(LOFAR::formatString ( 
          "Prop. set does not exists. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_PROP_SET_ALREADY_EXISTS:
      LOG_INFO(LOFAR::formatString ( 
          "Prop. set allready exists. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_DPTYPE_UNKNOWN:
      LOG_INFO(LOFAR::formatString ( 
          "Specified type not known. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_INTERNAL_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Internal error in PA. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_PI_INTERNAL_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Internal error in PI. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_APC_NOT_EXISTS:
      LOG_ERROR(LOFAR::formatString ( 
          "APC not exists. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_LINK_TIME_OUT:
      LOG_ERROR(LOFAR::formatString ( 
          "Linking of the prop. set could not be completed in time (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PA_SERVER_GONE:
      LOG_INFO(LOFAR::formatString ( 
          "Server of prop. set is gone (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;      
    default:
      break;
  }
}

string GPMController::determineDest(const string& scope) const
{
  string::size_type index = scope.find(':');
  string destDP("__gcf_DPA_server");
  if (index < scope.length())
  {
    destDP.insert(0, scope.c_str(), index + 1);
  }
  return destDP;
}

bool GPMController::checkDestination(const string& destAddr) const
{
  if (!GCFPVSSInfo::propExists(destAddr))
  {
    string destPA = destAddr;
    string::size_type pos = destPA.find(':');
    if (pos < destPA.length())
    {
      destPA.erase(pos, destPA.length());
    }
    else
    {
      destPA = GCFPVSSInfo::getLocalSystemName();
    }
    LOG_ERROR(formatString(
        "PA on system %s not reachable!",
        destPA.c_str()));
    return false;
  }
  else
  {
    return true;
  }
}

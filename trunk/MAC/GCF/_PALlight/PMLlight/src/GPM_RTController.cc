//#  GPM_RTController.cc: 
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

#include <GPM_RTController.h>
#include "GCF_RTMyPropertySet.h"
#include <stdio.h>
#include <Utils.h>
#include <PI_Protocol.ph>

static string sPMLTaskName("PMLlite");
GPMRTController* GPMRTController::_pInstance = 0;

extern void logResult(TPAResult result, GCFRTMyPropertySet& propSet);

GPMRTController::GPMRTController() :
  GCFTask((State)&GPMRTController::initial, sPMLTaskName)
{
  // register the protocol for debugging purposes
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);

  // initialize the port
  _propertyInterface.init(*this, "client", GCFPortInterface::SAP, PI_PROTOCOL);
}

GPMRTController::~GPMRTController()
{
}

GPMRTController* GPMRTController::instance()
{
  if (0 == _pInstance)
  {
    _pInstance = new GPMRTController();
    _pInstance->start();
  }

  return _pInstance;
}

void GPMController::deletePropSet(const GCFRTMyPropertySet& propSet)
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

TPMResult GPMController::registerScope(GCFRTMyPropertySet& propSet)
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
    if (_propertyInterface.isConnected())
    {
      PIRegisterScopeEvent request;
      request.seqnr = registerAction(propSet);
      request.scope = propSet.getScope();
      request.type = propSet.getType();
      request.isTemporary = propSet.isTemporary();
      _propertyInterface.send(request);
    }
  }
  return result;
}

TPMResult GPMController::unregisterScope(GCFRTMyPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
 
  if (_propertyInterface.isConnected())
  {
    PIUnregisterScopeEvent request;
    request.seqnr = registerAction(propSet);
    request.scope = propSet.getScope();
    _propertyInterface.send(request);
  }
  _myPropertySets.erase(propSet.getScope());

  return result;
}

unsigned short GPMController::registerAction(GCFRTMyPropertySet& propSet)
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

void GPMController::propertiesLinked(const string& scope,                            
                                     list<string>& propsToSubscribe, 
                                     TPAResult result)
{
  _propertyInterface.cancelAllTimers();
  if (_propertyInterface.isConnected())
  {
    PIPropSetLinkedEvent response;
    response.result = result;
    response.scope = scope;
    Utils::getPropertyListString(responseOut.propList, propsToSubscribe);
    _propertyInterface.send(response);
  }
}

void GPMController::propertiesUnlinked(const string& scope, 
                                       TPAResult result)
{
  if (_propertyInterface.isConnected())
  {
    PIPropSetUnlinkedEvent response;
    response.result = result;
    response.scope = scope;
    _propertyInterface.send(response);
  }
}

void GPMRTController::valueSet(const string& propName, const GCFPValue& value)
{
  if (_propertyInterface.isConnected())
  {        
    PIValueSetEvent indicationOut;
    indicationOut.name = propName;
    indicationOut.value._pValue = &value;
    _propertyInterface.send(indicationOut);
  }
}

GCFEvent::TResult GPMRTController::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      _propertyInterface.open();
      break;

    case F_CONNECTED:
      TRAN(GPMRTController::connected);
      break;

    case F_DISCONNECTED:
      _propertyInterface.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPMRTController::connected(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  TGCFResult result;
  
  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_WARN(LOFAR::formatString ( 
          "Connection lost to Supervisory Server"));
      TRAN(GPMRTController::initial);
      break;

    case F_ENTRY:
    {
      PIRegisterScopeEvent regRequest;
      GCFRTMyPropertySet* pPropertySet(0);
      for (TMyPropertySets::iterator iter = _myPropertySets.begin();
           iter != _myPropertySets.end(); ++iter)
      {
        pPropertySet = iter->second;
        assert(pPropertySet);
        regRequest.seqnr = registerAction(*pPropertySet);
        regRequest.scope = iter->first;
        regRequest.type = pPropertySet->getType();
        regRequest.isTemporary = pPropertySet->isTemporary();
        _propertyInterface.send(regRequest);
      }
      break;
    }  
    case PI_SCOPE_REGISTERED:
    {
      PIScopeRegisteredEvent response(e);
      result = (response.result == PI_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_ENABLE_ERROR);      
      GCFRTMyPropertySet* pPropertySet = (GCFRTMyPropertySet*) _actionSeqList[response.seqnr];
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

    case PI_SCOPE_UNREGISTERED:
    {
      PIScopeUnregisteredEvent response(e);
      GCFRTMyPropertySet* pPropertySet = (GCFRTMyPropertySet*) _actionSeqList[response.seqnr];
      result = (response.result == PI_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_DISABLE_ERROR);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PI_LINK_PROP_SET:
    {
      PILinkPropSetEvent request(e);
      LOG_INFO(LOFAR::formatString ( 
        "PA-REQ: Link properties of prop. set '%s'",        
        request.scope.c_str()));
        
      GCFRTMyPropertySet* pPropertySet = _myPropertySets[request.scope];
      if (pPropertySet)
      {
        pPropertySet->linkProperties();
      }
      else
      {
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropSetLinkedEvent response;
        response.result = PI_PS_GONE;
        _propertyInterface.send(response);
      }
      break;
    }
    case PI_UNLINK_PROP_SET:
    {
      PIUnlinkPropSetEvent request(e);
      LOG_INFO(LOFAR::formatString ( 
        "PA-REQ: Unlink properties of prop. set '%s'",
        request.scope.c_str()));
      GCFRTMyPropertySet* pPropertySet = _myPropertySets[request.scope];
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
        response.result = PI_PS_GONE;
        _propertyInterface.send(response);
      }
      break;
    }
    case PI_VALUE_CHANGED:
    {
      PIValueChangedEvent indicationIn(e);

      LOG_INFO(LOFAR::formatString ( 
          "SS-MSG: Property %s changed", 
          indicationIn.name.c_str()));
      string scope;
      scope.assign(indicationIn.name, 0, indicationIn.scopeLength);
      GCFRTMyPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        assert(indicationIn.value._pValue);
        pPropertySet->valueChanged(indicationIn.name, *indicationIn.value._pValue);
      }
      else
      {
        LOG_DEBUG(LOFAR::formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void logResult(TPIResult result, GCFRTMyPropertySet& propSet)
{
  switch (result)
  {
    case PI_NO_ERROR:
      break;
    case PI_UNKNOWN_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Unknown error"));      
      break;
    case PI_PS_GONE:
      LOG_ERROR(LOFAR::formatString ( 
          "The property set is gone while perfoming an action on it. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_MISSING_PROPS:
      LOG_ERROR(LOFAR::formatString ( 
          "One or more loaded properties are not owned by any application. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_WRONG_STATE:
      LOG_FATAL(LOFAR::formatString ( 
          "The my property set is in a wrong state. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_PROP_SET_NOT_EXISTS:
      LOG_INFO(LOFAR::formatString ( 
          "Prop. set does not exists. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_PROP_SET_ALLREADY_EXISTS:
      LOG_INFO(LOFAR::formatString ( 
          "Prop. set allready exists. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_DPTYPE_UNKNOWN:
      LOG_INFO(LOFAR::formatString ( 
          "Specified type not known. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_INTERNAL_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Internal error in PI. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_PA_INTERNAL_ERROR:
      LOG_FATAL(LOFAR::formatString ( 
          "Internal error in PA. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    case PI_APC_NOT_EXISTS:
      LOG_ERROR(LOFAR::formatString ( 
          "APC not exists. (%s:%s)",
          propSet.getType(), propSet.getScope().c_str()));
      break;
    default:
      break;
  }
}

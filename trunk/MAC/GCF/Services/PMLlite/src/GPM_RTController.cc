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

GPMRTController::GPMRTController() :
  GCFTask((State)&GPMRTController::initial, sPMLTaskName)
{
  // register the protocol for debugging purposes
  registerProtocol(PI_PROTOCOL, PI_PROTOCOL_signalnames);

  // initialize the port
  _supervisoryServer.init(*this, "client", GCFPortInterface::SAP, PI_PROTOCOL);
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

TPMResult GPMRTController::registerScope(GCFRTMyPropertySet& propSet)
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
    
    PIRegisterScopeEvent requestOut;
    requestOut.scope = propSet.getScope();
    sendMsgToPI(requestOut);
  }
  return result;
}

TPMResult GPMRTController::unregisterScope(GCFRTMyPropertySet& propSet, 
                                         bool permanent)
{
  TPMResult result(PM_NO_ERROR);
 
  // prop set could be unloaded in case of destruction of the propSet
  // calls this method
  if (propSet.isLoaded())
  {
    PIUnregisterScopeEvent requestOut;
    requestOut.scope = propSet.getScope();
    sendMsgToPI(requestOut);
  }
  if (permanent)
  {
    _propertySets.erase(propSet.getScope());
  }

  return result;
}

void GPMRTController::propertiesLinked(const string& scope, 
                           list<string>& propsToSubscribe, 
                           TPIResult result)
{
  PIPropertiesLinkedEvent responseOut;
  responseOut.result = result;
  responseOut.scope = scope;
  Utils::getPropertyListString(responseOut.propList, propsToSubscribe);
  sendMsgToPI(responseOut);
}

void GPMRTController::propertiesUnlinked(const string& scope, 
                           list<string>& propsToUnsubscribe, 
                           TPIResult result)
{
  PIPropertiesUnlinkedEvent responseOut;
  responseOut.result = result;
  responseOut.scope = scope;
  Utils::getPropertyListString(responseOut.propList, propsToUnsubscribe);
  sendMsgToPI(responseOut);
}

void GPMRTController::valueSet(const string& propName, const GCFPValue& value)
{
  if (_supervisoryServer.isConnected())
  {        
    PIValueSetEvent indicationOut;
    indicationOut.name = propName;
    indicationOut.value._pValue = &value;
    _supervisoryServer.send(indicationOut);
  }
}

void GPMRTController::sendMsgToPI(GCFEvent& e)
{
  if (_supervisoryServer.isConnected())
  {    
    _supervisoryServer.send(e);
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
      _supervisoryServer.open();
      break;

    case F_CONNECTED:
      TRAN(GPMRTController::connected);
      break;

    case F_DISCONNECTED:
      _supervisoryServer.setTimer(1.0); // try again after 1 second
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
      LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
          "Connection lost to Supervisory Server"));
      TRAN(GPMRTController::initial);
      break;

    case F_ENTRY:
    {
      PIRegisterScopeEvent requestOut;
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        requestOut.scope = iter->first;
        sendMsgToPI(requestOut);
      }
      break;
    }  
    case PI_SCOPE_REGISTERED:
    {
      PIScopeRegisteredEvent responseIn(e);
      if (responseIn.result == PI_SCOPE_ALREADY_REGISTERED)
      {
        LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
            "A property set with scope %s already exists in the system",
            responseIn.scope.c_str()));        
      }
      result = (responseIn.result == PI_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSLOAD_ERROR);      
      GCFRTMyPropertySet* pPropertySet = _propertySets[responseIn.scope];
      if (result != GCF_NO_ERROR)
      {
        _propertySets.erase(responseIn.scope);        
      }
      if (pPropertySet)
      {
        pPropertySet->scopeRegistered(result);
      }
      break;
    }

    case PI_SCOPE_UNREGISTERED:
    {
      PIScopeUnregisteredEvent responseIn(e);
      GCFRTMyPropertySet* pPropertySet = _propertySets[responseIn.scope];
      result = (responseIn.result == PI_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSUNLOAD_ERROR);
      if (pPropertySet)
      {
        _propertySets.erase(responseIn.scope);
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PI_LINK_PROPERTIES:
    {
      PILinkPropertiesEvent requestIn(e);
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PI-REQ: Link properties %s on scope %s",
        requestIn.propList.c_str(),
        requestIn.scope.c_str()));
        
      GCFRTMyPropertySet* pPropertySet = _propertySets[requestIn.scope];
      if (pPropertySet)
      {
        list<string> propertyList;
        Utils::getPropertyListFromString(propertyList, requestIn.propList);
        pPropertySet->linkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_DEBUG(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            requestIn.scope.c_str()));

        PIPropertiesLinkedEvent responseOut;
        responseOut.result = PI_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        sendMsgToPI(responseOut);
      }
      break;
    }
    case PI_UNLINK_PROPERTIES:
    {
      PIUnlinkPropertiesEvent requestIn(e);
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PI-REQ: Unlink properties %s on scope %s",
        requestIn.propList.c_str(), 
        requestIn.scope.c_str()));

      GCFRTMyPropertySet* pPropertySet = _propertySets[requestIn.scope];
      if (pPropertySet)
      {
        list<string> propertyList;
        Utils::getPropertyListFromString(propertyList, requestIn.propList);
        pPropertySet->unlinkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_DEBUG(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            requestIn.scope.c_str()));

        PIPropertiesUnlinkedEvent responseOut;
        responseOut.result = PI_PROP_SET_GONE;
        responseOut.scope = requestIn.scope;
        sendMsgToPI(responseOut);
      }
      break;
    }
    case PI_VALUE_CHANGED:
    {
      PIValueChangedEvent indicationIn(e);

      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
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
        LOFAR_LOG_DEBUG(PML_STDOUT_LOGGER, ( 
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

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
#define DECLARE_SIGNAL_NAMES
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

    GCFEvent e(PI_REGISTERSCOPE);
    sendMyPropSetMsg(e, propSet.getScope());
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
    GCFEvent e(PI_UNREGISTERSCOPE);
    sendMyPropSetMsg(e, propSet.getScope());
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
  PIPropertieslinkedEvent e(result);
  sendMyPropSetMsg(e, scope, propsToSubscribe);
}

void GPMRTController::propertiesUnlinked(const string& scope, 
                           list<string>& propsToUnsubscribe, 
                           TPIResult result)
{
  PIPropertiesunlinkedEvent e(result);
  sendMyPropSetMsg(e, scope, propsToUnsubscribe);
}

void GPMRTController::valueSet(const string& propName, const GCFPValue& value)
{
  if (_supervisoryServer.isConnected())
  {        
    GCFEvent e(PI_VALUESET);
    unsigned int dataLength = Utils::packString(propName, _buffer, MAX_BUF_SIZE);
    dataLength += value.pack(_buffer + dataLength, MAX_BUF_SIZE - dataLength);
    e.length += dataLength;    
    _supervisoryServer.send(e, _buffer, dataLength);
  }
}

void GPMRTController::sendMyPropSetMsg(GCFEvent& e, const string& scope, list<string>& props)
{
  if (_supervisoryServer.isConnected())
  {    
    unsigned short dataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
    dataLength += Utils::packPropertyList(props, _buffer + dataLength, MAX_BUF_SIZE - dataLength);
    e.length += dataLength;
    _supervisoryServer.send(e, _buffer, dataLength);
  }
}

void GPMRTController::sendMyPropSetMsg(GCFEvent& e, const string& scope)
{
  if (_supervisoryServer.isConnected())
  {    
    unsigned short dataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
    e.length += dataLength;
    _supervisoryServer.send(e, _buffer, dataLength);
  }
}

GCFEvent::TResult GPMRTController::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _supervisoryServer.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(GPMRTController::connected);
      break;

    case F_DISCONNECTED_SIG:
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
  static char* pData = 0;
  static string scope = "";
  
  switch (e.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
          "Connection lost to Supervisory Server"));
      TRAN(GPMRTController::initial);
      break;

    case F_ENTRY_SIG:
    {
      GCFEvent rse(PI_REGISTERSCOPE);
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        sendMyPropSetMsg(rse, iter->first);
      }
      break;
    }  
    case PI_SCOPEREGISTERED:
    {
      PIScoperegisteredEvent* pResponse = static_cast<PIScoperegisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PIScoperegisteredEvent);
      Utils::unpackString(pData, scope);
      if (pResponse->result == PI_SCOPE_ALREADY_REGISTERED)
      {
        LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
            "A property set with scope %s already exists in the system",
            scope.c_str()));        
      }
      result = (pResponse->result == PI_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSLOAD_ERROR);      
      GCFRTMyPropertySet* pPropertySet = _propertySets[scope];
      if (result != GCF_NO_ERROR)
      {
        _propertySets.erase(scope);        
      }
      if (pPropertySet)
      {
        pPropertySet->scopeRegistered(result);
      }
      break;
    }

    case PI_SCOPEUNREGISTERED:
    {
      PIScopeunregisteredEvent* pResponse = static_cast<PIScopeunregisteredEvent*>(&e);
      assert(pResponse);
      pData = ((char*)&e) + sizeof(PIScopeunregisteredEvent);
      Utils::unpackString(pData, scope);
      GCFRTMyPropertySet* pPropertySet = _propertySets[scope];
      result = (pResponse->result == PI_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSUNLOAD_ERROR);
      if (pPropertySet)
      {
        _propertySets.erase(scope);
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PI_LINKPROPERTIES:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      unsigned int scopeDataLength = Utils::unpackString(pData, scope);
      string linkListData(pData + scopeDataLength + Utils::SLEN_FIELD_SIZE, 
        e.length - sizeof(GCFEvent) - scopeDataLength - Utils::SLEN_FIELD_SIZE);
      list<string> propertyList;
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PI-REQ: Link properties %s on scope %s",
        linkListData.c_str(),
        scope.c_str()));
      Utils::unpackPropertyList(pData + scopeDataLength, propertyList);
      GCFRTMyPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        pPropertySet->linkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_TRACE(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
        PIPropertieslinkedEvent e(PI_PROP_SET_GONE);
        sendMyPropSetMsg(e, scope);
      }
      break;
    }
    case PI_UNLINKPROPERTIES:
    {
      pData = ((char*)&e) + sizeof(GCFEvent);
      unsigned int scopeDataLength = Utils::unpackString(pData, scope);
      string unlinkListData(pData + scopeDataLength + Utils::SLEN_FIELD_SIZE, 
        e.length - sizeof(GCFEvent) - scopeDataLength - Utils::SLEN_FIELD_SIZE);
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PI-REQ: Unlink properties %s on scope %s",
        unlinkListData.c_str(), 
        scope.c_str()));
      list<string> propertyList;
      Utils::unpackPropertyList(pData + scopeDataLength, propertyList);
      GCFRTMyPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_TRACE(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
        PIPropertiesunlinkedEvent e(PI_PROP_SET_GONE);
        sendMyPropSetMsg(e, scope);
      }
      break;
    }
    case PI_VALUECHANGED:
    {
      PIValuechangedEvent* pMsg = static_cast<PIValuechangedEvent*>(&e);
      assert(pMsg);
      pData = ((char*)&e) + sizeof(PIValuechangedEvent);
      string propName;
      unsigned int propDataLength = Utils::unpackString(pData, propName);
      scope.assign(propName, 0, pMsg->scopeLength);

      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
          "SS-MSG: Property %s changed", 
          propName.c_str()));
    
      GCFRTMyPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        unsigned int valueBufLength = e.length - propDataLength - sizeof(PIValuechangedEvent);        
        GCFPValue* pValue = GCFPValue::unpackValue(pData + propDataLength, valueBufLength);
        assert(pValue);
        pPropertySet->valueChanged(propName, *pValue);
        delete pValue;
      }
      else
      {
        LOFAR_LOG_TRACE(PML_STDOUT_LOGGER, ( 
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

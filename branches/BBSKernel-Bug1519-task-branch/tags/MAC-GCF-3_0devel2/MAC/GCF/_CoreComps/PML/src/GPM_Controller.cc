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
#define DECLARE_SIGNAL_NAMES
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
  memset(_buffer, '0', MAX_BUF_SIZE);
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
  
  unsigned short seqnr = getFreeSeqnrForApcRequest();
  _apcList[seqnr] = &apc; 
  
  PALoadapcEvent e(seqnr, loadDefaults);
  sendAPCRequest(e, apc);

  return result;
}

TPMResult GPMController::unloadAPC(GCFApc& apc)
{
  TPMResult result(PM_NO_ERROR);
  
  unsigned short seqnr = getFreeSeqnrForApcRequest();
  _apcList[seqnr] = &apc; 
  
  PAUnloadapcEvent e(seqnr);
  sendAPCRequest(e, apc);

  return result;
}

TPMResult GPMController::reloadAPC(GCFApc& apc)
{
  TPMResult result(PM_NO_ERROR);
  
  unsigned short seqnr = getFreeSeqnrForApcRequest();
  _apcList[seqnr] = &apc; 
  
  PAReloadapcEvent e(seqnr);
  sendAPCRequest(e, apc);

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

void GPMController::sendAPCRequest(GCFEvent& e, const GCFApc& apc)
{
  if (_propertyAgent.isConnected())
  {
    unsigned int dataLength = Utils::packString(apc.getName(), _buffer, MAX_BUF_SIZE);
    dataLength += Utils::packString(apc.getScope(), _buffer + dataLength, MAX_BUF_SIZE - dataLength);
    e.length += dataLength;
    _propertyAgent.send(e, _buffer, dataLength);
  }
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

    PARegisterscopeEvent e(0);
    sendMyPropSetMsg(e, propSet.getScope());
  }
  return result;
}

TPMResult GPMController::unregisterScope(GCFMyPropertySet& propSet, 
                                         bool permanent)
{
  TPMResult result(PM_NO_ERROR);
 
  // prop set could be unloaded in case of destruction of the propSet
  // calls this method
  if (propSet.isLoaded())
  {
    PAUnregisterscopeEvent e(0);
    sendMyPropSetMsg(e, propSet.getScope());
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
  PAPropertieslinkedEvent e(0, result);
  sendMyPropSetMsg(e, scope);
}

void GPMController::propertiesUnlinked(const string& scope, TPAResult result)
{
  PAPropertiesunlinkedEvent e(0, result);
  sendMyPropSetMsg(e, scope);
}

void GPMController::sendMyPropSetMsg(GCFEvent& e, const string& scope)
{
  if (_propertyAgent.isConnected())
  {
    unsigned int dataLength = Utils::packString(scope, _buffer, MAX_BUF_SIZE);
    e.length += dataLength;
    _propertyAgent.send(e, _buffer, dataLength);
  }
}

GCFEvent::TResult GPMController::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _propertyAgent.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(GPMController::connected);
      break;

    case F_DISCONNECTED_SIG:
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
  static char* pData = 0;
  static string scope = "";

  switch (e.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
          "Connection lost to Property Agent"));
      TRAN(GPMController::initial);
      break;

    case F_ENTRY_SIG:
    {
      PARegisterscopeEvent rse(0);
      for (TPropertySets::iterator iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        sendMyPropSetMsg(rse, iter->first);
      }
      PALoadapcEvent lae(0, false);
      GCFApc* pApc;
      for (TApcList::iterator iter = _apcList.begin();
           iter != _apcList.end(); ++iter)
      {
        pApc = iter->second;
        assert(pApc);
        lae.seqnr = iter->first;
        lae.loadDefaults = pApc->mustLoadDefaults();
        sendAPCRequest(lae, *pApc);
      }
      break;
    }  
    case PA_SCOPEREGISTERED:
    {
      PAScoperegisteredEvent* pResponse = static_cast<PAScoperegisteredEvent*>(&e);
      assert(pResponse);
      pData = (char*)(&e) + sizeof(PAScoperegisteredEvent);
      Utils::unpackString(pData, scope);
      if (pResponse->result == PA_SCOPE_ALREADY_REGISTERED)
      {
        LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
            "A property set with scope %s already exists in the system",
            scope.c_str()));        
      }
      result = (pResponse->result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSLOAD_ERROR);      
      GCFMyPropertySet* pPropertySet = _propertySets[scope];
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

    case PA_SCOPEUNREGISTERED:
    {
      PAScopeunregisteredEvent* pResponse = static_cast<PAScopeunregisteredEvent*>(&e);
      assert(pResponse);
      pData = (char*)(&e) + sizeof(PAScopeunregisteredEvent);
      Utils::unpackString(pData, scope);
      GCFMyPropertySet* pPropertySet = _propertySets[scope];
      result = (pResponse->result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPROPSUNLOAD_ERROR);
      if (pPropertySet)
      {
        _propertySets.erase(scope);
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PA_LINKPROPERTIES:
    {
      pData = (char*)(&e) + sizeof(PALinkpropertiesEvent);
      unsigned int scopeDataLength = Utils::unpackString(pData, scope);
      string linkListData(pData + scopeDataLength + Utils::SLEN_FIELD_SIZE, 
        e.length - sizeof(PALinkpropertiesEvent) - scopeDataLength - Utils::SLEN_FIELD_SIZE);
      list<string> propertyList;
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PA-REQ: Link properties %s on scope %s",
        linkListData.c_str(),
        scope.c_str()));
        
      Utils::unpackPropertyList(pData + scopeDataLength, propertyList);
      GCFMyPropertySet* pPropertySet = _propertySets[scope];
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
            scope.c_str()));
        PAPropertieslinkedEvent e(0, PA_PROP_SET_GONE);
        sendMyPropSetMsg(e, scope);
      }
      break;
    }
    case PA_UNLINKPROPERTIES:
    {
      pData = (char*)(&e) + sizeof(PAUnlinkpropertiesEvent);
      unsigned int scopeDataLength = Utils::unpackString(pData, scope);
      string unlinkListData(pData + scopeDataLength + Utils::SLEN_FIELD_SIZE, 
        e.length - sizeof(PALinkpropertiesEvent) - scopeDataLength - Utils::SLEN_FIELD_SIZE);
      LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PA-REQ: Unlink properties %s on scope %s",
        unlinkListData.c_str(), 
        scope.c_str()));
      list<string> propertyList;
      Utils::unpackPropertyList(pData + scopeDataLength, propertyList);
      GCFMyPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties(propertyList);
      }
      else
      {
        LOFAR_LOG_TRACE(PML_STDOUT_LOGGER, ( 
            "Property set with scope %d was deleted in the meanwhile", 
            scope.c_str()));
        PAPropertiesunlinkedEvent e(0, PA_PROP_SET_GONE);
        sendMyPropSetMsg(e, scope);
      }
      break;
    }
    case PA_APCLOADED:
    {
      PAApcloadedEvent* pResponse = static_cast<PAApcloadedEvent*>(&e);
      assert(pResponse);
      result = (pResponse->result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_APCLOAD_ERROR);        
      GCFApc* pApc = _apcList[pResponse->seqnr];
      if (pApc)
      {
        logResult(pResponse->result, *pApc);
        _apcList.erase(pResponse->seqnr);
        pApc->loaded(result);
      }
      break;
    }
    case PA_APCUNLOADED:
    {
      PAApcunloadedEvent* pResponse = static_cast<PAApcunloadedEvent*>(&e);
      assert(pResponse);
      result = (pResponse->result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_APCUNLOAD_ERROR);      
      GCFApc* pApc = _apcList[pResponse->seqnr];
      if (pApc)
      {
        logResult(pResponse->result, *pApc);
        _apcList.erase(pResponse->seqnr);
        pApc->unloaded(result);
      }
      break;
    }
    case PA_APCRELOADED:
    {
      PAApcreloadedEvent* pResponse = static_cast<PAApcreloadedEvent*>(&e);
      assert(pResponse);
      result = (pResponse->result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_APCRELOAD_ERROR);
      GCFApc* pApc = _apcList[pResponse->seqnr];
      if (pApc)
      {
        logResult(pResponse->result, *pApc);
        _apcList.erase(pResponse->seqnr);
        pApc->reloaded(result);
      }
      break;
    }
    case F_DISPATCHED_SIG:
    case F_TIMER_SIG:
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

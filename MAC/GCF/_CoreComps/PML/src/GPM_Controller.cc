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
#include "GCF_SupTask.h"
#include "GPM_PropertySet.h"
#include <stdio.h>
#define DECLARE_SIGNAL_NAMES
#include "PA_Protocol.ph"

static string sPMLTaskName("PML");

GPMController::GPMController(GCFSupervisedTask& supervisedTask) :
  GCFTask((State)&GPMController::initial, sPMLTaskName),
  _supervisedTask(supervisedTask),
  _scadaService(*this),
  _isBusy(false),
  _preparing(false)
{
  // register the protocol for debugging purposes
  registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

  // initialize the port
  _propertyAgent.init(*this, "pml", GCFPortInterface::SAP, PA_PROTOCOL);
}

GPMController::~GPMController()
{
}

TPMResult GPMController::loadAPC(const string& apcName, const string& scope)
{
  TPMResult result(PM_NO_ERROR);
  
  if (!_propertyAgent.isConnected())
  {
    result = PM_PA_NOTCONNECTED;
  }
  else if (_isBusy)
  {
    result = PM_IS_BUSY;
  }
  else
  {
    PALoadapcEvent e(0);
    unsigned short bufLength(apcName.size() + scope.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", apcName.size(), apcName.c_str(), 
                                    scope.size(),   scope.c_str()   );
    e.length += bufLength;
    _isBusy = true;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
    
  }
  return result;
}

TPMResult GPMController::unloadAPC(const string& apcName, const string& scope)
{
  TPMResult result(PM_NO_ERROR);
  
  if (!_propertyAgent.isConnected())
  {
    result = PM_PA_NOTCONNECTED;
  }
  else if (_isBusy)
  {
    result = PM_IS_BUSY;
  }
  else
  {
    PAUnloadapcEvent e(0);
    unsigned short bufLength(apcName.size() + scope.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", apcName.size(), apcName.c_str(), 
                                    scope.size(),   scope.c_str()   );
    e.length += bufLength;
    _isBusy = true;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
  return result;
}

TPMResult GPMController::reloadAPC(const string& apcName, const string& scope)
{
  TPMResult result(PM_NO_ERROR);
  
  if (!_propertyAgent.isConnected())
  {
    result = PM_PA_NOTCONNECTED;
  }
  else if (_isBusy)
  {
    result = PM_IS_BUSY;
  }
  else
  {
    PAReloadapcEvent e(0);
    unsigned short bufLength(apcName.size() + scope.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", apcName.size(), apcName.c_str(), 
                                    scope.size(),   scope.c_str()   );
    e.length += bufLength;
    _isBusy = true;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
  return result;
}

TPMResult GPMController::loadMyProperties(TPropertySet& newSet)
{
  TPMResult result(PM_NO_ERROR);
  string scope = newSet.scope;
  TPropertySetIter iter = _propertySets.find(scope);
  if (iter->first == scope)
  {
    result = PM_SCOPE_ALREADY_EXISTS;
  }
  else
  {
    GPMPropertySet* pNewPropertySet = new GPMPropertySet(*this, newSet);
    _propertySets[scope] = pNewPropertySet;
    
    if (_propertyAgent.isConnected())
    {
      _isBusy = true;      
      registerScope(scope);
    }
  }
  return result;
}

TPMResult GPMController::unloadMyProperties(const string& scope)
{
  TPMResult result(PM_NO_ERROR);
  
  TPropertySetIter iter = _propertySets.find(scope);
  if (iter->first != scope)
  {
    result = PM_SCOPE_NOT_EXISTS;
  }
  else
  {    
    GPMPropertySet* pNewPropertySet = iter->second;
    delete pNewPropertySet;
    _propertySets.erase(scope);
        
    if (_propertyAgent.isConnected())
    {
      PAUnregisterscopeEvent e(0);
      unsigned short bufLength(scope.size() + 3);
      char* buffer = new char[bufLength + 1];
      sprintf(buffer, "%03x%s", scope.size(), scope.c_str());
      e.length += bufLength;
      _isBusy = true;      
      _propertyAgent.send(e, buffer, bufLength);s
      delete [] buffer;
    }
  }
  return result;
}

TPMResult GPMController::set(const string& propName, const GCFPValue& value)
{
  TPMResult result(PM_NO_ERROR);
  
  GPMPropertySet* pNewPropertySet = findPropertySet(propName);

  if (!pNewPropertySet)
  {
    result = (_scadaService.set(propName, value) == SA_NO_ERROR ?
              PM_NO_ERROR : PM_SCADA_ERROR);
  }
  else
  {
    result = pNewPropertySet->setValue(propName, value);
  }
  
  return result;  
}

TPMResult GPMController::get(const string& propName)
{
  TPMResult result(PM_NO_ERROR);
  
  GPMPropertySet* pNewPropertySet = findPropertySet(propName);

  if (!pNewPropertySet)
  {
    result = (_scadaService.get(propName) == SA_NO_ERROR ?
              PM_NO_ERROR : PM_SCADA_ERROR);
  }
  else
  {
    GCFPValue* pValue;
    result = pNewPropertySet->getValue(propName, &pValue);
    if (result == PM_NO_ERROR && pValue != 0)
    {
      TGetData* pGetData = new TGetData;
      pGetData->pValue = pValue;
      pGetData->pPropName = new string(propName);
      _propertyAgent.setTimer(0, 0, 0, 0, pGetData);
    }
  }
  
  return result;  
}

TPMResult GPMController::getMyOldValue(const string& propName, GCFPValue** pValue)
{
  TPMResult result(PM_NO_ERROR);
  
  GPMPropertySet* pNewPropertySet = findPropertySet(propName);

  if (!pNewPropertySet)
  {
    result = PM_PROP_NOT_EXISTS;
  }
  else
  {
    result = pNewPropertySet->getOldValue(propName, pValue);
  }
  
  return result;  
}

void GPMController::valueChanged(string& propName, GCFPValue& value)
{
  _supervisedTask.valueChanged(propName, value);
}

void GPMController::valueGet(string& propName, GCFPValue& value)
{
  _supervisedTask.valueGet(propName, value);
}

void GPMController::propertiesLinked(const string& scope, list<string>& notLinkedProps)
{
  if (_propertyAgent.isConnected())
  {
    PAPropertieslinkedEvent e(0, PM_NO_ERROR);
    string allPropNames;
    for (list<string>::iterator iter = notLinkedProps.begin(); 
         iter != notLinkedProps.end(); ++iter)
    {
      allPropNames += *iter;
      allPropNames += '|';
    }
    unsigned short bufLength(scope.size() + allPropNames.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", scope.size(), scope.c_str(), 
                                    allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

void GPMController::propertiesUnlinked(const string& scope, list<string>& notUnlinkedProps)
{
  if (_propertyAgent.isConnected())
  {
    PAPropertiesunlinkedEvent e(0, PM_NO_ERROR);
    string allPropNames;
    for (list<string>::iterator iter = notUnlinkedProps.begin(); 
         iter != notUnlinkedProps.end(); ++iter)
    {
      allPropNames += *iter;
      allPropNames += '|';
    }
    unsigned short bufLength(scope.size() + allPropNames.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", scope.size(), scope.c_str(), 
                                    allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

GPMPropertySet* GPMController::findPropertySet(const string& propName)
{
  GPMPropertySet* pResult(0);
  string scope(propName);
  TPropertySetIter iter;
  size_t lastUSpos;
  do
  {
    lastUSpos = scope.rfind('_');
    if (lastUSpos < scope.size())
    {
      scope.erase(lastUSpos);   
      iter = _propertySets.find(scope);
      pResult = iter->second;
    }
    else
    {
      scope = "";
    }   
  } while (scope != "" && pResult == 0);
  
  return pResult;
}

int GPMController::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
    case F_TIMER_SIG:
      _propertyAgent.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(&GPMController::connected);
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

int GPMController::connected(GCFEvent& e, GCFPortInterface& /*p*/)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED_SIG:
      cout << "Lost connection to Property Agent" << endl;
      TRAN(&GPMController::initial);
      break;

    case F_ENTRY_SIG:
    {
      _isBusy = true;
      _preparing = (_propertySets.size() > 0);
      _counter = 0;
      for (TPropertySetIter iter = _propertySets.begin();
           iter != _propertySets.end(); ++iter)
      {
        registerScope(iter->first);
        _counter++;
      }
      break;
    }  
    case PA_SCOPEREGISTERED:
    {
      PAScoperegisteredEvent* response = static_cast<PAScoperegisteredEvent*>(&e);
      if (response->result == 0) // no error
      {
        char* pResponseData = ((char*)response) + sizeof(PAScoperegisteredEvent);
        unsigned int scopeNameLength(0);
        sscanf(pResponseData, "%03x", &scopeNameLength);
        string scope(pResponseData + 3, scopeNameLength);
        _supervisedTask.myPropertiesLoaded(scope);
        if (_preparing)
        {
          _counter--;
          if (_counter == 0)
          {       
            _isBusy = false;
            _preparing = false;
          }
        }
        else
          _isBusy = false;
      }
      break;
    }

    case PA_SCOPEUNREGISTERED:
    {
      PAScopeunregisteredEvent* response = static_cast<PAScopeunregisteredEvent*>(&e);
      if (response->result == 0) // no error
      {
        char* pResponseData = ((char*)response) + sizeof(PAScopeunregisteredEvent);
        unsigned int scopeNameLength(0);
        sscanf(pResponseData, "%03x", &scopeNameLength);
        string scope(pResponseData + 3, scopeNameLength);
        _supervisedTask.myPropertiesUnloaded(scope);
        _isBusy = false;
      }
      break;
    }

    case PA_LINKPROPERTIES:
    {
      PALinkpropertiesEvent* response = static_cast<PALinkpropertiesEvent*>(&e);
      char* pScopeData = ((char*)response) + sizeof(PALinkpropertiesEvent);
      unsigned int scopeNameLength(0);
      sscanf(pScopeData, "%03x", &scopeNameLength);
      string scope(pScopeData + 3, scopeNameLength);
      list<string> propertyList;
      unpackPropertyList(pScopeData + 3 + scopeNameLength, propertyList);
      GPMPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        pPropertySet->linkProperties(response->seqnr, propertyList);
      }
      break;
    }
    case PA_UNLINKPROPERTIES:
    {
      PAUnlinkpropertiesEvent* response = static_cast<PAUnlinkpropertiesEvent*>(&e);
      char* pScopeData = ((char*)response) + sizeof(PAUnlinkpropertiesEvent);
      unsigned int scopeNameLength(0);
      sscanf(pScopeData, "%03x", &scopeNameLength);
      string scope(pScopeData + 3, scopeNameLength);
      list<string> propertyList;
      unpackPropertyList(pScopeData + 3 + scopeNameLength, propertyList);
      GPMPropertySet* pPropertySet = _propertySets[scope];
      if (pPropertySet)
      {
        pPropertySet->unlinkProperties(response->seqnr, propertyList);
      }
      break;
    }
    case PA_APCLOADED:
    {
      PAApcloadedEvent* response = static_cast<PAApcloadedEvent*>(&e);
      if (response->result == 0) // no error
      {
        char* pApcData = ((char*)response) + sizeof(PAApcloadedEvent);
        unsigned int apcNameLength(0);
        sscanf(pApcData, "%03x", &apcNameLength);
        string apcName(pApcData + 3, apcNameLength);
        char* pScopeData = pApcData + 3 + apcNameLength;
        unsigned int scopeNameLength(0);
        sscanf(pScopeData, "%03x", &scopeNameLength);
        string scope(pScopeData + 3, scopeNameLength);
        _supervisedTask.apcLoaded(apcName, scope);
      }
      break;
    }
    case PA_APCUNLOADED:
    {
      PAApcunloadedEvent* response = static_cast<PAApcunloadedEvent*>(&e);
      if (response->result == 0) // no error
      {
        char* pApcData = ((char*)response) + sizeof(PAApcunloadedEvent);
        unsigned int apcNameLength(0);
        sscanf(pApcData, "%03x", &apcNameLength);
        string apcName(pApcData + 3, apcNameLength);
        char* pScopeData = pApcData + 3 + apcNameLength;
        unsigned int scopeNameLength(0);
        sscanf(pScopeData, "%03x", &scopeNameLength);
        string scope(pScopeData + 3, scopeNameLength);
        _supervisedTask.apcUnloaded(apcName, scope);
      }
      break;
    }
    case PA_APCRELOADED:
    {
      PAApcreloadedEvent* response = static_cast<PAApcreloadedEvent*>(&e);
      if (response->result == 0) // no error
      {
        char* pApcData = ((char*)response) + sizeof(PAApcreloadedEvent);
        unsigned int apcNameLength(0);
        sscanf(pApcData, "%03x", &apcNameLength);
        string apcName(pApcData + 3, apcNameLength);
        char* pScopeData = pApcData + 3 + apcNameLength;
        unsigned int scopeNameLength(0);
        sscanf(pScopeData, "%03x", &scopeNameLength);
        string scope(pScopeData + 3, scopeNameLength);
        _supervisedTask.apcReloaded(apcName, scope);
      }
      break;
    }
    case F_TIMER_SIG:
    {
      GCFTimerEvent* pTimer = static_cast<GCFTimerEvent*>(&e);
      if (pTimer->arg)
      {
        TGetData* pGetData = static_cast<TGetData*>pTimer->arg;
        if (pGetData)
        {
          _supervisoryTask.valueGet(*pGetData->pPropName, *pGetData->pValue);
          delete pGetData->pPropName;
          delete pGetData->pValue;
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

void GPMController::registerScope(const string& scope)
{
  PARegisterscopeEvent e(0);
  unsigned short bufLength(scope.size() + 3);
  char* buffer = new char[bufLength + 1];
  sprintf(buffer, "%03x%s", scope.size(), scope.c_str());
  e.length += bufLength;
  _propertyAgent.send(e, buffer, bufLength);
  delete [] buffer;
}

void GPMController::unpackPropertyList(char* pListData, list<string>& propertyList)
{
  unsigned int dataLength;
  char* pPropertyData;
  sscanf(pListData, "%03x", &dataLength);
  pPropertyData = pListData + 3;
  propertyList.clear();
  if (dataLength > 0)
  {
    string propName;
    char* pPropName = strtok(pPropertyData, "|");
    while (pPropName && dataLength > 0)
    {
      propName = pPropName;      
      pPropName = strtok(NULL, "|");
      dataLength -= (propName.size() + 1);
      propertyList.push_front(propName);
    }
  }
}

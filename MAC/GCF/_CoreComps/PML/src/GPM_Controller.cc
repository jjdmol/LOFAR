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
#include "PropertyAgent.ph"

static string sPMLTaskName("PML");

GPMController::GPMController(GCFSupervisedTask& supervisedTask) :
  GCFTask((State)&GPMController::initial, sPMLTaskName),
  _supervisedTask(supervisedTask),
  _scadaService(*this)
{
  // register the protocol for debugging purposes
  registerProtocol(PMLPA_PROTOCOL, PMLPA_PROTOCOL_signalnames);

  // initialize the port
  _propertyAgent.init(*this, "pml", GCFPortInterface::SAP, PMLPA_PROTOCOL);
}

GPMController::~GPMController()
{
}

TPMResult GPMController::loadAPC(string& apcName, string& scope)
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
    GCFPALoadapcEvent e(0);
    unsigned short bufLength(apcName.size() + scope.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", apcName.size(), apcName.c_str(), 
                                    scope.size(),   scope.c_str()   );
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
  return result;
}

TPMResult GPMController::unloadAPC(string& apcName, string& scope)
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
    GCFPAUnloadapcEvent e(0);
    unsigned short bufLength(apcName.size() + scope.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", apcName.size(), apcName.c_str(), 
                                    scope.size(),   scope.c_str()   );
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
  return result;
}

TPMResult GPMController::reloadAPC(string& apcName, string& scope)
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
    GCFPAReloadapcEvent e(0);
    unsigned short bufLength(apcName.size() + scope.size() + 6);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s%03x%s", apcName.size(), apcName.c_str(), 
                                    scope.size(),   scope.c_str()   );
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
  return result;
}

TPMResult GPMController::loadMyProperties(TPropertySet& newSet)
{
  TPMResult result(PM_NO_ERROR);
  
  TPropertySetIter iter = _propertySets.find(newSet.scope);
  if (iter->first.compare(newSet.scope) == 0)
  {
    result = PM_SCOPE_ALREADY_EXISTS;
  }
  else
  {
    GPMPropertySet* pNewPropertySet = new GPMPropertySet(*this, newSet);
    _propertySets[newSet.scope] = pNewPropertySet;
    
    if (_propertyAgent.isConnected())
    {
      GCFPARegisterscopeEvent e(0);
      unsigned short bufLength(strlen(newSet.scope) + 3);
      char* buffer = new char[bufLength + 1];
      sprintf(buffer, "%03x%s", strlen(newSet.scope), newSet.scope);
      e.length += bufLength;
      _propertyAgent.send(e, buffer, bufLength);
      delete [] buffer;
    }
  }
  return result;
}

TPMResult GPMController::unloadMyProperties(TPropertySet& newSet)
{
  TPMResult result(PM_NO_ERROR);
  
  TPropertySetIter iter = _propertySets.find(newSet.scope);
  if (iter->first.compare(newSet.scope) != 0)
  {
    result = PM_SCOPE_NOT_EXISTS;
  }
  else
  {    
    GPMPropertySet* pNewPropertySet = iter->second;
    delete pNewPropertySet;
    _propertySets.erase(newSet.scope);
        
    if (_propertyAgent.isConnected())
    {
      GCFPAUnregisterscopeEvent e(0);
      unsigned short bufLength(strlen(newSet.scope) + 3);
      char* buffer = new char[bufLength + 1];
      sprintf(buffer, "%03x%s", strlen(newSet.scope), newSet.scope);
      e.length += bufLength;
      _propertyAgent.send(e, buffer, bufLength);
      delete [] buffer;
    }
  }
  return result;
}

TPMResult GPMController::set(string& propName, GCFPValue& value)
{
  TPMResult result(PM_NO_ERROR);
  
  GPMPropertySet* pNewPropertySet = findPropertySet(propName);

  if (!pNewPropertySet)
  {
    result = _scadaService.set(propName, value);
  }
  else
  {
    result = pNewPropertySet->set(propName, value);
  }
  
  return result;  
}

TPMResult GPMController::get(string& propName)
{
  TPMResult result(PM_NO_ERROR);
  
  GPMPropertySet* pNewPropertySet = findPropertySet(propName);

  if (!pNewPropertySet)
  {
    result = _scadaService.get(propName);
  }
  else
  {
    GCFPValue* pValue;
    result = pNewPropertySet->get(propName, &pValue);
    if (result == PM_NO_ERROR && pValue != 0)
    {
      TGetData* pGetData = new TGetData;
      pGetData->pValue = pValue;
      pGetData->pPropName = new string(propName);
      _propertAgent.setTimer(0, 0, 0, 0, pGetData);
    }
  }
  
  return result;  
}

TPMResult GPMController::getMyOldValue(string& propName, GCFPValue** value)
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

void GPMController::propertiesLinked(list<string>& notLinkedProps)
{
  if (_propertyAgent.isConnected())
  {
    GCFPAPropertieslinkedEvent e(0);
    string allPropNames;
    for (list<string>::iterator iter = notLinkedProps.begin(); 
         iter != notLinkedProps.end(); ++iter)
    {
      allPropNames += iter;
      allPropNames += '|';
    }
    unsigned short bufLength(allPropNames.size() + 3);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s", allPropNames.size(), allPropNames.c_str());
    e.length += bufLength;
    _propertyAgent.send(e, buffer, bufLength);
    delete [] buffer;
  }
}

void GPMController::propertiesUnlinked(list<string>& notUnlinkedProps)
{
  if (_propertyAgent.isConnected())
  {
    GCFPAPropertieslinkedEvent e(0);
    string allPropNames;
    for (list<string>::iterator iter = notLinkedProps.begin(); 
         iter != notLinkedProps.end(); ++iter)
    {
      allPropNames += iter;
      allPropNames += '|';
    }
    unsigned short bufLength(allPropNames.size() + 3);
    char* buffer = new char[bufLength + 1];
    sprintf(buffer, "%03x%s", allPropNames.size(), allPropNames.c_str());
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
  size_t lastUSpos
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
  } while (scope != "" && pResult == 0)
  
  return pResult;
}

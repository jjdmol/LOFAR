//#  GPA_ScopeManager.cc: 
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

#include "GPA_ScopeManager.h"
#include "GPA_Controller.h"
#include <stdio.h>
#include "PA_Protocol.ph"

GPAScopeManager::GPAScopeManager(GPAController& controller) :
  _controller(controller),
  _counter(0)
{
}

GPAScopeManager::~GPAScopeManager()
{
}

TPAResult GPAScopeManager::linkProperties(list<string>& propList)
{
  TPAResult result(PA_NO_ERROR);
  _counter = 0;
  resetScopeList();
  result = fillScopeLists(propList);
  PALinkpropertiesEvent e(0);
  sendUnLinkEvents(e);
  return result;
}

TPAResult GPAScopeManager::unlinkProperties(list<string>& propList)
{
  TPAResult result(PA_NO_ERROR);

  _counter = 0;
  resetScopeList();
  result = fillScopeLists(propList);
  PAUnlinkpropertiesEvent e(0);
  sendUnLinkEvents(e);  
  return result;
}

TPAResult GPAScopeManager::propertiesLinked(const string& scope)
{
  TPAResult result(PA_NO_ERROR);
  TScopeData* pScopeData(0);
  TScopeListIter iter = _scopeList.find(scope);
  if (iter->first == scope)
  {
    pScopeData = &iter->second;
    if (pScopeData)
    {
      // TODO: find out a safer way to see whether all responses are received
      pScopeData->respond = true; // does nothing for now
      _counter--;
      if (_counter == 0)
      {
        _controller.apcLoaded(result);
      }
    }    
  }
  
  return result;
}

TPAResult GPAScopeManager::propertiesUnlinked(const string& scope)
{
  TPAResult result(PA_NO_ERROR);
  TScopeData* pScopeData(0);
  TScopeListIter iter = _scopeList.find(scope);
  if (iter->first == scope)
  {
    pScopeData = &iter->second;
    if (pScopeData)
    {
      // TODO: find out a safer way to see whether all responses are received
      pScopeData->respond = true; // does nothing for now
      _counter--;
      if (_counter == 0)
      {
        _controller.apcUnloaded(result);
      }
    }    
  }
  return result;
}

TPAResult GPAScopeManager::registerScope(const string& scope, GCFPortInterface& port)
{
  TPAResult result(PA_NO_ERROR);
  TScopeListIter iter = _scopeList.find(scope);
  if (iter == _scopeList.end())
  {
    // creates a new scope entry
    _scopeList[scope].pPort = &port;
    _scopeList[scope].respond = true;    
  }
  else
    result = PA_SCOPE_ALREADY_REGISTERED;
  
  return result;
}

TPAResult GPAScopeManager::unregisterScope(const string& scope)
{
  TPAResult result(PA_NO_ERROR);
  TScopeListIter iter = _scopeList.find(scope);
  if (iter != _scopeList.end())
  {
    _scopeList.erase(scope);
  }
  else
    result = PA_SCOPE_IS_NOT_REGISTERED;
  
  return result;
}

void GPAScopeManager::deleteScopesByPort(const GCFPortInterface& requestPort, 
                        list<string>& deletedScopes)
{
  TScopeData* pScopeData;
  deletedScopes.clear();
  for (TScopeListIter iter = _scopeList.begin(); 
       iter != _scopeList.end(); ++iter)
  {
    pScopeData = &iter->second;
    if (pScopeData)
    {
      if (&requestPort == pScopeData->pPort)
      {
        deletedScopes.push_back(iter->first);
      }
    }
  }
  for (list<string>::iterator iter = deletedScopes.begin();
       iter != deletedScopes.end(); ++iter)
  {
    _scopeList.erase(*iter);
  }
}

void GPAScopeManager::getSubScopes(const string& scope, list<string>& subscopes)
{
  size_t subScopeLength(0);
  const string* pScope(0);
  subscopes.clear();
  
  for (TScopeListIter iter = _scopeList.begin(); 
       iter != _scopeList.end(); ++iter)
  {
    pScope = &iter->first;
    subScopeLength = pScope->size();
    if (pScope->find(scope) < subScopeLength)
    {
      subscopes.push_back(*pScope);
    }
  }
}

void GPAScopeManager::deleteAllScopes()
{
  _scopeList.clear();
}

void GPAScopeManager::resetScopeList()
{
  TScopeData* pScopeData;
  for (TScopeListIter iter = _scopeList.begin(); 
       iter != _scopeList.end(); ++iter)
  {
    pScopeData = &iter->second;
    if (pScopeData)
    {
      pScopeData->propList.clear();
      pScopeData->respond = true;
    }
  }
}

TPAResult GPAScopeManager::fillScopeLists(list<string>& propList)
{
  TPAResult result(PA_NO_ERROR);
  size_t propNameLength(0);
  size_t scopeNameLength(0);
  const string* pScope(0);
  const string* pRightScope(0);
  for (list<string>::iterator pPropName = propList.begin();
       pPropName != propList.end(); ++pPropName)
  {
    propNameLength = pPropName->size();
    scopeNameLength = 0;
    pRightScope = 0;
    for (TScopeListIter iter = _scopeList.begin(); 
         iter != _scopeList.end(); ++iter)
    {
      pScope = &iter->first;
      if (pPropName->find(*pScope) < propNameLength)
      {
        if (scopeNameLength < pScope->size())
        {
          pRightScope = pScope;
          scopeNameLength = pScope->size();
        }        
      }
    }
    if (pRightScope)
    {
      // plus 1 means erase the '_' after scope too
      pPropName->erase(0, pRightScope->size() + 1); 
      
      _scopeList[*pRightScope].propList.push_back(*pPropName);
    }
    else
      result = PA_PROP_NOT_VALID;
  }  
  
  return result;
}

void GPAScopeManager::sendUnLinkEvents(GCFEvent& e)
{
  TScopeData* pScopeData;
  const string* pScope;
  list<string>* pPropList;

  for (TScopeListIter iter = _scopeList.begin(); 
       iter != _scopeList.end(); ++iter)
  {
    pScope = &iter->first;
    pScopeData = &iter->second;
    if (pScopeData)
    {
      pPropList = &pScopeData->propList;
      if (pPropList->size() > 0)
      {
        pScopeData->respond = false;
        _counter++;
        GCFPortInterface* pPort = pScopeData->pPort;
        string allPropNames;
        for (list<string>::iterator pPropName = pPropList->begin(); 
             pPropName != pPropList->end(); ++pPropName)
        {
          allPropNames += *pPropName;
          allPropNames += '|';
        }
        unsigned short bufLength(pScope->size() + allPropNames.size() + 6);
        char* buffer = new char[bufLength + 1];
        sprintf(buffer, "%03x%s%03x%s", pScope->size(), pScope->c_str(), 
                                        allPropNames.size(), allPropNames.c_str());
        e.length += bufLength;
        pPort->send(e, buffer, bufLength);
        delete [] buffer;
      }
    }
  }
}

bool GPAScopeManager::waitForAsyncResponses()
{
  return (_counter > 0);
}

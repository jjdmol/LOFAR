//#  GPA_UsecountManager.cc: 
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

#include "GPA_UsecountManager.h"
#include "GPA_Controller.h"
#include "SAL/GCF_PValue.h"

GPAUsecountManager::GPAUsecountManager(GPAController& controller) :
  _controller(controller),
  _state(DECREMENT),
  _counter(0)
{
}

GPAUsecountManager::~GPAUsecountManager()
{
}

TPAResult GPAUsecountManager::incrementUsecount(list<string>& propList)
{
  TPAResult result(PA_NO_ERROR);
  TPropListIter iter;
  _counter = 0;
  for (list<string>::iterator pPropName = propList.begin(); 
       pPropName != propList.end(); ++pPropName)
  {
    iter = _propList.find(*pPropName);
    if (iter->first == *pPropName)
    {
      iter->second++;
    }
    else
    {      
      if (createProp(*pPropName, "BOOL_VAL") == SA_NO_ERROR)
      {
        _counter++;
      }
    }
  }
  return result;
}

TPAResult GPAUsecountManager::decrementUsecount(list<string>& propList)
{
  TPAResult result(PA_NO_ERROR);
  TPropListIter iter;
  _counter = 0;
  for (list<string>::iterator pPropName = propList.begin(); 
       pPropName != propList.end(); ++pPropName)
  {
    iter = _propList.find(*pPropName);
    if (iter->first == *pPropName)
    {
      iter->second--;
      if (iter->second == 0)
      {
        if (deleteProp(*pPropName) == SA_NO_ERROR)
        {
          _state = DECREMENT;
          _counter++;
        }
      }
    }
  }  
  return result;
}

TPAResult GPAUsecountManager::deletePropertiesByScope(const string& scope, 
                                                      list<string>& subScopes)
{
  TPAResult result(PA_NO_ERROR);
  const string* pPropName;
  size_t propNameLength;
  bool belongsToASubScope(false);
  _counter = 0;
  
  for (TPropListIter iter = _propList.begin();
       iter != _propList.end(); ++iter)
  {
    pPropName = &iter->first;
    propNameLength = pPropName->size();
    if (pPropName->find(scope) < propNameLength)
    {
      belongsToASubScope = false;
      for (list<string>::iterator pSubScope = subScopes.begin();
           pSubScope != subScopes.end(); ++pSubScope)
      {
        if (pPropName->find(*pSubScope) < propNameLength)
        {
          belongsToASubScope = true;
          break;
        }
      }
      if (!belongsToASubScope)
      {
        if (deleteProp(*pPropName) == SA_NO_ERROR)
        {
          _state = DELETE_BY_SCOPE;
          _counter++;
        }
      }      
    }
  }
  return result;
}

void GPAUsecountManager::deleteAllProperties()
{
  _counter = 0;
  for (TPropListIter iter = _propList.begin();
       iter != _propList.end(); ++iter)
  {
    if (deleteProp(iter->first) == SA_NO_ERROR)
    {
      _state = DELETE_ALL;
      _counter++;
    }
  }
}

void GPAUsecountManager::propCreated(string& propName)
{
  _counter--;
  _tempPropList.push_back(propName);
  if (_counter == 0)
  {
    _controller.propertiesCreated(_tempPropList);
    _tempPropList.clear();
  }
}

void GPAUsecountManager::propDeleted(string& propName)
{
  _counter--;
  _propList.erase(propName);
  _tempPropList.push_back(propName);
  if (_counter == 0)
  {
    switch (_state)
    {
      case DECREMENT:  
        _controller.propertiesDeleted(_tempPropList);
        break;
      case DELETE_BY_SCOPE:
        _controller.allPropertiesDeletedByScope();
        break;
      case DELETE_ALL:
        _controller.allPropertiesDeleted();
        break;
    }
    _tempPropList.clear();
  }
}

bool GPAUsecountManager::waitForAsyncResponses()
{
  return (_counter > 0);
}

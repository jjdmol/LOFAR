//#  GPM_PropertySet.cc: 
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

#include "GPM_PropertySet.h"

GPMPropertySet::GPMPropertySet(GPMController& controller, TPropertySet& propSet) : 
  _controller(controller)
{
  GPMProperty* pProperty;
  _scope = propSet.scope;
  for (unsigned int i = 0; i < propSet.nrOfProperties; i++)
  {
    // TODO: should be checked, whether the no double properties are created here
    pProperty = new GPMProperty(propSet.props[i]);
    _properties[pProperty->getName()] = pProperty;    
  }
}  

GPMPropertySet::~GPMPropertySet()
{
  GPMProperty* pProperty;
  for (TPropertyIter iter = _properties.begin(); 
        iter != _timers.end(); ++iter) 
  {
    pProperty = iter->second;
    if (pProperty)
      delete pProperty;
  }
}

void GPMPropertySet::propSubscribed(string& propName)
{
  // TODO: Make this more secure (see linkProperties)
  cutScope(propName);
  _properties[propName]->setLinked(true);
  _tempLinkList.erase(propName);
  _counter--;
  if (_counter == 0)
  {
    _controller.propertiesLinked(_tempLinkList);
  }
}

void GPMPropertySet::propUnsubscribed(string& propName)
{
  // TODO: Make this more secure (see unlinkProperties)
  cutScope(propName);
  _properties[propName]->setLinked(false);
  _tempLinkList.erase(propName);
  _counter--;
  if (_counter == 0)
  {
    _controller.propertiesUnlinked(_tempLinkList);
  }
}

void GPMPropertySet::propValueChanged(string& propName, GCFPValue& value)
{
  cutScope(propName);
  GPMProperty* pProperty = _properties[propName];
  if (pProperty->isLinked())
  {
    pProperty->setValue(value);
  }
  _controller.valueChanged(_scope + "_" + propName, value);
}

TPMResult GPMPropertySet::linkProperties(list<string>& properties)
{
  TPMResult result(PM_NO_ERROR);
  GPMProperty* pProperty;
  _tempLinkList = properties;
  if (_counter > 0)
  {
    result = PM_PROP_SET_BUSY;
  }
  else 
  {
    for (list<string>::iterator iter = properties.begin(); 
         iter != iter.end(); ++iter)
    {
      pProperty = _properties[iter];
      if (!pProperty)
      {
        if (result == PM_NO_ERROR) result = PM_PROP_LIST_FAILURE;
      }
      else if (pProperty->isLinked())
      {
        if (result == PM_NO_ERROR) result = PM_PROP_ALREADY_LINKED;
      }      
      else
      {
        subscribe(_scope + "_" + iter);
        _counter++;        
      }
    }
  }
    
  return result;
}

TPMResult GPMPropertySet::unlinkProperties(list<string>& properties)
{
  TPMResult result(PM_NO_ERROR);
  GPMProperty* pProperty;
  _tempLinkList = properties;
  
  if (_counter > 0)
  {
    result = PM_PROP_SET_BUSY;
  }
  else 
  {
    for (list<string>::iterator iter = properties.begin(); 
         iter != iter.end(); ++iter)
    {
      pProperty = _properties[iter];
      if (!pProperty)
      {
        if (result == PM_NO_ERROR) result = PM_PROP_LIST_FAILURE;
      }
      else if (!pProperty->isLinked())
      {
        if (result == PM_NO_ERROR) result = PM_PROP_NOT_LINKED;
      }      
      else
      {
        unsubscribe(_scope + "_" + iter);
        _counter++;        
      }
    }
  }
    
  return result;
}

TPMResult GPMPropertySet::get(const string& propName, GCFPValue** pValue)
{
  TPMResult result(PM_NO_ERROR);
  GPMProperty* pProperty;
  
  if ((result = cutScope(propName)) != PM_NO_ERROR)
  {
  }
  else if ((pProperty = _properties[propName]) == 0)
  {
    result = PM_PROP_NOT_IN_SET;
  }
  else
  {
    *pValue = pProperty->getValue();
  }

  return result;
}

TPMResult GPMPropertySet::set(const string& propName, const GCFPValue& value)
{
  TPMResult result(PM_NO_ERROR);
  GPMProperty* pProperty;
  
  if ((result = cutScope(propName)) != PM_NO_ERROR)
  {
  }
  else if ((pProperty = _properties[propName]) == 0)
  {
    result = PM_PROP_NOT_IN_SET;
  }
  else if ((result = pProperty->setValue(value)) == PM_NO_ERROR)
  {
    if (pProperty->isLinked())
      set(_scope + "_" + propName, value);
  }  

  return result;
}

TPMResult GPMPropertySet::getOldValue(const string& propName, GCFPValue** value)
{
  TPMResult result(PM_NO_ERROR);
  GPMProperty* pProperty;
  
  if ((result = cutScope(propName)) != PM_NO_ERROR)
  {
  }
  else if ((pProperty = _properties[propName]) == 0)
  {
    result = PM_PROP_NOT_IN_SET;
  }
  else
  {
    *pValue = pProperty->getValue(false); // false == oldValue
  }

  return result;
}

TPMResult GPMPropertySet::cutScope(string& propName)
{
  TPMResult result(PM_NO_ERROR);
  
  if (propName.find(_scope) == 0)
  {
    // plus 1 means erase the '_' after scope too
    propName.erase(0, _scope.size() + 1); 
  }
  else
  {
    result = PM_CONTAINS_NO_SCOPE;
  }
  
  return result;
}


//#  GCF_PropertySetBase.cc: 
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

#include <GCF/GCF_PropertySetBase.h>
#include <GCF/GCF_PropertySet.h>
#include <GCF/GCF_PropertyBase.h>
#include <Utils.h>
#include <GPM_Defines.h>

GCFPropertySetBase::GCFPropertySetBase (string scope, 
                                        GCFAnswer* pAnswerObj) : 
  _pAnswerObj(pAnswerObj),
  _scope(scope),
  _dummyProperty("", this)
{
  if (!Utils::isValidPropName(scope.c_str()))
  {
    LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
        "Scope %s meets not the name convention! Set to \"\"",
        scope.c_str()));
    _scope = "";
  }
}


GCFPropertySetBase::~GCFPropertySetBase()
{
  clearAllProperties();
  _dummyProperty.resetPropSetRef();
}

GCFPropertyBase* GCFPropertySetBase::getProperty (const string propName) const
{
  string shortPropName(propName);
  cutScope(shortPropName);
  
  TPropertyList::const_iterator iter = _properties.find(shortPropName);
  
  if (iter != _properties.end())
  {
    return iter->second;
  }
  else
  {
    return 0;
  }
}

GCFPropertyBase& GCFPropertySetBase::operator[] (const string propName)
{ 
  GCFPropertyBase* pProperty = getProperty(propName);
  if (!pProperty)
  {
    pProperty = &_dummyProperty;
  }
  return *pProperty;
}

TGCFResult GCFPropertySetBase::setValue (const string propName, 
                                         const GCFPValue& value)
{
  GCFPropertyBase* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}
                             
void GCFPropertySetBase::setAnswer (GCFAnswer* pAnswerObj)
{
  GCFPropertyBase* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    assert(pProperty);
    pProperty->setAnswer(pAnswerObj);
  }
  _pAnswerObj = pAnswerObj;
}

bool GCFPropertySetBase::exists (const string propName) const
{
  GCFPropertyBase* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->exists();    
  }
  else 
  {
    return false;
  }
}

void GCFPropertySetBase::addProperty(const string& propName, GCFPropertyBase& prop)
{
  assert(propName.length() > 0);
  
  string shortPropName(propName);
  cutScope(shortPropName);
  
  TPropertyList::iterator iter = _properties.find(shortPropName);
  if (iter == _properties.end())
  {
    _properties[shortPropName] = &prop;
  }
}

bool GCFPropertySetBase::cutScope(string& propName) const 
{
  bool scopeFound(false);
  
  if (propName.find(_scope) == 0)
  {
    // plus 1 means erase the GCF_PROP_NAME_SEP after scope too
    propName.erase(0, _scope.size() + 1); 
    scopeFound = true;
  }
  
  return scopeFound;
}

void GCFPropertySetBase::clearAllProperties()
{
  GCFPropertyBase* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    assert(pProperty);
    pProperty->resetPropSetRef();
    delete pProperty;
  }
}


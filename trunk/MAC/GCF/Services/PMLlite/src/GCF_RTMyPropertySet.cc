//#  GCF_RTMyPropertySet.cc: 
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

#include <GCF_RTMyPropertySet.h>
#include <GCF_RTMyProperty.h>
#include <GPM_RTController.h>
#include <GCF_RTAnswer.h>
#include <GSA_Service.h>

GCFRTMyPropertySet::GCFRTMyPropertySet(const TPropertySet& propSet,
                                   GCFRTAnswer* pAnswerObj) : 
  _scope(propSet.scope), 
  _pAnswerObj(pAnswerObj),
  _isLoaded(false),
  _isBusy(false),
  _counter(0),
  _missing(0),
  _propSet(propSet)  
{
  GCFRTMyProperty* pProperty;
  const char* propName;
  
  if (!GSAService::validatePropName(_scope.c_str()))
  {
    LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
        "Scope %s meets not the name convention! Set to \"\"",
        scope.c_str()));
    _scope = "";
  }
  for (unsigned int i = 0; i < _propSet.nrOfProperties; i++)
  { 
    propName = _propSet.properties[i].propName;
    if (GSAService::validatePropName(propName))
    {
      pProperty = new GCFRTMyProperty(_propSet.properties[i], *this);
      addProperty(propName, *pProperty);
    }
    else
    {
      LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
          "Property %s meets not the name convention! NOT CREATED",
          propName));      
    }
  }
  if (pAnswerObj)
  {
    setAnswer(pAnswerObj);
  }
}  

GCFRTMyPropertySet::~GCFRTMyPropertySet ()
{
  GPMRTController* pController = GPMRTController::instance();
  assert(pController);
  // delete this set from the controller permanent
  // this means no response will be send to this object
  // on response of the PA
  pController->unregisterScope(*this, true); 
  clearAllProperties();  
}
  
TGCFResult GCFRTMyPropertySet::load ()
{
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (_isLoaded)
  {
    result = GCF_ALREADY_LOADED;
  }
  else
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "REQ: Register scope %s",
        getScope().c_str()));
    const TProperty* pPropertyFields;
    for (unsigned int i = 0; i < _propSet.nrOfProperties; i++)
    {
      pPropertyFields = &_propSet.properties[i];
      if (pPropertyFields->defaultValue)
      {
        setValue(pPropertyFields->propName, pPropertyFields->defaultValue);
      }
    }
    GPMRTController* pController = GPMRTController::instance();
    assert(pController);
    TPMResult pmResult = pController->registerScope(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
    else if (pmResult == PM_SCOPE_ALREADY_EXISTS)
    {
      LOFAR_LOG_ERROR(PML_STDOUT_LOGGER, ( 
          "Property set with scope %s already exists in this Application",
          getScope().c_str()));    
      result = GCF_SCOPE_ALREADY_REG;
    }
  }
  return result;
}

TGCFResult GCFRTMyPropertySet::unload ()
{
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    result = GCF_BUSY;
  }
  else if (!_isLoaded)
  {
    result = GCF_NOT_LOADED;
  }
  else
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "REQ: Unregister scope %s",
        getScope().c_str()));

    GPMRTController* pController = GPMRTController::instance();
    assert(pController);

    TPMResult pmResult = pController->unregisterScope(*this);
    assert(pmResult == PM_NO_ERROR);

    _isBusy = true;
  }
  return result;
}

TGCFResult GCFRTMyPropertySet::setValue (const string propName, 
                                       const string value)
{
  GCFRTMyProperty* pProperty = static_cast<GCFRTMyProperty*>(getProperty(propName));
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}

GCFPValue* GCFRTMyPropertySet::getValue (const string propName)
{
  GCFRTMyProperty* pProperty = static_cast<GCFRTMyProperty*>(getProperty(propName));
  if (pProperty)
  {
    return pProperty->getValue();
  }
  else 
  {
    return 0;
  }
}
                                     
GCFPValue* GCFRTMyPropertySet::getOldValue (const string propName)
{
  GCFRTMyProperty* pProperty = static_cast<GCFRTMyProperty*>(getProperty(propName));
  if (pProperty)
  {
    return pProperty->getOldValue();
  }
  else 
  {
    return 0;
  }
}                                         

void GCFRTMyPropertySet::scopeRegistered (TGCFResult result)
{
  assert(!_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;
  if (result == GCF_NO_ERROR)
  {
    _isLoaded = true;
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PI-RESP: Scope %s registered",
        _scope.c_str()));
  }

  dispatchAnswer(F_MYPLOADED_SIG, result);
}

void GCFRTMyPropertySet::scopeUnregistered (TGCFResult result)
{
  assert(_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;
  _isLoaded = false;
  
  LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
      "PI-RESP: Scope %s unregistered",
      _scope.c_str()));

  dispatchAnswer(F_MYPUNLOADED_SIG, result);
}

void GCFRTMyPropertySet::linkProperties(list<string>& properties)
{
  GCFRTMyProperty* pProperty;
  _missing = 0;

  list<string>::iterator iter = properties.begin(); 

  for (list<string>::iterator iter = properties.begin(); 
       iter != properties.end(); ++iter)
  {
    pProperty = getProperty(*iter);
    if (pProperty)
    {
      pProperty->link();
    }
    else
    {
      _missing++;
    }
  }    
  GPMRTController* pController = GPMRTController::instance();
  assert(pController);
  pController->propertiesLinked(_scope, _missing > 0);
}

void GCFRTMyPropertySet::unlinkProperties(list<string>& properties)
{
  GCFRTMyProperty* pProperty;
  
  assert (_counter == 0);

  for (list<string>::iterator iter = properties.begin(); 
       iter != properties.end(); ++iter)
  {
    pProperty = static_cast<GCFRTMyProperty*> (getProperty(*iter));
    if (pProperty) pProperty->unlink();
  }

  GPMRTController* pController = GPMRTController::instance();
  assert(pController);
  pController->propertiesUnlinked(_scope);
}

void GCFRTMyPropertySet::dispatchAnswer(unsigned short sig, TGCFResult result)
{
  if (_pAnswerObj != 0)
  {
    GCFMYPropAnswerEvent e(sig);
    e.pScope = _scope.c_str();
    e.result = result;
    _pAnswerObj->handleAnswer(e);
  }
}

GCFRTMyProperty* GCFRTMyPropertySet::getProperty (const string propName) const
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

TGCFResult GCFRTMyPropertySet::setValue (const string propName, 
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
                             
void GCFRTMyPropertySet::setAnswer (GCFAnswer* pAnswerObj)
{
  GCFRTMyProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    assert(pProperty);
    pProperty->setAnswer(pAnswerObj);
  }
  _pAnswerObj = pAnswerObj;
}

bool GCFRTMyPropertySet::exists (const string propName) const
{
  GCFPropertyBase* pProperty = getProperty(propName);
  return (pProperty != 0);
}

void GCFRTMyPropertySet::addProperty(const string& propName, GCFRTMyProperty& prop)
{
  assert(pProperty);
  assert(propName.length() > 0);
  
  string shortPropName(propName);
  cutScope(shortPropName);
  
  TPropertyList::iterator iter = _properties.find(shortPropName);
  if (iter == _properties.end())
  {
    _properties[shortPropName] = &prop;
  }
}

bool GCFRTMyPropertySet::cutScope(string& propName) const 
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

void GCFRTMyPropertySet::clearAllProperties()
{
  GCFRTMyProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    assert(pProperty);
    delete pProperty;
  }
}

void GCFRTMyPropertySet::valueSet(const string& propName, const GCFPValue& value) const
{
  GPMRTController* pController = GPMRTController::instance();
  assert(pController);
  pController->valueSet(propName, value);  
}

void GCFRTMyPropertySet::valueChanged(string propName, const GCFPValue& value)
{
  GCFPropertyBase* pProperty = getProperty(propName);
  assert(pProperty);
  pProperty->valueChanged(value);
}
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

#include <GCF/PALlight/GCF_RTMyPropertySet.h>
#include <GCF/PALlight/GCF_RTMyProperty.h>
#include <GCF/PALlight/GPM_RTController.h>
#include <GCF/PALlight/GCF_RTAnswer.h>
#include <GCF/Utils.h>

GCFRTMyPropertySet::GCFRTMyPropertySet(const char* name,
                                       const TPropertySet& propSet,
                                       GCFRTAnswer* pAnswerObj) : 
  _scope(name),
  _state(S_DISABLED),
  _pAnswerObj(pAnswerObj),
  _propSet(propSet),
  _dummyProperty(*this)
{
  GCFRTMyProperty* pProperty;
  const char* propName;
  
  if (!Utils::isValidPropName(_scope.c_str()))
  {
    LOG_WARN(LOFAR::formatString ( 
        "Scope %s meets not the name convention! Set to \"\"",
        _scope.c_str()));
    _scope = "";
  }
  for (unsigned int i = 0; i < _propSet.nrOfProperties; i++)
  { 
    propName = _propSet.properties[i].propName;
    if (Utils::isValidPropName(propName))
    {
      pProperty = new GCFRTMyProperty(_propSet.properties[i], *this);
      addProperty(propName, *pProperty);
    }
    else
    {
      LOG_WARN(LOFAR::formatString ( 
          "Property %s meets not the name convention! NOT CREATED",
          propName));      
    }
  }
  if (_pAnswerObj)
  {
    setAnswer(_pAnswerObj);
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

  pController->deletePropSet(*this);   
}
  
TGCFResult GCFMyPropertySet::enable ()
{
  TGCFResult result(GCF_NO_ERROR);
    
  if (_state != S_DISABLED)
  {
    wrongState("enable");
    result = GCF_WRONG_STATE;
  }
  else
  {
    LOG_INFO(LOFAR::formatString ( 
        "REQ: Register scope %s",
        getScope().c_str()));
    GPMRTController* pController = GPMRTController::instance();
    assert(pController);
    TPMResult pmResult = pController->registerScope(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _state = S_ENABLING;
    }
    else if (pmResult == PM_SCOPE_ALREADY_EXISTS)
    {
      LOG_ERROR(LOFAR::formatString ( 
          "Property set with scope %s already exists in this Application",
          getScope().c_str()));    
      result = GCF_SCOPE_ALREADY_REG;
    }
  }
  return result;
}

TGCFResult GCFMyPropertySet::disable ()
{
  TGCFResult result(GCF_NO_ERROR);
  
  switch (_state)
  {
    case S_LINKING:
      assert(_counter > 0);
      _state = S_DELAYED_DISABLING;
      break;
    case S_LINKED:
      GCFRTMyProperty* pProperty;
      for (TPropertyList::iterator iter = _properties.begin();
           iter != _properties.end(); ++iter)
      {
        pProperty = iter->second;
        assert(pProperty);
        if (pProperty->isLinked())
        {
          pProperty->unlink();
        }
      }
      // intentional fall through
    case S_ENABLED:
    {
      LOG_INFO(LOFAR::formatString ( 
          "REQ: Unregister scope %s",
          getScope().c_str()));
  
      GPMRTController* pController = GPMRTController::instance();
      assert(pController);
  
      TPMResult pmResult = pController->unregisterScope(*this);
      assert(pmResult == PM_NO_ERROR);  
      _state = S_DISABLING;
      
      break;
    }
    default:
      wrongState("disable");
      result = GCF_WRONG_STATE;
      break;
  }
  return result;
}

void GCFRTMyPropertySet::scopeRegistered (TGCFResult result)
{
  assert(_state == S_ENABLING);
  if (result == GCF_NO_ERROR)
  {
    _state = S_ENABLED;
    LOG_INFO(LOFAR::formatString ( 
        "PA-RESP: Scope %s registered",
        getScope().c_str()));
  }
  else
  {
    _state = S_DISABLED;
  }

  dispatchAnswer(F_MYPS_ENABLED, result);
}

void GCFRTMyPropertySet::scopeUnregistered (TGCFResult result)
{
  assert(_state == S_DISABLING);
   
  LOG_INFO(LOFAR::formatString ( 
      "PA-RESP: Scope %s unregistered",
      getScope().c_str()));

  _state = S_DISABLED;
  dispatchAnswer(F_MYPS_DISABLED, result);
}

void GCFRTMyPropertySet::linkProperties()
{
  GPMRTController* pController = GPMController::instance();
  assert(pController);
  list<string> propsToSubscribe;
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKED:
    case S_LINKING:
      wrongState("linkProperties");
      pController->propertiesLinked(getScope(), propsToSubscribe, PA_WRONG_STATE);
      break;
    case S_ENABLED:
    {
      GCFRTMyProperty* pProperty(0);
      TGCFResult result;

      assert(_counter == 0);
      _missing = 0;
      _state = S_LINKING;

      for(TPropertyList::iterator iter = _properties.begin(); 
          iter != _properties.end(); ++iter)
      {
        pProperty = iter->second;
        assert(pProperty);
        pProperty->link();
        if (pProperty->testAccessMode(GCF_WRITABLE_PROP))
        {
          propsToSubscribe.push_back(pProperty->getName());
        }     
      }      
      _state = S_LINKED;
      pController->propertiesLinked(getScope(), propsToSubscribe, PA_NO_ERROR);
      break;
    }

    case S_DELAYED_DISABLING:
      pController->propertiesLinked(getScope(), propsToSubscribe, PA_PS_GONE);
      _state = S_ENABLED;
      disable();
      break;
    case S_DISABLED:
    case S_DISABLING:
      pController->propertiesLinked(getScope(), propsToSubscribe, PA_PS_GONE);
      break;
  }
}

void GCFRTMyPropertySet::unlinkProperties(list<string>& properties)
{
  GPMRTController* pController = GPMController::instance();
  assert(pController);
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKING:
    case S_ENABLED:
    case S_DELAYED_DISABLING:
      wrongState("unlinkProperties");
      pController->propertiesUnlinked(getScope(), PA_WRONG_STATE);
      break;
    case S_DISABLED:
    case S_DISABLING:
      pController->propertiesUnlinked(getScope(), PA_PS_GONE);
      break;
    case S_LINKED:
    {
      _state = S_ENABLED;
      GCFRTMyProperty* pProperty(0);
      for (TPropertyList::iterator iter = _properties.begin(); 
           iter != _properties.end(); ++iter)
      {
        pProperty = iter->second;
        if (pProperty) pProperty->unlink();
      }  
      pController->propertiesUnlinked(getScope(), PA_NO_ERROR);
      break;
    }
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

GCFRTMyProperty& GCFRTMyPropertySet::operator[] (const string propName)
{ 
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (!pProperty)
  {
    pProperty = &_dummyProperty;
  }
  return *pProperty;
}

TGCFResult GCFRTMyPropertySet::setValue (const string propName, 
                                         const GCFPValue& value)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}
                             
TGCFResult GCFRTMyPropertySet::setValue (const string propName, 
                                       const string value)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
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
  GCFRTMyProperty* pProperty = getProperty(propName);
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
  GCFRTMyProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->getOldValue();
  }
  else 
  {
    return 0;
  }
}                                         

void GCFRTMyPropertySet::setAnswer (GCFRTAnswer* pAnswerObj)
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
  GCFRTMyProperty* pProperty = getProperty(propName);
  return (pProperty != 0);
}

void GCFRTMyPropertySet::valueSet(const string& propName, const GCFPValue& value) const
{
  GPMRTController* pController = GPMRTController::instance();
  assert(pController);
  pController->valueSet(propName, value);  
}

void GCFRTMyPropertySet::valueChanged(string propName, const GCFPValue& value)
{
  GCFRTMyProperty* pProperty = getProperty(propName);
  assert(pProperty);
  pProperty->valueChanged(value);
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

void GCFRTMyPropertySet::addProperty(const string& propName, GCFRTMyProperty& prop)
{
  assert(propName.length() > 0);
  
  string shortPropName(propName);
  cutScope(shortPropName);
  
  TPropertyList::iterator iter = _properties.find(shortPropName);
  if (iter == _properties.end())
  {
    _properties[shortPropName] = &prop;
  }
  else
  {
    LOG_DEBUG(LOFAR::formatString ( 
      "Property %s already existing in scope '%s'",
      shortPropName.c_str(),
      _scope.c_str()));
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

void GCFRTMyPropertySet::wrongState(const char* request)
{
  char* stateString(0);
  switch (_state)
  {
    case S_DISABLED:
      stateString = "DISABLED";
      break;
    case S_DISABLING:
      stateString = "DISABLING";
      break;
    case S_ENABLED:
      stateString = "ENABLED";
      break;
    case S_ENABLING:
      stateString = "ENABLING";
      break;
    case S_LINKING:
      stateString = "LINKING";
      break;
    case S_LINKED:
      stateString = "LINKED";
      break;
    case S_DELAYED_DISABLING:
      stateString = "DELAYED_DISABLING";
      break;    
  }
  LOG_WARN(LOFAR::formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        getScope().c_str(),
        stateString));  
}


//#  GCF_MyPropertySet.cc: 
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

#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_MyProperty.h>
#include <GPM_Controller.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/Utils.h>

GCFMyPropertySet::GCFMyPropertySet(const char* name,
                                   const TPropertySet& typeInfo,                                   
                                   GCFAnswer* pAnswerObj) : 
  GCFPropertySet(name, typeInfo, pAnswerObj),
  _counter(0),
  _missing(0), 
  _state(S_DISABLED)
{
  loadPropSetIntoRam();
}


GCFMyPropertySet::~GCFMyPropertySet ()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  // delete this set from the controller permanent
  // this means no response will be send to this object
  // on response of the PA
  pController->unregisterScope(*this, true); 
}

GCFProperty* GCFMyPropertySet::createPropObject(TProperty& propInfo)
{
  return new GCFMyProperty(propInfo, *this);
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
    const TProperty* pPropertyFields;
    for (unsigned int i = 0; i < _propSetInfo.nrOfProperties; i++)
    {
      pPropertyFields = &_propSetInfo.properties[i];
      if (pPropertyFields->defaultValue)
      {
        setValue(pPropertyFields->propName, pPropertyFields->defaultValue);
      }
    }
    GPMController* pController = GPMController::instance();
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
  
  if (_state != S_ENABLED)
  {
    wrongState("disable");
    result = GCF_WRONG_STATE;
  }
  else
  {
    LOG_INFO(LOFAR::formatString ( 
        "REQ: Unregister scope %s",
        getScope().c_str()));

    GPMController* pController = GPMController::instance();
    assert(pController);
    GCFMyProperty* pProperty;
    for (TPropertyList::iterator iter = _properties.begin();
         iter != _properties.end(); ++iter)
    {
      pProperty = (GCFMyProperty*)(iter->second);
      assert(pProperty);
      if (pProperty->isLinked())
      {
        pProperty->unlink();
      }
    }

    TPMResult pmResult = pController->unregisterScope(*this);
    assert(pmResult == PM_NO_ERROR);

    _state = 
  }
  return result;
}

TGCFResult GCFMyPropertySet::setValue (const string propName, 
                                       const string value)
{
  GCFMyProperty* pProperty = (GCFMyProperty*)getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}

GCFPValue* GCFMyPropertySet::getValue (const string propName)
{
  GCFMyProperty* pProperty = (GCFMyProperty*)getProperty(propName);
  if (pProperty)
  {
    return pProperty->getValue();
  }
  else 
  {
    return 0;
  }
}
                                     
GCFPValue* GCFMyPropertySet::getOldValue (const string propName)
{
  GCFMyProperty* pProperty = (GCFMyProperty*)getProperty(propName);
  if (pProperty)
  {
    return pProperty->getOldValue();
  }
  else 
  {
    return 0;
  }
}                                         
          
void GCFMyPropertySet::scopeRegistered (TGCFResult result)
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
    _state == S_DISABLED;
  }

  dispatchAnswer(F_MYPS_ENDABLED, result);
}

void GCFMyPropertySet::scopeUnregistered (TGCFResult result)
{
  assert(_state == S_DISABLING);
   
  LOG_INFO(LOFAR::formatString ( 
      "PA-RESP: Scope %s unregistered",
      getScope().c_str()));

  _state = S_DISABLED;
  dispatchAnswer(F_MYPS_DISABLED, result);
}

void GCFMyPropertySet::linkProperties()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKING:
    case S_LINKED:
      wrongState("linkProperties");
      pController->propertiesLinked(getScope(), PA_WRONG_STATE);
      break;
    case S_DISABLED:
    case S_DISABLING:
      pController->propertiesLinked(getScope(), PA_PROP_SET_GONE);
      break;
    case S_ENABLED:
      assert(_counter == 0);
      _missing = 0;
      _state = S_LINKING;
      retryLinking();
      break;
  }
}

void GCFMyPropertySet::retryLinking()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKED:
    case S_ENABLED:
      wrongState("retryLinking");
      pController->propertiesLinked(getScope(), PA_WRONG_STATE);
      break;
    case S_DISABLED:
    case S_DISABLING:
      pController->propertiesLinked(getScope(), PA_PROP_SET_GONE);
      break;
    case S_LINKING:
    {
      TPropertyList::iterator iter = _properties.begin();
      pProperty = (GCFMyProperty*) iter->second;
      assert(pProperty);
      if (pProperty->exists())
      {
        for(;iter != _properties.end(); ++iter)
        {
          pProperty = (GCFMyProperty*) iter->second;
          assert(pProperty);
          if (pProperty->exists())
          {
            if (pProperty->link()) 
            {
              _counter++;
            }
          }
          else 
          {
            _missing++;
          }
        }
        if (_counter == 0)
        {
          // no more asyncronous link responses will be expected and 
          // no more properties needed to be linked 
          // so we can return a response to the controller
          _state = S_LINKED;
          if (_missing > 0)
          {
            pController->propertiesLinked(getScope(), PA_MISSING_PROPS);
          }
          else
          {        
            pController->propertiesLinked(getScope(), PA_NO_ERROR);
          }
        }
      }
      break;
    }
  }
}

void GCFMyPropertySet::linked (GCFMyProperty& prop)
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  _counter--;
  if (_counter == 0)
  {
    switch (_state)
    {
      case S_ENABLING:
      case S_LINKED:
      case S_ENABLED:
        wrongState("linked");
        prop.unlink();
        pController->propertiesLinked(getScope(), PA_WRONG_STATE);
        break;
      case S_DISABLED:
      case S_DISABLING:
        prop.unlink();
        pController->propertiesLinked(getScope(), PA_PROP_SET_GONE);
        break;
      case S_LINKING:
      {
        _state = S_LINKED;
        if (_missing > 0)
        {
          pController->propertiesLinked(getScope(), PA_MISSING_PROPS);
        }
        else
        {        
          pController->propertiesLinked(getScope(), PA_NO_ERROR);
        }
        break;
      }
    }
  }
  else if (_state == S_DISABLING)
  {
    prop.unlink();
  }
}

void GCFMyPropertySet::unlinkProperties()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKED:
    case S_ENABLED:
      wrongState("unlinkProperties");
      prop.unlink();
      pController->propertiesUnlinked(getScope(), PA_WRONG_STATE);
      break;
    case S_DISABLED:
    case S_DISABLING:
      prop.unlink();
      pController->propertiesUnlinked(getScope(), PA_PROP_SET_GONE);
      break;
    case S_LINKED:
    {
      _state = S_ENABLED;
      for (TPropertyList::iterator iter = _properties.begin(); 
           iter != _properties.end(); ++iter)
      {
        pProperty = (GCFMyProperty*) iter->second;
        if (pProperty) pProperty->unlink();
      }  
      pController->propertiesUnlinked(getScope(), PA_NO_ERROR);
      break;
    }
  }
}

void GCFMyPropertySet::wrongState(const char* request)
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
  }
  LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        stateString));  
}
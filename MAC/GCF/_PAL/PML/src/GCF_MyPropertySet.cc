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
#include <GCF/GCF_Defines.h>

GCFMyPropertySet::GCFMyPropertySet(const char* name,
                                   const TPropertySet& typeInfo,                                   
                                   GCFAnswer* pAnswerObj,
                                   TDefaultUse defaultUse) : 
  GCFPropertySet(name, typeInfo, pAnswerObj),
  _state(S_DISABLED),
  _defaultUse(defaultUse),
  _counter(0),
  _missing(0)
{
  loadPropSetIntoRam();
}

GCFMyPropertySet::GCFMyPropertySet(const char* name,
                                   const TPropertySet& typeInfo,
                                   TDefaultUse defaultUse) : 
  GCFPropertySet(name, typeInfo, 0),
  _state(S_DISABLED),
  _defaultUse(defaultUse),
  _counter(0),
  _missing(0)
{
  loadPropSetIntoRam();
}

GCFMyPropertySet::~GCFMyPropertySet ()
{
  if (_state != S_DISABLED)
  {
    GPMController* pController = GPMController::instance();
    assert(pController);
    // delete this set from the controller permanent
    // this means no response will be send to this object
    // on response of the PA
    pController->unregisterScope(*this); 
  }
}

GCFProperty* GCFMyPropertySet::createPropObject(const TProperty& propInfo)
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
  
  switch (_state)
  {
    case S_LINKING:
      assert(_counter > 0);
      _state = S_DELAYED_DISABLING;
      break;
    case S_LINKED:
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
      // intentional fall through
    case S_ENABLED:
    {
      LOG_INFO(LOFAR::formatString ( 
          "REQ: Unregister scope %s",
          getScope().c_str()));
  
      GPMController* pController = GPMController::instance();
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
    _state = S_DISABLED;
  }

  dispatchAnswer(F_MYPS_ENABLED, result);
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

bool GCFMyPropertySet::linkProperties()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  bool successful(true);
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKING:
    case S_LINKED:
    case S_DELAYED_DISABLING:
      wrongState("linkProperties");
      pController->propertiesLinked(getScope(), PA_WRONG_STATE);
      break;
    case S_DISABLED:
    case S_DISABLING:
      pController->propertiesLinked(getScope(), PA_PS_GONE);
      break;
    case S_ENABLED:
      assert(_counter == 0);
      _missing = 0;
      _state = S_LINKING;
      successful = tryLinking();
      break;
  }
  return successful;
}

bool GCFMyPropertySet::tryLinking()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  bool successful(true);
  switch (_state)
  {
    case S_ENABLING:
    case S_LINKED:
    case S_ENABLED:
      wrongState("tryLinking");
      pController->propertiesLinked(getScope(), PA_WRONG_STATE);
      break;
    case S_DELAYED_DISABLING:
      pController->propertiesLinked(getScope(), PA_PS_GONE);
      _state = S_ENABLED;
      disable();
      break;
    case S_DISABLED:
    case S_DISABLING:
      pController->propertiesLinked(getScope(), PA_PS_GONE);
      break;
    case S_LINKING:
    {
      GCFMyProperty* pProperty(0);
      TGCFResult result;
      for(TPropertyList::iterator iter = _properties.begin(); 
          iter != _properties.end(); ++iter)
      {
        pProperty = (GCFMyProperty*) iter->second;
        assert(pProperty);
        if (pProperty->exists())
        {
                 
          if (pProperty->link((_defaultUse == USE_MY_DEFAULTS), result)) 
          {
            _counter++;
          }
          if (result != GCF_NO_ERROR)
          {
            _missing++;
          }
        }
        else 
        {
          _missing++;
        }
      }      
      if (_counter == 0)
      {
        if (_missing == _properties.size())
        {
          // propset is not yet known in this application, retry it with a 
          // 0 timer
          _missing = 0;
          successful = false;
          break;
        }
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
      break;
    }
  }
  return successful;
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
      case S_DELAYED_DISABLING:
        pController->propertiesLinked(getScope(), PA_PS_GONE);
        _state = S_LINKED;
        disable();
        break;
      case S_DISABLED:
      case S_DISABLING:
        prop.unlink();
        pController->propertiesLinked(getScope(), PA_PS_GONE);
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
      GCFMyProperty* pProperty(0);
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

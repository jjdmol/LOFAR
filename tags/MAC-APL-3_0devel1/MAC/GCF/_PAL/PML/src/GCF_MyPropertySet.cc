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
                                   const char* type, 
                                   TPSCategory category,
                                   GCFAnswer* pAnswerObj,
                                   TDefaultUse defaultUse) : 
  GCFPropertySet(name, type, pAnswerObj),
  _state(S_DISABLED),
  _defaultUse((category != PS_CAT_TEMPORARY ? defaultUse : USE_MY_DEFAULTS)),
  _category(category),
  _counter(0),
  _missing(0)
{
  loadPropSetIntoRam();
}

GCFMyPropertySet::GCFMyPropertySet(const char* name,
                                   const char* type, 
                                   TPSCategory category,
                                   TDefaultUse defaultUse) : 
  GCFPropertySet(name, type, 0),
  _state(S_DISABLED),
  _defaultUse((category != PS_CAT_TEMPORARY ? defaultUse : USE_MY_DEFAULTS)),
  _category(category),
  _counter(0),
  _missing(0)
{
  loadPropSetIntoRam();
}

GCFMyPropertySet::~GCFMyPropertySet ()
{
  if (_state != S_DISABLED)
  {
    // delete this set from the controller permanent
    // this means no response will be send to this object
    // on response of the PA
    assert(_pController);
    _pController->unregisterScope(*this); 
  }
}

GCFProperty* GCFMyPropertySet::createPropObject(const TPropertyInfo& propInfo)
{
  return new GCFMyProperty(propInfo, *this);
}
  
TGCFResult GCFMyPropertySet::enable ()
{
  TGCFResult result(GCF_NO_ERROR);
    
  LOG_INFO(LOFAR::formatString ( 
      "REQ: Enable property set '%s'",
      getScope().c_str()));
  if (_state != S_DISABLED)
  {
    wrongState("enable");
    result = GCF_WRONG_STATE;
  }
  else
  {
    assert(_pController);
    TPMResult pmResult = _pController->registerScope(*this);
    
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
  
  LOG_INFO(LOFAR::formatString ( 
      "REQ: Disable property set '%s'",
      getScope().c_str()));
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
        if (pProperty->isMonitoringOn())
        {
          pProperty->unlink();
        }
      }
      // intentional fall through
    case S_ENABLED:
    {
  
      assert(_pController);
      TPMResult pmResult = _pController->unregisterScope(*this);
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
  LOG_INFO(LOFAR::formatString ( 
      "PA-RESP: Property set '%s' is enabled%s",
      getScope().c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));
  if (result == GCF_NO_ERROR)
  {
    _state = S_ENABLED;
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
      "Property set '%s' is disabled%s",
      getScope().c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));

  _state = S_DISABLED;
  dispatchAnswer(F_MYPS_DISABLED, result);
}

bool GCFMyPropertySet::linkProperties()
{
  bool successful(true);
  assert(_pController);
  switch (_state)
  {
    case S_DISABLED:
    case S_DISABLING:
      _pController->propertiesLinked(getScope(), PA_PS_GONE);
      break;
      
    case S_ENABLED:
      assert(_counter == 0);
      _missing = 0;
      _state = S_LINKING;
      successful = tryLinking();
      break;
      
    default:
      wrongState("linkProperties");
      _pController->propertiesLinked(getScope(), PA_WRONG_STATE);
      break;
  }
  return successful;
}

bool GCFMyPropertySet::tryLinking()
{
  bool successful(true);
  assert(_pController);
  switch (_state)
  {
    case S_DELAYED_DISABLING:
      _pController->propertiesLinked(getScope(), PA_PS_GONE);
      _state = S_ENABLED;
      disable();
      break;
      
    case S_DISABLED:
    case S_DISABLING:
      _pController->propertiesLinked(getScope(), PA_PS_GONE);
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
          _pController->propertiesLinked(getScope(), PA_MISSING_PROPS);
        }
        else
        {        
          _pController->propertiesLinked(getScope(), PA_NO_ERROR);
        }
      }
      break;
    }
    default:
      wrongState("tryLinking");
      _pController->propertiesLinked(getScope(), PA_WRONG_STATE);
      break;
  }
  
  return successful;
}

void GCFMyPropertySet::linked (GCFMyProperty& prop)
{
  _counter--;
  if (_counter == 0)
  {
    assert(_pController);
    switch (_state)
    {
      case S_DELAYED_DISABLING:
        _pController->propertiesLinked(getScope(), PA_NO_ERROR);
        _state = S_LINKED;
        disable();
        break;
        
      case S_DISABLED:
      case S_DISABLING:
        prop.unlink();
        _pController->propertiesLinked(getScope(), PA_PS_GONE);
        break;
        
      case S_LINKING:
      {
        _state = S_LINKED;
        if (_missing > 0)
        {
          _pController->propertiesLinked(getScope(), PA_MISSING_PROPS);
        }
        else
        {        
          _pController->propertiesLinked(getScope(), PA_NO_ERROR);
        }
        break;
      }
      default:
        wrongState("linked");
        prop.unlink();
        _pController->propertiesLinked(getScope(), PA_WRONG_STATE);
        break;
    }
  }
  else if (_state == S_DISABLING)
  {
    prop.unlink();
  }    
}

void GCFMyPropertySet::unlinkProperties()
{
  assert(_pController);
  switch (_state)
  {
    case S_DISABLED:
    case S_DISABLING:
      _pController->propertiesUnlinked(getScope(), PA_PS_GONE);
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
      _pController->propertiesUnlinked(getScope(), PA_NO_ERROR);
      break;
    }
    default:
      wrongState("unlinkProperties");
      _pController->propertiesUnlinked(getScope(), PA_WRONG_STATE);
      break;      
  }
}

void GCFMyPropertySet::setAllAccessModes(TAccessMode mode, bool on)
{
  GCFMyProperty* pProperty;
  for(TPropertyList::iterator iter = _properties.begin(); 
      iter != _properties.end(); ++iter)
  {
    pProperty = (GCFMyProperty*) iter->second;
    assert(pProperty);
    pProperty->setAccessMode(mode, on);    
  }
}

void GCFMyPropertySet::initProperties(const TPropertyConfig config[])
{
  GCFMyProperty* pProperty;
  unsigned int i = 0;
  while (config[i].propName != 0)
  {
    pProperty = (GCFMyProperty*) getProperty(config[i].propName);
    if (pProperty)
    {
      if (config[i].defaultValue)
      {
        pProperty->setValue(config[i].defaultValue);
      }
      if (~config[i].accessMode & GCF_READABLE_PROP)
        pProperty->setAccessMode(GCF_READABLE_PROP, false);    
      if (~config[i].accessMode & GCF_WRITABLE_PROP)
        pProperty->setAccessMode(GCF_WRITABLE_PROP, false);    
    }
    i++;
  }
}

void GCFMyPropertySet::wrongState(const char* request)
{
  const char* stateString[] =
  {
    "DISABLED", 
    "DISABLING", 
    "ENABLING", 
    "ENABLED", 
    "LINKING", 
    "LINKED", 
    "DELAYED_DISABLING"
  };
  LOG_WARN(LOFAR::formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        getScope().c_str(),
        stateString[_state]));  
}

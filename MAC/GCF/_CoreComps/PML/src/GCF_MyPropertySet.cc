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

#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_MyProperty.h>
#include <GPM_Controller.h>
#include <GCF/GCF_Answer.h>
#include <Utils.h>

GCFMyPropertySet::GCFMyPropertySet(const TPropertySet& propSet,
                                   GCFAnswer* pAnswerObj) : 
  GCFPropertySetBase(propSet.scope, pAnswerObj),
  _isLoaded(false),
  _isBusy(false),
  _counter(0),
  _missing(0),
  _propSet(propSet)  
{
  init();
}  

GCFMyPropertySet::GCFMyPropertySet(const TPropertySet& propSet,
                                   const char* scope,
                                   GCFAnswer* pAnswerObj) : 
  GCFPropertySetBase((scope ? scope : propSet.scope), pAnswerObj),
  _isLoaded(false),
  _isBusy(false),
  _counter(0),
  _missing(0),
  _propSet(propSet)  
{
  init();
}

void GCFMyPropertySet::init()
{
  GCFMyProperty* pProperty;
  const char* propName;
  for (unsigned int i = 0; i < _propSet.nrOfProperties; i++)
  { 
    propName = _propSet.properties[i].propName;
    if (Utils::isValidPropName(propName))
    {
      pProperty = new GCFMyProperty(_propSet.properties[i], *this);
      addProperty(propName, *pProperty);
    }
    else
    {
      LOFAR_LOG_WARN(PML_STDOUT_LOGGER, ( 
          "Property %s meets not the name convention! NOT CREATED",
          propName));      
    }
  }
  if (getAnswerObj())
  {
    setAnswer(getAnswerObj());
  }
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
  
TGCFResult GCFMyPropertySet::load ()
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
    GPMController* pController = GPMController::instance();
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

TGCFResult GCFMyPropertySet::unload ()
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

    GPMController* pController = GPMController::instance();
    assert(pController);
    GCFMyProperty* pProperty;
    for (TPropertyList::iterator iter = _properties.begin();
         iter != _properties.end(); ++iter)
    {
      pProperty = static_cast<GCFMyProperty*>(iter->second);
      assert(pProperty);
      if (pProperty->isLinked())
      {
        pProperty->unlink();
      }
    }

    TPMResult pmResult = pController->unregisterScope(*this);
    assert(pmResult == PM_NO_ERROR);

    _isBusy = true;
  }
  return result;
}

TGCFResult GCFMyPropertySet::setValue (const string propName, 
                                       const string value)
{
  GCFMyProperty* pProperty = static_cast<GCFMyProperty*>(getProperty(propName));
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
  GCFMyProperty* pProperty = static_cast<GCFMyProperty*>(getProperty(propName));
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
  GCFMyProperty* pProperty = static_cast<GCFMyProperty*>(getProperty(propName));
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
  assert(!_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;
  if (result == GCF_NO_ERROR)
  {
    _isLoaded = true;
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "PA-RESP: Scope %s registered",
        getScope().c_str()));
  }

  dispatchAnswer(F_MYPLOADED_SIG, result);
}

void GCFMyPropertySet::scopeUnregistered (TGCFResult result)
{
  assert(_isLoaded);
  assert(_isBusy);
  
  _isBusy = false;
  _isLoaded = false;
  
  LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
      "PA-RESP: Scope %s unregistered",
      getScope().c_str()));

  dispatchAnswer(F_MYPUNLOADED_SIG, result);
}

void GCFMyPropertySet::linkProperties(list<string>& properties)
{
  if (_isLoaded && !_isBusy)
  {
    _tempLinkList = properties;
  
    assert(_counter == 0);
    
    _missing = 0;
    retryLinking();
  }
  else
  {  
    GPMController* pController = GPMController::instance();
    assert(pController);
    pController->propertiesLinked(getScope(), PA_PROP_SET_GONE);
  }
}

void GCFMyPropertySet::retryLinking()
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  if (_isLoaded && !_isBusy)
  {
    if (_tempLinkList.size() > 0)
    {
      GCFMyProperty* pProperty;
      string fullPropName;
  
      list<string>::iterator iter = _tempLinkList.begin(); 
  
      while (iter != _tempLinkList.end())
      {
        pProperty = static_cast<GCFMyProperty*> (getProperty(*iter));
        if (pProperty)
        {
          if (pProperty->exists())
          {
            if (pProperty->link()) // true means async
            {
              _counter++;
            }      
            iter = _tempLinkList.erase(iter);        
          }
          else
            break;
        }
        else
        {
          _missing++;
        }
      }    
      if (_counter == 0 && _tempLinkList.size() == 0)
      {
        // no more asyncronous link responses will be expected and 
        // no more properties needed to be linked 
        // so we can return a response to the controller
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
  }
  else
  {
    pController->propertiesLinked(getScope(), PA_PROP_SET_GONE);
  }
}

void GCFMyPropertySet::linked ()
{
  _counter--;
  if (_counter == 0 && _tempLinkList.size() == 0)
  {
    GPMController* pController = GPMController::instance();
    assert(pController);
    if (_isLoaded && !_isBusy)
    {
      if (_missing > 0)
      {
        pController->propertiesLinked(getScope(), PA_MISSING_PROPS);
      }
      else
      {
        pController->propertiesLinked(getScope(), PA_NO_ERROR);
      }
    }
    else
    {
      pController->propertiesLinked(getScope(), PA_PROP_SET_GONE);
    }
  }
}

void GCFMyPropertySet::unlinkProperties(list<string>& properties)
{
  GPMController* pController = GPMController::instance();
  assert(pController);
  if (_isLoaded && !_isBusy)
  {
    GCFMyProperty* pProperty;
    
    assert (_counter == 0);
  
    for (list<string>::iterator iter = properties.begin(); 
         iter != properties.end(); ++iter)
    {
      pProperty = static_cast<GCFMyProperty*> (getProperty(*iter));
      if (pProperty) pProperty->unlink();
    }  
    pController->propertiesUnlinked(getScope(), PA_NO_ERROR);
  }
  else
  {
    pController->propertiesUnlinked(getScope(), PA_PROP_SET_GONE);
  }
}

void GCFMyPropertySet::dispatchAnswer(unsigned short sig, TGCFResult result)
{
  if (getAnswerObj() != 0)
  {
    GCFMYPropAnswerEvent e(sig);
    e.pScope = getScope().c_str();
    e.result = result;
    getAnswerObj()->handleAnswer(e);
  }
}

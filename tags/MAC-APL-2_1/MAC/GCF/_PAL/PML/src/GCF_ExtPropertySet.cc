//#  GCF_ExtPropertySet.cc: 
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

#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_Answer.h>
#include "GPM_Controller.h"
#include <GCF/Utils.h>

GCFExtPropertySet::GCFExtPropertySet(const char* name, 
                                     const char* type,                                      
                                     GCFAnswer* pAnswerObj) :
  GCFPropertySet(name, type, pAnswerObj), 
  _isLoaded(false)
{
  loadPropSetIntoRam();
}

GCFExtPropertySet::~GCFExtPropertySet()
{
  if (_isLoaded)
  {
    assert(_pController);
    _pController->unloadPropSet(*this);  
  }
}

GCFProperty* GCFExtPropertySet::createPropObject(const TPropertyInfo& propInfo)
{
  return new GCFExtProperty(propInfo, *this);
}

TGCFResult GCFExtPropertySet::load()
{  
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    LOG_INFO(LOFAR::formatString ( 
        "This property set with Instance name (%s) is busy with an action. Ignored!",
        getScope().c_str()));
    result = GCF_BUSY;
  }
  else if (_isLoaded)
  {
    LOG_INFO(LOFAR::formatString ( 
        "This instance of the property set with Instance name (%s) is already loaded. Ignored!",
        getScope().c_str()));
    result = GCF_ALREADY_LOADED;
  }
  else if (getScope().length() == 0 || 
           !Utils::isValidPropName(getScope().c_str()))
  {
    LOG_INFO(LOFAR::formatString ( 
        "Instance name not set or meets not the naming convention (%s). Ignored!",
        getScope().c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {
    LOG_INFO(LOFAR::formatString ( 
        "REQ: Load ext. property set %s",
        getScope().c_str()));

    assert(_pController);
    TPMResult pmResult = _pController->loadPropSet(*this);
    
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
    else
    {
      result = GCF_EXTPS_LOAD_ERROR;
    }
  }
  return result;
}

void GCFExtPropertySet::loaded(TGCFResult result)
{
  assert(_isBusy);
  assert(!_isLoaded);
  _isBusy = false;
  LOG_INFO(LOFAR::formatString ( 
      "PA-RESP: Prop. set '%s' is loaded%s",
      getScope().c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));
  if (result == GCF_NO_ERROR)
  {
    _isLoaded = true;
  }

  dispatchAnswer(F_EXTPS_LOADED, result);
}

TGCFResult GCFExtPropertySet::unload()
{  
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    LOG_INFO(LOFAR::formatString ( 
        "This property set with Instance name (%s) is busy with an action. Ignored!",
        getScope().c_str()));
    result = GCF_BUSY;
  }
  else if (!_isLoaded)
  {
    LOG_INFO(LOFAR::formatString ( 
        "This instance of the property set with Instance name (%s) was not loaded here. Ignored!",
        getScope().c_str()));
    result = GCF_NOT_LOADED;
  }
  else if (getScope().length() == 0 || 
           !Utils::isValidPropName(getScope().c_str()))
  {
    LOG_INFO(LOFAR::formatString ( 
        "Instance name not set or meets not the naming convention (%s). Ignored!",
        getScope().c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {    
    LOG_INFO(LOFAR::formatString ( 
        "REQ: Unload ext. property set %s",
        getScope().c_str()));

    assert(_pController);
    TPMResult pmResult = _pController->unloadPropSet(*this);
    
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
    else
    {
      result = GCF_EXTPS_UNLOAD_ERROR;
    }
  }
  return result;
}

void GCFExtPropertySet::unloaded(TGCFResult result)
{
  assert(_isBusy);
  assert(_isLoaded);
  _isBusy = false;
  LOG_INFO(LOFAR::formatString ( 
      "PA-RESP: Prop. set '%s' is unloaded%s",
      getScope().c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));

  _isLoaded = false;
  
  GCFExtProperty* pProperty(0);
  for (TPropertyList::iterator iter = _properties.begin();
       iter != _properties.end(); ++iter)
  {
    pProperty = (GCFExtProperty*) iter->second;
    assert(pProperty);
    if (pProperty->isSubscribed())
    {
      pProperty->unsubscribe();
    }
  }

  dispatchAnswer(F_EXTPS_UNLOADED, result);  
}

void GCFExtPropertySet::serverIsGone()
{
  assert(_isLoaded);

  LOG_INFO(LOFAR::formatString ( 
      "PA-IND: Server for prop. set '%s' is gone",
      getScope().c_str()));
  _isLoaded = false;

  GCFExtProperty* pProperty(0);
  for (TPropertyList::iterator iter = _properties.begin();
       iter != _properties.end(); ++iter)
  {
    pProperty = (GCFExtProperty*) iter->second;
    assert(pProperty);
    if (pProperty->isSubscribed())
    {
      pProperty->unsubscribe();
    }
  }

  dispatchAnswer(F_SERVER_GONE, GCF_NO_ERROR);  
}

TGCFResult GCFExtPropertySet::requestValue(const string propName) const
{
  GCFProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->requestValue();    
  }
  else 
  {
    LOG_INFO(LOFAR::formatString ( 
        "This property set has no property '%s'. Ignored!",
        propName.c_str()));
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFExtPropertySet::subscribeProp(const string propName) const
{
  GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
  if (pProperty)
  {
    return pProperty->subscribe();    
  }
  else 
  {
    LOG_INFO(LOFAR::formatString ( 
        "This property set has no property '%s'. Ignored!",
        propName.c_str()));
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFExtPropertySet::unsubscribeProp(const string propName) const
{
  GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
  if (pProperty)
  {
    return pProperty->unsubscribe();    
  }
  else 
  {
    LOG_INFO(LOFAR::formatString ( 
        "This property set has no property '%s'. Ignored!",
        propName.c_str()));
    return GCF_PROP_NOT_IN_SET;
  }
}

bool GCFExtPropertySet::isPropSubscribed (const string propName) const
{
  GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
  if (pProperty)
  {
    return pProperty->isSubscribed();    
  }
  else 
  {
    LOG_INFO(LOFAR::formatString ( 
        "This property set has no property '%s'.",
        propName.c_str()));
    return false;
  }
}

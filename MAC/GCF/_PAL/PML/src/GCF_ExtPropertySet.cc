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
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_ExtProperty.h>

GCFExtPropertySet::GCFExtPropertySet(const char* name, 
                               const TPropertySet& propSetInfo,
                               GCFAnswer* pAnswerObj) :
  GCFPropertySet(name, propSetInfo, pAnswerObj), 
  _isLoaded(false)
{
  loadPropSetIntoRam();
}

GCFProperty* GCFExtPropertySet::createPropObject(TProperty& propInfo) const
{
  return new GCFExtProperty(propInfo, *this);
}

TGCFResult GCFExtPropertySet::load()
{  
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This property set with Instance name (%s) is busy with an action. Ignored!",
        getScope().c_str()));
    result = GCF_BUSY;
  }
  else if (_isLoaded)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This instance of the property set with Instance name (%s) is already loaded. Ignored!",
        getScope().c_str()));
    result = GCF_ALREADY_LOADED;
  }
  else if (getScope().length() == 0 || 
           !Utils::isValidPropName(getScope().c_str()))
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "Instance name not set or meets not the naming convention (%s). Ignored!",
        getScope().c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {
    LOG_INFO(LOFAR::formatString ( 
        "REQ: Load ext. property set %s",
        getScope().c_str()));

    GPMController* pController = GPMController::instance();
    assert(pController);
    TPMResult pmResult = pController->loadPropSet(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
  }
  return result;
}

TGCFResult GCFExtPropertySet::unload()
{  
  TGCFResult result(GCF_NO_ERROR);
  
  if (_isBusy)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This property set with Instance name (%s) is busy with an action. Ignored!",
        getScope().c_str()));
    result = GCF_BUSY;
  }
  else if (!_isLoaded)
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This instance of the property set with Instance name (%s) was not loaded here. Ignored!",
        getScope().c_str()));
    result = GCF_NOT_LOADED;
  }
  else if (_name.length() == 0 || 
           _scope.length() == 0 || 
           !Utils::isValidPropName(_scope.c_str()))
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "APC name or scope not set or scope (%s) meets not the naming convention. Ignored!",
        _scope.c_str()));
    result = GCF_NO_PROPER_DATA;
  }
  else
  {    
    GPMController* pController = GPMController::instance();
    assert(pController);
    _loadDefaults = false;
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "REQ: Unload ext. property set %s",
        getScope().c_str()));
    TPMResult pmResult = pController->unloadPropSet(*this);
    if (pmResult == PM_NO_ERROR)
    {
      _isBusy = true;
    }
  }
  return result;
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
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This property set has no property '%s'. Ignored!",
        propName.c_str()));
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFExtPropertySet::subscribe(const string propName) const
{
  GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
  if (pProperty)
  {
    return pProperty->subscribe();    
  }
  else 
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This property set has no property '%s'. Ignored!",
        propName.c_str()));
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFExtPropertySet::unsubscribe(const string propName) const
{
  GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
  if (pProperty)
  {
    return pProperty->unsubscribe();    
  }
  else 
  {
    LOFAR_LOG_INFO(PML_STDOUT_LOGGER, ( 
        "This property set has no property '%s'. Ignored!",
        propName.c_str()));
    return GCF_PROP_NOT_IN_SET;
  }
}
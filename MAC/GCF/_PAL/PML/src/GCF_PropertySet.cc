//#  GCF_PropertySet.cc: 
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

#include <GCF/PAL/GCF_PropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/GCF_PValue.h>
#include <GCF/Utils.h>
#include <GPM_Defines.h>
#include <GPM_Controller.h>

const TPropertyInfo dummyPropInfo("DUMMY", LPT_BOOL);

GCFPropertySet::GCFPropertySet (const char* name,
                                const char* type,
                                GCFAnswer* pAnswerObj) : 
  _pController(0),
  _pAnswerObj(pAnswerObj),
  _scope((name ? name : "")),
  _type((type ? type : "")),
  _dummyProperty(dummyPropInfo, this),
  _isBusy(false)  
{
  if (!Utils::isValidScope(_scope.c_str()))
  {
    LOG_WARN(LOFAR::formatString ( 
        "Scope %s meets not the name convention! Set to \"\"",
        _scope.c_str()));
    _scope = "";
  }
  _pController = GPMController::instance();
  assert(_pController);
}

GCFPropertySet::~GCFPropertySet()
{
  clearAllProperties();
  _dummyProperty.resetPropSetRef();

  assert(_pController);
  
  _pController->deletePropSet(*this); 
  GPMController::release();  
  _pController = 0;  
}

void GCFPropertySet::loadPropSetIntoRam()
{
  unsigned int sysNr = GCFPVSSInfo::getSysId(_scope);
  GCFPVSSInfo::getTypeStruct(_type, _propSetInfo, sysNr);
  GCFProperty* pProperty;
  TPropertyInfo* pPropInfo;
  for (TPropInfoList::iterator iter = _propSetInfo.begin();
       iter != _propSetInfo.end(); ++iter)
  { 
    pPropInfo = &(*iter);
    pProperty = createPropObject(*pPropInfo);
    addProperty(pPropInfo->propName, *pProperty);
  }
  if (_pAnswerObj)
  {
    setAnswer(_pAnswerObj);
  }
}

GCFProperty* GCFPropertySet::getProperty (const string& propName) const
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

GCFProperty& GCFPropertySet::operator[] (const string& propName)
{ 
  GCFProperty* pProperty = getProperty(propName);
  if (!pProperty)
  {
    pProperty = &_dummyProperty;
  }
  return *pProperty;
}

TGCFResult GCFPropertySet::setValue (const string& propName, 
                                     const GCFPValue& value)
{
  GCFProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}

TGCFResult GCFPropertySet::setValue (const string& propName, 
                                     const string& value)
{
  GCFProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->setValue(value);    
  }
  else 
  {
    return GCF_PROP_NOT_IN_SET;
  }
}
                             
void GCFPropertySet::setAnswer (GCFAnswer* pAnswerObj)
{
  GCFProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    assert(pProperty);
    pProperty->setAnswer(pAnswerObj);
  }
  _pAnswerObj = pAnswerObj;
}

bool GCFPropertySet::exists (const string& propName) const
{
  GCFProperty* pProperty = getProperty(propName);
  if (pProperty)
  {
    return pProperty->exists();    
  }
  else 
  {
    return false;
  }
}

void GCFPropertySet::configure(const string& apcName)
{
  LOG_INFO(LOFAR::formatString ( 
      "REQ: Configure prop. set '%s' with apc '%s'",
      getScope().c_str(), 
      apcName.c_str()));

  assert(_pController);

  TPMResult pmResult = _pController->configurePropSet(*this, apcName);
  assert(pmResult == PM_NO_ERROR);
}

void GCFPropertySet::configured(TGCFResult result, const string& apcName)
{
  LOG_INFO(LOFAR::formatString ( 
      "REQ: Prop. set '%s' with apc '%s' configured%s",
      getScope().c_str(), 
      apcName.c_str(), 
      (result == GCF_NO_ERROR ? "" : " (with errors)")));
  if (_pAnswerObj != 0)
  {
    GCFConfAnswerEvent e;
    e.pScope = getScope().c_str();
    e.pApcName = apcName.c_str();
    e.result = result;
    _pAnswerObj->handleAnswer(e);
  }
}

void GCFPropertySet::addProperty(const string& propName, GCFProperty& prop)
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

bool GCFPropertySet::cutScope(string& propName) const 
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

void GCFPropertySet::clearAllProperties()
{
  GCFProperty* pProperty;
  for (TPropertyList::iterator iter = _properties.begin(); 
        iter != _properties.end(); ++iter) 
  {
    pProperty = iter->second;
    assert(pProperty);
    pProperty->resetPropSetRef();
    delete pProperty;
  }
}

void GCFPropertySet::dispatchAnswer(unsigned short sig, TGCFResult result)
{
  if (_pAnswerObj != 0)
  {
    GCFPropSetAnswerEvent e(sig);
    string fullScope(getFullScope());
    e.pScope = fullScope.c_str();
    e.result = result;
    _pAnswerObj->handleAnswer(e);
  }
}

const string GCFPropertySet::getFullScope () const 
{ 
  if (_scope.find(':') < _scope.length())
  {
    return _scope;
  }
  else
  {
    // system name is not specified on the scope
    return GCFPVSSInfo::getLocalSystemName() + ":" + _scope; 
  }
}

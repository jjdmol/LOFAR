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

#include <lofar_config.h>

#include <GCF/PAL/GCF_PropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/GCF_PValue.h>
#include <GCF/Utils.h>
#include <GPM_Defines.h>
#include <GPM_Controller.h>

namespace LOFAR {
 namespace GCF {
using namespace Common;  
  namespace PAL {
const TPropertyInfo dummyPropInfo("DUMMY", LPT_BOOL);

//
// GCFPropertySet (name, type, answer*)
//
GCFPropertySet::GCFPropertySet (const char* name,
                                const char* type,
                                GCFAnswer* pAnswerObj) : 
	_pController  (0),
	_pAnswerObj   (pAnswerObj),
	_scope		  ((name ? name : "")),
	_type		  ((type ? type : "")),
	_dummyProperty(dummyPropInfo, this),
	_isBusy		  (false)  
{
	LOG_TRACE_FLOW(formatString("GCFPropertySet()(%s,%s)", name, type));

	if (!Common::isValidScope(_scope.c_str())) {
		LOG_WARN(LOFAR::formatString("Scope %s meets not the name convention! Set to \"\"",
									_scope.c_str()));
		_scope = "";
	}

	_pController = GPMController::instance();
	ASSERT(_pController);
}

//
// ~GCFPropertySet
//
GCFPropertySet::~GCFPropertySet()
{
	clearAllProperties();
	_dummyProperty.resetPropSetRef();

	ASSERT(_pController);

	_pController->deletePropSet(*this); 
	GPMController::release();  
	_pController = 0;  
}

//
// loadPropSetIntoRam()
//
void GCFPropertySet::loadPropSetIntoRam()
{
	// Get ID of this PVSS database
	int8 sysNr = GCFPVSSInfo::getSysId(_scope);
	if (sysNr == 0) {
		sysNr = GCFPVSSInfo::getLocalSystemId();
	}

	GCFPVSSInfo::getTypeStruct(_type, _propSetInfo, sysNr);

	GCFProperty* 	pProperty;
	TPropertyInfo*	pPropInfo;
	for (TPropInfoList::iterator iter = _propSetInfo.begin();
									iter != _propSetInfo.end(); ++iter) { 
		pPropInfo = &(*iter);
		pProperty = createPropObject(*pPropInfo);
		addProperty(pPropInfo->propName, *pProperty);
	}

	if (_pAnswerObj) {
		setAnswer(_pAnswerObj);
	}
}

//
// getProperty (propName)
//
GCFProperty* GCFPropertySet::getProperty (const string& propName) const
{
	string 	shortPropName(propName);

	cutScope(shortPropName);

	TPropertyList::const_iterator iter = _properties.find(shortPropName);
	if (iter != _properties.end()) {
		return iter->second;
	}
	else {
		return 0;
	}
}

//
// operator[](propName)
//
GCFProperty& GCFPropertySet::operator[] (const string& propName)
{ 
	GCFProperty* pProperty = getProperty(propName);
	if (!pProperty) {
		pProperty = &_dummyProperty;
	}

	return *pProperty;
}

//
// setValueTimed(propName, value, timestamp, wantAnswer)
//
TGCFResult GCFPropertySet::setValueTimed (const string& propName, 
                                     const GCFPValue& value,
                                     double timestamp, 
                                     bool wantAnswer)
{
	GCFProperty* pProperty = getProperty(propName);
	if (pProperty) {
		return pProperty->setValueTimed(value, timestamp, wantAnswer);    
	}
	else {
		return GCF_PROP_NOT_IN_SET;
	}
}

//
// setValueTimed(string, value, timestamp, wantAnswer)
//
TGCFResult GCFPropertySet::setValueTimed (const string& propName, 
                                     const string& value,
                                     double timestamp, 
                                     bool wantAnswer)
{
	GCFProperty* pProperty = getProperty(propName);
	if (pProperty) {
		return pProperty->setValueTimed(value, timestamp, wantAnswer);    
	}
	else {
		return GCF_PROP_NOT_IN_SET;
	}
}
                             
//
// setAnswer(answer*)
//
void GCFPropertySet::setAnswer (GCFAnswer* pAnswerObj)
{
	GCFProperty* pProperty;
	for (TPropertyList::iterator iter = _properties.begin(); 
									iter != _properties.end(); ++iter) {
		pProperty = iter->second;
		ASSERT(pProperty);
		pProperty->setAnswer(pAnswerObj);
	}
	_pAnswerObj = pAnswerObj;
}

//
// exists(propName)
//
bool GCFPropertySet::exists (const string& propName) const
{
	GCFProperty* pProperty = getProperty(propName);
	if (pProperty) {
		return pProperty->exists();    
	}
	else {
		return false;
	}
}

//
// configure(APCname)
//
void GCFPropertySet::configure(const string& apcName)
{
	LOG_DEBUG(formatString ("REQ: Configure prop. set '%s' with apc '%s'",
												getScope().c_str(), apcName.c_str()));

	ASSERT(_pController);

	TPMResult pmResult = _pController->configurePropSet(*this, apcName);
	ASSERT(pmResult == PM_NO_ERROR);
}

//
// configured (result, APCname)
//
void GCFPropertySet::configured(TGCFResult result, const string& apcName)
{
	LOG_DEBUG(LOFAR::formatString ("REQ: Prop. set '%s' with apc '%s' configured%s",
								getScope().c_str(), apcName.c_str(), 
								(result == GCF_NO_ERROR ? "" : " (with errors)")));

	if (_pAnswerObj != 0) {
		GCFConfAnswerEvent e;
		string fullScope(getFullScope());
		e.pScope = fullScope.c_str();
		e.pApcName = apcName.c_str();
		e.result = result;
		_pAnswerObj->handleAnswer(e);
	}
}

//
// addProperty(propName, prop)
//
void GCFPropertySet::addProperty(const string& propName, GCFProperty& prop)
{
	ASSERT(propName.length() > 0);

	string shortPropName(propName);
	cutScope(shortPropName);

	TPropertyList::iterator iter = _properties.find(shortPropName);
	if (iter == _properties.end()) {
		_properties[shortPropName] = &prop;
	}
}

//
// cutScope(propName)
//
bool GCFPropertySet::cutScope(string& propName) const 
{
	bool scopeFound(false);

	if (propName.find(_scope) == 0) {
		// plus 1 means erase the GCF_PROP_NAME_SEP after scope too
		propName.erase(0, _scope.size() + 1); 
		scopeFound = true;
	}

	return scopeFound;
}

//
// clearAllProperties()
//
void GCFPropertySet::clearAllProperties()
{
	GCFProperty* pProperty;
	for (TPropertyList::iterator iter = _properties.begin(); 
									iter != _properties.end(); ++iter) {
		pProperty = iter->second;
		ASSERT(pProperty);
		pProperty->resetPropSetRef();
		delete pProperty;
	}
}

//
// dispatchAnswer(signal, result)
//
void GCFPropertySet::dispatchAnswer(unsigned short sig, TGCFResult result)
{
	if (_pAnswerObj != 0) {
		GCFPropSetAnswerEvent e(sig);
		string fullScope(getFullScope());
		e.pScope = fullScope.c_str();
		e.result = result;
		_pAnswerObj->handleAnswer(e);
	}
}

//
// getFullScope()
//
const string GCFPropertySet::getFullScope () const 
{ 
	if (_scope.find(':') < _scope.length()) {
		return _scope;
	}
	else {
		// system name is not specified on the scope
		return GCFPVSSInfo::getLocalSystemName() + ":" + _scope; 
	}
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

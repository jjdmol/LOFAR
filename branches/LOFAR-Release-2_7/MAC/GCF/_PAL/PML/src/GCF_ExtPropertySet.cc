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

#include <lofar_config.h>

#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/PAL/GCF_Answer.h>
#include "GPM_Controller.h"
#include <GCF/Utils.h>

namespace LOFAR {
 namespace GCF {
  using namespace Common;
  namespace PAL {

//
// GCFExtPropertySet(name,type,answer*)
//
GCFExtPropertySet::GCFExtPropertySet(const char*	name, 
									 const char*	type,
									 GCFAnswer*		pAnswerObj) :
	GCFPropertySet(name, type, pAnswerObj), 
	_isLoaded(false)
{
	LOG_DEBUG(formatString("GCFExtPropertySet()(scope=%s,type=%s)",
							getScope().c_str(), getType().c_str()));
	loadPropSetIntoRam();
}


//
// GCFExtPropertySet()
//
GCFExtPropertySet::~GCFExtPropertySet()
{
	if (_isLoaded) {
		ASSERT(_pController);
		_pController->unloadPropSet(*this);  
	}
}


//
// createPropObject
//
GCFProperty* GCFExtPropertySet::createPropObject(const TPropertyInfo& propInfo)
{
	return (new GCFExtProperty(propInfo, *this));
}


//
// load()
//
TGCFResult GCFExtPropertySet::load()
{  
	if (_isBusy) {
		LOG_DEBUG(formatString("PropertySet(%s) is busy and can not be loaded. Ignored!", 
							 getScope().c_str()));
		return (GCF_BUSY);
	}

	if (_isLoaded) {
		LOG_DEBUG(formatString("PropertySet(%s) is already loaded. Ignored!", 
								getScope().c_str()));
		return(GCF_ALREADY_LOADED);
	}

	if (getScope().length() == 0) {
		LOG_DEBUG(formatString ("load:Instance name not set.", getScope().c_str()));
		return(GCF_NO_PROPER_DATA);
	}

	LOG_DEBUG(formatString("REQ: Load ext. property set %s", getScope().c_str()));

	ASSERT(_pController);
	TPMResult pmResult = _pController->loadPropSet(*this);
	if (pmResult != PM_NO_ERROR) {
		return(GCF_EXTPS_LOAD_ERROR);
	}

	// finally! everything went well, mark it busy.
	_isBusy = true;
	return (GCF_NO_ERROR);
}


//
// loaded(result)
//
void GCFExtPropertySet::loaded(TGCFResult result)
{
	ASSERT(_isBusy);
	ASSERT(!_isLoaded);

	_isBusy = false;
	LOG_DEBUG(formatString("PA-RESP: Prop. set '%s' is loaded%s", getScope().c_str(), 
									(result == GCF_NO_ERROR ? "" : " (with errors)")));
	if (result == GCF_NO_ERROR) {
		_isLoaded = true;
	}

	dispatchAnswer(F_EXTPS_LOADED, result);
}

//
// unload()
//
TGCFResult GCFExtPropertySet::unload()
{  
	if (_isBusy) {
		LOG_DEBUG(formatString("PropertySet(%s) is busy and can not be unloaded!",
							getScope().c_str()));
		return (GCF_BUSY);
	}

	if (!_isLoaded) {
		LOG_DEBUG(formatString ("PropertySet(%s) was not loaded here!", 
								getScope().c_str()));
		return (GCF_NOT_LOADED);
	}

	if (getScope().length() == 0) {
		LOG_DEBUG(formatString ("unload:Instance name is not set.", getScope().c_str()));
		return (GCF_NO_PROPER_DATA);
	}

	LOG_DEBUG(formatString ("REQ: Unload ext. property set %s", getScope().c_str()));

	ASSERT(_pController);
	TPMResult pmResult = _pController->unloadPropSet(*this);
	if (pmResult != PM_NO_ERROR) {
		return (GCF_EXTPS_UNLOAD_ERROR);
	}

	_isBusy = true;
	return (GCF_NO_ERROR);
}

//
// unloaded(result)
//
void GCFExtPropertySet::unloaded(TGCFResult result)
{
	ASSERT(_isBusy);
	ASSERT(_isLoaded);

	_isBusy   = false;
	_isLoaded = false;
	LOG_DEBUG(formatString ("PA-RESP: Prop. set '%s' is unloaded%s", getScope().c_str(), 
							  (result == GCF_NO_ERROR ? "" : " (with errors)")));

	// unsubscribe to all properties
	GCFExtProperty* pProperty(0);
	for (TPropertyList::iterator iter = _properties.begin(); 
								 iter != _properties.end(); ++iter) {
		pProperty = (GCFExtProperty*) iter->second;
		ASSERT(pProperty);
		if (pProperty->isSubscribed()) {
			pProperty->unsubscribe();
		}
	}

	dispatchAnswer(F_EXTPS_UNLOADED, result);  
}

//
// serverIsGone()
//
void GCFExtPropertySet::serverIsGone()
{
	_isBusy = false;

	if (!_isLoaded) {
		return;
	}
		
	LOG_INFO(formatString ("PA-IND: Server for prop. set '%s' is gone", 
							getScope().c_str()));
	_isLoaded = false;

	// unsubscribe to all properties
	GCFExtProperty* pProperty(0);
	for (TPropertyList::iterator iter = _properties.begin();
								iter != _properties.end(); ++iter) {
		pProperty = (GCFExtProperty*) iter->second;
		ASSERT(pProperty);
		if (pProperty->isSubscribed()) {
			pProperty->unsubscribe();
		}
	}

	dispatchAnswer(F_SERVER_GONE, GCF_NO_ERROR);  
}


//
// requestValue(propname)
//
TGCFResult GCFExtPropertySet::requestValue(const string propName) const
{
	GCFProperty* pProperty = getProperty(propName);
	if (pProperty) {
		return (pProperty->requestValue());
	}

	LOG_DEBUG(formatString ("PropertySet(%s) has no property.", propName.c_str()));
	return (GCF_PROP_NOT_IN_SET);
}

//
// subscribeProp(propname)
//
TGCFResult GCFExtPropertySet::subscribeProp(const string propName) const
{
	GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
	if (pProperty) {
		return (pProperty->subscribe());
	}

	LOG_DEBUG(formatString ("PropertySet has no property '%s'.", propName.c_str()));
	return (GCF_PROP_NOT_IN_SET);
}

//
// unsubscribeProp(propname)
//
TGCFResult GCFExtPropertySet::unsubscribeProp(const string propName) const
{
	GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
	if (pProperty) {
		return (pProperty->unsubscribe());
	}

	LOG_DEBUG(formatString ("PropertySet has no property '%s'.", propName.c_str()));
	return (GCF_PROP_NOT_IN_SET);
}

//
// isPropSubscribed(propname)
//
bool GCFExtPropertySet::isPropSubscribed (const string propName) const
{
	GCFExtProperty* pProperty = (GCFExtProperty*) getProperty(propName);
	if (pProperty) {
		return pProperty->isSubscribed();    
	}

	LOG_DEBUG(formatString ("PropertySet has no property '%s'.", propName.c_str()));
	return (false);
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

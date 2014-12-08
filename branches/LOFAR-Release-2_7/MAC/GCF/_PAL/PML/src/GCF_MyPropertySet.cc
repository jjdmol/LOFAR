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

#include <lofar_config.h>

#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_MyProperty.h>
#include <GPM_Controller.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/Utils.h>
#include <GCF/GCF_Defines.h>

namespace LOFAR {
 namespace GCF {
using namespace Common;  
  namespace PAL {

//
// GCFMyPropertySet(name, type, cat, answer*, useDflts)
//
GCFMyPropertySet::GCFMyPropertySet(const char* name,
                                   const char* type, 
                                   TPSCategory category,
                                   GCFAnswer*  pAnswerObj,
                                   TDefaultUse defaultUse) : 
  GCFPropertySet(name, type, pAnswerObj),
  _state(S_DISABLED),
  _defaultUse((category != PS_CAT_TEMPORARY ? defaultUse : USE_MY_DEFAULTS)),
  _category(category),
  _counter(0),
  _missing(0)
{
	LOG_DEBUG(formatString("GCFMyPropertySet()(scope=%s,type=%s)",
								getScope().c_str(), getType().c_str()));
	loadPropSetIntoRam();
}


//
// GCFMyPropertySet(name, type, cat, useDflts)
//
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
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
										getScope().c_str(),getType().c_str()));
	loadPropSetIntoRam();
}

//
// GCFMyPropertySet()
//
GCFMyPropertySet::~GCFMyPropertySet ()
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)",__PRETTY_FUNCTION__,getScope().c_str(),getType().c_str()));

	if (_state != S_DISABLED && _state != S_DISABLING) {
		// the baseclass deletes this set from the controller permanent
		// this means no response will be send to this object
		// on response of the PA
		ASSERT(_pController);
		_pController->unregisterScope(*this); 
	}
}

//
// createPropObject(propInfo)
//
GCFProperty* GCFMyPropertySet::createPropObject(const TPropertyInfo& propInfo)
{
	return new GCFMyProperty(propInfo, *this);
}
  
//
// enable
//
TGCFResult GCFMyPropertySet::enable ()
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
											getScope().c_str(), getType().c_str()));

	// PS should be in disabled state
 	if (_state != S_DISABLED) {		
		wrongState("enable");
		return(GCF_WRONG_STATE);
	}

	// register PS
	ASSERT(_pController);
	// ask controller to send a request to the PA.
	TPMResult pmResult = _pController->registerScope(*this); 

	if (pmResult == PM_NO_ERROR) {
		_state = S_ENABLING;
		return (GCF_NO_ERROR);
	}

	if (pmResult == PM_SCOPE_ALREADY_EXISTS) {
		LOG_ERROR(LOFAR::formatString ( 
				  "Property set with scope %s already exists in this Application",
				  getScope().c_str()));    
		return (GCF_SCOPE_ALREADY_REG);
	}

	return (GCF_NO_ERROR);
}

//
// disable()
//
TGCFResult GCFMyPropertySet::disable ()
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
												getScope().c_str(), getType().c_str()));
	TGCFResult result(GCF_NO_ERROR);

	switch (_state) {
	case S_LINKING:		// still the linking process?
		ASSERT(_counter > 0);
		_state = S_DELAYED_DISABLING;
	break;

	case S_LINKED:		// when linked, unlink it first
		GCFMyProperty* pProperty;
		for (TPropertyList::iterator iter = _properties.begin(); 
									 iter != _properties.end(); ++iter) {
			pProperty = (GCFMyProperty*)(iter->second);
			ASSERT(pProperty);
			if (pProperty->isMonitoringOn()) {
				pProperty->unlink();
			}
		}
	// intentional fall through

	case S_ENABLED: {	// only enabled, not linked?
		ASSERT(_pController);
		// send PA a request to unregister this propertySet.
		TPMResult pmResult = _pController->unregisterScope(*this);
		ASSERT(pmResult == PM_NO_ERROR);  // REO ASSERT????
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

//
// getValue(propName)
//
GCFPValue* GCFMyPropertySet::getValue (const string propName)
{
	GCFMyProperty* pProperty = (GCFMyProperty*)getProperty(propName);
	if (pProperty) {
		return pProperty->getValue();
	}

	return 0;
}
                                     
//
// getOldValue(propName)
//
GCFPValue* GCFMyPropertySet::getOldValue (const string propName)
{
	GCFMyProperty* pProperty = (GCFMyProperty*)getProperty(propName);
	if (pProperty) {
		return pProperty->getOldValue();
	}

    return 0;
}                                         
          
//
// scopeRegistered(result)
//
// EventHandler: Handle the response from the registering of a PS.
//
void GCFMyPropertySet::scopeRegistered (TGCFResult result)
{
	ASSERT(_state == S_ENABLING);

	LOG_INFO(LOFAR::formatString ("PA-RESP: Property set '%s' is enabled%s", 
				getScope().c_str(), (result == GCF_NO_ERROR ? "" : " (with errors)")));

	if (result == GCF_NO_ERROR) {
		_state = S_ENABLED;
	}
	else {
		_state = S_DISABLED;
	}

	dispatchAnswer(F_MYPS_ENABLED, result);
}

//
// scopeUnregistered(result)
//
// EventHandler: Handle the response from the unregistering of a PS.
//
void GCFMyPropertySet::scopeUnregistered (TGCFResult result)
{
	ASSERT(_state == S_DISABLING);

	LOG_DEBUG(LOFAR::formatString("Property set '%s' is disabled%s", getScope().c_str(), 
								(result == GCF_NO_ERROR ? "" : " (with errors)")));

	_state = S_DISABLED;

	dispatchAnswer(F_MYPS_DISABLED, result);
}

//
// linkProperties
//
bool GCFMyPropertySet::linkProperties()
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
										getScope().c_str(), getType().c_str()));
	bool successful(true);
	ASSERT(_pController);

	switch (_state) {
	case S_DISABLED:
	case S_DISABLING:
		_pController->propertiesLinked(getScope(), PA_PS_GONE);
	break;

	case S_ENABLED:
		ASSERT(_counter == 0);
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

//
// tryLinking()
//
bool GCFMyPropertySet::tryLinking()
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
											getScope().c_str(),getType().c_str()));
	bool successful(true);
	ASSERT(_pController);

	switch (_state) {
	case S_DELAYED_DISABLING:
		_pController->propertiesLinked(getScope(), PA_PS_GONE);
		_state = S_ENABLED;
		disable();
	break;

	case S_DISABLED:
	case S_DISABLING:
		_pController->propertiesLinked(getScope(), PA_PS_GONE);
	break;

	case S_LINKING: {
		GCFMyProperty* pProperty(0);
		TGCFResult result;
		for (TPropertyList::iterator iter = _properties.begin(); 
									iter != _properties.end(); ++iter) {
			pProperty = (GCFMyProperty*) iter->second;
			ASSERT(pProperty);
			if (pProperty->exists()) {
				if (pProperty->link((_defaultUse == USE_MY_DEFAULTS), result)) {
					_counter++;
				}
				if (result != GCF_NO_ERROR) {
					_missing++;
				}
			}
			else {
				_missing++;
			}
		}      
		if (_counter == 0) {
			if (_missing == _properties.size()) {
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
			_pController->propertiesLinked(getScope(), (_missing > 0 ? PA_MISSING_PROPS : PA_NO_ERROR));
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

//
// linked(PS)
//
// EventHandler
//
void GCFMyPropertySet::linked (GCFMyProperty& prop)
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
									getScope().c_str(), getType().c_str()));
	_counter--;
	if (_counter == 0) {
		ASSERT(_pController);

		switch (_state) {
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

		case S_LINKING: {
			_state = S_LINKED;
			if (_missing > 0) {
				_pController->propertiesLinked(getScope(), PA_MISSING_PROPS);
			}
			else {        
				_pController->propertiesLinked(getScope(), PA_NO_ERROR);
			}
		}
		break;

		default:
			wrongState("linked");
			prop.unlink();
			_pController->propertiesLinked(getScope(), PA_WRONG_STATE);
		break;
		}
	}
	else if (_state == S_DISABLING) {
		prop.unlink();
	}    
}

//
// unlinkProperties()
//
void GCFMyPropertySet::unlinkProperties()
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__, 
											getScope().c_str(),getType().c_str()));
	ASSERT(_pController);

	switch (_state) {
	case S_DISABLED:
	case S_DISABLING:
		_pController->propertiesUnlinked(getScope(), PA_PS_GONE);
		break;

	case S_LINKED: {
		_state = S_ENABLED;
		GCFMyProperty* pProperty(0);
		for (TPropertyList::iterator iter = _properties.begin(); 
									   iter != _properties.end(); ++iter) {
			pProperty = (GCFMyProperty*) iter->second;
			if (pProperty) {
				pProperty->unlink();
			}
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

//
// setAllAccessModes(mode, on)
//
void GCFMyPropertySet::setAllAccessModes(TAccessMode mode, bool on)
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
											getScope().c_str(),getType().c_str()));

	GCFMyProperty* pProperty;
	for(TPropertyList::iterator iter = _properties.begin(); 
								iter != _properties.end(); ++iter) {
		pProperty = (GCFMyProperty*) iter->second;
		ASSERT(pProperty);
		pProperty->setAccessMode(mode, on);    
	}
}

//
// initProperties(config[])
//
void GCFMyPropertySet::initProperties(const TPropertyConfig config[])
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
											getScope().c_str(),getType().c_str()));

	GCFMyProperty* pProperty;
	unsigned int i = 0;
	while (config[i].propName != 0) {
		pProperty = (GCFMyProperty*) getProperty(config[i].propName);
		if (pProperty) {
			if (config[i].defaultValue) {
				pProperty->setValue(config[i].defaultValue);
			}
			if (~config[i].accessMode & GCF_READABLE_PROP) {
				pProperty->setAccessMode(GCF_READABLE_PROP, false);    
			}
			if (~config[i].accessMode & GCF_WRITABLE_PROP) {
				pProperty->setAccessMode(GCF_WRITABLE_PROP, false);    
			}
		}
		i++;
	}
}

//
// wrongState(request)
//
void GCFMyPropertySet::wrongState(const char* request)
{
	LOG_DEBUG(formatString("%s(scope=%s,type=%s)", __PRETTY_FUNCTION__,
											getScope().c_str(),getType().c_str()));
	const char* stateString[] = {
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
				request, getScope().c_str(), stateString[_state]));  
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

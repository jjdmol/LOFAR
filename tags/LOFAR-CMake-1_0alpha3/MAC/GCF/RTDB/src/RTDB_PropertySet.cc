//#  RTDB_PropertySet.cc: 
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

#include <Common/StreamUtil.h>
//#include <GCF/Utils.h>
#include <GCF/PVSS/GCF_PValue.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include "PropSetResponse.h"

#include <unistd.h>

namespace LOFAR {
  namespace GCF {
//	using namespace Common;
	using namespace TM;
	using namespace PVSS;
	namespace RTDB {

//
// RTDBPropertySet (name, type, answer*)
//
RTDBPropertySet::RTDBPropertySet (const string& 	name,
                                  const string& 	type,
								  uint32			accessType,
								  GCFTask*			clientTask) :
	itsScope	  	  (name),
	itsType		  	  (type),
	itsAccessType 	  (accessType),
	itsService	  	  (0),
	itsOwnResponse	  (0),
	itsExtResponse	  (new DPanswer(clientTask)),
	itsExtSubscription(false),
	itsExtConfirmation(true)
{
	LOG_TRACE_FLOW_STR("RTDBPropertySet(" << name << "," << type << "," << accessType << ")");

	itsOwnResponse = new PropSetResponse(this);
	ASSERTSTR(itsOwnResponse, "Can't allocate Response class for PropertySet " << name);

	itsService = new PVSSservice(itsOwnResponse);
	ASSERTSTR(itsService, "Can't connect to PVSS database for " << itsScope);

	// check name convention of DP
	if (!PVSSinfo::isValidScope(itsScope.c_str())) {
		LOG_WARN_STR("Scope " << itsScope << " does not meet the nameconvention! Set to \"\"");
		itsScope = "";
	}

	// create dp if it doesn't exist yet.
	if (!PVSSinfo::propExists(itsScope)) {
		itsService->dpCreate(itsScope, itsType);
		// note: PVSS will call via PropSetResponse object dpCreated.
	}
	else {
		// DP already exists in the DB, immediately continue with 'created' function.
		dpCreated(itsScope, SA_NO_ERROR);
	}
}

//
// ~RTDBPropertySet
//
RTDBPropertySet::~RTDBPropertySet()
{
	LOG_TRACE_FLOW_STR("~RTDBPropertySet(" << "?" << ")");

	_deleteAllProperties();// cleanup propMap

	if (itsAccessType & PSAT_TMP_MASK) {
		itsService->dpDelete(itsScope);
		itsService->doWork();		// force DBmanager to do its work
		while (PVSSinfo::propExists(itsScope)) {
			itsService->doWork();		// force DBmanager to do its work
			usleep(10000);	// 10 ms
		}
	}

	delete itsService;
}

//
// setValue(propName, value, timestamp, immediately)
//
PVSSresult RTDBPropertySet::setValue (const string& 	propName, 
									  const GCFPValue&	value,
									  double 			timestamp,
									  bool				immediately)
{
	// search property
	Property* propPtr = _getProperty(propName);
	if (!propPtr) {
		dpeValueSet(propName, SA_PROP_DOES_NOT_EXIST);
		return (SA_PROP_DOES_NOT_EXIST);
	}

	// if ConditionWrite=true and value not changed then we are ready.
	if ((itsAccessType & PSAT_CW) && propPtr->initialized && (*(propPtr->value) == value)) {
		LOG_TRACE_COND_STR("CW: value of " << propName << " not changed: " << value.getValueAsString());
		return (SA_NO_ERROR);
	}
	// adopt value
	propPtr->value->copy(value);
	propPtr->initialized = true;

	// update admin
	propPtr->dirty = !immediately;
	if (!immediately) {
		LOG_TRACE_COND("Not immediately, postponing update");
		return (SA_NO_ERROR);
	}

	// write to database
	string	fullName(itsScope);
	if (!propPtr->isBasicType) {		// add element name when not a basictype
		fullName += GCF_PROP_NAME_SEP + propName;
	}
	return (itsService->dpeSet(fullName, value, timestamp));
}

//
// setValue(string, value, timestamp, immediately)
//
PVSSresult RTDBPropertySet::setValue (const string&		propName, 
									  const string&		value,
									  double 			timestamp,
									  bool				immediately)
{
	// search property
	Property* propPtr = _getProperty(propName);
	if (!propPtr) {
		dpeValueSet(propName, SA_PROP_DOES_NOT_EXIST);
		return (SA_PROP_DOES_NOT_EXIST);
	}

	// if ConditionWrite=true and value not changed then we are ready.
	if ((itsAccessType & PSAT_CW) && propPtr->initialized && propPtr->value->getValueAsString() == value) {
		LOG_TRACE_COND_STR("CW: value of " << propName << " not changed: " << value);
		return (SA_NO_ERROR);
	}

	// adopt value
	if ((propPtr->value->setValue(value)) != GCF_NO_ERROR) {
		dpeValueSet(propName, SA_SETPROP_FAILED);
		return (SA_SETPROP_FAILED);
	}
	propPtr->initialized = true;

	// update admin
	propPtr->dirty = !immediately;
	if (!immediately) {
		return (SA_NO_ERROR);
	}

	// write to database.
	string	fullName(itsScope);
	if (!propPtr->isBasicType) {		// add element name when not a basictype
		fullName += GCF_PROP_NAME_SEP + propName;
	}
	return (itsService->dpeSet(fullName, *(propPtr->value), timestamp));
}
                             
//
// getValue(propname, GCFPValue)
//
PVSSresult	RTDBPropertySet::getValue(const string&			propName,
									  GCFPValue&			returnVar)
{
	Property* propPtr = _getProperty(propName);
	if (!propPtr) {
		return (SA_PROP_DOES_NOT_EXIST);
	}

	returnVar.copy(*(propPtr->value));
	LOG_TRACE_COND_STR("getValue(" << propName << ")=" << returnVar.getValueAsString());

	return (SA_NO_ERROR);
}

//
// flush()
//
PVSSresult	RTDBPropertySet::flush()
{
	PropertyMap_t::iterator PMiter(itsPropMap.begin());
	PropertyMap_t::iterator PMend (itsPropMap.end());

	// basictypes need special handling
	if (PMiter->second->isBasicType) {
		if (PMiter->second->dirty) {
			return (itsService->dpeSet(itsScope, *(PMiter->second->value)));
		}
		LOG_DEBUG_STR("No flush needed");
		return (SA_NO_ERROR);
	}

	// DP is a structure, collect dirty elements
	vector<string>			dpeNames;
	vector<GCFPValue*>		dpeValues;
	while (PMiter!= PMend) {
		if (PMiter->second->dirty) {
			dpeNames.push_back(PMiter->first);
			dpeValues.push_back(PMiter->second->value);
			PMiter->second->dirty = false;	// assume update will be succesful
		}
		++PMiter;
	}

	// something to flush?
	if (dpeNames.empty()) {
		LOG_DEBUG_STR("No flush needed");
		return (SA_NO_ERROR);
	}

	// write to database
	LOG_DEBUG_STR("Updating: " << dpeNames);
	return (itsService->dpeSetMultiple(itsScope, dpeNames, dpeValues));
}


#if 0
//
// configure(APCname)
//
void RTDBPropertySet::configure(const string& apcName)
{
	LOG_DEBUG(formatString ("REQ: Configure prop. set '%s' with apc '%s'",
												getScope().c_str(), apcName.c_str()));

	TPMResult pmResult = _pController->configurePropSet(*this, apcName);
	ASSERT(pmResult == PM_NO_ERROR);
}
#endif

//
// getFullScope()
//
string RTDBPropertySet::getFullScope () const 
{ 
	if (itsScope.find(GCF_SYS_NAME_SEP) == string::npos) {
		return (itsScope);
	}

	// system name is not specified on the scope
	return (PVSSinfo::getLocalSystemName() + GCF_SYS_NAME_SEP + itsScope); 
}

// ------------------------------ Internal routines ------------------------------
//
// _createAllProperties()
//
void RTDBPropertySet::_createAllProperties()
{
	// Get ID of this PVSS database
	int sysNr = PVSSinfo::getSysId(itsScope);
	if (sysNr == 0) {
		sysNr = PVSSinfo::getLocalSystemId();
	}

	// allocate a list that can be filled with the PropInfo of all elements
    typedef list<TPropertyInfo> PropInfoList_t;
    PropInfoList_t 		itsPropInfoList;

	// load structure of propSet into itsPropInfoList
	PVSSinfo::getTypeStruct(itsType, itsPropInfoList, sysNr);

	// When the propertySet is a basic type the PropInfoList contains only
	// one element with the type but the name is empty. Assign the name 'value'
	// to it by default.
	bool	isBasicType(false);
	if (itsPropInfoList.size() == 1 && itsPropInfoList.begin()->propName.empty()) {
		itsPropInfoList.begin()->propName = "value";
		isBasicType = true;
	}

	// create a Property object for every element and add it to our map.
	Property* 	propPtr;
	for (PropInfoList_t::iterator PIiter = itsPropInfoList.begin();
									PIiter != itsPropInfoList.end(); ++PIiter) { 
		propPtr = new Property(GCFPValue::createMACTypeObject(PIiter->type), isBasicType);
		ASSERTSTR(propPtr, "Allocation of property " << PIiter->propName << " failed");
		_addProperty(PIiter->propName, propPtr);
	}
}

//
// _deleteAllProperties()
//
void RTDBPropertySet::_deleteAllProperties()
{
	Property*	propPtr;
	for (PropertyMap_t::iterator PMiter = itsPropMap.begin(); 
									PMiter != itsPropMap.end(); ++PMiter) {
		propPtr = PMiter->second;
		delete propPtr;
	}
}

//
// _addProperty(propName, prop)
//
void RTDBPropertySet::_addProperty(const string& propName, Property* prop)
{
	ASSERT(propName.length() > 0);

	if (itsPropMap.find(propName) != itsPropMap.end()) {	// double name!
		LOG_ERROR_STR("Property " << propName << " double defined in " << itsScope);
		return;
	}

	// add it to the map
	itsPropMap[propName] = prop;
	LOG_TRACE_VAR_STR("Added property " << propName << ", mapsize=" << itsPropMap.size());

	// when user like to readback the value, get actual value and take a subscribtion.
	if (itsAccessType & PSAT_RD_MASK) {
		string	fullName(itsScope);
		if (!prop->isBasicType) {		// add element name when not a basictype
			fullName += GCF_PROP_NAME_SEP + propName;
		}
		itsService->dpeGet(fullName);

		// and take a subscribtion on changes (made by someone else).
		itsService->dpeSubscribe(fullName);
	}
}

//
// _cutScope(propName)
//
// Cuts the propertySET name from the propertyname.
//
void RTDBPropertySet::_cutScope(string& propName) const 
{
	// search for name of PS
	string::size_type	EOname(propName.find(itsScope));

	if (EOname != string::npos) {		// found? cut of DPSet name
		propName.erase(0, EOname + itsScope.length() + 1); 	// including seperator
	}
}

//
// _getProperty (propName)
//
RTDBPropertySet::Property* RTDBPropertySet::_getProperty (const string& propName) const
{
	PropertyMap_t::const_iterator PMiter = itsPropMap.find(propName);
	if (PMiter != itsPropMap.end()) {
		return (PMiter->second);
	}

	ASSERTSTR(false, "Property " << propName << " not in the PropertySet"); //REO

	return (0);
}

// -------------------- Callback routines for PVSS --------------------
//
// dpCreated(PSname, result)
//
void RTDBPropertySet::dpCreated(const string&	propName, PVSSresult		result)
{
	LOG_TRACE_FLOW_STR("RTDBPropertySet::dpCreated(" << propName << "," << result << ")");

	// Now that DP exists in the database we can access the elements.
	_createAllProperties();

	// Pass 'created' event to client.
	if (itsExtConfirmation) {
		itsExtResponse->dpCreated(itsScope, result);
	}
}

//
// dpDeleted (PSname, result)
//
void RTDBPropertySet::dpDeleted (const string& propName, PVSSresult	result)
{
	if (itsExtConfirmation) {
		itsExtResponse->dpDeleted(propName, result);
	}
}

//
// dpeSubscribed(propName, result)
//
void RTDBPropertySet::dpeSubscribed (const string& propName, PVSSresult	result)
{
	if (itsExtSubscription) {
		itsExtResponse->dpeSubscribed(propName, result);
	}
}

//
// dpeSubscribed(propName, result)
//
void RTDBPropertySet::dpeSubscriptionLost (const string& propName, PVSSresult	result)
{
	if (itsExtConfirmation) {
		itsExtResponse->dpeSubscriptionLost(propName, result);
	}
}

//
// dpeSubscribed(propName, result)
//
void RTDBPropertySet::dpeUnsubscribed (const string& propName, PVSSresult	result)
{
	if (itsExtConfirmation) {
		itsExtResponse->dpeUnsubscribed(propName, result);
	}
}

//
// dpeSubscribed(propName, result)
//
void RTDBPropertySet::dpQuerySubscribed	 (uint32 queryId, PVSSresult	result)
{
	if (itsExtConfirmation) {
		itsExtResponse->dpQuerySubscribed(queryId, result);
	}
}

//
// dpQueryChanged(propName, result, names, values, times)
//
void RTDBPropertySet::dpQueryChanged (uint32 queryId, 		 PVSSresult result,
									  const GCFPVDynArr&	DPnames,
									  const GCFPVDynArr&	DPvalues,
									  const GCFPVDynArr&	DPtypes)
{
	// highly unlikely we ever need this, but you never know.
	if (itsExtConfirmation) {
		itsExtResponse->dpQueryChanged(queryId, result, DPnames, DPvalues, DPtypes);
	}
}

//
// dpeValueSet(result)
//
void RTDBPropertySet::dpeValueSet(const string&		propName, PVSSresult	result)
{
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR ("Setting new value to " << propName << " failed");
	}

//	if (itsAccessType & PSAT_RD_MASK) {
	if (itsExtConfirmation) {
		itsExtResponse->dpeValueSet(propName, result);
	}
}

//
// dpeValueGet(result, value)
//
void RTDBPropertySet::dpeValueGet(const string&		propName, PVSSresult	result, const GCFPValue&	value)
{
LOG_DEBUG_STR("RTDBPropertySet::dpeValueGet(" << propName << ")");
	if (result == SA_NO_ERROR) {
		string		shortName(propName);
		_cutScope(shortName);
		if (shortName.empty()) {
			shortName = "value";
		}
		Property*	propPtr = _getProperty(shortName);
		propPtr->value->copy(value);
	}
	else {
		LOG_ERROR_STR ("Get Value of " << propName << " resulted in error " << result);
	}
}

//
// dpeValueChanged(result, value)
//
void RTDBPropertySet::dpeValueChanged(const string&		propName, PVSSresult	result, const GCFPValue&	value)
{
	LOG_DEBUG_STR("RTDBPropertySet::dpeValueChanged(" << propName << ")");

	// find property
	string		shortName(propName);
	_cutScope(shortName);
	if (shortName.empty()) {
		shortName = "value";
	}
	Property*	propPtr = _getProperty(shortName);

	// if property is not yet initialized than this call is from our initial dpeGet.
	bool	informClient(false);
	if (!propPtr->initialized) {
		propPtr->value->copy(value);
		propPtr->initialized = true;
		LOG_DEBUG("RTDBPropertySet::dpeValueChanged:internal event");
		informClient = true;
//		return;
	} 
	else {
		// if property is changed adopt the value if it all went ok.
		if (*(propPtr->value) != value) {
			if (result == SA_NO_ERROR) {
				propPtr->value->copy(value);
				informClient = true;
			}
		}
	}

	// notify user when he is interested in it.
	if (informClient && (itsAccessType & PSAT_RD_MASK)) {
		LOG_DEBUG("RTDBPropertySet::dpeValueChanged:propagate");
		itsExtResponse->dpeValueChanged(propName, result, value);
	}
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

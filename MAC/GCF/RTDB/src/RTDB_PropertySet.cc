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

#include <GCF/Utils.h>
#include <GCF/GCF_PValue.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include "RTDB_Property.h"
#include "PropSetResponse.h"

namespace LOFAR {
  namespace GCF {
  using namespace Common;
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {
const TPropertyInfo dummyPropInfo("DUMMY", LPT_BOOL);

//
// RTDBPropertySet (name, type, answer*)
//
RTDBPropertySet::RTDBPropertySet (const string& 	name,
                                  const string& 	type,
								  PSAccessType		accessType,
								  GCFTask*			clientTask) :
	itsScope	  (name),
	itsType		  (type),
	itsAccessType (accessType),
	itsService	  (0),
	itsOwnResponse(0),
	itsExtResponse(new DPanswer(clientTask)),
	itsIsTemp	  (accessType == PS_AT_OWNED_TEMP)
{
	LOG_TRACE_FLOW_STR("RTDBPropertySet(" << name << "," << type << "," << accessType << ")");

	itsOwnResponse = new PropSetResponse(this, itsExtResponse);
	ASSERTSTR(itsOwnResponse, "Can't allocate Response class for PropertySet " << name);

	itsService = new PVSSservice(itsOwnResponse);
	ASSERTSTR(itsService, "Can't connect to PVSS database for " << itsScope);

	// check name convention of DP
	if (!Common::isValidScope(itsScope.c_str())) {
		LOG_WARN_STR("Scope " << itsScope << " does not meet the nameconvention! Set to \"\"");
		itsScope = "";
	}

	// create dp if it doesn't exist yet.
	if (!PVSSinfo::propExists(itsScope)) {
		itsService->dpCreate(itsScope, itsType);
		// note: PVSS will call via PropSetResponse object _dpCreated.
	}
	else {
		// DP already exists in the database, immediately continue with 'created' function.
		_dpCreated(SA_NO_ERROR);
	}
}

//
// _dpCreated(result)
//
// Note: called by PropSetResponse.
//
void RTDBPropertySet::_dpCreated(PVSSresult		result)
{
	LOG_TRACE_FLOW_STR("RTDBPropertySet::_dpCreated(" << result << ")");

	// Now that Dp exists in the database we can access the elements.
	_createAllProperties();

	// Pass 'created' event to client.
	itsExtResponse->dpCreated(itsScope, result);
}

//
// ~RTDBPropertySet
//
RTDBPropertySet::~RTDBPropertySet()
{
	LOG_TRACE_FLOW_STR("~RTDBPropertySet(" << "?" << ")");

	_deleteAllProperties();// cleanup propMap

	if (itsIsTemp) {
		itsService->dpDelete(itsScope);
	}

	delete itsService;
}

#if 0
//
// operator[](propName)
//
RTDBProperty& RTDBPropertySet::operator[] (const string& propName)
{ 
	RTDBProperty* 	propPtr = _getProperty(propName);
// TODO
//	if (propPtr == 0) {
//		propPtr = &dummyPropInfo;
//	}

	return (*propPtr);
}
#endif

//
// setValueTimed(propName, value, timestamp, wantAnswer)
//
PVSSresult RTDBPropertySet::setValueTimed (const string& propName, 
                                     const GCFPValue& value,
                                     double timestamp, 
                                     bool wantAnswer)
{
	RTDBProperty* propPtr = _getProperty(propName);
	if (!propPtr) {
		return (SA_PROP_DOES_NOT_EXIST);
	}

	return propPtr->setValueTimed(value, timestamp, wantAnswer);

}

//
// setValueTimed(string, value, timestamp, wantAnswer)
//
PVSSresult RTDBPropertySet::setValueTimed (const string& propName, 
                                     const string& value,
                                     double timestamp, 
                                     bool wantAnswer)
{
	RTDBProperty* propPtr = _getProperty(propName);
	if (!propPtr) {
		return (SA_PROP_DOES_NOT_EXIST);
	}

	return propPtr->setValueTimed(value, timestamp, wantAnswer);
}
                             
//
// exists(propName)
//
bool RTDBPropertySet::exists (const string& propName) const
{
	RTDBProperty* propPtr = _getProperty(propName);
	if (propPtr) {
		return (PVSSinfo::propExists(getFullScope() + GCF_PROP_NAME_SEP + propName));
	}

	return (false);
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
	int8 sysNr = PVSSinfo::getSysId(itsScope);
	if (sysNr == 0) {
		sysNr = PVSSinfo::getLocalSystemId();
	}

	// load structure of propSet into itsPropInfoList
	PVSSinfo::getTypeStruct(itsType, itsPropInfoList, sysNr);

	// create a Property object for every element
	RTDBProperty* 	propPtr;
	for (PropInfoList_t::iterator PIiter = itsPropInfoList.begin();
									PIiter != itsPropInfoList.end(); ++PIiter) { 
//		propPtr = new RTDBProperty(itsScope, *PIiter);
		propPtr = new RTDBProperty(itsScope, *PIiter, itsExtResponse);
		_addProperty(PIiter->propName, *propPtr);
	}
}

//
// _deleteAllProperties()
//
void RTDBPropertySet::_deleteAllProperties()
{
	RTDBProperty*	propPtr;
	for (PropertyMap_t::iterator PMiter = itsPropMap.begin(); 
									PMiter != itsPropMap.end(); ++PMiter) {
		propPtr = PMiter->second;
		delete propPtr;
	}
}

//
// _addProperty(propName, prop)
//
void RTDBPropertySet::_addProperty(const string& propName, RTDBProperty& prop)
{
	ASSERT(propName.length() > 0);

//	string shortPropName(propName);		// modifyable copy
//	_cutScope(shortPropName);			// cut off propertySET name

	if (itsPropMap.find(propName) != itsPropMap.end()) {	// double name!
		LOG_ERROR_STR("Property " << propName << " double defined in " << itsScope);
		return;
	}

	// add it to the map
	itsPropMap[propName] = &prop;
	LOG_TRACE_VAR_STR("Added property " << propName << ", mapsize=" << itsPropMap.size());
}

//
// _cutScope(propName)
//
// Cuts the propertySET name from the propertyname.
//
void RTDBPropertySet::_cutScope(string& propName) const 
{
	string::size_type	EOname(propName.find(itsScope));

	if (EOname != string::npos) {		// found?
		propName.erase(0, EOname + 1); 	// including seperator
		LOG_DEBUG_STR("_cutScope to " << propName);
	}
}

//
// _getProperty (propName)
//
RTDBProperty* RTDBPropertySet::_getProperty (const string& propName) const
{
//	string 	shortPropName(propName);	// modifyable copy
//	_cutScope(shortPropName);

	LOG_TRACE_VAR_STR("itsPropMap.size() = " << itsPropMap.size());
	LOG_TRACE_VAR_STR("propName = " << propName);

	PropertyMap_t::const_iterator PMiter = itsPropMap.find(propName);
	if (PMiter != itsPropMap.end()) {
		return (PMiter->second);
	}

	ASSERTSTR(false, "Property " << propName << " not in the PropertySet"); //REO

	return (0);
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

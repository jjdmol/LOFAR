//#  RTDB_Property.cc: 
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

#include <GCF/PVSS/PVSSinfo.h>
#include "PropResponse.h"
#include "RTDB_Property.h"

namespace LOFAR {
  namespace GCF {
  using namespace Common;
  using namespace PVSS;
	namespace RTDB {

//
// RTDBProperty(name, type, accesstype)
//
RTDBProperty::RTDBProperty (const TPropertyInfo&		propInfo,
						  PVSSresponse*				clientResponse) :
	itsPropInfo	  (propInfo),
	itsCurValue   (0),
	itsOldValue	  (0),
	itsService	  (0),
	itsOwnResponse(new PropResponse(this)),
	itsExtResponse(clientResponse)
{
	// construct storage for local values
	itsCurValue = GCFPValue::createMACTypeObject(propInfo.type);
	ASSERTSTR(itsCurValue, "Property " << propInfo.propName << " of type " 
				<< propInfo.type << " could not be created locally");
	itsOldValue = itsCurValue->clone();

	itsService = new PVSSservice(itsOwnResponse);
	ASSERTSTR(itsService, "Can't connect to PVSS database(" << propInfo.propName << ")");

	// get current value.
	itsService->dpeGet(itsPropInfo.propName);

	// and take a subscribtion on changes (made by someone else).
	itsService->dpeSubscribe(itsPropInfo.propName);
}


//
// ~RTDBProperty()
//
RTDBProperty::~RTDBProperty()
{
	if (itsOldValue) {
		delete itsOldValue;
	}

	if (itsCurValue) {
		delete itsCurValue;
	}
}


//
// getValue()
//
GCFPValue& RTDBProperty::getValue()
{ 
	return (*itsCurValue);
}

//
// setValueTimed(value, timestamp, wantAnswer)
//
PVSSresult RTDBProperty::setValueTimed(const GCFPValue& value, double timestamp, bool wantAnswer)
{ 
	return (itsService->dpeSet(itsPropInfo.propName, value, timestamp, wantAnswer));
}

//
// setValueTimed(string, timestamp, wantAnswer)
//
PVSSresult RTDBProperty::setValueTimed(const string& value, double timestamp, bool wantAnswer)
{ 
	GCFPValue* pValue = GCFPValue::createMACTypeObject(itsPropInfo.type);
	if (!pValue) { 
		return (SA_VARIABLE_WRONG_TYPE);
	}

	if ((pValue->setValue(value)) != GCF_NO_ERROR) { // assign value to  GCFPValue
		return (SA_SETPROP_FAILED);
	}

	return (itsService->dpeSet(itsPropInfo.propName, *pValue, timestamp, wantAnswer));
}

void RTDBProperty::valueSetAck(PVSSresult	result)
{
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR ("Setting new value to " << itsPropInfo.propName << " failed");
		return;
	}

	itsOldValue->copy(*itsCurValue);
}

void RTDBProperty::valueGetAck(PVSSresult	result, const GCFPValue&	value)
{
	if (result == SA_NO_ERROR) {
		itsCurValue->copy(value);
	}
	else {
		LOG_ERROR_STR ("Get Value of " << itsPropInfo.propName << " resulted in error " 
						<< result);
	}
}

void RTDBProperty::valueChangedAck(PVSSresult	result, const GCFPValue&	value)
{
	if (result == SA_NO_ERROR) {
		itsCurValue->copy(value);
		// TODO: pass event to PS or user.
	}
}



  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

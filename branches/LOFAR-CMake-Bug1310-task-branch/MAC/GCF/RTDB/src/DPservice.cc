//#  DPservice.cc: class for accessing the datapoints directly (w/o a PropertySet).
//#
//#  Copyright (C) 2007
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

//#include <GCF/Utils.h>
#include <GCF/PVSS/GCF_PValue.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/DPservice.h>
#include "DPresponse.h"
#include "DPanswer.h"

#include <unistd.h>

namespace LOFAR {
  namespace GCF {
//	using namespace Common;
	using namespace TM;
	using namespace PVSS;
	namespace RTDB {

//
// DPservice (clienttask, report_result_back)
//
DPservice::DPservice(GCFTask*		clientTask) :
	itsService	    (0),
	itsOwnResponse  (0),
	itsExtResponse  (new DPanswer(clientTask))
{
	LOG_TRACE_FLOW_STR("DPservice()");

	itsOwnResponse = new DPresponse(this);
	ASSERTSTR(itsOwnResponse, "Can't allocate Response class for DPservice");

	itsService = new PVSSservice(itsOwnResponse);
	ASSERTSTR(itsService, "Can't connect to PVSS database");
}

//
// ~DPservice
//
DPservice::~DPservice()
{
	LOG_TRACE_FLOW_STR("~DPservice()");

	delete itsService;
	delete itsOwnResponse;
}

//
// setValue(DPname, value, timestamp)
//
PVSSresult DPservice::setValue (const string& 		DPname, 
								const GCFPValue& 	value,
								double 				timestamp,
								bool				wantAnswer)
{
	return (itsService->dpeSet(DPname, value, timestamp, wantAnswer));
}

//
// setValue(string, value, timestamp)
//
PVSSresult DPservice::setValue (const string&		DPname, 
								const string&		value,
								TMACValueType		type,
								double 				timestamp,	
								bool				wantAnswer)
{
	// first create a GCFValue object of the right type.
	GCFPValue*	valueObj = GCFPValue::createMACTypeObject(type);
	ASSERTSTR(valueObj, "Allocation of GCFPValue for DP " << DPname << " failed");

	// write value to it.
	if (valueObj->setValue(value) != GCF_NO_ERROR) {
		LOG_WARN_STR("Could not set value for DP " << DPname);
		itsExtResponse->dpeValueSet(DPname, SA_SETPROP_FAILED);
		return (SA_SETPROP_FAILED);
	}

	// finally write value to the database.
	return (itsService->dpeSet(DPname, *valueObj, timestamp, wantAnswer));
}

PVSSresult DPservice::setValue (const string&		DPname, 
								vector<string>		dpeNames,
								vector<GCFPValue*>	dpeValues,
								double 				timestamp,
								bool				wantAnswer)
{
	return (itsService->dpeSetMultiple(DPname, dpeNames, dpeValues, timestamp, wantAnswer));
}

                             
//
// getValue(propname, GCFPValue)
//
PVSSresult	DPservice::getValue(const string&	DPname)
{
	return (itsService->dpeGet(DPname));
}

//
// query(from, where)
//
PVSSresult DPservice::query(const string&	from, const string&		where)
{
	return (itsService->dpQuerySubscribeSingle(from, where));
}


//
// cancelQuery(queryID)
//
PVSSresult DPservice::cancelQuery(uint32	queryID)
{
	return (itsService->dpQueryUnsubscribe(queryID));
}




// -------------------- Callback routines for PVSS --------------------

//
// dpeValueSet(result)
//
void DPservice::dpeValueSet(const string&		DPname, PVSSresult	result)
{
	LOG_DEBUG("DPservice::dpeValueSet");
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR ("Setting new value to " << DPname << " failed");
	}

	itsExtResponse->dpeValueSet(DPname, result);
}

//
// dpeValueGet(result, value)
//
void DPservice::dpeValueGet(const string&		DPname, PVSSresult	result, const GCFPValue&	value)
{
	LOG_DEBUG_STR("DPservice::dpeValueGet(" << DPname << ")");

	// notify user when he is interested in it.
	LOG_DEBUG("DPservice::dpeValueChanged:propagate");
	itsExtResponse->dpeValueGet(DPname, result, value);
}

//
// dpQuerySubscribed(result, value)
//
void DPservice::dpQuerySubscribed(uint32	queryID, PVSSresult	result)
{
	LOG_DEBUG_STR("DPservice::dpQuerySubscribed(" << queryID << ")");

	// notify user when he is interested in it.
	LOG_DEBUG("DPservice::dpQuerySubscribed:propagate");
	itsExtResponse->dpQuerySubscribed(queryID, result);
}

//
// dpQueryUnsubscribed(result, value)
//
void DPservice::dpQueryUnsubscribed(uint32	queryID, PVSSresult	result)
{
	LOG_DEBUG_STR("DPservice::dpQueryUnsubscribed(" << queryID << ")");

	// notify user when he is interested in it.
	LOG_DEBUG("DPservice::dpQueryUnsubscribed:propagate");
	itsExtResponse->dpQueryUnsubscribed(queryID, result);
}

//
// dpQueryChanged(QueryID, result, DPnames, DPvalues, DPtypes)
//
void DPservice::dpQueryChanged(uint32 queryID,		PVSSresult result,
							  const GCFPVDynArr&	DPnames,
							  const GCFPVDynArr&	DPvalues,
							  const GCFPVDynArr&	DPtypes)
{
	LOG_DEBUG_STR("DPservice::dpQueryChanged(" << queryID << ")");

	// notify user when he is interested in it.
	LOG_DEBUG("DPservice::dpQueryChanged:propagate");
	itsExtResponse->dpQueryChanged(queryID, result, DPnames, DPvalues, DPtypes);
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

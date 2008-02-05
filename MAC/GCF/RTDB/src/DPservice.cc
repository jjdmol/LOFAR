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

#include <GCF/Utils.h>
#include <GCF/GCF_PValue.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/DPservice.h>
#include "DPresponse.h"
#include "DPanswer.h"

#include <unistd.h>

namespace LOFAR {
  namespace GCF {
	using namespace Common;
	using namespace TM;
	using namespace PVSS;
	namespace RTDB {

//
// DPservice (clienttask, report_result_back)
//
DPservice::DPservice(GCFTask*		clientTask,
					 bool			reportBack) :
	itsService	    (0),
	itsOwnResponse  (0),
	itsExtResponse  (new DPanswer(clientTask)),
	itsPassResult	(reportBack)
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
								double 				timestamp)
{
	return (itsService->dpeSet(DPname, value, timestamp));
}

//
// setValue(string, value, timestamp)
//
PVSSresult DPservice::setValue (const string&		DPname, 
								const string&		value,
								TMACValueType		type,
								double 				timestamp)
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
	return (itsService->dpeSet(DPname, *valueObj, timestamp));
}
                             
//
// getValue(propname, GCFPValue)
//
PVSSresult	DPservice::getValue(const string&	DPname)
{
	return (itsService->dpeGet(DPname));
}


// -------------------- Callback routines for PVSS --------------------

//
// dpeValueSet(result)
//
void DPservice::dpeValueSet(const string&		DPname, PVSSresult	result)
{
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR ("Setting new value to " << DPname << " failed");
	}

	if (itsPassResult) {
		itsExtResponse->dpeValueSet(DPname, result);
	}
}

//
// dpeValueGet(result, value)
//
void DPservice::dpeValueGet(const string&		DPname, PVSSresult	result, const GCFPValue&	value)
{
	LOG_DEBUG_STR("DPservice::dpeValueGet(" << DPname << ")");

	// notify user when he is interested in it.
	if (itsPassResult) {
		LOG_DEBUG("DPservice::dpeValueChanged:propagate");
		itsExtResponse->dpeValueGet(DPname, result, value);
	}
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

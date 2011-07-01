//#  GCF_RTDBPort.cc: communication port via the PVSS database
//#
//#  Copyright (C) 2011
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
//#  $Id: GCF_RTDBPort.cc 17192 2011-01-25 12:04:52Z overeem $
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/GCF_PVBlob.h>
#include <GCF/RTDB/GCF_RTDBPort.h>
#include "PortResponse.h"

namespace LOFAR {
  using MACIO::GCFEvent;
  namespace GCF {
	using TM::GCFTask;
	using namespace PVSS;
    namespace RTDB {

//
// GCFRTDBPort(task, portname, type, protocol, DPname)
//
GCFRTDBPort::GCFRTDBPort(GCFTask& 		task, 
						 const string& 	name, 
						 TPortType 		type, 
						 int 			protocol,
						 const string&	DPname) : 
    GCFRawPort  (task, name, type, protocol),
	itsService  (0),
	itsResponse (0),
	itsDPname   (DPname),
	itsIsOpened (false)
{
	LOG_TRACE_FLOW("RTDBPort()");

	itsResponse = new PortResponse(this);
	ASSERTSTR(itsResponse, "Can't allocate PortResponse class for RTDBPort " << name);

	itsService = new PVSSservice(itsResponse);
	ASSERTSTR(itsService, "Can't connect to PVSS for RTDBPort " << name);
}

//
// ~GCFRTDBPort()
//
GCFRTDBPort::~GCFRTDBPort()
{
	LOG_TRACE_FLOW("~RTDBPort()");

	delete itsService;
	delete itsResponse;
}

//
// open()
//
bool GCFRTDBPort::open()
{
	// check syntax of DPname
	if (!PVSSinfo::isValidPropName(itsDPname.c_str())) {
		LOG_ERROR_STR("Datapoint " << itsDPname << " of RTDBPort " << getName() << "has the wrong syntax");
		return (false);
	}

	// create DP if it doesn't exist yet.
	if (!PVSSinfo::propExists(itsDPname)) {
		itsService->dpCreate(itsDPname, "RTDBPort");
		// note: PVSS will call dpCreated from PortResponse
		// assume no failure...
		return (true);
	}

	itsIsOpened = true;
	schedule_connected();
	return (true);
}

//
// close()
//
bool GCFRTDBPort::close()
{
	itsIsOpened = false;
	itsService->dpeUnsubscribe(itsDPname);
	// note: PVSS will call dpeUnsubscribed with result
	return (true);
}

//
// send(event)
//
ssize_t GCFRTDBPort::send(GCFEvent& event)
{
	event.pack();
	PVSSresult result = itsService->dpeSet(itsDPname+".blob", 
						GCFPVBlob((unsigned char*)event.packedBuffer(), event.bufferSize()), 0.0, false);

	if (result != SA_NO_ERROR) {
		LOG_ERROR_STR("Send to RTDBPort " << getName() << "(DP=" << itsDPname << ") went wrong: " << PVSSerrstr(result));
		LOG_DEBUG_STR("PVSSresult=" << result);
		return (-1);
	}

	return (event.bufferSize());
}

//
// recv(buffer, count)
//
ssize_t GCFRTDBPort::recv(void* /*buf*/, size_t /*count*/)
{
  return (0);
}

// -------------------- Internal functions --------------------

void GCFRTDBPort::dpCreated (const string& DPname, PVSSresult result)
{
	// called from PVSS when a DP was created during 'open'
	if (result == SA_NO_ERROR) {
		// Take a subscription on the DP, reuse result variable.
		result = itsService->dpeSubscribe(DPname);
	}

	if (result != SA_NO_ERROR) {
		LOG_ERROR_STR("Opening (creating) datapoint " << DPname << " failed with PVSS error " 
						<< PVSSerrstr(result));
		itsIsOpened = false;
		schedule_disconnected();
	}
}

void GCFRTDBPort::dpeSubscribed (const string& DPname, PVSSresult result)
{
	if (result == SA_NO_ERROR) {
		itsIsOpened = true;
		schedule_connected();
		return;
	}

	LOG_ERROR_STR("Opening (subscribing) datapoint " << DPname << " failed with PVSS error " 
					<< PVSSerrstr(result));
	itsIsOpened = false;
	schedule_disconnected();
}

void GCFRTDBPort::dpeSubscriptionLost (const string& DPname, PVSSresult /*result*/)
{
	LOG_ERROR_STR("Lost connection with datapoint " << DPname << " for RTDBPort " << getName());

	itsIsOpened = false;
	schedule_disconnected();
}

void GCFRTDBPort::dpeUnsubscribed (const string& /*DPname*/, PVSSresult /*result*/)
{
	itsIsOpened = false;
	schedule_disconnected();
}

void GCFRTDBPort::dpeValueChanged (const string& DPname, PVSSresult result, const GCFPValue& value)
{
	// Note: comparable with a DATAIN event
	LOG_DEBUG_STR("New value received for " << DPname << " of RTDBPort " << getName());
//	hexdump(value.getValue(), value.getLen());
	GCFPVBlob*	pBlob 		 = (GCFPVBlob*) &value;
	GCFEvent*	pActualEvent = (GCFEvent*) pBlob->getValue();
	const_cast<GCFTask*>(getTask())->doEvent(*pActualEvent, *this);
}



    } // namespace TM
  }	// namespace GCF
} // namespace LOFAR

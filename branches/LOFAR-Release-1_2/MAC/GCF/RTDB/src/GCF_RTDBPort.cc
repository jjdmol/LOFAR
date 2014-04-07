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

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/GCF_PVBlob.h>
#include <GCF/PVSS/GCF_PVInteger.h>
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
						 const string&	DPname) : 
    GCFRawPort  (task, name, GCFPortInterface::SAP, 0),
	itsService  (0),
	itsResponse (0),
	itsDPname   (DPname),
	itsIsOpened (false)
{
	LOG_TRACE_FLOW_STR("GCFRTDBPort(" << DPname << ")");

	itsResponse = new PortResponse(this);
	ASSERTSTR(itsResponse, "Can't allocate PortResponse class for RTDBPort " << name);

	itsService = new PVSSservice(itsResponse);
	ASSERTSTR(itsService, "Can't connect to PVSS for RTDBPort " << name);

	srandom(*((uint*)itsService));
	itsOwnID = random();
	LOG_DEBUG_STR("ID of port " << name << " is " << itsOwnID);
}

//
// ~GCFRTDBPort()
//
GCFRTDBPort::~GCFRTDBPort()
{
	LOG_TRACE_FLOW_STR("~GCFRTDBPort(" << itsDPname << ")");

	delete itsService;
	delete itsResponse;
}

//
// open()
//
bool GCFRTDBPort::open()
{
	LOG_TRACE_FLOW_STR("GCFRTDBPort::open(" << itsDPname << ")");

	// check syntax of DPname
	if (!PVSSinfo::isValidPropName(itsDPname.c_str())) {
		LOG_ERROR_STR("Datapoint " << itsDPname << " of RTDBPort " << getName() << "has the wrong syntax");
		return (false);
	}

	// DP must exist in the database
	if (!PVSSinfo::propExists(itsDPname)) {
		LOG_ERROR_STR("Datapoint " << itsDPname << " does not exist in the database. Open() failed!");
		schedule_disconnected();
		// note: PVSS will call dpCreated from PortResponse
		// assume no failure...
		return (false);
	}

	// Take a subscription on the DP, reuse result variable.
	PVSSresult	result = itsService->dpeSubscribe(itsDPname+".blob");
	if (result != SA_NO_ERROR) {
		LOG_ERROR_STR("Opening datapoint " << itsDPname << " failed with PVSS error " << PVSSerrstr(result));
		return (false);
	}
	// the dpSubscribed routine of PVSS will result in a F_CONN or F_DISCO.

	return (true);
}

//
// close()
//
bool GCFRTDBPort::close()
{
	LOG_TRACE_FLOW_STR("GCFRTDBPort::close(" << itsDPname << ")");

	itsIsOpened = false;
	itsService->dpeUnsubscribe(itsDPname+".blob");
	// note: PVSS will call dpeUnsubscribed with result
	return (true);
}

//
// send(event)
//
ssize_t GCFRTDBPort::send(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR("GCFRTDBPort::send(" << itsDPname << "):" << event);

	// pack event and send it together with my own portID to the database
	event.pack();
	GCFPVBlob	theData((unsigned char*)&itsOwnID, sizeof(itsOwnID), true);
	theData.addValue((unsigned char*)event.packedBuffer(), event.bufferSize());
	PVSSresult result = itsService->dpeSet(itsDPname+".blob", theData, 0.0, false);

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

void GCFRTDBPort::dpeSubscribed (const string& DPname, PVSSresult result)
{
	LOG_TRACE_FLOW_STR("GCFRTDBPort::dpSubscribed(" << DPname << "," << result << ")");
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

void GCFRTDBPort::dpeUnsubscribed (const string& DPname, PVSSresult result)
{
	LOG_TRACE_FLOW_STR("GCFRTDBPort::dpUnsubscribed(" << DPname << "," << result << ")");
	itsIsOpened = false;
	schedule_disconnected();
}

void GCFRTDBPort::dpeValueChanged (const string& DPname, PVSSresult result, const GCFPValue& value)
{
	if (result != SA_NO_ERROR) {
		LOG_ERROR_STR("Error " << result << "received for valuechange on DP " << DPname << " of port " << getName());
		return;
	}

	// Note: comparable with a DATAIN event
	GCFPVBlob*	pBlob = (GCFPVBlob*) &value;
	const char*	pMsg  = (const char*) pBlob->getValue();

	// Is event a message we sent ourselves? yes->skip
	long		senderID;
	memcpy(&senderID, pMsg, sizeof(senderID));
	if (senderID == itsOwnID) {
		return;
	}
	LOG_DEBUG_STR("New value received on RTDBPort " << getName() << " from senderID: " << senderID);

	// reconstruct a GCFEvent.
	GCFEvent*	newEvent(new GCFEvent);
	char*		eventBuf(0);
	int			offset(sizeof(senderID));
	memcpy(&newEvent->signal, pMsg+offset, sizeof(newEvent->signal));
	offset += sizeof(newEvent->signal);
	memcpy(&newEvent->length, pMsg+offset, sizeof(newEvent->length));
	offset += sizeof(newEvent->length);
	if (newEvent->length > 0) {
		eventBuf = new char[GCFEvent::sizePackedGCFEvent + newEvent->length];
		ASSERTSTR(eventBuf, "Could not allocate buffer for " << 
								GCFEvent::sizePackedGCFEvent + newEvent->length << " bytes");
		memcpy(eventBuf,                  &newEvent->signal, GCFEvent::sizeSignal);
		memcpy(eventBuf + GCFEvent::sizeSignal, &newEvent->length, GCFEvent::sizeLength);
		memcpy(eventBuf + GCFEvent::sizePackedGCFEvent, pMsg+offset, newEvent->length);
	}
	newEvent->_buffer = eventBuf;

	// forward event to task
	const_cast<GCFTask*>(getTask())->doEvent(*newEvent, *this);
	delete newEvent;
}



    } // namespace TM
  }	// namespace GCF
} // namespace LOFAR

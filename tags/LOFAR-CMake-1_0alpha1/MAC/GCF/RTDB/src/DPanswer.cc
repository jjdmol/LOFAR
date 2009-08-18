//#  DPanswer.cc: 
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
#include <Common/LofarLogger.h>	

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Scheduler.h>
#include <DP_Protocol.ph>
#include "DPanswer.h"

namespace LOFAR {
  using namespace MACIO;
  namespace GCF {
    namespace RTDB {

// initialise static datamember
TM::GCFDummyPort	DPanswer::gDummyPort(0, "DPanswer", DP_PROTOCOL);

void DPanswer::dpCreated (const string& DPname, PVSSresult	result)
{
	LOG_TRACE_FLOW_STR("DPanswer::dpCreated(" << DPname << "," << result << ")");

	DPCreatedEvent		DPEvent;
	DPEvent.DPname = DPname;
	DPEvent.result = result;

	_dispatchEvent(DPEvent);
}

void DPanswer::dpDeleted (const string& DPname, PVSSresult result)
{
	DPDeletedEvent		DPEvent;
	DPEvent.DPname = DPname;
	DPEvent.result = result;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpeSubscribed (const string& dpeName, PVSSresult result)
{
	DPSubscribedEvent		DPEvent;
	DPEvent.DPname = dpeName;
	DPEvent.result = result;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpeSubscriptionLost (const string& dpeName, PVSSresult /*result*/)
{
	DPLostSubscriptionEvent		DPEvent;
	DPEvent.DPname = dpeName;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpeUnsubscribed (const string& dpeName, PVSSresult result)
{
	DPUnsubscribedEvent		DPEvent;
	DPEvent.DPname = dpeName;
	DPEvent.result = result;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpeValueGet (const string& dpeName, PVSSresult result, const GCFPValue& value)
{
	DPGetEvent		DPEvent;
	DPEvent.DPname  = dpeName;
	DPEvent.result = result;
	DPEvent.value = value;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpeValueChanged (const string& dpeName, PVSSresult result, const GCFPValue& value)
{
	DPChangedEvent		DPEvent;
	DPEvent.DPname  = dpeName;
	DPEvent.result = result;
	DPEvent.value = value;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpeValueSet (const string& dpeName, PVSSresult result)
{
	DPSetEvent		DPEvent;
	DPEvent.DPname = dpeName;
	DPEvent.result = result;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpQuerySubscribed (uint32 queryId, PVSSresult result)
{
	DPQuerySubscribedEvent		DPEvent;
	DPEvent.QryID  = queryId;
	DPEvent.result = result;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpQueryUnsubscribed (uint32 queryId, PVSSresult result)
{
	DPQueryUnsubscribedEvent		DPEvent;
	DPEvent.QryID  = queryId;
	DPEvent.result = result;
	_dispatchEvent(DPEvent);
}

void DPanswer::dpQueryChanged(uint32 queryId,		PVSSresult result,
							  const GCFPVDynArr&	DPnames,
							  const GCFPVDynArr&	DPvalues,
							  const GCFPVDynArr&	DPtimes)
{
	DPQueryChangedEvent		DPEvent;
	DPEvent.QryID    = queryId;
	DPEvent.result   = result;
	DPEvent.DPnames  = DPnames;
	DPEvent.DPvalues = DPvalues;
	DPEvent.DPtimes  = DPtimes;
	_dispatchEvent(DPEvent);
}

//
// _dispatchEvent(event)
//
// Serialize the given event, reconstruct new event and send it.
//
void DPanswer::_dispatchEvent(GCFEvent&	event)
{
	if (!itsTask) {		// allow empty taskPointers
		return;
	}

	// save signal from original event.
	uint16			signal(event.signal);
	const uint32	GCFEVENT_LEN = sizeof(GCFEvent);

	// serialize object.
	uint32		requiredLength;
	char* 		packedBuffer = (char*)event.pack(requiredLength);

	// get length from packed eventbuffer (we already know the signal)
	uint32		length;
	memcpy(&length,packedBuffer+sizeof(signal),sizeof(length));

	// reconstruct the event in the newEventBuffer.
	char *newEventBuffer = new char[GCFEVENT_LEN + length];
	ASSERTSTR(newEventBuffer, "Can't allocate buffer for event of type " << signal);

	GCFEvent* pActualEvent = (GCFEvent*)newEventBuffer;	// cast buffer to a EventPtr
	pActualEvent->signal = signal;
	pActualEvent->length = length;
	memcpy(newEventBuffer + GCFEVENT_LEN, packedBuffer + sizeof(signal) + sizeof(length), length);

	// Finally we can send the reconstructed event.
	itsTask->doEvent(*pActualEvent, gDummyPort);

	// and delete it again.
	delete newEventBuffer;
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

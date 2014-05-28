//#  RTmetadata.cc: store metadata in PVSS
//#
//#  Copyright (C) 2014
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
//#  $Id: RTmetadata.cc 14961 2010-02-10 15:51:20Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <unistd.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/RTmetadata.h>

#define MAX_QUEUED_EVENTS 100

namespace LOFAR {
  namespace MACIO {

//
// RTmetadata (observationID, registrationName, hostName)
//
RTmetadata::RTmetadata(uint32		observationID,
		       const string&	registrationName,
		       const string&	hostName) :
	itsObsID		 (observationID),
	itsRegisterName	 	 (registrationName),
	itsHostName		 (hostName),
	itsKVTport		 (NULL)
{
	itsLogEvents.kvps.reserve(MAX_QUEUED_EVENTS);
	itsQueuedEvents.reserve(MAX_QUEUED_EVENTS);
}

//
// ~RTmetadata()
//
RTmetadata::~RTmetadata()
{
	if (itsThread != NULL) {
		itsThread->cancel();

		// joins in ~Thread()
	}
}


//
// start()
//
void RTmetadata::start()
{
	// Some tests clear the supplied hostname (don't use PVSSGatewayStub).
	// Code under test may still log(), but that will be lost as intended.
	if (itsHostName.empty()) {
		LOG_WARN("Empty hostname, so written PVSS data points will be dropped.");
		return;
	}

	if (itsThread == NULL) {
		itsThread.reset(new Thread(this, &RTmetadata::rtmLoop, "RTMetadata (PVSS) thread: "));
	}
}

//
// log(KVpair)
//
void RTmetadata::log(const KVpair& pair)
{
	ScopedLock lock(itsQueuedEventsMutex);

	// /*If the events are not being sent, */limit the queued number.
	//
	// We could replace old events by new events, but we'd have to ensure
	// somehow that we don't drop e.g. observationName events we send once
	// at the start that PVSS needs to interpret the context of all events.
	if (/*itsKVTport != NULL || */itsQueuedEvents.size() < MAX_QUEUED_EVENTS) {
		itsQueuedEvents.push_back(pair);
	}
}


//
// log(vector<KVpair>)
//
void RTmetadata::log(const vector<KVpair>& pairs)
{
	ScopedLock lock(itsQueuedEventsMutex);

	// comments in log() above apply here too
	size_t nfree = MAX_QUEUED_EVENTS - itsQueuedEvents.size();
	size_t count = std::min(pairs.size(), nfree);
	if (count > 0) {
		itsQueuedEvents.insert(itsQueuedEvents.end(),
				       pairs.begin(), pairs.begin() + count);
	}
}


// -------------------- Internal routines --------------------

//
// rtmLoop()
//
void RTmetadata::rtmLoop()
{
	useconds_t sleepTime = 250000; // 250 ms

	// Keep trying to set up a connection to the PVSSGateway,
	// and then to send events. Until we get cancelled.
	while (true) {
		try {
			setupConnection();

			sendEventsLoop();

		} catch (LOFAR::AssertError& exc) {
			LOG_WARN_STR("Connection failure to PVSS Gateway. Re-connecting after timeout... Error: " << exc.what());
			delete itsKVTport;
		} catch (...) {
			LOG_DEBUG("Caught cancellation (or unknown) exception. Stopping...");
			delete itsKVTport;
			throw; // cancellation exc must be re-thrown
		}

		::usleep(sleepTime);
		if (sleepTime <= 16000000) { // 16 sec
			sleepTime *= 2;
		}
	}
}

//
// setupConnection()
//
void RTmetadata::setupConnection()
{
	// Use synchronous socket (last arg), since we already have a thread
	// to provide full async (and thread-safety on log()).
	itsKVTport = new EventPort(MAC_SVCMASK_PVSSGATEWAY, false, KVT_PROTOCOL,
				   itsHostName, true); // may throw AssertError exc

	ASSERTSTR(itsKVTport->connect(), "failed to connect to PVSSGateway");

	LOG_DEBUG("Registering at PVSSGateway");
	KVTRegisterEvent regEvent;
	regEvent.obsID = itsObsID;
	regEvent.name  = itsRegisterName;
	ASSERTSTR(itsKVTport->send(&regEvent),
		  "failed to send registration to PVSSGateway"); // send() may throw AssertError exc

	LOG_DEBUG("Waiting for PVSSGateway register acknowledgement");
	GCFEvent* ackPtr;
	ASSERTSTR((ackPtr = itsKVTport->receive()) != NULL,
		  "bad registration ack from PVSSGateway"); // receive may throw AssertError exc
	KVTRegisterAckEvent ack(*ackPtr);
	ASSERTSTR(ack.obsID == itsObsID && ack.name == itsRegisterName,
		  "PVSSGateway identity error");

	LOG_DEBUG("Connected to and registered at the PVSSGateway");
}

//
// sendEventsLoop()
//
void RTmetadata::sendEventsLoop()
{
	while (true) {
		{
			ScopedLock lock(itsQueuedEventsMutex);
			itsQueuedEvents.swap(itsLogEvents.kvps);
		}

		// use negative seqnrs to avoid ack messages
		itsLogEvents.seqnr -= 1;
		itsKVTport->send(&itsLogEvents); // may throw AssertError exc
		itsLogEvents.kvps.clear();

		// Lazy way to send buffered PVSS events. No need to hurry.
		::usleep(100000); // 100 ms
	}
}

  } // namespace MACIO
} // namespace LOFAR

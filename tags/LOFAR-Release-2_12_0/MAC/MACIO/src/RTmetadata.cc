//#  RTmetadata.cc: store metadata in PVSS
//#
//#  Copyright (C) 2013-2014
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <unistd.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/RTmetadata.h>

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
	itsKVTport		 (NULL),
	itsNrEventsDropped	 (0)
{
	// use negative seqnr to avoid ack messages
	itsLogEvents.seqnr = -1;

	itsLogEvents.kvps.reserve(MAX_QUEUED_EVENTS);
	itsQueuedEvents.reserve(MAX_QUEUED_EVENTS);
}

//
// ~RTmetadata()
//
RTmetadata::~RTmetadata()
{
	if (itsThread) {
		// Give itsThread time to send the last events (best effort).
		// We cannot do that while a cancellation exc is already in
		// progress in case the connection hangs.
		::usleep(100000); // 100 ms

		itsThread->cancel();

		// joins in ~Thread()
	}

	if (itsNrEventsDropped > 0) {
		LOG_WARN_STR("[RTmetadata " << itsRegisterName << "] dropped " << itsNrEventsDropped << " PVSS events");
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
		LOG_WARN_STR("[RTmetadata (PVSS) " << itsRegisterName << "] Empty PVSSGateway hostname; dropping all events");
		return;
	}

	ScopedLock lock(itsQueuedEventsMutex);
	if (!itsThread) {
		itsThread.reset(new Thread(this, &RTmetadata::rtmLoop, "[RTMetadata (PVSS) send thread] "));
	}
}

//
// log(KVpair)
//
void RTmetadata::log(const KVpair& pair)
{
	LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] log() " << pair);

	ScopedLock lock(itsQueuedEventsMutex);

	// Limit the queue size, possibly losing events.
	//
	// We could replace old events by new ones, but then we'd have to ensure
	// somehow that we don't drop e.g. the observationID event we send once
	// at the start that PVSS needs to interpret the context of all events.
	size_t queueSize = itsQueuedEvents.size();
	if (queueSize < MAX_QUEUED_EVENTS) {
		itsQueuedEvents.push_back(pair);
		if (queueSize == 0) {
			itsQueuedEventsCond.signal();
		}
	} else {
		itsNrEventsDropped += 1;
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
		if (nfree == MAX_QUEUED_EVENTS) {
			itsQueuedEventsCond.signal();
		}
	}

	itsNrEventsDropped += pairs.size() - count;
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
			// not reached
		} catch (LOFAR::AssertError& exc) {
			LOG_WARN_STR("[RTmetadata " << itsRegisterName << "] Connection failure to PVSS Gateway: " << exc.what() << ". Will attempt to reconnect in " << sleepTime << " us.");
			delete itsKVTport;
			itsKVTport = 0;
		} catch (...) {
			LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] Caught cancellation (or unknown) exception. Stopping...");
			delete itsKVTport;
			itsKVTport = 0;
			throw; // cancellation exc must be re-thrown
		}

		// capped, binary back-off in case of connection trouble
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

	LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] Creating EventPort for host " << itsHostName);
	// Note: the EventPort connect()s in the constructor
	itsKVTport = new EventPort(MAC_SVCMASK_PVSSGATEWAY, false, KVT_PROTOCOL,
				   itsHostName, true); // may throw AssertError exc

	LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] Registering at PVSSGateway");
	KVTRegisterEvent regEvent;
	regEvent.obsID = itsObsID;
	regEvent.name  = itsRegisterName;
	ASSERTSTR(itsKVTport->send(&regEvent),
		  "failed to send registration to PVSSGateway"); // send() may throw AssertError exc

	LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] Waiting for PVSSGateway register acknowledgement");
	GCFEvent* ackPtr;
	ASSERTSTR((ackPtr = itsKVTport->receive()) != NULL,
		  "bad registration ack from PVSSGateway"); // receive may throw AssertError exc
	KVTRegisterAckEvent ack(*ackPtr);
	ASSERTSTR(ack.obsID == itsObsID && ack.name == itsRegisterName,
		  "PVSSGateway identity error");

	LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] Connected to and registered at the PVSSGateway");
}

//
// sendEventsLoop()
//
void RTmetadata::sendEventsLoop()
{
	while (true) {
		itsLogEvents.kvps.clear();

		{
			ScopedLock lock(itsQueuedEventsMutex);

			if (itsQueuedEvents.empty()) {
				itsQueuedEventsCond.wait(itsQueuedEventsMutex);
			}
			itsQueuedEvents.swap(itsLogEvents.kvps);
		}

		LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] sending " << itsLogEvents.kvps.size() << " PVSS DPs; 1st: " <<
		              itsLogEvents.kvps[0] << " last: " << itsLogEvents.kvps[itsLogEvents.kvps.size() - 1]);
		itsKVTport->send(&itsLogEvents); // may throw AssertError exc
		LOG_DEBUG_STR("[RTmetadata " << itsRegisterName << "] sent " << itsLogEvents.kvps.size() << " PVSS DPs");
	}
}

  } // namespace MACIO
} // namespace LOFAR

//#
//#  ACMProxy.cc: implementation of ACMProxy class
//#
//#  Copyright (C) 2002-2004
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
//#  $Id: ACMProxy.cc 11768 2008-09-17 14:18:33Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include "ACMProxy.h"

using namespace std;
using namespace blitz;
namespace LOFAR {
  using namespace LOFAR;
  using namespace GCF::TM;
  using namespace RSP_Protocol;
  using namespace RTC;
	namespace ICAL {

#define START_DELAY 4L

ACMProxy::ACMProxy(const string& name, ACCcache&	theACCs) :
	GCFTask			  ((State)&ACMProxy::con2RSPDriver, name),
	itsACCs			  (theACCs),
	itsRSPDriver	  (0),
	itsTimerPort	  (0),
	itsRequestPool	  (0),
	itsFirstSubband	  (0),
	itsLastSubband	  (0),
	itsRequestSubband (0),
	itsReceiveSubband (0)
{
	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

	itsRSPDriver = new GCFTCPPort(*this, MAC_SVCMASK_RSPDRIVER, GCFPortInterface::SAP, RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Can't allocate a TCP port for collection ACM data at the RSPDriver");

	itsTimerPort = new GCFTimerPort(*this, "ACMheartbeat");
	ASSERTSTR(itsTimerPort, "Can't allocate a general purpose timer");

	itsRequestPool = new RequestPool(globalParameterSet()->getInt("CalServer.requestPoolSize", 5));
	ASSERTSTR(itsRequestPool, "Can't allocate buffer for RSP XC requests");

	itsFirstSubband   = globalParameterSet()->getInt("CalServer.firstSubband",0);
	itsLastSubband    = globalParameterSet()->getInt("CalServer.lastSubband",MAX_SUBBANDS-1);
	itsRequestSubband = itsLastSubband+1;
	itsReceiveSubband = itsLastSubband+1;
}

ACMProxy::~ACMProxy()
{
	if (itsRSPDriver)   { itsRSPDriver->close(); delete itsRSPDriver; }
	if (itsTimerPort)   { itsTimerPort->close(); delete itsTimerPort; }
	if (itsRequestPool) { delete itsRequestPool; }
}

//
// con2RSPDriver(event, port)
//
GCFEvent::TResult ACMProxy::con2RSPDriver(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("con2RSPDriver:" << eventName(event) << "@" << port.getName());

	switch(event.signal) {
	case F_ENTRY: {
		itsTimerPort->cancelAllTimers();
		itsSubscrHandle = 0;
		LOG_INFO("Trying to connect to the RSPDriver for ACM collection");
		itsRSPDriver->autoOpen(30,0,2);	// try 30 times with interval of 2 seconds
	}
	break;

	case F_INIT:
	break;

	case F_CONNECTED: {
		if (itsRSPDriver->isConnected()) {
			LOG_INFO("Connected to RSPDriver, asking station configuration");
			RSPGetconfigEvent getconfig;
			itsRSPDriver->send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent ack(event);
		// prepare RCUmask for RSPcommands
		itsRCUmask.reset();
		for (int i = 0; i < ack.n_rcus; i++) {
			itsRCUmask.set(i);
		}
		// wait for start of new cycle or continue interrupted cycle
		if (itsReceiveSubband > itsLastSubband) {
			TRAN(ACMProxy::idle);
		}
		else {
			TRAN(ACMProxy::getXCsubscription);
		}
	}
	break;

	case F_DISCONNECTED: {
		LOG_ERROR("No connection with the RSPDriver, retry in 3 seconds");
		itsRSPDriver->close();
		itsTimerPort->setTimer(3.0);
	}
	break;

	case F_TIMER: {
		if (!port.isConnected()) {
			LOG_DEBUG("itsRSPDriver->autoOpen(30,0,2)");
			itsRSPDriver->autoOpen(30,0,2);
		}
	}
	break;

	case F_EXIT:
		// stop timer, we're connected
		itsTimerPort->cancelAllTimers();
	break;

	default:
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// idle(event, port)
//
// wait for new start of cycle
//
GCFEvent::TResult ACMProxy::idle(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("idle:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsTimerPort->setTimer(1.0, 1.0); // check every second
	}
	break;

	case F_EXIT: {
		itsTimerPort->cancelAllTimers();
	}
	break;

	case F_TIMER: {
		if (itsACCs.getBack().isWaiting4Start()) {
			LOG_DEBUG_STR(itsACCs.getBack());
			LOG_INFO("New start of collection cycle requested, starting it.");
			itsACCs.getBack().started();
			LOG_DEBUG_STR(itsACCs.getBack());
			itsRequestSubband = itsFirstSubband;
			itsReceiveSubband = itsFirstSubband;
			TRAN(ACMProxy::getXCsubscription);
		}
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO("lost connection with the RSPDriver, going to reconnect state");
		itsRSPDriver->close();
		TRAN(ACMProxy::con2RSPDriver);
	}
	break;
	}

	return (GCFEvent::HANDLED);
}


//
// getXCsubscription(event, port)
//
GCFEvent::TResult ACMProxy::getXCsubscription(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("getXCsubscription:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// subscribe to statistics
		RSPSubxcstatsEvent subxc;
		itsStartTime.setNow();
		itsStartTime += (long)START_DELAY; // start START_DELAY seconds from now
		subxc.timestamp = itsStartTime + (long)1; // wait 1 second to get result
		subxc.period = 1;
		itsRSPDriver->send(subxc);
	}
	break;

	case RSP_SUBXCSTATSACK: {
		RSPSubxcstatsackEvent ack(event);
		if (ack.status == RSP_BUSY) {
			LOG_INFO ("RSPDriver, retrying a XCsubscription over 5 seconds.");
			itsTimerPort->setTimer(5.0);
			break;
		}
		if (ack.status != RSP_SUCCESS) {
			LOG_FATAL("SUBCXSTATSACK returned error status");
			exit(EXIT_FAILURE);
		}
		itsSubscrHandle = ack.handle;
		TRAN(ACMProxy::collectACinfo);
	}
	break;

	case F_TIMER: {
		TRAN(ACMProxy::getXCsubscription);	// redo request for subscription
	}
	break;

	case F_EXIT: {
		itsTimerPort->cancelAllTimers();
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO("lost connection with the RSPDriver, going to reconnect state");
		itsRSPDriver->close();
		TRAN(ACMProxy::con2RSPDriver);
	}
	break;
	}

	return (GCFEvent::HANDLED);
}


//
// collectACinfo(event, port)
//
// collect the AC info for every subband
//
GCFEvent::TResult ACMProxy::collectACinfo(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("collectACinfo:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsTimerPort->setTimer(1.0, 1.0); // heartbeat at 1 second interval

		if (itsRequestSubband == itsFirstSubband) {		// start of new cycle?
			itsStartTime.setNow();
			itsStartTime += (long)START_DELAY; // start START_DELAY seconds from now
			LOG_INFO_STR("starttime for ACM collection: " << itsStartTime);
			// wait for timer
		}
	}
	break;

	case F_EXIT: {
		itsTimerPort->cancelAllTimers();
	}
	break;

	case F_TIMER: {
		Timestamp	justPast;
		justPast.setNow();

		// is abort requested?
		if (itsACCs.getBack().isAborted()) {
			LOG_INFO ("Aborting collection of ACMinfo on request of main task!");
			itsRequestPool->clearBeforeTime(justPast-(2*START_DELAY));	// clear request admin
			itsRequestSubband = itsLastSubband+1;		// mark subband admin ready
			itsReceiveSubband = itsLastSubband+1;
			itsACCs.getBack().setReady(true);			// communicate back that we are ready
			LOG_DEBUG_STR(itsACCs.getBack());
			TRAN(ACMProxy::unsubscribing);
			break;
		}
			
		// delete requests that were missed !!!???
		itsRequestPool->clearBeforeTime(justPast-1L);

		// send subband request
		if (itsRequestSubband <= itsLastSubband && !itsRequestPool->full()) {
			RSPSetsubbandsEvent	sse;
			sse.timestamp = itsStartTime;
			sse.rcumask   = itsRCUmask;
			sse.subbands.setType(SubbandSelection::XLET);
			sse.subbands().resize(1, 1);
			sse.subbands() = itsRequestSubband;
			LOG_DEBUG_STR("XLETrequest " << itsRequestSubband << " for " << itsStartTime);
			itsRSPDriver->send(sse);	// will result in RSP_SUBBANDSELECTACK
			itsRequestPool->add(itsRequestSubband, itsStartTime+1L); 
		}
	}
	break;

    case RSP_SETSUBBANDSACK: {
		RSPSetsubbandsackEvent ack(event);
		if (ack.status == RSP_SUCCESS) {
			itsRequestSubband++;
			LOG_DEBUG_STR("XLETrequest incremented to " << itsRequestSubband);
			// wait for timer to send next
		} 
		else {
			LOG_FATAL_STR("Request for subband " << itsRequestSubband << " failed, retrying");
			itsRequestPool->remove(itsRequestSubband);
			// just wait for timer
		}
		itsStartTime.setNow();
		itsStartTime += (long)START_DELAY;
      }
      break;

	case RSP_UPDXCSTATS: {
		RSPUpdxcstatsEvent upd(event);
		if (upd.status == RSP_SUCCESS) {
			int	subbandNr = itsRequestPool->findOnTimestamp(upd.timestamp);
			if (subbandNr < 0) {
				LOG_DEBUG_STR("XCstat for timestamp " << upd.timestamp << " not in our admin");
				break;
			}
			if (subbandNr != itsReceiveSubband) {
				LOG_WARN_STR("Expected ACM for subband " << itsReceiveSubband << " iso " << subbandNr);
			}
			LOG_DEBUG_STR("ACK: XC subband " << subbandNr << " @ " << upd.timestamp);
			LOG_DEBUG_STR("upd.stats().shape=" << upd.stats().shape());
			itsACCs.getBack().updateACM(subbandNr, upd.timestamp, upd.stats());
			itsRequestPool->remove(subbandNr);
			itsReceiveSubband = subbandNr+1;
			// test for end of cycle
			if (itsReceiveSubband > itsLastSubband) {
				itsACCs.getBack().setReady(true);
				LOG_DEBUG_STR(itsACCs.getBack());
				TRAN(ACMProxy::unsubscribing);
			}
		} 
		else {
			LOG_FATAL("UPDXCSTATS returned error code");
			exit(EXIT_FAILURE);
		}
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
		port.close();

		// whenever we come back restart the collectcycle with the subband we are waiting for now.
		itsRequestSubband = itsReceiveSubband;
		LOG_DEBUG_STR("REQUESTSUBBAND SET TO " << itsRequestSubband);

		TRAN(ACMProxy::con2RSPDriver);
	}
	break;

	default:
		return (GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}


//
// unsubscribing(event, port)
//
GCFEvent::TResult ACMProxy::unsubscribing(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("unsubscribing:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		RSPUnsubxcstatsEvent unsub;
		unsub.handle = itsSubscrHandle;
		itsRSPDriver->send(unsub);
	}
	break;

	case RSP_UNSUBXCSTATSACK: {
		RSPUnsubxcstatsackEvent ack(event);
		if (ack.status == RSP_BUSY) {
			LOG_INFO_STR("RSPDRiver busy, retrying unscubscription on XCstat over 5 seconds");
			itsTimerPort->setTimer(5.0);
			break;
		}
		if (ack.status != RSP_SUCCESS) {
			LOG_FATAL("UNSUBXCSTATSACK returned error status");
			exit(EXIT_FAILURE);
		}

		itsSubscrHandle = 0;
		LOG_INFO("finished collecting ACMs");
		TRAN(ACMProxy::idle);
	}
	break;

	case F_TIMER:
		// ignore timer event, probably a (late) queued heartbeat
	break;

	case F_EXIT: {
		itsTimerPort->cancelAllTimers();
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
		port.close();

		TRAN(ACMProxy::con2RSPDriver);
	}
	break;

	default:
		return (GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

  } // namespace ICAL
} // namespace LOFAR

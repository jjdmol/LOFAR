//
//  tStateResult.cc: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2009
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tStateResult.h"
#include "tServer.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tStateResult::tStateResult(string name) : 
	GCFTask     ((State)&tStateResult::initial, name),
	itsServer	(0),
	itsConn 	(0)
{ 
	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tStateResult::~tStateResult()
{
}

//
// initial
//
// Start a server task that will open the listener after 25 second. Start an autoOpen call
// in this taks and wait till the connection is established.
//
GCFEvent::TResult tStateResult::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tStateResult::initial: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		return (GCFEvent::HANDLED);

	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nStarting the server task");
		tServer*	itsServer = new tServer("Server", 0);
		itsServer->start();
		itsConn = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SAP, ECHO_PROTOCOL, false);
		ASSERTSTR(itsConn, "Can't allocate a TCPport");
		itsConn->autoOpen(10, 0, 2);	// try to connect to my server
	}
	break;

	case F_CONNECTED:
		TRAN(tStateResult::collecting);
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("HOUSTON HAVE A PROBLEM");
		break;

	case F_EXIT:
		LOG_INFO("Deliberately returning F_EXIT as NOT_HANDLED");
		return (GCFEvent::NOT_HANDLED);

	default:
		LOG_WARN_STR("DEFAULT in initial: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// collecting
//
// We send some pings to our echo server and accept some of the replies and
// return the others as NEXT_STATE or NOT_HANDLED.
//
GCFEvent::TResult tStateResult::collecting(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tStateResult::collecting: " << eventName(event.signal));
//	GCFScheduler::instance()->printEventQueue();

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG_STR("Sending ping nr 1");
		EchoPingEvent	ping;
		ping.seqnr=1;
		itsConn->send(ping);
	}
	break;

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr % 5 == 0) {
			LOG_DEBUG_STR("Received echo " << echo.seqnr << ", returning NOT_HANDLED");
			status = GCFEvent::NOT_HANDLED;
		} 
		else if (echo.seqnr % 3 == 0) {
			LOG_DEBUG_STR("Received echo " << echo.seqnr << ", returning NEXT_STATE");
			LOG_DEBUG_STR("event=" << event);
			LOG_DEBUG_STR("echo =" << echo);
			status = GCFEvent::NEXT_STATE;
		}
		else {
			LOG_DEBUG_STR("Received echo " << echo.seqnr << ", returning HANDLED");
			status = GCFEvent::HANDLED;
		}

		if (echo.seqnr < 10) {
			EchoPingEvent	ping;
			ping.seqnr = ++echo.seqnr;
			ping.ping_time.tv_sec= 100000 + ping.seqnr;
			ping.someName = formatString("ping has seqnr %d", ping.seqnr);
			LOG_DEBUG_STR("Sending ping " << ping);
			itsConn->send(ping);
		}
		else {
			LOG_DEBUG ("Going to the last state, expecting 3 ECHO's from this state");
			TRAN(tStateResult::lastState);
		}
	}
	break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("WE STILL HAVE A PROBLEM");
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in collecting");
		break;

	default:
		LOG_WARN_STR("DEFAULT in collecting: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	LOG_DEBUG("EOS collecting");
	return status;
}

//
// lastState
//
// Start a server task that will open the listener after 25 second. Start an autoOpen call
// in this taks and wait till the connection is established.
//
GCFEvent::TResult tStateResult::lastState(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tStateResult::lastState: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		break;

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		LOG_DEBUG_STR("Received postponed ECHO event:" << echo);
		if (echo.seqnr < 9) {
			return (GCFEvent::HANDLED);
		}
		LOG_DEBUG("Program ran succesfully");
		GCFScheduler::instance()->stop();
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("WE STILL HAVE A PROBLEM");
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in lastState");
		break;

	default:
		LOG_WARN_STR("DEFAULT in lastState: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF::TM;

//
// MAIN()
//
int main(int argc, char* argv[])
{
	GCFScheduler*	theScheduler(GCFScheduler::instance());
	theScheduler->init(argc, argv);

	tStateResult	clientTask("clientTask");
	clientTask.start(); // make initial transition

	theScheduler->run();

	return (0);
}

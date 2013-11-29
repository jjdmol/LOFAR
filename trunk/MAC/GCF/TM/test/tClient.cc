//
//  tClient.cc: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2006
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
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tClient.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tClient::tClient(string name) : 
	GCFTask         ((State)&tClient::connect2Server, name),
	itsTimerPort    (0),
	itsDataPort		(0)
{ 
}

tClient::~tClient()
{
	delete itsTimerPort;
	delete itsDataPort;
}

//
// connect2Server
//
// We simply set one timer in different ways and wait for the timer to expire.
//
GCFEvent::TResult tClient::connect2Server(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tClient::connect2Server: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort = new GCFTimerPort(*this, "timerPort");
		ASSERTSTR(itsTimerPort, "Failed to open timerport");
		break;

	case F_INIT:
		LOG_DEBUG_STR("trying to connect 2 the server");
		itsDataPort = new GCFTCPPort(*this, "EchoServer:v1.0", GCFPortInterface::SAP, ECHO_PROTOCOL, false);
		ASSERTSTR(itsDataPort, "Cannot allocate a data port");
		itsDataPort->open();
		break;

	case F_CONNECTED: 
		LOG_INFO("YES, we are connected, Sending a ping within 5 seconds");
		itsTimerPort->setTimer(5.0);
		break;

	case F_TIMER:
		TRAN(tClient::sendPing);
		break;

	case F_DISCONNECTED: 
		LOG_INFO("SH..., the connection failed");
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in connect2Server");
		break;

	default:
		LOG_WARN_STR("DEFAULT in connect2Server: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendPing
//
// Send a Ping command and wait for the response
//
GCFEvent::TResult tClient::sendPing(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tClient::sendPing: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		EchoPingEvent	ping;
		ping.seqnr = 25;
		itsDataPort->send(ping);
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		LOG_INFO("YES, we got an echo, closing connection");
		itsDataPort->close();
		break;
	}

	case F_DISCONNECTED: 
		LOG_INFO("Port is closed");
		GCFScheduler::instance()->stop();
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in sendPing");
		break;

	default:
		LOG_WARN_STR("DEFAULT in connect2Server: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


  } // namespace TM
 } // namespace GCF
} // namespace LOFAR


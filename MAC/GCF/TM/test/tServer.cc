//
//  tServer.cc: Implements an echo server
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
#include "tServer.h"
#include "Echo_Protocol.ph"

namespace LOFAR {
 namespace GCF {
  namespace TM {

tServer::tServer(string name, uint	startupDelay) : 
	GCFTask((State)&tServer::initial, name),
	itsListener(0),
	itsTimerPort(0),
	itsStartupDelay(startupDelay)
{
	registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

	itsListener = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SPP, ECHO_PROTOCOL);
	ASSERTSTR(itsListener, "failed to alloc listener");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "failed to alloc listener");
}

GCFEvent::TResult tServer::initial(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT: {
		int	timerID = itsTimerPort->setTimer(1.0*itsStartupDelay);
		LOG_DEBUG_STR("WAITING " << itsStartupDelay << " seconds before starting server, timerID=" << timerID);
	}
	break;

    case F_TIMER:
		LOG_DEBUG("STARTING server");
		itsListener->open();
		break;

    case F_CONNECTED:
		if (itsListener->isConnected()) {
			TRAN(tServer::connected);
		}
		break;

    case F_DISCONNECTED:
		ASSERTSTR(false, "Bailing out because server could not be started");
		GCFScheduler::instance()->stop();
		break;

    default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

GCFEvent::TResult tServer::connected(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case ECHO_PING: {
		EchoPingEvent	ping(event);
		EchoEchoEvent	echo;
		echo.seqnr     = ping.seqnr;
		echo.ping_time = ping.ping_time;
		echo.someName = formatString("Echo event has seqnr %d", echo.seqnr);
		port.send(echo);
		break;
	}

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR


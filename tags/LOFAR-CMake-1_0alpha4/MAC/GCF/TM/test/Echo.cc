//
//  Echo.cc: Implementation of the Echo task class.
//
//  Copyright (C) 2003
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
#include "Echo_Protocol.ph"
#include "Echo.h"

static 	int		gDelay = 0;
static	timeval	gTime;
static	int		gSeqnr;

namespace LOFAR {
 namespace GCF {
  namespace TM {

Echo::Echo(string name) : GCFTask((State)&Echo::initial, name)
{
  // register the protocol for debugging purposes
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

  // initialize the port
  server = new GCFTCPPort(*this, "EchoServer:test", GCFPortInterface::MSPP, ECHO_PROTOCOL);
}

GCFEvent::TResult Echo::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("Echo::initial: " << eventName(e));

  GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_INIT:
		break;

    case F_ENTRY:
		server->open();
		break;

    case F_CONNECTED:
		if (server->isConnected()) {
			TRAN(Echo::connected);
		}
		break;

    default:
		LOG_DEBUG_STR("$$$ DEFAULT at initial of " << eventName(e));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

GCFEvent::TResult Echo::connected(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("Echo::connected: " << eventName(e));

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ACCEPT_REQ: {
		GCFTCPPort*		clientSock = new GCFTCPPort();
		clientSock->init(*this, "clientSock", GCFPortInterface::SPP, ECHO_PROTOCOL);
		ASSERTSTR(clientSock, "Blerk");
		server->accept(*clientSock);
		break;
	}

	case F_DISCONNECTED:
		cout << "Lost connection to client" << endl;
		p.close();
		TRAN(Echo::initial);
		break;

	case ECHO_PING: {
		EchoPingEvent ping(e);
		// for instance these 3 lines can force an interrupt on the parallele port if
		// the pin 9 and 10 are connected together. The interrupt can be seen by means of
		// the F_DATAIN signal (see below)
		// Send a string, which starts with a 'S'|'s' means simulate an clock pulse interrupt 
		// (in the case below only one character is even valid)
		// otherwise an interrupt will be forced by means of setting the pin 9.
		cout << "PING received (seqnr=" << ping.seqnr << ")" << endl;
		cout << "delay = " << gDelay << endl;
		server->setTimer((gDelay < 0) ? -1.0 * gDelay : 1.0 * gDelay);
		gSeqnr = ping.seqnr;
		gTime  = ping.ping_time;
		break;
	}

	case F_TIMER: {
		if (gDelay < 0) {
			p.close();
			cout << "connection closed by me." << endl;
			break;
		}

		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = gSeqnr;
		echo.ping_time = gTime;
		echo.echo_time = echo_time;

		server->send(echo);

		cout << "ECHO sent" << endl;
		break;
	}

	case F_DATAIN: {
		cout << "Clock pulse: ";
		// always the recv has to be invoked. Otherwise this F_DATAIN keeps comming 
		// on each select
		char pulse[4096]; // size >= 1
		p.recv(pulse, 4096); // will always return 1 if an interrupt was occured and 0 if not
		pulse[1] = 0; // if interrupt occured the first char is filled with a '0' + number of occured interrupts 
		// in the driver sinds the last recv
		cout << pulse << endl;
		break;
	}

	case F_CONNECTED:
		break;

	default:
		LOG_DEBUG_STR("$$$ DEFAULT at connected of " << eventName(e));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF::TM;
using namespace std;

int main(int argc, char* argv[])
{
	GCFScheduler::instance()->init(argc, argv);

	switch (argc) {
	case 1:		gDelay = 0;
		break;
	case 2:		gDelay = atoi(argv[1]);
		break;
	default:
		cout << "Syntax: " << argv[0] << " [delay]" << endl;
		cout << "  When delay is a positive value the server will wait that many seconds " << 
				"before sending the respons." << endl;
		cout << "  When delay is a negative value the server will disconnect after that " <<
				"many seconds without sending the answer." << endl;
		return (1);
	}

	Echo echo_task("ECHO");
	echo_task.start(); // make initial transition
	GCFScheduler::instance()->run();

	return (0);
}

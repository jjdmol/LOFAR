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

static 	int			gDelay = 0;
static	timeval		gTime;
static	int			gSeqnr;
static	LOFAR::GCF::TM::GCFTCPPort*	gClientPort;

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
	LOG_DEBUG_STR("Echo::waitForConnection: " << eventName(e));

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
		status = GCFEvent::HANDLED;
		break;
	}

	return (status);
}

GCFEvent::TResult Echo::connected(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("Echo::connected: " << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ACCEPT_REQ: {
		gClientPort = new GCFTCPPort();
		ASSERTSTR(gClientPort, "Could not allocate a TCPPort for the client connection");
		gClientPort->init(*this, "clientSock", GCFPortInterface::SPP, ECHO_PROTOCOL);
		if (!server->accept(*gClientPort)) {
			cout << "could not setup a connection!" << endl;
			delete gClientPort;
		}
		break;
	}

	case F_DISCONNECTED:
		if (&port == gClientPort) {
			cout << "Lost connection to client" << endl;
			gClientPort->close();
			delete gClientPort;
			gClientPort = 0;
			server->cancelAllTimers();
		}
		else {
			cout << "Listener closed?" << endl;
			if (gClientPort) { 
				gClientPort->close(); 
				delete gClientPort; 
				gClientPort = 0; 
			}
			server->close(); 
			TRAN(Echo::initial);
		}
		break;

	case ECHO_PING: {
		EchoPingEvent ping(event);
		cout << "PING received (seqnr=" << ping.seqnr << ")" << endl;
		cout << "delay = " << gDelay << endl;
		server->setTimer((gDelay < 0) ? -1.0 * gDelay : 1.0 * gDelay);
		gSeqnr = ping.seqnr;
		gTime  = ping.ping_time;
		break;
	}

	case F_TIMER: {
		if (!gClientPort) {
			break;
		}

		if (gDelay < 0) {
			gClientPort->close();
			delete gClientPort;
			gClientPort = 0;
			cout << "connection closed by me." << endl;
			break;
		}

		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = gSeqnr;
		echo.ping_time = gTime;
		echo.echo_time = echo_time;

		gClientPort->send(echo);

		cout << "ECHO sent" << endl;
		break;
	}

	case F_CONNECTED:
		break;

	default:
		status = GCFEvent::HANDLED;
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

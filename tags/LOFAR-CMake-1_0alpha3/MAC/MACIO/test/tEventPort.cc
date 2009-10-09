//#  tEventPort.cc: Program to test the EventPort class
//#
//#  Copyright (C) 2007
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
#include <MACIO/EventPort.h>
#include <MACIO/MACServiceInfo.h>
#include "Echo_Protocol.ph"

using namespace LOFAR;
using namespace LOFAR::MACIO;

static	EchoPingEvent		pingEvent;
static	EventPort*			echoPort;

int main (int	argc, char*argv[]) 
{
	bool	syncMode;
	switch (argc) {
	case 2:	
		syncMode = (argv[1][0] == 's' || argv[1][0] == 'S');
		break;
	default:
		cout << "Syntax: " << argv[0] << " a | s" << endl;
		cout << "  the argument ('a' or 's') chooses the asynchrone or synchrone behaviour" << endl
			 << "of the EventPort. The synchrone mode expects the ServiceBroker and the"  << endl
			 << "Echo server running. If not it will assert. In this mode the EventPort" << endl
			 << "wait forever for the answer." << endl 
			 << "In asynchrone mode the EventPort will never complain and try forever to" << endl
			 << "reach each of the connection stages and receive the messages. You can" << endl
			 << "check the EventPort.status() to see in what state the port is." << endl;
		return (1);
	}

	INIT_LOGGER("tEventPort");

	// open port	
	LOG_DEBUG_STR ("Operating in " << ((syncMode) ? "" : "a") << "synchrone mode.");
	echoPort = new EventPort("ECHO:EchoServer", false, ECHO_PROTOCOL, "", syncMode);

	// construct event
	pingEvent.seqnr = 25;
	timeval		pingTime;
	gettimeofday(&pingTime, 0);
	pingEvent.ping_time = pingTime;

	LOG_DEBUG("sending the ping event");
	if (syncMode) {
		// NOTE: we could also use the while-loop of the asyncmode here.
		echoPort->send(&pingEvent);
	}
	else {
		while (!echoPort->send(&pingEvent)) {
			cout << "state = " << echoPort->getStatus() << endl;
			sleep (1);
			;
		}
	}

	LOG_DEBUG("going to wait for the answer event");
	GCFEvent*	ackPtr;
	if (syncMode) {
		// NOTE: we could also use the while-loop of the asyncmode here.
		ackPtr = echoPort->receive();
	}
	else {
		while (!(ackPtr = echoPort->receive())) {
			cout << "state = " << echoPort->getStatus() << endl;
			sleep (1);
			;
		}
	}
	EchoEchoEvent	ack(*ackPtr);

	LOG_DEBUG_STR("seqnr: " << ack.seqnr);
	double	delta =  (1.0 * ack.echo_time.tv_sec + (ack.echo_time.tv_usec / 1000000.0));
			delta -= (1.0 * ack.ping_time.tv_sec + (ack.ping_time.tv_usec / 1000000.0));
	LOG_DEBUG_STR("dTime: " << delta << " sec");

	return (0);
}


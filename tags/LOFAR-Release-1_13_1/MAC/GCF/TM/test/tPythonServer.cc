//#  tPythontest.cc: Program to test the EventPort class
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
//#  $Id: tPythontest.cc 14959 2010-02-09 23:18:39Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include "tPythonServer.h"
#include "PythonTest_Protocol.ph"


namespace LOFAR {
 using namespace PythonTest_Protocol;
 namespace GCF {
  namespace TM {

tPythonServer::tPythonServer(string name, uint	portNr) : 
	GCFTask((State)&tPythonServer::initial, name),
	itsListener(0),
	itsTimerPort(0),
	itsPortNr(portNr)
{
	registerProtocol(PYTHONTEST_PROTOCOL, PYTHONTEST_PROTOCOL_STRINGS);

	itsListener = new GCFTCPPort(*this, "PythonTest:Server", GCFPortInterface::MSPP, PYTHONTEST_PROTOCOL);
	ASSERTSTR(itsListener, "failed to alloc listener");
	itsListener->setPortNumber(portNr);

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "failed to alloc listener");
}

GCFEvent::TResult tPythonServer::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		itsListener->open();
	break;

    case F_CONNECTED:
		if (itsListener->isConnected()) {
			TRAN(tPythonServer::connected);
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

GCFEvent::TResult tPythonServer::connected(GCFEvent& event, GCFPortInterface& port)
{
	cout << "Event: " << eventName(event) << "@" << port.getName() << endl;
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ACCEPT_REQ: {
		GCFTCPPort* client = new GCFTCPPort();
		client->init(*this, "client", GCFPortInterface::SPP, PYTHONTEST_PROTOCOL);
		if (!itsListener->accept(*client)) {
			delete client;
		} else {
			LOG_INFO("NEW CLIENT CONNECTED");
		}
	}
	break;

	case TEST_PTC_INTEGERS: {
		TEST_PTCIntegersEvent	message(event);
		cout <<  message << endl;
		port.send(message);
	} break;

	case TEST_PTC_FLOATS: {
		TEST_PTCFloatsEvent	message(event);
		cout <<  message << endl;
		port.send(message);
	} break;

	case TEST_PTC_STRINGS: {
		TEST_PTCStringsEvent	message(event);
		cout <<  message << endl;
		port.send(message);
	} break;

	case TEST_PTC_BOOLS: {
		TEST_PTCBoolsEvent	message(event);
		cout <<  message << endl;
		port.send(message);
	} break;

    case F_DISCONNECTED:
		LOG_DEBUG_STR("SERVER received 'disconnect', closing port");
		port.close();
//		TRAN(tPythonServer::initial);	// hope this will work...
		break;

	default:
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

//
// MAIN()
//
int main(int argc, char* argv[])
{
	if (argc != 2) {
		cout << "Syntax: " << argv[0] << " portnr" << endl;
		return (-1);
	}

	GCFScheduler*	theScheduler(GCFScheduler::instance());
	theScheduler->init(argc, argv);

	tPythonServer	server("serverTask", atoi(argv[1]));
	server.start(); // make initial transition

	theScheduler->run();

	return (0);
}

